#!/usr/bin/env python3
"""Lightweight unit tests for wasmtime_full_backend_matrix.py."""

from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path
from unittest import mock

THIS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(THIS_DIR))

import wasmtime_full_backend_matrix as matrix


WASMTIME_ALIGNMENT_DIAGNOSTIC = (
    "error while executing at wasm backtrace:\n"
    "  1: Pointer not aligned to 4: Region { start: 1, len: 8 }"
)


def result(outcome: str, *, stdout: str = "", stderr: str = "", returncode: int = 1) -> matrix.ProcessResult:
    return matrix.ProcessResult(
        argv=["runtime"],
        returncode=returncode,
        timed_out=False,
        elapsed_ms=0.0,
        stdout=stdout,
        stderr=stderr,
        outcome=outcome,
    )


class WasiPointerAlignmentTrapTests(unittest.TestCase):
    def test_uwvm_marker_is_canonicalized(self) -> None:
        stderr = (
            "uwvm: [fatal] WASI Preview 1 guest pointer alignment trap: "
            "fd_write.iovs (ciovec), guest offset = 1, required alignment = 4 bytes"
        )
        self.assertEqual(matrix.classify(-4, False, "", stderr), "trap:wasi-pointer-alignment")

    def test_uwvm_marker_requires_fatal_prefix(self) -> None:
        stderr = (
            "WASI Preview 1 guest pointer alignment trap: "
            "fd_write.iovs (ciovec), guest offset = 1, required alignment = 4 bytes"
        )
        self.assertEqual(matrix.classify(1, False, "", stderr), "exit:1")

    def test_wasmtime_region_diagnostic_is_canonicalized(self) -> None:
        stderr = "error while executing at wasm backtrace:\n  1: Pointer not aligned to 4: Region { start: 1, len: 8 }"
        self.assertEqual(matrix.classify(1, False, "", stderr), "trap:wasi-pointer-alignment")

    def test_cross_runtime_alignment_traps_compare_equal(self) -> None:
        uwvm = result(
            "trap:wasi-pointer-alignment",
            stderr="WASI Preview 1 guest pointer alignment trap",
            returncode=-4,
        )
        wasmtime = result(
            "trap:wasi-pointer-alignment",
            stderr="Pointer not aligned to 4: Region { start: 1, len: 8 }",
        )
        self.assertTrue(matrix.same_result(uwvm, wasmtime))

    def test_alignment_and_unrelated_traps_do_not_compare_equal(self) -> None:
        alignment = result("trap:wasi-pointer-alignment")
        memory_oob = result("trap:memory-oob")
        self.assertFalse(matrix.same_result(alignment, memory_oob))

    def test_generic_alignment_wording_is_not_canonicalized(self) -> None:
        self.assertEqual(matrix.classify(1, False, "", "return pointer not aligned"), "exit:1")
        self.assertEqual(matrix.classify(1, False, "", "pointer not aligned"), "exit:1")

    def test_wasmtime_region_requires_backtrace_context(self) -> None:
        stderr = "Pointer not aligned to 4: Region { start: 1, len: 8 }"
        self.assertEqual(matrix.classify(1, False, "", stderr), "exit:1")

    def test_non_abi_alignment_and_wrong_region_shape_are_not_canonicalized(self) -> None:
        context = "error while executing at wasm backtrace:\n"
        self.assertEqual(
            matrix.classify(1, False, "", context + "Pointer not aligned to 16: Region { start: 1, len: 8 }"),
            "exit:1",
        )
        self.assertEqual(
            matrix.classify(1, False, "", context + "Pointer not aligned to 4: Region { len: 8, start: 1 }"),
            "exit:1",
        )

    def test_alignment_text_on_stdout_is_not_canonicalized(self) -> None:
        self.assertEqual(matrix.classify(1, False, WASMTIME_ALIGNMENT_DIAGNOSTIC, ""), "exit:1")

    def test_success_and_timeout_take_precedence_over_diagnostics(self) -> None:
        self.assertEqual(matrix.classify(0, False, "", WASMTIME_ALIGNMENT_DIAGNOSTIC), "ok")
        self.assertEqual(matrix.classify(124, True, "", WASMTIME_ALIGNMENT_DIAGNOSTIC), "timeout")

    def test_core_wasm_unaligned_access_is_not_canonicalized(self) -> None:
        self.assertEqual(matrix.classify(1, False, "", "unaligned i32.load at address 1"), "exit:1")


