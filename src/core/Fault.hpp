#pragma once
#include "core_peripherals/SCB.hpp"
#include "kvasir/Register/Register.hpp"

#include <cstdint>
#include <optional>
#include <utility>

using Kvasir::Register::apply;
using Kvasir::Register::read;

// Kvasir::Fault::Core, next to the SDK's Kvasir::Fault (FaultHandler.hpp): its log module is
// "fault.core", derived from the scope.
namespace Kvasir::Fault::Core {
using SCB_R = Kvasir::Peripheral::SCB::Registers<>;

enum class FaultType : std::uint8_t { Hard };

// Armv6-M has no fault status registers: every fault is a HardFault. DFSR (and SHCSR) belong
// to the Debug Extension (DDI0419E, SCB table footnote d, md l.8547), "access from the
// processor is IMPLEMENTATION DEFINED" (C1.6, md l.10865), and the Cortex-M0+ TRM lists DFSR
// with the debug registers that "software cannot access" (DDI0484C 7.2, Table 7-6 and the
// Note at its end, md l.1612-1669). So nothing is read from them: what they return is
// unspecified.
enum class FaultDescription : std::uint8_t { EscalatedFault };

struct FaultInfo {
    FaultType                    type;
    FaultDescription             description;
    std::optional<std::uint32_t> fault_address;
    std::uint32_t                status_bits;
};

static constexpr FaultInfo GetFaultInfo() {
    FaultInfo info{};
    info.type        = FaultType::Hard;
    info.description = FaultDescription::EscalatedFault;
    info.status_bits = static_cast<std::uint32_t>(apply(read(SCB_R::ICSR::vectactive)));
    return info;
}

// A plain copy of the fault registers for the SDK's full fault record (Kvasir::CrashRecord::Full,
// FaultHandler.hpp): no analysis, no log, nothing that can fault. Addresses from the Armv6-M ARM DDI0419E SCB table (md l.8539).
struct Snapshot {
    std::uint32_t cfsr, hfsr, mmfar, bfar, shcsr, icsr, sfsr, sfar;
    bool          faultRegs;   // Armv6-M has no fault status registers: all 0
    bool          secureRegs;
};

inline Snapshot snapshot() {
    return Snapshot{
      .cfsr  = 0,
      .hfsr  = 0,
      .mmfar = 0,
      .bfar  = 0,
      .shcsr = 0,   // Debug Extension only, not readable by software (see above)
      .icsr = Kvasir::Register::readWhole(Kvasir::Peripheral::SCB::Registers<>::ICSR::FULLREGISTER),
      .sfsr = 0,
      .sfar = 0,
      .faultRegs  = false,
      .secureRegs = false};
}

// No CFSR/HFSR on Armv6-M (above): nothing to clear.
inline void clearFaultStatus(std::uint32_t,
                             std::uint32_t) {}

inline void clearFaultStatus() {}

using EarlyInitList = decltype(MPL::list());

static inline void Log([[maybe_unused]] std::uint32_t const* stack_ptr,
                       [[maybe_unused]] std::uint32_t        lr_value) {
    [[maybe_unused]] FaultInfo const fault_info = GetFaultInfo();

    UC_LOG_C(
      "COREFAULT type({}Fault) info({}) flags({:#08x}) address({:#08x}) "
      "registers: PC={:#08x} R0={:#08x} R1={:#08x} R2={:#08x} R3={:#08x} R12={:#08x} LR={:#08x} "
      "xPSR={:#08x} EXC_RETURN={:#08x}",
      fault_info.type,
      fault_info.description,
      fault_info.status_bits,
      fault_info.fault_address,
      stack_ptr[6],
      stack_ptr[0],
      stack_ptr[1],
      stack_ptr[2],
      stack_ptr[3],
      stack_ptr[4],
      stack_ptr[5],
      stack_ptr[7],
      lr_value);
}

};   // namespace Kvasir::Fault::Core
