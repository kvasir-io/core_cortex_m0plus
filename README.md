# core_cortex_m0plus

The Cortex-M0+ (Armv6-M) core layer: `cmake/core.cmake` (target flags, `core.svd` converted with the write-only
guard), `src/core/*.hpp`, and core_cortex_common as the submodule `src/cortex_common`.

- **Priority:** 2 bits (`core.svd` `PRI_%s` width 2): levels 0-3.
- **Exceptions:** `CoreInterrupts` = the common five; `Core::InterruptRules` = Armv6-M (exceptions 4-10 and 12-13
  reserved, DDI0419E Table B1-3), HardFault the one fault exception.
- **Features** (`core/Features.hpp`): `KVASIR_CORE_ARMV6M` 1, everything else 0 - no DWT cycle counter (DDI0419E
  Table C1-18), no stack limit registers, no fault status registers, and software cannot read DHCSR ("Software
  cannot access the debug registers", Cortex-M0+ TRM DDI0484C).
- **`Core::haltingDebug()`** (`cortex_common/Debug.hpp`): always `unknown`, for that reason.
- **`Startup::Core::SecondaryCoreTraits::cpacrEnable`:** 0 (no coprocessors).
- Conformance tree: `tests/conformance` (paired with chip_rp2040).
