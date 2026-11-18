"""Compile in temporary storage; no tracked historical binaries are touched."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "2η Άσκηση/Κώδικες Ασκήσεων/mandel_cond.c"


class MandelConditionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.worker = Path(cls.tmp.name) / "worker"
        cls.cli = Path(cls.tmp.name) / "cli"
        subprocess.run(["gcc", "-O1", "-pthread", str(ROOT / "tests/condition_spurious_wakeup.c"), "-o", str(cls.worker)], check=True)
        subprocess.run(["gcc", "-O1", "-pthread", str(SOURCE), str(ROOT / "tests/mandel_cli_stubs.c"), "-o", str(cls.cli)], check=True)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def test_spurious_wakeup_rechecks_row_predicate(self):
        run = subprocess.run([str(self.worker)], capture_output=True, timeout=5)
        self.assertEqual(run.returncode, 0, run.stderr.decode())

    def test_positive_counts_including_one_and_ten(self):
        for count in ["1", "2", "10"]:
            with self.subTest(count=count):
                run = subprocess.run([str(self.cli), count], capture_output=True, timeout=5)
                self.assertEqual(run.returncode, 0, run.stderr.decode())
                self.assertEqual(run.stdout.count(b"\n"), 50)

    def test_invalid_counts_are_rejected(self):
        for count in ["0", "-1", "abc", "2x", "2147483648", "99999999999999999999999"]:
            with self.subTest(count=count):
                run = subprocess.run([str(self.cli), count], capture_output=True, timeout=5)
                self.assertNotEqual(run.returncode, 0)


if __name__ == "__main__":
    unittest.main()
