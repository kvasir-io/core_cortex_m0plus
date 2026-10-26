#pragma once
// The core's start: startup() here, cleanCore() and SecondaryCoreTraits from core_cortex_common (one body for every
// core, chosen by the capability macros).
#include "cortex_common/CleanCore.hpp"
#include "cortex_common/SecondaryCoreTraits.hpp"

namespace Kvasir::Startup::Core {

// Armv6-M has nothing to set up before the first frame: no stack limit registers, no FPU.
inline void startup() {}

}   // namespace Kvasir::Startup::Core
