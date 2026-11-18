/* Exercise the coursework worker with a controlled spurious wakeup. */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
static int injected_wait(pthread_cond_t *, pthread_mutex_t *);
#define pthread_cond_wait injected_wait
#define main coursework_main
#include "../2η Άσκηση/Κώδικες Ασκήσεων/mandel_cond.c"
#undef main
#undef pthread_cond_wait

static sem_t entered_wait;
static int wait_count;
static int output_count;
static int output_color;

static int injected_wait(pthread_cond_t *condition, pthread_mutex_t *mutex)
{
    if (++wait_count == 1) {
        sem_post(&entered_wait);
        return 0; /* Legal wakeup without a change to the ordering predicate. */
    }
    return pthread_cond_wait(condition, mutex);
}

/* Bound numerical work; observe which row the real worker outputs. */
int mandel_iterations_at_point(double x, double y, int max) { return (int)y; }
unsigned char xterm_color(int color) { return color; }
void set_xterm_color(int fd, unsigned char color)
{
    output_count++;
    output_color = color;
}
void reset_xterm_color(int fd) {}

int main(void)
{
    pthread_t worker;
    pthread_cond_t conditions[2];
    int color = -1;
    struct thread_info_struct info = {.color_val = &color, .thrid = 1, .thrcnt = 2};
    x_chars = 1; y_chars = 2; xstep = 1; ystep = 1; ymax = 2;
    counter = 0; cond = conditions;
    /* Production lock uses PTHREAD_MUTEX_INITIALIZER. */
    pthread_cond_init(&conditions[0], NULL);
    pthread_cond_init(&conditions[1], NULL);
    sem_init(&entered_wait, 0, 0);
    pthread_create(&worker, NULL, compute_and_output_mandel_line, &info);
    sem_wait(&entered_wait);
    /* Worker holds lock until it actually waits or incorrectly emits a row. */
    pthread_mutex_lock(&lock);
    int premature_output = counter != 0 || output_count != 0;
    counter = 1;
    pthread_cond_broadcast(&conditions[1]);
    pthread_mutex_unlock(&lock);
    pthread_join(worker, NULL);
    pthread_cond_destroy(&conditions[0]);
    pthread_cond_destroy(&conditions[1]);
    pthread_mutex_destroy(&lock);
    sem_destroy(&entered_wait);
    if (premature_output || output_count != 1 || output_color != 1 || counter != 2) {
        fprintf(stderr, "FAIL: a spurious wakeup emitted row 1 before row 0\n");
        return 1;
    }
    fprintf(stderr, "PASS: row predicate survives a spurious wakeup\n");
    return 0;
}
