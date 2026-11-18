# Operating Systems Laboratory

**Michail Athanasios Peppas** and **Andreas Saounatsos** developed and evaluated Linux programs across three NTUA ECE laboratory assignments. The portfolio demonstrates system-call and process control, ordered output from concurrent Mandelbrot workers, and experiments with private and shared virtual-memory mappings.

| Assignment | Technical work | Evidence |
| --- | --- | --- |
| 1 | Character counting with `open`, `read`, and `write`; process lifetime and signals; pipe-based result collection. | [System calls and IPC report](1%CE%B7%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7/%CE%91%CE%BD%CE%B1%CF%86%CE%BF%CF%81%CE%AC%201%CE%B7%CF%82%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7%CF%82%20-%20oslab005.pdf) |
| 2 | POSIX threads, atomics and mutexes, and semaphore- and condition-variable-controlled Mandelbrot output. | [Thread synchronization report](2%CE%B7%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7/%CE%91%CE%BD%CE%B1%CF%86%CE%BF%CF%81%CE%AC%202%CE%B7%CF%82%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7%CF%82%20-%20oslab005.pdf), [condition-variable tests](tests/test_mandel_cond.py) |
| 3 | `mmap` observations of private/shared mappings and copy-on-write, plus multiprocess Mandelbrot renderers. | [Process memory report](3%CE%B7%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7/%CE%91%CE%BD%CE%B1%CF%86%CE%BF%CF%81%CE%AC%203%CE%B7%CF%82%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7%CF%82%20-%20oslab005.pdf), [source reconstruction records](student-deltas/) |

## Build and reproduction

Use Linux, GCC and GNU Make. Assignment 1 is self-contained:

```bash
make -C "1η Άσκηση/Κώδικες Ασκήσεων"
```

Assignment 2 and the Assignment 3.2 Mandelbrot renderers require the [authorized CSLab source inputs and local reconstruction steps](docs/COURSE_MATERIALS.md). Restore the Mandelbrot variants against the exact `mandel.c` baseline in a separate working copy. The reconstruction tool checks the supplied and reconstructed hashes. Two Assignment 2 targets derive from `simplesync.c` with `-DSYNC_MUTEX` or `-DSYNC_ATOMIC`. The Mandelbrot renderers use ANSI colors and are intended for a compatible terminal.

Assignment 3.1 has a self-contained virtual-memory experiment with identical, independently implemented [working](3%CE%B7%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7/%CE%9A%CF%8E%CE%B4%CE%B9%CE%BA%CE%B5%CF%82%20%CE%91%CF%83%CE%BA%CE%AE%CF%83%CE%B5%CF%89%CE%BD/3.1/mmap.c) and [submission-layout](3%CE%B7%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7/%CE%A0%CE%B1%CF%81%CE%B1%CE%B4%CE%BF%CF%84%CE%AD%CE%BF%20-%203%CE%B7%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7%20-%20oslab005/3.1/mmap.c) sources. It maps a read-only file and two anonymous pages, then synchronizes parent and child observations of copy-on-write, shared updates and per-process protection. It needs Linux with procfs, a C compiler and GNU Make. The default input is the colocated `file.txt`. See the [experiment guide](docs/COURSE_MATERIALS.md#memory-mapping-source-and-provenance) for the protocol and measurement limits.

```bash
cd "3η Άσκηση/Κώδικες Ασκήσεων/3.1"
make mmap
./mmap
```

## Validation and archival evidence

Local verification rebuilt the five Assignment 1 targets and confirmed four character matches in a two-line fixture. Using the exact reconstructed source and external header, three focused Mandelbrot tests passed. They cover argument validation, output-row counts, and an injected spurious wakeup. The [memory-mapping regressions](tests/test_mmap.py) built and ran both independent layouts in temporary directories. They verified exact file reads, private and shared write visibility, and the child and parent mapping permissions after protection changes. Fractal-pixel accuracy, thread performance and the other lab targets remain outside these checks.

Greek directory names and historical submission filenames remain part of the provenance. Working-source and submitted copies can differ. New inputs and outputs belong in a disposable working copy. Some historical `make clean` rules use broad filename patterns.

## Attribution and license

The [MIT license](LICENSE) covers the original project contributions, including the independent memory-mapping example. The memory experiment follows the NTUA CSLab assignment requirements. Credit for externally sourced laboratory materials, starter sources and Vangelis Koukis’s thread examples remains with their respective authors. The original team report and archived results document the historical coursework run. [Course inputs and provenance](docs/COURSE_MATERIALS.md)
