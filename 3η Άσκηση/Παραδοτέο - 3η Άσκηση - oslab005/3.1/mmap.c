/* SPDX-License-Identifier: MIT */
#define _DEFAULT_SOURCE
#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

enum phase {
    INSPECT_AFTER_FORK = 1,
    CHANGE_PRIVATE,
    CHANGE_SHARED,
    PROTECT_SHARED
};

struct mappings {
    size_t page_size;
    size_t file_size;
    unsigned char *private_page;
    unsigned char *shared_page;
    const unsigned char *file_view;
};

static int display_maps(const char *label, const void *address, int expected_write)
{
    FILE *maps = fopen("/proc/self/maps", "r");
    if (maps == NULL) {
        perror("/proc/self/maps");
        return -1;
    }

    char *line = NULL;
    size_t capacity = 0;
    int result = -1;
    if (address == NULL)
        printf("[%s] process map\n", label);

    while (getline(&line, &capacity, maps) >= 0) {
        if (address == NULL) {
            fputs(line, stdout);
            continue;
        }
        uintptr_t begin, end;
        char permissions[5];
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %4s",
                   &begin, &end, permissions) == 3 &&
            (uintptr_t)address >= begin && (uintptr_t)address < end) {
            printf("[%s] %s", label, line);
            int writable = permissions[1] == 'w';
            result = expected_write < 0 || writable == expected_write ? 0 : -1;
            break;
        }
    }
    if (address == NULL && !ferror(maps))
        result = 0;
    if (result < 0)
        fprintf(stderr, "Could not verify mapping: %s\n", label);
    free(line);
    fclose(maps);
    return result;
}

static void report_residency(const char *label, void *address, size_t page_size)
{
    unsigned char state;
    printf("[%s] address=%p ", label, address);
    if (mincore(address, page_size, &state) < 0) {
        printf("residency unavailable: %s\n", strerror(errno));
        return;
    }
    puts(state & 1 ? "resident" : "not resident");
}

