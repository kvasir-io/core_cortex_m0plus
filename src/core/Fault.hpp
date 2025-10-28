#pragma once
#include "core_peripherals/SCB.hpp"
#include "kvasir/Register/Register.hpp"

#include <cstdint>
#include <optional>
#include <utility>

using Kvasir::Register::apply;
using Kvasir::Register::read;

namespace Kvasir::Core::Fault {

using SCB_R = Kvasir::Peripheral::SCB::Registers<>;

enum class FaultType : std::uint8_t { Hard };

enum class FaultDescription : std::uint8_t {
    ExternalDebug,
    VectorCatch,
    DWTTrap,
    Breakpoint,
    HaltRequest,
    EscalatedFault
};

struct FaultInfo {
    FaultType                    type;
    FaultDescription             description;
    std::optional<std::uint32_t> fault_address;
    std::uint32_t                status_bits;
};

static constexpr FaultInfo GetFaultInfo() {
    // Cortex-M0 has limited fault analysis - only DFSR and ICSR available
    auto fault_regs = apply(read(SCB_R::DFSR::external),
                            read(SCB_R::DFSR::vcatch),
                            read(SCB_R::DFSR::dwttrap),
                            read(SCB_R::DFSR::bkpt),
                            read(SCB_R::DFSR::halted),
                            read(SCB_R::ICSR::vectactive));

    auto external   = static_cast<std::uint32_t>(get<0>(fault_regs));
    auto vcatch     = static_cast<std::uint32_t>(get<1>(fault_regs));
    auto dwttrap    = static_cast<std::uint32_t>(get<2>(fault_regs));
    auto bkpt       = static_cast<std::uint32_t>(get<3>(fault_regs));
    auto halted     = static_cast<std::uint32_t>(get<4>(fault_regs));
    auto vectactive = static_cast<std::uint32_t>(get<5>(fault_regs));

    std::uint32_t const status_bits
      = (external << 4) | (vcatch << 3) | (dwttrap << 2) | (bkpt << 1) | halted;

    FaultInfo info{};
    info.type = FaultType::Hard;

    if(external) {
        info.description = FaultDescription::ExternalDebug;
        info.status_bits = status_bits;
        return info;
    }
    if(vcatch) {
        info.description = FaultDescription::VectorCatch;
        info.status_bits = status_bits;
        return info;
    }
    if(dwttrap) {
        info.description = FaultDescription::DWTTrap;
        info.status_bits = status_bits;
        return info;
    }
    if(bkpt) {
        info.description = FaultDescription::Breakpoint;
        info.status_bits = status_bits;
        return info;
    }
    if(halted) {
        info.description = FaultDescription::HaltRequest;
        info.status_bits = status_bits;
        return info;
    }

    // Default: Cortex-M0 escalates most faults to HardFault
    info.description = FaultDescription::EscalatedFault;
    info.status_bits = vectactive;
    return info;
}

using EarlyInitList = decltype(MPL::list());

static inline void Log([[maybe_unused]] std::uint32_t const* stack_ptr,
                       [[maybe_unused]] std::uint32_t        lr_value) {
    [[maybe_unused]] FaultInfo const fault_info = Core::Fault::GetFaultInfo();

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

};   // namespace Kvasir::Core::Fault
