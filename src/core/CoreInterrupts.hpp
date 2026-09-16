#pragma once

// The common exceptions come from core_cortex_common; a Cortex-M0+ has no others.
#include "cortex_common/CoreInterrupts.hpp"

namespace Kvasir {
struct CoreInterrupts : CommonCoreInterrupts {};
}   // namespace Kvasir