static int map_input_file(struct mappings *memory, const char *path)
{
    int fd = open(path, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
    if (fd < 0) {
        perror(path);
        return -1;
    }
    struct stat info;
    if (fstat(fd, &info) < 0) {
        perror("fstat");
        close(fd);
        return -1;
    }
    if (!S_ISREG(info.st_mode) || info.st_size <= 0 ||
        (uintmax_t)info.st_size > SIZE_MAX) {
        fprintf(stderr, "%s requires a non-empty regular file\n", path);
        close(fd);
        return -1;
    }
    memory->file_size = (size_t)info.st_size;
    memory->file_view = mmap(NULL, memory->file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    int saved_error = errno;
    close(fd);
    if (memory->file_view == MAP_FAILED) {
        errno = saved_error;
        perror("mmap file");
        return -1;
    }

    printf("[file] %zu bytes\n", memory->file_size);
    if (fwrite(memory->file_view, 1, memory->file_size, stdout) != memory->file_size ||
        putchar('\n') == EOF)
        return -1;
    return display_maps("file mapping", memory->file_view, 0);
}

static int send_phase(int channel, unsigned char phase)
{
    ssize_t sent;
    do {
        sent = send(channel, &phase, sizeof phase, MSG_NOSIGNAL);
    } while (sent < 0 && errno == EINTR);
    return sent == sizeof phase ? 0 : -1;
}

static int receive_phase(int channel, unsigned char *phase)
{
    ssize_t received;
    do {
        received = recv(channel, phase, sizeof *phase, 0);
    } while (received < 0 && errno == EINTR);
    return received == sizeof *phase ? 0 : -1;
}

static int child_experiment(int channel, const struct mappings *memory)
{
    for (unsigned char phase = INSPECT_AFTER_FORK; phase <= PROTECT_SHARED; ++phase) {
        unsigned char command;
        if (receive_phase(channel, &command) < 0 || command != phase)
            return -1;

        switch (phase) {
        case INSPECT_AFTER_FORK:
            if (display_maps("child after fork", NULL, -1) < 0)
                return -1;
            report_residency("child private before write", memory->private_page, memory->page_size);
            break;
        case CHANGE_PRIVATE:
            memory->private_page[0] = 'C';
            printf("[child private after write] byte=%u\n", memory->private_page[0]);
            report_residency("child private after write", memory->private_page, memory->page_size);
            break;
        case CHANGE_SHARED:
            memory->shared_page[0] = 'S';
            report_residency("child shared after write", memory->shared_page, memory->page_size);
            break;
        case PROTECT_SHARED:
            if (mprotect(memory->shared_page, memory->page_size, PROT_READ) < 0) {
                perror("mprotect child shared");
                return -1;
            }
            if (display_maps("child shared read-only", memory->shared_page, 0) < 0)
                return -1;
            break;
        }
        if (send_phase(channel, phase) < 0)
            return -1;
    }
    return 0;
}

static int exchange_phase(int channel, unsigned char phase)
{
    unsigned char acknowledgement;
    if (send_phase(channel, phase) < 0 ||
        receive_phase(channel, &acknowledgement) < 0 ||
        acknowledgement != phase) {
        fprintf(stderr, "Child did not complete experiment phase %u\n", phase);
        return -1;
    }
    return 0;
}

static int parent_experiment(int channel, const struct mappings *memory)
{
    if (display_maps("parent after fork", NULL, -1) < 0)
        return -1;
    report_residency("parent private before write", memory->private_page, memory->page_size);
    if (exchange_phase(channel, INSPECT_AFTER_FORK) < 0 ||
        exchange_phase(channel, CHANGE_PRIVATE) < 0)
        return -1;
    printf("[parent private after child write] byte=%u\n", memory->private_page[0]);
    report_residency("parent private after child write", memory->private_page, memory->page_size);
    if (memory->private_page[0] != 0)
        return -1;
    if (exchange_phase(channel, CHANGE_SHARED) < 0)
        return -1;
    printf("[parent shared after child write] byte=%u\n", memory->shared_page[0]);
    report_residency("parent shared after child write", memory->shared_page, memory->page_size);
    if (memory->shared_page[0] != 'S')
        return -1;
    if (exchange_phase(channel, PROTECT_SHARED) < 0)
        return -1;
    return display_maps("parent shared remains writable", memory->shared_page, 1);
}

static int unmap_regions(const struct mappings *memory)
{
    int result = 0;
    if (memory->private_page != MAP_FAILED && munmap(memory->private_page, memory->page_size) < 0) {
        perror("munmap private");
        result = -1;
    }
    if (memory->shared_page != MAP_FAILED && munmap(memory->shared_page, memory->page_size) < 0) {
        perror("munmap shared");
        result = -1;
    }
    if (memory->file_view != MAP_FAILED && munmap((void *)memory->file_view, memory->file_size) < 0) {
        perror("munmap file");
        result = -1;
    }
    return result;
}

static int observe_fork(const struct mappings *memory)
{
    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
        perror("socketpair");
        return -1;
    }
    pid_t child = fork();
    if (child < 0) {
        perror("fork");
        close(sockets[0]);
        close(sockets[1]);
        return -1;
    }
    if (child == 0) {
        close(sockets[0]);
        int result = child_experiment(sockets[1], memory);
        close(sockets[1]);
        int cleanup = unmap_regions(memory);
        _exit(result == 0 && cleanup == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    close(sockets[1]);
    int result = parent_experiment(sockets[0], memory);
    close(sockets[0]);  /* Unblocks the child if the parent stopped early. */

    int status;
    pid_t waited;
    do {
        waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (waited < 0) {
        perror("waitpid");
        return -1;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fputs("Child did not finish the experiment\n", stderr);
        return -1;
    }
    return result;
}

static int run_experiment(struct mappings *memory, const char *input)
{
    if (display_maps("initial", NULL, -1) < 0)
        return -1;
    memory->private_page = mmap(NULL, memory->page_size, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory->private_page == MAP_FAILED) {
        perror("mmap private");
        return -1;
    }
    if (display_maps("with private page", NULL, -1) < 0 ||
        display_maps("private mapping", memory->private_page, 1) < 0)
        return -1;
    report_residency("private before initialization", memory->private_page, memory->page_size);
    memset(memory->private_page, 0, memory->page_size);
    report_residency("private after initialization", memory->private_page, memory->page_size);
    if (map_input_file(memory, input) < 0)
        return -1;

    memory->shared_page = mmap(NULL, memory->page_size, PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (memory->shared_page == MAP_FAILED) {
        perror("mmap shared");
        return -1;
    }
    memory->shared_page[0] = 0;
    if (display_maps("shared mapping", memory->shared_page, 1) < 0)
        return -1;
    return observe_fork(memory);
}

int main(int argc, char **argv)
{
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [file]\n", argv[0]);
        return EXIT_FAILURE;
    }
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        fputs("Cannot determine the system page size\n", stderr);
        return EXIT_FAILURE;
    }
    struct mappings memory = {
        .page_size = (size_t)page_size,
        .private_page = MAP_FAILED,
        .shared_page = MAP_FAILED,
        .file_view = MAP_FAILED,
    };
    /* The child acknowledges a phase only after its observations are printed. */
    setvbuf(stdout, NULL, _IONBF, 0);
    int result = run_experiment(&memory, argc == 2 ? argv[1] : "file.txt");
    int cleanup = unmap_regions(&memory);
    if (result != 0 || cleanup != 0)
        return EXIT_FAILURE;
    puts("Memory-mapping experiment complete");
    return EXIT_SUCCESS;
}
