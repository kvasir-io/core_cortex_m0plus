#pragma once
// What this core package provides, checked against the SDK's derivation (kvasir/Port/Capabilities.hpp, the macros
// `#if` tests above the core use): a core whose facts the compiler macros do not give stops the build here.
#include "kvasir/Port/Capabilities.hpp"

static_assert(KVASIR_CORE_ARMV6M == 1);
static_assert(KVASIR_CORE_ARMV7EM == 0);
static_assert(KVASIR_CORE_ARMV8M_MAIN == 0);
static_assert(KVASIR_CORE_HAS_CYCLE_COUNTER
              == 0);   // the Armv6-M DWT has no CYCCNT: DDI0419E Table C1-18
static_assert(KVASIR_CORE_HAS_DEBUGEN_READ
              == 0);   // "Software cannot access the debug registers" (Cortex-M0+ TRM DDI0484C)
static_assert(KVASIR_CORE_HAS_STACK_LIMITS == 0);   // no MSPLIM/PSPLIM before Armv8-M
static_assert(KVASIR_CORE_HAS_FAULT_STATUS == 0);   // no CFSR/HFSR on Armv6-M
