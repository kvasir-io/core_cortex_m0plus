#pragma once
#include "core_peripherals/SCB.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Util/StaticString.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

using Kvasir::Register::apply;
using Kvasir::Register::read;
using Kvasir::Register::write;

namespace Kvasir::Core::Fault {

using SCB_R = Kvasir::Peripheral::SCB::Registers<>;

struct FaultContext {
    std::uint32_t        r0;
    std::uint32_t        r1;
    std::uint32_t        r2;
    std::uint32_t        r3;
    std::uint32_t        r12;
    std::uint32_t        lr;
    std::uint32_t        pc;
    std::uint32_t        xpsr;
    std::uint32_t        exc_return;
    std::uint32_t const* stack_pointer;
};

struct FaultInfo {
    Kvasir::StaticString<64>     type;
    std::string_view             description;
    std::optional<std::uint32_t> fault_address;
    std::uint32_t                status_bits;
};

static FaultInfo GetFaultInfo() {
    // Cortex-M0 has limited fault analysis compared to M4
    // We can only read basic fault status from DFSR and ICSR
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

    std::uint32_t status_bits
      = (external << 4) | (vcatch << 3) | (dwttrap << 2) | (bkpt << 1) | halted;

    if(external) {
        return {"HardFault", "External debug request", std::nullopt, status_bits};
    } else if(vcatch) {
        return {"HardFault", "Vector catch", std::nullopt, status_bits};
    } else if(dwttrap) {
        return {"HardFault", "DWT trap", std::nullopt, status_bits};
    } else if(bkpt) {
        return {"HardFault", "Breakpoint", std::nullopt, status_bits};
    } else if(halted) {
        return {"HardFault", "Halt request", std::nullopt, status_bits};
    } else {
        // Default HardFault - Cortex-M0 escalates most faults to HardFault
        return {"HardFault", "Escalated fault (M0 limited analysis)", std::nullopt, vectactive};
    }
}

static FaultContext CaptureFaultContext(std::uint32_t const* stack_ptr,
                                        std::uint32_t        lr_value) {
    FaultContext ctx{};

    ctx.r0            = stack_ptr[0];
    ctx.r1            = stack_ptr[1];
    ctx.r2            = stack_ptr[2];
    ctx.r3            = stack_ptr[3];
    ctx.r12           = stack_ptr[4];
    ctx.lr            = stack_ptr[5];
    ctx.pc            = stack_ptr[6];
    ctx.xpsr          = stack_ptr[7];
    ctx.exc_return    = lr_value;
    ctx.stack_pointer = stack_ptr;

    return ctx;
}

static constexpr std::pair<std::string_view,
                           std::uint32_t>
info() {
    using namespace std::literals;
    return {"HardFault"sv, 0};
}

static inline void Log(std::uint32_t const* stack_ptr,
                       std::uint32_t        lr_value) {
    [[maybe_unused]] auto const ctx        = Core::Fault::CaptureFaultContext(stack_ptr, lr_value);
    auto                        fault_info = Core::Fault::GetFaultInfo();

    // For Cortex-M0, we don't have fault address registers
    // Use a placeholder or omit the address field
    [[maybe_unused]] auto fault_addr = fault_info.fault_address.value_or(0);

    UC_LOG_C(
      "COREFAULT type({}) info({}) flags({:#08x}) address({:#08x}) "
      "registers: PC={:#08x} R0={:#08x} R1={:#08x} R2={:#08x} R3={:#08x} R12={:#08x} LR={:#08x} "
      "xPSR={:#08x}",
      fault_info.type,
      fault_info.description,
      fault_info.status_bits,
      fault_addr,
      ctx.pc,
      ctx.r0,
      ctx.r1,
      ctx.r2,
      ctx.r3,
      ctx.r12,
      ctx.lr,
      ctx.xpsr);
}

};   // namespace Kvasir::Core::Fault
