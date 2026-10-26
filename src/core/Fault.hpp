#pragma once
#include "core_peripherals/SCB.hpp"
#include "cortex_common/FaultSnapshot.hpp"
#include "kvasir/Mpl/Types.hpp"
#include "kvasir/Register/Register.hpp"

#include <cstdint>
#include <optional>
#include <uc_log/uc_log.hpp>

// Kvasir::Fault::Core, next to the SDK's Kvasir::Fault (FaultHandler.hpp): its log module is
// "fault.core", derived from the scope.
namespace Kvasir::Fault::Core {
using ScbR = Kvasir::Peripheral::SCB::Registers<>;

enum class FaultType : std::uint8_t { hard };

// Armv6-M has no fault status registers: every fault is a HardFault. DFSR (and SHCSR) belong
// to the Debug Extension (DDI0419E, SCB table footnote d, md l.8547), "access from the
// processor is IMPLEMENTATION DEFINED" (C1.6, md l.10865), and the Cortex-M0+ TRM lists DFSR
// with the debug registers that "software cannot access" (DDI0484C 7.2, Table 7-6 and the
// Note at its end, md l.1612-1669). So nothing is read from them: what they return is
// unspecified.
enum class FaultDescription : std::uint8_t { escalatedFault };

struct FaultInfo {
    FaultType                    type;
    FaultDescription             description;
    std::optional<std::uint32_t> faultAddress;
    std::uint32_t                statusBits;
};

// The fault registers for the SDK's full fault record (Kvasir::CrashRecord::Full, FaultHandler.hpp) and the
// COREFAULT line (cortex_common/FaultSnapshot.hpp): no analysis, no log, nothing that can fault. Addresses from the
// Armv6-M ARM DDI0419E SCB table (md l.8539).
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

// The COREFAULT line of one fault, from the snapshot the fault handler took: every fault is a HardFault, `flags` is
// ICSR.VECTACTIVE (no second register read). Its own text, not the Armv7-M/Armv8-M line of cortex_common/FaultLog.hpp
// (no `forced`, `{:#08x}`).
inline void log([[maybe_unused]] std::uint32_t const* stackPtr,
                [[maybe_unused]] std::uint32_t        lrValue,
                [[maybe_unused]] Snapshot const&      s) {
    [[maybe_unused]] FaultInfo const faultInfo{
      .type         = FaultType::hard,
      .description  = FaultDescription::escalatedFault,
      .faultAddress = std::nullopt,
      .statusBits   = Kvasir::Register::extract(ScbR::ICSR::vectactive, s.icsr)};

    UC_LOG_C(
      "COREFAULT type({}Fault) info({}) flags({:#08x}) address({:#08x}) "
      "registers: PC={:#08x} R0={:#08x} R1={:#08x} R2={:#08x} R3={:#08x} R12={:#08x} LR={:#08x} "
      "xPSR={:#08x} EXC_RETURN={:#08x}",
      faultInfo.type,
      faultInfo.description,
      faultInfo.statusBits,
      faultInfo.faultAddress,
      stackPtr[6],
      stackPtr[0],
      stackPtr[1],
      stackPtr[2],
      stackPtr[3],
      stackPtr[4],
      stackPtr[5],
      stackPtr[7],
      lrValue);
}

}   // namespace Kvasir::Fault::Core
