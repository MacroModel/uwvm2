#!/usr/bin/env python3
"""Regression tests for the lightweight header/module dependency checkers."""

from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path
from types import ModuleType


THIS_DIR = Path(__file__).resolve().parent


def load_checker(module_name: str, filename: str) -> ModuleType:
    spec = importlib.util.spec_from_file_location(module_name, THIS_DIR / filename)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load checker: {filename}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)
    return module


PRAGMA_CHECKER = load_checker("uwvm_test_pragma_once_guard", "check_pragma_once_guard.py")
MODULE_CHECKER = load_checker("uwvm_test_module_dependencies", "check_uwvm_module.py")


def module_dependencies(text: str) -> list[str]:
    return MODULE_CHECKER.extract_imports_from_cppm_or_module_cpp(
        text
    ) + MODULE_CHECKER.extract_global_fragment_includes(text)


def guarded_dependencies(text: str) -> list[str]:
    return MODULE_CHECKER.extract_guarded_includes(text)


class PragmaOnceGuardTests(unittest.TestCase):
    def check_temporary_header(self, text: str) -> str | None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "surface.h"
            path.write_text(text, encoding="utf-8")
            return PRAGMA_CHECKER.check_header(str(path))

    def test_dual_surface_include_before_guard_is_reported(self) -> None:
        message = self.check_temporary_header(
            """\
#pragma once
#include <uwvm2/utils/container/impl.h>
#ifndef UWVM_MODULE
#endif
"""
        )

        self.assertIsNotNone(message)
        self.assertIn(":2:", message)
        self.assertIn("escapes", message)

    def test_split_helper_without_module_guard_is_out_of_scope(self) -> None:
        message = self.check_temporary_header(
            """\
#pragma once
#include "native_unwind_platform.h"
"""
        )

        self.assertIsNone(message)


