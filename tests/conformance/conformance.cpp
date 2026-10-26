// The core contract of core_cortex_m0plus as it is today (Kvasir_SDK tests/conformance/CoreContract.hpp), compiled with
// the paired chip: every row the core has is REQUIRE, every row it lacks is GAP - a GAP that the core fills fails here
// and moves to REQUIRE.

// the config header first: the chip below needs its chip variant, log backend and log clock (uc_log.hpp would read it
// too, but the chip headers before it need the same)
#include "kvasir_config.hpp"

// then the paired chip and this core
#include "chip/chip.hpp"
#include "core/Fault.hpp"
#include "core/StartUp.hpp"
#include "core/core.hpp"
// the core's debug query (core.hpp brings it and core/Features.hpp)
#include "cortex_common/Debug.hpp"

// last: the contract rows see only what is declared before them
#include "CoreContract.hpp"

#include <concepts>
#include <cstdint>

// the reset path's hooks and the fault handler's core half
KVASIR_CORE_CONFORMANCE_REQUIRE(
  "Startup::Core::startup() / cleanCore()",
  requires {
      Kvasir::Startup::Core::startup();
      Kvasir::Startup::Core::cleanCore();
  });
KVASIR_CORE_CONFORMANCE_REQUIRE(
  "Fault::Core::snapshot() / clearFaultStatus()",
  requires {
      Kvasir::Fault::Core::snapshot();
      Kvasir::Fault::Core::clearFaultStatus();
  });
// the fault handler's other two calls (Runtime/FaultHandler.hpp): the COREFAULT line from the snapshot it took, and
// the Startup actions that enable the core's fault exceptions
KVASIR_CORE_CONFORMANCE_REQUIRE(
  "Fault::Core::Log(frame, excReturn, Snapshot const&) / EarlyInitList",
  requires(std::uint32_t const*                 frame,
           std::uint32_t                        excReturn,
           Kvasir::Fault::Core::Snapshot const& s) {
      { Kvasir::Fault::Core::Log(frame, excReturn, s) } -> std::same_as<void>;
      typename Kvasir::Fault::Core::EarlyInitList;
  });
// NVIC, SysTick, the reset request
KVASIR_CORE_CONFORMANCE_REQUIRE(
  "Nvic::makeEnable / makeDisable / makeClearPending",
  requires {
      Kvasir::Nvic::makeEnable(Kvasir::Interrupt::systick);
      Kvasir::Nvic::makeDisable(Kvasir::Interrupt::systick);
      Kvasir::Nvic::makeClearPending(Kvasir::Interrupt::systick);
  });
KVASIR_CORE_CONFORMANCE_REQUIRE(
  "Systick::SystickClockBase",
  requires { Conformance::SystickClock::now(); });
KVASIR_CORE_CONFORMANCE_REQUIRE(
  "SystemControl::SystemReset",
  requires { Kvasir::SystemControl::SystemReset{}; });
// known gaps (each moves to REQUIRE when the core gets it)
// the M0+ cannot read DHCSR itself (its cortex_common/Debug.hpp has haltingDebug() only, `unknown`)
KVASIR_CORE_CONFORMANCE_GAP("Core::Debug::isDebuggerConnected()",
                            Kvasir::Conformance::Row::coreDebugIsDebuggerConnected);
// "is halting debug on": off / on / unknown on every core (cortex_common/Debug.hpp)
KVASIR_CORE_CONFORMANCE_REQUIRE("Core::haltingDebug()",
                                Kvasir::Conformance::Row::coreHaltingDebug);
static_assert(Kvasir::Core::haltingDebug() == Kvasir::Core::HaltingDebug::unknown);
// core/Features.hpp (from core.hpp) asserts every KVASIR_CORE_* against Port/Capabilities.hpp: one source
KVASIR_CORE_CONFORMANCE_REQUIRE("Startup::Core::SecondaryCoreTraits",
                                Kvasir::Conformance::Row::coreSecondaryCoreTraits);
static_assert(Kvasir::Startup::Core::SecondaryCoreTraits::cpacrEnable == 0);
