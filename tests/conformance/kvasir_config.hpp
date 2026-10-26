#pragma once
// The image config header as a test fixture, for the paired chip (an RP2040): what a firmware's kvasir_config.hpp holds
// - the chip variant, the log backend and a complete log clock. uc_log/uc_log.hpp reads it first (UC_LOG_CONFIG_HEADER,
// set by kvasir_conformance_project), and every conformance TU and every header-alone TU starts with it. Like every
// config header it never includes uc_log/uc_log.hpp. No board: nothing here names pins or a probe.

// the chip variant first (the package defines it): the chip headers (core/Systick.hpp below too) read it
#include <chip/rp_common/PinConfig.hpp>
// Kvasir::Chip::part comes from the chip package (chip_rp2040: KVASIR_CHIP_RP2040)

#include <chip/rp_common/WaitBounds.hpp>   // Register::CpuHzCeiling<Tag::User>: the loss policy's CPU ceiling
#include <uc_log/AlwaysAttached.hpp>
#include <uc_log/LogClock.hpp>
#include <uc_log/RttLog.hpp>

namespace uc_log {
template<>
struct ComBackend<uc_log::Tag::User>
  : public uc_log::RttLog<
      uc_log::AlwaysAttached,
      uc_log::Rings<1024>,
      uc_log::IsrPolicy::MaskedRecord,
      uc_log::LossPolicy::Counted<Kvasir::Register::CpuHzCeiling<Kvasir::Tag::User>::value>> {};
}   // namespace uc_log

#include <core/Systick.hpp>

namespace Conformance {
inline constexpr auto clockSpeedValue = 125'000'000;

struct SystickClockConfig {
    static constexpr auto clockBase      = Kvasir::Systick::useProcessorClock;
    static constexpr auto clockSpeed     = clockSpeedValue;
    static constexpr auto minOverrunTime = std::chrono::years(15);
};

using SystickClock = Kvasir::Systick::SystickClockBase<SystickClockConfig>;
}   // namespace Conformance

namespace uc_log {
template<>
struct LogClock<uc_log::Tag::User> : public Conformance::SystickClock {};
}   // namespace uc_log