class ModuleDependencyTests(unittest.TestCase):
    def test_conditional_export_import_is_reported(self) -> None:
        module_text = """\
export module uwvm2.example;
#if defined(UWVM_RUNTIME_LLVM_JIT)
export import :runtime_aot;
#endif
"""

        self.assertEqual(
            MODULE_CHECKER.find_conditionally_exported_imports(module_text),
            [(3, ":runtime_aot")],
        )

    def test_backend_guard_after_unconditional_import_is_legal(self) -> None:
        module_text = """\
export module uwvm2.example;
export import :runtime_aot;
#if defined(UWVM_RUNTIME_LLVM_JIT)
int backend_declaration;
#endif
"""

        self.assertEqual(
            MODULE_CHECKER.find_conditionally_exported_imports(module_text), []
        )

    def test_frontend_module_partition_removal_is_reported(self) -> None:
        xmake_text = """\
add_files("src/uwvm2/uwvm/**.cppm")
remove_files("src/uwvm2/uwvm/cmdline/params/runtime_aot.cppm")
"""

        self.assertEqual(
            MODULE_CHECKER.find_frontend_module_removals(xmake_text), [2]
        )

    def test_additional_direct_module_import_is_legal(self) -> None:
        module_text = """\
export module uwvm2.example;
import uwvm2.utils.container;
import uwvm2.parser;
"""
        header_text = """\
#pragma once
#ifndef UWVM_MODULE
#include <uwvm2/utils/container/impl.h>
#endif
"""

        ok, differences = MODULE_CHECKER.compare_dependency_coverage(
            module_dependencies(module_text), guarded_dependencies(header_text)
        )

        self.assertTrue(ok)
        self.assertEqual(differences, [])

    def test_header_dependency_missing_from_module_is_reported(self) -> None:
        module_text = "export module uwvm2.example;\n"
        header_text = """\
#pragma once
#ifndef UWVM_MODULE
#include <uwvm2/utils/container/impl.h>
#endif
"""

        ok, differences = MODULE_CHECKER.compare_dependency_coverage(
            module_dependencies(module_text), guarded_dependencies(header_text)
        )

        self.assertFalse(ok)
        self.assertEqual(
            differences,
            ["Header dependencies missing from module unit: uwvm2.utils.container"],
        )

    def test_global_module_fragment_include_satisfies_header_dependency(self) -> None:
        module_text = """\
module;
#include <uwvm2/utils/container/impl.h>
export module uwvm2.example;
"""
        header_text = """\
#pragma once
#ifndef UWVM_MODULE
#include <uwvm2/utils/container/impl.h>
#endif
"""

        ok, differences = MODULE_CHECKER.compare_dependency_coverage(
            module_dependencies(module_text), guarded_dependencies(header_text)
        )

        self.assertTrue(ok)
        self.assertEqual(differences, [])

    def test_quoted_errno_system_header_is_not_a_partition(self) -> None:
        self.assertIsNone(
            MODULE_CHECKER.normalize_header_to_import_name("errno.h", is_local=True)
        )

    def test_existing_quoted_header_is_a_local_partition(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "surface.h"
            (Path(directory) / "define.h").write_text("#pragma once\n", encoding="utf-8")
            source.write_text(
                """\
#pragma once
#ifndef UWVM_MODULE
#include "define.h"
#endif
""",
                encoding="utf-8",
            )

            dependencies = MODULE_CHECKER.extract_guarded_includes(
                source.read_text(encoding="utf-8"), source_path=str(source)
            )

        self.assertEqual(dependencies, [":define"])

    def test_import_with_trailing_comment_is_recognized(self) -> None:
        imports = MODULE_CHECKER.extract_imports_from_cppm_or_module_cpp(
            "import uwvm2.utils.container; // dependency rationale\n"
        )

        self.assertEqual(imports, ["uwvm2.utils.container"])

    def test_win32_color_provider_must_precede_textual_surface(self) -> None:
        surface_text = """\
#include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>
inline void report() { use(UWVM_COLOR_RED); }
"""
        module_text = """\
export module uwvm2.example;
#include "surface.h"
import uwvm2.uwvm_predefine.utils.ansies;
"""

        self.assertFalse(
            MODULE_CHECKER.win32_text_attr_provider_precedes_surface(
                module_text, surface_text, "surface.h"
            )
        )

    def test_win32_color_provider_before_textual_surface_is_legal(self) -> None:
        surface_text = """\
#include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>
inline void report() { use(UWVM_COLOR_RED); }
"""
        module_text = """\
export module uwvm2.example;
import uwvm2.uwvm_predefine.utils.ansies;
#include "surface.h"
"""

        self.assertTrue(
            MODULE_CHECKER.win32_text_attr_provider_precedes_surface(
                module_text, surface_text, "surface.h"
            )
        )

    def test_win32_color_partition_is_not_an_external_provider(self) -> None:
        surface_text = """\
#include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>
inline void report() { use(UWVM_COLOR_RED); }
"""
        module_text = """\
export module uwvm2.example;
import uwvm2.utils.ansies:win32_text_attr;
#include "surface.h"
"""

        # Partitions may only be imported by units of their owning named module;
        # ordinary consumers must import the primary module or a re-exporter.
        self.assertFalse(
            MODULE_CHECKER.win32_text_attr_provider_precedes_surface(
                module_text, surface_text, "surface.h"
            )
        )

    def test_low_level_text_attr_provider_cannot_satisfy_uwvm_color(self) -> None:
        surface_text = """\
#include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>
inline void report() { use(UWVM_COLOR_RED); }
"""
        module_text = """\
export module uwvm2.example;
import uwvm2.utils.ansies;
#include "surface.h"
"""

        self.assertFalse(
            MODULE_CHECKER.win32_text_attr_provider_precedes_surface(
                module_text, surface_text, "surface.h"
            )
        )

    def test_low_level_provider_satisfies_direct_text_attr_macros(self) -> None:
        surface_text = """\
#include <uwvm2/utils/ansies/win32_text_attr_push_macro.h>
inline void report() { use(UWVM_WIN32_TEXTATTR_RED); }
"""
        module_text = """\
export module uwvm2.example;
import uwvm2.utils.ansies;
#include "surface.h"
"""

        self.assertTrue(
            MODULE_CHECKER.win32_text_attr_provider_precedes_surface(
                module_text, surface_text, "surface.h"
            )
        )

    def test_win32_color_documentation_without_macro_header_is_ignored(self) -> None:
        surface_text = "/// UWVM_COLOR_RED is documented here.\n"
        module_text = """\
export module uwvm2.example;
#include "surface.h"
"""

        self.assertTrue(
            MODULE_CHECKER.win32_text_attr_provider_precedes_surface(
                module_text, surface_text, "surface.h"
            )
        )


if __name__ == "__main__":
    unittest.main()
