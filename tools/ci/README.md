# Linux/QEMU correctness matrix

`run_linux_qemu_matrix.sh` runs existing binaries and fixtures; it does not build
the runtime or toolchain. Use `--help` for platform wrappers and resource caps.
The default is one target process and zero extra compiler workers. Run lightweight
mock regressions with `bash tools/ci/test_run_linux_qemu_matrix.sh`.

## Tier-2 evidence boundary

`tiered_full_ready_oob` checks **T2 compilation/publication readiness plus the
expected final trap and logical stack**, not execution of a T2 entry. Its fixed
hot-call loop is not a synchronization handshake, and the same final trap can
occur in T0 or T1. `tiered-full-request` / `tiered-full-ready` are compiler events;
the current runtime exports no T2-specific executed-entry witness.

The case's PASS detail therefore includes `t2_publication=ready`,
`t2_entry_execution=unverified` and `t2_unwind_coverage=unverified`. The same
evidence limit is recorded in metadata. Do not count this PASS as T2 execution
or T2 unwind coverage. Missing readiness or an incorrect trap still fails.

The readiness case requires `--compile-threads >= 2`; smaller budgets are
preserved and the case is SKIP. It uses `instruction`, or `auto` only when the
probe resolves it to `instruction`. Native unwind disables background T2 in the
current runtime, so a matrix with no compatible selected policy gets an explicit
SKIP rather than a purported T2 test. Native-unwind correctness is tested by
other applicable matrix cases, without implying T2 coverage.

TSV retains its existing 20 columns and PASS/FAIL/SKIP vocabulary. These labels
describe each case's documented scope, not complete production certification.
