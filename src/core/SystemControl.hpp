#pragma once

#include "chip/Interrupt.hpp"
#include "core/Nvic.hpp"
#include "core_peripherals/SCB.hpp"
#include "kvasir/Common/Interrupt.hpp"
#include "kvasir/Register/Utility.hpp"

#include <cstdint>

namespace Kvasir {
namespace SystemControl {
    using SystemReset = decltype(Kvasir::Peripheral::SCB::Registers<>::AIRCR::overrideDefaults(
      write(Kvasir::Peripheral::SCB::Registers<>::AIRCR::VECTKEYValC::request_reset),
      write(Kvasir::Peripheral::SCB::Registers<>::AIRCR::SYSRESETREQValC::request_reset)));
}
namespace Nvic {

    namespace detail {
        using SCU_R = Kvasir::Peripheral::SCB::Registers<>;
    }
    // Systick
    template<>
    struct MakeAction<Action::SetPending, Index<Interrupt::systick.index()>>
      : decltype(write(detail::SCU_R::ICSR::PENDSTSETValC::set_pending)) {
        static_assert(
          Detail::interuptIndexValid(
            Interrupt::systick.index(),
            std::begin(InterruptOffsetTraits<void>::noSetPending),
            std::end(InterruptOffsetTraits<void>::noSetPending)),
          "Unable to set pending on this interrupt, index is out of range");
    };

    template<>
    struct MakeAction<Action::ClearPending, Index<Interrupt::systick.index()>>
      : decltype(write(detail::SCU_R::ICSR::PENDSTCLRValC::clear)) {
        static_assert(
          Detail::interuptIndexValid(
            Interrupt::systick.index(),
            std::begin(InterruptOffsetTraits<void>::noClearPending),
            std::end(InterruptOffsetTraits<void>::noClearPending)),
          "Unable to clear pending on this interrupt, index is out of range");
    };

    template<int Priority>
    struct PriorityDisambiguator<Priority, Interrupt::systick.index()>
      : decltype(write(
          detail::SCU_R::SHPR3::pri_15,
          Register::value<(unsigned(Priority) << 6U)>())) {};

    // PendSv
    template<>
    struct MakeAction<Action::SetPending, Index<Interrupt::pendSV.index()>>
      : decltype(write(detail::SCU_R::ICSR::PENDSVSETValC::set_pending)) {
        static_assert(
          Detail::interuptIndexValid(
            Interrupt::pendSV.index(),
            std::begin(InterruptOffsetTraits<void>::noSetPending),
            std::end(InterruptOffsetTraits<void>::noSetPending)),
          "Unable to set pending on this interrupt, index is out of range");
    };

    template<>
    struct MakeAction<Action::ClearPending, Index<Interrupt::pendSV.index()>>
      : decltype(write(detail::SCU_R::ICSR::PENDSVCLRValC::clear)) {
        static_assert(
          Detail::interuptIndexValid(
            Interrupt::pendSV.index(),
            std::begin(InterruptOffsetTraits<void>::noClearPending),
            std::end(InterruptOffsetTraits<void>::noClearPending)),
          "Unable to clear pending on this interrupt, index is out of range");
    };

    template<int Priority>
    struct PriorityDisambiguator<Priority, Interrupt::pendSV.index()>
      : decltype(write(
          detail::SCU_R::SHPR3::pri_14,
          Register::value<(unsigned(Priority) << 6U)>())) {};

    // nonMaskableInt
    template<>
    struct MakeAction<Action::SetPending, Index<Interrupt::nonMaskableInt.index()>>
      : decltype(write(detail::SCU_R::ICSR::NMIPENDSETValC::set_pending)) {
        static_assert(
          Detail::interuptIndexValid(
            Interrupt::nonMaskableInt.index(),
            std::begin(InterruptOffsetTraits<void>::noSetPending),
            std::end(InterruptOffsetTraits<void>::noSetPending)),
          "Unable to set pending on this interrupt, index is out of range");
    };

    // SVCall
    template<int Priority>
    struct PriorityDisambiguator<Priority, Interrupt::sVCall.index()>
      : decltype(write(
          detail::SCU_R::SHPR2::pri_11,
          Register::value<(unsigned(Priority) << 6U)>())) {};
}   // namespace Nvic
}   // namespace Kvasir