class ProvenanceProbeTests(unittest.TestCase):
    def test_successful_version_probe_records_text(self) -> None:
        probe = matrix.probe_command([sys.executable, "--version"], Path.cwd(), 2.0)
        self.assertEqual(probe["status"], "ok")
        self.assertIn("Python", probe["text"])

    def test_missing_probe_is_structured_and_non_fatal(self) -> None:
        probe = matrix.probe_command(["/definitely/missing/uwvm-matrix-tool"], Path.cwd(), 2.0)
        self.assertEqual(probe["status"], "unavailable")
        self.assertEqual(probe["result"]["outcome"], "spawn-error")
        self.assertIsNotNone(probe["error"])

    def test_collected_provenance_is_strict_json(self) -> None:
        executable = Path(sys.executable)
        provenance = matrix.collect_provenance(
            Path.cwd(),
            executable,
            executable,
            executable,
            executable,
            executable,
            executable,
            2.0,
        )
        encoded = json.dumps(provenance, allow_nan=False)
        self.assertIn('"corpus"', encoded)
        self.assertIn('"tools"', encoded)

    def test_corpus_revision_is_extracted_from_git_probe(self) -> None:
        revision = "0123456789abcdef0123456789abcdef01234567"
        revision_probe = {
            "status": "ok",
            "text": revision,
            "error": None,
            "result": {"stdout": revision + "\n"},
        }
        status_probe = {
            "status": "ok",
            "text": None,
            "error": None,
            "result": {"stdout": ""},
        }
        with mock.patch.object(matrix, "probe_command", side_effect=[revision_probe, status_probe]):
            provenance = matrix.corpus_git_provenance(Path("/corpus"), 2.0)
        self.assertEqual(provenance["revision"], revision)
        self.assertFalse(provenance["dirty"])
        self.assertFalse(provenance["tracked_changes"])
        self.assertFalse(provenance["untracked_changes"])
        self.assertFalse(provenance["submodule_changes"])
        self.assertEqual(provenance["status"], [])

    def test_dirty_corpus_status_is_not_hidden_by_clean_head_revision(self) -> None:
        revision = "0123456789abcdef0123456789abcdef01234567"
        revision_probe = {
            "status": "ok",
            "text": revision,
            "error": None,
            "result": {"stdout": revision + "\n"},
        }
        status_probe = {
            "status": "ok",
            "text": "1 .M N... 100644 100644 100644 abc def all/example.wat\n? generated.wat",
            "error": None,
            "result": {"stdout": "1 .M N... 100644 100644 100644 abc def all/example.wat\n? generated.wat\n"},
        }
        with mock.patch.object(matrix, "probe_command", side_effect=[revision_probe, status_probe]):
            provenance = matrix.corpus_git_provenance(Path("/corpus"), 2.0)
        self.assertTrue(provenance["dirty"])
        self.assertTrue(provenance["tracked_changes"])
        self.assertTrue(provenance["untracked_changes"])
        self.assertFalse(provenance["submodule_changes"])
        self.assertEqual(
            provenance["status"],
            ["1 .M N... 100644 100644 100644 abc def all/example.wat", "? generated.wat"],
        )

    def test_submodule_status_is_reported_separately(self) -> None:
        revision = "0123456789abcdef0123456789abcdef01234567"
        revision_probe = {
            "status": "ok",
            "text": revision,
            "error": None,
            "result": {"stdout": revision + "\n"},
        }
        status_probe = {
            "status": "ok",
            "text": "1 .M S.M. 160000 160000 160000 abc def nested-corpus",
            "error": None,
            "result": {"stdout": "1 .M S.M. 160000 160000 160000 abc def nested-corpus\n"},
        }
        with mock.patch.object(matrix, "probe_command", side_effect=[revision_probe, status_probe]):
            provenance = matrix.corpus_git_provenance(Path("/corpus"), 2.0)
        self.assertTrue(provenance["dirty"])
        self.assertTrue(provenance["tracked_changes"])
        self.assertFalse(provenance["untracked_changes"])
        self.assertTrue(provenance["submodule_changes"])

    def test_non_object_git_output_is_reported_without_inventing_a_revision(self) -> None:
        revision_probe = {
            "status": "ok",
            "text": "not-a-git-object",
            "error": None,
            "result": {"stdout": "not-a-git-object\n"},
        }
        status_probe = {
            "status": "failed",
            "text": None,
            "error": None,
            "result": {"stdout": ""},
        }
        with mock.patch.object(matrix, "probe_command", side_effect=[revision_probe, status_probe]):
            provenance = matrix.corpus_git_provenance(Path("/corpus"), 2.0)
        self.assertIsNone(provenance["revision"])
        self.assertIsNone(provenance["dirty"])
        self.assertIsNone(provenance["tracked_changes"])
        self.assertIsNone(provenance["untracked_changes"])
        self.assertIsNone(provenance["submodule_changes"])
        self.assertEqual(provenance["revision_probe"]["status"], "failed")
        self.assertIn("non-object-id", provenance["revision_probe"]["error"])


if __name__ == "__main__":
    unittest.main()
