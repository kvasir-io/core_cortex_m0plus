#pragma once

#include "SystemControl.hpp"
#include "core_peripherals/SYSTICK.hpp"
#include "kvasir/Atomic/Atomic.hpp"
#include "kvasir/Common/Interrupt.hpp"
#include "kvasir/Register/Register.hpp"
#include "kvasir/Register/Utility.hpp"

#include <atomic>
#include <chrono>

namespace Kvasir {
namespace Systick {
    using SystickRegs                       = Kvasir::Peripheral::SYSTICK::Registers<>;
    static constexpr auto useExternalClock  = SystickRegs::CSR::CLKSOURCEValC::external;
    static constexpr auto useProcessorClock = SystickRegs::CSR::CLKSOURCEValC::processor;
}   // namespace Systick

namespace Nvic {
    using SystickRegs = Kvasir::Peripheral::SYSTICK::Registers<>;
    template<>
    struct MakeAction<Action::Enable, Index<Kvasir::Interrupt::systick.index()>>
      : decltype(write(SystickRegs::CSR::TICKINTValC::interrupt_enabled)) {
        static_assert(
          Detail::interuptIndexValid(
            Interrupt::systick.index(),
            std::begin(InterruptOffsetTraits<void>::noEnable),
            std::end(InterruptOffsetTraits<void>::noEnable)),
          "Unable to enable this interrupt, index is out of range");
    };

    template<>
    struct MakeAction<Action::Disable, Index<Kvasir::Interrupt::systick.index()>>
      : decltype(write(SystickRegs::CSR::TICKINTValC::interrupt_disabled)) {
        static_assert(
          Detail::interuptIndexValid(
            Interrupt::systick.index(),
            std::begin(InterruptOffsetTraits<void>::noDisable),
            std::end(InterruptOffsetTraits<void>::noDisable)),
          "Unable to disable this interrupt, index is out of range");
    };
    template<>
    struct MakeAction<Action::Read, Index<Kvasir::Interrupt::systick.index()>>
      : decltype(read(SystickRegs::CSR::tickint)) {};
}   // namespace Nvic

namespace Systick {
    template<typename TConfig>
    struct SystickClockBase {
    private:
        // needed config
        // clockSpeed
        // clockBase
        // minOverrunTime
        using Config                              = TConfig;
        static constexpr std::uint64_t ClockSpeed = Config::clockSpeed;
        using Regs                                = Kvasir::Peripheral::SYSTICK::Registers<>;

    public:
        // chrono interface
        using duration = std::chrono::
          duration<std::int64_t, std::ratio<1, Config::clockSpeed>>;   // std::chrono::nanoseconds;
        using rep        = typename duration::rep;
        using priode     = typename duration::period;
        using time_point = std::chrono::time_point<SystickClockBase, duration>;

        static constexpr bool is_steady = true;

        template<typename Rep, typename Period>
        friend constexpr std::
          enable_if_t<!std::is_same_v<std::chrono::duration<Rep, Period>, duration>, time_point>
          operator+(time_point t, std::chrono::duration<Rep, Period> d) {
            return t + std::chrono::duration_cast<duration>(d);
        }

        template<typename Rep, typename Period>
        friend constexpr std::
          enable_if_t<!std::is_same_v<std::chrono::duration<Rep, Period>, duration>, time_point>
          operator-(time_point t, std::chrono::duration<Rep, Period> d) {
            return t - std::chrono::duration_cast<duration>(d);
        }

    private:
        static_assert(
          Config::clockSpeed < std::numeric_limits<std::uint64_t>::max() / 100'000'000ULL,
          "ClockSpeed to high");

        template<std::uint64_t OverRunValue, typename = void>
        struct GetOverrunType {
            using type = std::uint64_t;
        };
        template<std::uint64_t OverRunValue>
        struct GetOverrunType<
          OverRunValue,
          std::enable_if_t<(OverRunValue <= std::numeric_limits<std::uint32_t>::max())>> {
            using type = std::uint32_t;
        };
        template<std::uint64_t OverRunValue>
        using GetOverrunTypeT = typename GetOverrunType<OverRunValue, void>::type;

