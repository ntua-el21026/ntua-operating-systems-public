"""Build both independent mmap examples and check their Linux VM observations."""

from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest


ROOT = Path(__file__).resolve().parents[1]
LAYOUTS = {
    "working": ROOT / "3η Άσκηση/Κώδικες Ασκήσεων/3.1",
    "submission": ROOT / "3η Άσκηση/Παραδοτέο - 3η Άσκηση - oslab005/3.1",
}


class MemoryMappingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        workspace = TemporaryDirectory(prefix="mmap-regression-")
        cls.addClassCleanup(workspace.cleanup)
        cls.worktrees = {}

        for label, source in LAYOUTS.items():
            worktree = Path(workspace.name) / label
            worktree.mkdir()
            for file in ("Makefile", "mmap.c", "file.txt"):
                shutil.copy2(source / file, worktree)
            subprocess.run(
                [
                    "make", "-B", "mmap",
                    "CFLAGS=-O2 -std=c11 -Wall -Wextra -Wpedantic -Werror",
                ],
                cwd=worktree, check=True, timeout=15,
            )
            cls.worktrees[label] = worktree

    def invoke(self, worktree, *args):
        return subprocess.run(
            ["./mmap", *args], cwd=worktree, capture_output=True, timeout=5,
        )

    def test_layouts_use_the_same_independent_implementation(self):
        working = (self.worktrees["working"] / "mmap.c").read_bytes()
        submission = (self.worktrees["submission"] / "mmap.c").read_bytes()
        self.assertEqual(working, submission)

    def test_default_file_maps_the_fixture_read_only(self):
        for label, worktree in self.worktrees.items():
            with self.subTest(layout=label):
                fixture = worktree / "file.txt"
                before = fixture.read_bytes()
                run = self.invoke(worktree)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertIn(f"[file] {len(before)} bytes\n".encode() + before, run.stdout)
                self.assertRegex(run.stdout, rb"(?m)^\[file mapping\] \S+ r--p ")
                self.assertEqual(fixture.read_bytes(), before)

    def test_anonymous_page_residency_after_first_write(self):
        for label, worktree in self.worktrees.items():
            with self.subTest(layout=label):
                run = self.invoke(worktree)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertRegex(
                    run.stdout, rb"(?m)^\[private before initialization\] address=0x[0-9a-f]+ not resident$",
                )
                self.assertRegex(
                    run.stdout, rb"(?m)^\[private after initialization\] address=0x[0-9a-f]+ resident$",
                )

    def test_private_and_shared_writes_follow_mapping_semantics(self):
        for label, worktree in self.worktrees.items():
            with self.subTest(layout=label):
                run = self.invoke(worktree)
                self.assertEqual(run.returncode, 0, run.stderr)
                ordered = (
                    b"[initial] process map",
                    b"[private before initialization]",
                    b"[private after initialization]",
                    b"[parent after fork] process map",
                    b"[child after fork] process map",
                    b"[child private after write] byte=67",
                    b"[parent private after child write] byte=0",
                    b"[parent shared after child write] byte=83",
                    b"[child shared read-only]",
                    b"[parent shared remains writable]",
                    b"Memory-mapping experiment complete",
                )
                last = -1
                for marker in ordered:
                    index = run.stdout.find(marker, last + 1)
                    self.assertGreater(index, last, marker)
                    last = index

    def test_mprotect_changes_only_the_child_mapping(self):
        for label, worktree in self.worktrees.items():
            with self.subTest(layout=label):
                run = self.invoke(worktree)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertRegex(run.stdout, rb"(?m)^\[child shared read-only\] \S+ r--s ")
                self.assertRegex(run.stdout, rb"(?m)^\[parent shared remains writable\] \S+ rw-s ")

    def test_explicit_filename_maps_binary_bytes(self):
        fixture = b"mapping\x00fixture\xff"
        for label, worktree in self.worktrees.items():
            with self.subTest(layout=label):
                path = worktree / "sample with spaces.bin"
                path.write_bytes(fixture)
                run = self.invoke(worktree, path.name)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertIn(f"[file] {len(fixture)} bytes\n".encode() + fixture, run.stdout)
                self.assertEqual(path.read_bytes(), fixture)

    def test_missing_empty_and_non_regular_files_are_rejected(self):
        for label, worktree in self.worktrees.items():
            (worktree / "empty").touch()
            for filename in ("missing", "empty", "."):
                with self.subTest(layout=label, input=filename):
                    run = self.invoke(worktree, filename)
                    self.assertNotEqual(run.returncode, 0)
                    self.assertIn(filename.encode(), run.stderr)
                    self.assertNotIn(b"experiment complete", run.stdout)

    def test_unexpected_arguments_are_rejected(self):
        for label, worktree in self.worktrees.items():
            with self.subTest(layout=label):
                run = self.invoke(worktree, "one", "two")
                self.assertNotEqual(run.returncode, 0)
                self.assertIn(b"Usage:", run.stderr)
