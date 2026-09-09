#pragma once

// The Armv6-M barrier and event instructions, spelled once. Every one of them clobbers
// "memory" so the compiler cannot move an ordinary load or store across it, which is the
// whole point of a barrier: the instruction orders the hardware, the clobber orders the
// compiler.
namespace Kvasir::Core {

// Data memory barrier: every explicit memory access before it completes before any after it
// is observed. This is the ordering primitive for data shared between cores or with a DMA.
[[gnu::always_inline]] inline void dmb() { asm volatile("dmb" ::: "memory"); }

// Data synchronisation barrier: like dmb, and additionally waits for the accesses to finish.
[[gnu::always_inline]] inline void dsb() { asm volatile("dsb" ::: "memory"); }

// Instruction synchronisation barrier: flush the pipeline so instructions after it are
// fetched with the effect of whatever system register write preceded it.
[[gnu::always_inline]] inline void isb() { asm volatile("isb" ::: "memory"); }

// Send event: wakes every core sleeping in wfe(). Cheap, safe to issue spuriously.
[[gnu::always_inline]] inline void sev() { asm volatile("sev" ::: "memory"); }

// Wait for event: sleep until the event register is set, then clear it and return. Sources:
// another core's sev(), an interrupt (a pending one returns at once) and an exception return
// on this core. Always re-check the condition after waking: the event register can be set
// for any of these reasons.
[[gnu::always_inline]] inline void wfe() { asm volatile("wfe" ::: "memory"); }

// Wait for interrupt.
[[gnu::always_inline]] inline void wfi() { asm volatile("wfi" ::: "memory"); }

}   // namespace Kvasir::Core
