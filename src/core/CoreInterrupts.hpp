#pragma once

// The common exceptions come from core_cortex_common; a Cortex-M0+ has no others.
#include "cortex_common/CoreInterrupts.hpp"
#include "cortex_common/InterruptRules.hpp"
#include "kvasir/Mpl/Types.hpp"

namespace Kvasir {
struct CoreInterrupts : CommonCoreInterrupts {};

namespace Core {
    // Armv6-M's exception rules; HardFault is the one fault exception.
    struct InterruptRules : CoreInterruptRules<ArmProfile::v6m> {
        using FaultInterruptIndexs = brigand::list<decltype(CoreInterrupts::hardFault)>;
    };
}   // namespace Core
}   // namespace Kvasir