        static constexpr std::uint32_t calcReloadValue(std::uint64_t clockSpeed) {
            (void)clockSpeed;
            return (1U << 24U) - 1U;
        }

        static constexpr std::uint64_t
        calcOverRunValue(std::uint64_t clockSpeed, std::chrono::nanoseconds overrunTime) {
            std::uint64_t NanoSecPerOverrun
              = ((std::uint64_t(calcReloadValue(clockSpeed)) + 1ULL) * 1'000'000'000ULL)
              / clockSpeed;
            return std::uint64_t(overrunTime.count()) / NanoSecPerOverrun;
        }

        using overrunT = GetOverrunTypeT<calcOverRunValue(ClockSpeed, Config::minOverrunTime)>;
        static inline std::atomic<overrunT> overruns{};

        static void onIsr() {
            overrunT old = overruns.load(std::memory_order_relaxed);
            ++old;
            overruns.store(old, std::memory_order_relaxed);
        }

        static void delay_ticks(std::uint32_t ticksToWait) {
            std::uint32_t const countStart    = apply(read(Regs::CVR::current));
            overrunT const      overrunsStart = overruns.load(std::memory_order_relaxed);
            while(true) {
                std::uint32_t const countNow    = apply(read(Regs::CVR::current));
                overrunT const      overrunsNow = overruns.load(std::memory_order_relaxed);
                auto const          countsRaw   = std::int32_t(countStart - countNow);
                std::uint32_t const countsElapsed
                  = countsRaw >= 0 ? std::uint32_t(countsRaw)
                                   : countStart + (calcReloadValue(ClockSpeed) - countNow);
                if(
                  countsElapsed >= ticksToWait || overrunsNow - overrunsStart >= 2
                  || (countNow < countStart && overrunsNow - overrunsStart == 1))
                {
                    break;
                }
            }
        }

    public:
        [[clang::no_sanitize("unsigned-integer-overflow")]] static time_point now() {
            static constexpr auto reloadValue = calcReloadValue(ClockSpeed);

            std::uint32_t currentCount{};
            overrunT      localOverruns;

            while(true) {
                currentCount  = apply(read(Regs::CVR::current));
                localOverruns = overruns.load(std::memory_order_relaxed);
                if(!fieldEquals(Regs::CSR::COUNTFLAGValC::timer_has_counted_to_0)) {
                    break;
                }
            }
            auto const cnd  = duration{reloadValue - currentCount};
            auto const ovd  = duration{localOverruns * (reloadValue + 1)};
            auto const time = time_point{cnd + ovd};
            return time;
        }

        template<typename Duration, typename duration::rep value>
        static void delay() {
            static constexpr auto reloadValue = calcReloadValue(ClockSpeed);
            static constexpr auto ticksToWait
              = std::chrono::duration_cast<duration>(Duration{value}).count();
            if constexpr(ticksToWait >= reloadValue) {
                static constexpr auto count
                  = std::uint32_t(double(ticksToWait) / double(reloadValue));
                static constexpr auto last
                  = std::uint32_t(double(ticksToWait) - double(count) * double(reloadValue));
                std::uint32_t c = count;
                while(c != 0) {
                    delay_ticks(reloadValue);
                    --c;
                }
                delay_ticks(last);

            } else {
                delay_ticks(ticksToWait);
            }
        }

        // kvasir init
        static constexpr auto initStepPeripheryConfig = list(
          write(Config::clockBase),
          write(Regs::RVR::reload, Register::value<calcReloadValue(ClockSpeed)>()),
          write(Regs::CSR::ENABLEValC::counter_is_disabled),
          makeDisable(Interrupt::systick),
          write(Regs::CVR::current, Register::value<0>()));

        static constexpr auto initStepInterruptConfig = list(
          action(Nvic::Action::setPriority0, Interrupt::systick),
          action(Nvic::Action::clearPending, Interrupt::systick));

        static constexpr auto initStepPeripheryEnable = list(
          write(Regs::CSR::ENABLEValC::counter_is_operating),
          makeEnable(Interrupt::systick));

        static constexpr Nvic::
          Isr<std::addressof(onIsr), std::decay_t<decltype(Interrupt::systick)>>
            isr{};
    };
}   // namespace Systick
}   // namespace Kvasir
