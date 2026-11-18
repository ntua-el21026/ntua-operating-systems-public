# NTUA Operating Systems Laboratory materials

The teaching PDFs and slide files below were course materials. Use the [CSLab laboratory page](https://www.cslab.ece.ntua.gr/courses/os/) and the [NTUA course catalogue](https://www.ece.ntua.gr/en/education/undergraduate/courses?courseId=3237) to identify the original edition.

- `1η Άσκηση/os-lab-01-intro.ppsx`
- `1η Άσκηση/os-lab-exer1.pdf.pdf`
- `2η Άσκηση/os-lab-exer2.pdf`
- `2η Άσκηση/os-lab02-intro.pdf`
- `3η Άσκηση/os-lab-exer3-vm.pdf`
- `3η Άσκηση/os-lab03-mmap.pptm`
- `docs/Διαλέξεις/os-01-introduction.pdf`
- `docs/Διαλέξεις/os-02a-processes.pdf`
- `docs/Διαλέξεις/os-02b-processes.ppsx`
- `docs/Διαλέξεις/os-02c-processes.ppsx`
- `docs/Διαλέξεις/os-02d-processes.pdf`
- `docs/Διαλέξεις/os-03-scheduling.pdf`
- `docs/Διαλέξεις/os-04-synchronization.pdf`
- `docs/Διαλέξεις/os-05-MM.ppsx`
- `docs/Διαλέξεις/os-06-VM1.pdf`
- `docs/Διαλέξεις/os-06-VM2.ppsx`
- `docs/Διαλέξεις/os-07-filesystems.pdf`
- `Προπαρασκευαστική Άσκηση/os-lab-00-intro.pdf`
- `Προπαρασκευαστική Άσκηση/os-lab-exer0.pdf`

## Material roles and reproduction

| Material from the course workspace | Role | Retained project evidence and reproduction |
| --- | --- | --- |
| 19 teaching PDFs and slide files listed above | Lab specifications and reference lectures on process control, synchronization, memory management, and filesystems | Full report inspection. Statement and slide access depends on the authorized 2025–26 course edition. |
| Lab 2 starter examples and shared Mandelbrot code | Thread, synchronization and renderer baselines plus support code used by the corresponding targets and regressions | The team reports, distinct changes and independent test harness remain. Exact builds are conditional on matching local starter files. |
| Lab 3 `help.c` and `help.h` | Process-map and page-table helpers used by the historical memory-mapping exercise | The independent example uses Linux procfs and standard library calls directly. It builds from the maintained source, Makefile and file fixture in each layout. The original report remains evidence of the earlier run. |
| Previously linked objects and executables | Historical compiled products with course-source dependencies | Assignment 1 outputs remain. Rebuild the other targets after supplying the inputs below. |
| Source listings and Lab 2/3 submission archive entries | Supporting evidence for the reports and historical submissions | Scoped replacement pages identify the exercise and original role. Related report bodies, observations and figures remain. The independent Lab 1 submission archive remains intact. |

## Authorized local build inputs

Obtain the 2025–26 assignment versions through authorized CSLab access. Place instructor files only in a separate working copy. Retain Vangelis Koukis/CSLab attribution on their original code.

- Lab 2 requires `kgarten.c`, `pthread-test.c`, `simplesync.c`, `mandel.c`, `mandel-lib.c` and `mandel-lib.h` in its `Κώδικες Ασκήσεων/` directory, then the locally reconstructed `mandel_cond.c` and `mandel_sema.c`.
- Lab 3 renderers use `mandel-lib.c` and `mandel-lib.h` in `Κώδικες Ασκήσεων/3.2/`, and the corresponding locally reconstructed `mandel-fork1.c` and `mandel-fork2.c` variants.

## Memory-mapping source and provenance

The [working source](../3%CE%B7%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7/%CE%9A%CF%8E%CE%B4%CE%B9%CE%BA%CE%B5%CF%82%20%CE%91%CF%83%CE%BA%CE%AE%CF%83%CE%B5%CF%89%CE%BD/3.1/mmap.c) and [submission-layout source](../3%CE%B7%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7/%CE%A0%CE%B1%CF%81%CE%B1%CE%B4%CE%BF%CF%84%CE%AD%CE%BF%20-%203%CE%B7%20%CE%86%CF%83%CE%BA%CE%B7%CF%83%CE%B7%20-%20oslab005/3.1/mmap.c) are identical independent implementations of Assignment 3.1. They were designed from the assignment's functional requirements and documented Linux interfaces. The original CSLab starter and helpers remain external. The team report and historical archive record the original coursework experiment, and their measurements and screenshots retain their original context.

Run `make mmap` within either `3.1` directory, then run `./mmap` to map the same-directory `file.txt`. To inspect another nonempty regular file, pass its path as the sole command-line argument. The demonstration maps file data read-only and prints those bytes verbatim. Use a stable file throughout the run.

The program allocates a page with `MAP_PRIVATE` and observes residency before and after the first write. A second anonymous mapping uses `MAP_SHARED`. After `fork`, a local Unix socket pair orders the observations. A child write changes only its private byte, whereas a child write to the shared page becomes visible in the parent. Once the child changes its shared mapping to read-only, both processes inspect their own entries in `/proc/self/maps`. The child's mapping reads `r--s`; the parent's remains `rw-s`. The processes release their mappings, and the parent waits for the child to finish.

`mincore` reports a momentary residency observation. The public example uses it and the observed bytes to illustrate demand allocation, sharing and copy-on-write without relying on physical page-frame numbers. The historical physical-address observations remain in the report. The program requires no elevated privileges. [Linux memory mappings](https://man7.org/linux/man-pages/man2/mmap.2.html) · [Page residency](https://man7.org/linux/man-pages/man2/mincore.2.html) · [Memory protection](https://man7.org/linux/man-pages/man2/mprotect.2.html)

From the repository root, run the regressions in disposable directories using Python's standard library, GCC and GNU Make.

```bash
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tests -p test_mmap.py -v
```

## Reconstructing exact student variants

Use the exact authorized `mandel.c` from Lab 2 as the baseline for the records below. Each checks both input and reconstructed output SHA-256. The record includes student differences and source ranges needed to assemble the original; the shared baseline bytes are supplied by the user.

```bash
python3 tools/restore_course_variant.py /path/to/authorized/mandel.c student-deltas/mandel_cond.c.delta.json "2η Άσκηση/Κώδικες Ασκήσεων/mandel_cond.c"
```

| Local assembly destination | Delta file | Literal characters / full variant |
|---|---|---:|
| `2η Άσκηση/Κώδικες Ασκήσεων/mandel_cond.c` | `student-deltas/mandel_cond.c.delta.json` | 3716/6074 |
| `2η Άσκηση/Κώδικες Ασκήσεων/mandel_sema.c` | `student-deltas/mandel_sema.c.delta.json` | 3088/5425 |
| `2η Άσκηση/Παραδοτέο - 2η Άσκηση - oslab005/mandel_cond.c` | `student-deltas/2η-submitted-mandel_cond.c.delta.json` | 3175/5518 |
| `2η Άσκηση/Παραδοτέο - 2η Άσκηση - oslab005/mandel_sema.c` | `student-deltas/mandel_sema.c.delta.json` | 3088/5425 |
| `3η Άσκηση/Κώδικες Ασκήσεων/3.2/mandel-fork1.c` | `student-deltas/mandel-fork1.c.delta.json` | 2848/5449 |
| `3η Άσκηση/Κώδικες Ασκήσεων/3.2/mandel-fork2.c` | `student-deltas/mandel-fork2.c.delta.json` | 2740/5155 |
| `3η Άσκηση/Παραδοτέο - 3η Άσκηση - oslab005/3.2/mandel-fork1.c` | `student-deltas/mandel-fork1.c.delta.json` | 2848/5449 |
| `3η Άσκηση/Παραδοτέο - 3η Άσκηση - oslab005/3.2/mandel-fork2.c` | `student-deltas/mandel-fork2.c.delta.json` | 2740/5155 |
