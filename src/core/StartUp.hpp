#pragma once
#include "core_peripherals/SCB.hpp"
#include "core_peripherals/SYSTICK.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/StartUp/LinkerSymbols.hpp"

#include <cstdint>

namespace Kvasir::Startup::Core {

static void startup() {}

// What a warm reset would have done and the SDK's naked reset entry does not (kvasir/StartUp/StartUp.hpp,
// KVASIR_RESET_ENTRY_ASM), for an image not entered from a reset: SysTick stopped, PendSV/SysTick no longer pending,
// VTOR on this image's table. Armv6-M has no FAULTMASK, fault status registers or FPU. Before .data and .bss: the
// register layer only, no static.
[[gnu::always_inline]] inline void cleanCore() {
    using SCB_R     = Kvasir::Peripheral::SCB::Registers<>;
    using SYSTICK_R = Kvasir::Peripheral::SYSTICK::Registers<>;
    using Kvasir::Register::apply;
    using Kvasir::Register::clear;
    using Kvasir::Register::set;
    using Kvasir::Register::write;
    // SYST_CSR (Armv6-M ARM DDI0419E B3.3.3): ENABLE and TICKINT off; CLKSOURCE kept, its reset value is
    // IMPLEMENTATION DEFINED (Table B3-13 "0x00000000 or 0x00000004") and SystickClock writes it anyway
    apply(clear(SYSTICK_R::CSR::enable), clear(SYSTICK_R::CSR::tickint));
    // ICSR (B3.2.4, Table B3-6): PENDSVCLR and PENDSTCLR written over the register's defaults, one store
    // and no read; the SVD leaves the RO status fields read-write, so a plain set() would read first.
    apply(SCB_R::ICSR::overrideDefaults(set(SCB_R::ICSR::pendsvclr), set(SCB_R::ICSR::pendstclr)));
    // VTOR (B3.2.5: RAZ/WI where not implemented)
    apply(
      write(SCB_R::VTOR::FULLREGISTER,
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&_LINKER_vectors_start_))));
    asm volatile("dsb\n isb" ::: "memory");
}

}   // namespace Kvasir::Startup::Core
