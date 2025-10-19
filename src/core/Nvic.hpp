#pragma once

#include "chip/Interrupt.hpp"
#include "core_peripherals/NVIC.hpp"
#include "kvasir/Common/Interrupt.hpp"
#include "kvasir/Mpl/Utility.hpp"
#include "kvasir/Register/Register.hpp"

namespace Kvasir { namespace Nvic {
    namespace Detail {
        using namespace Register;

        template<unsigned A, int BitPos>
        using BlindSet
          = Register::Action<WOBitLocT<Register::Address<A, maskFromRange(31, 0)>, BitPos>,
                             WriteLiteralAction<(1U << unsigned(BitPos))>>;

        template<unsigned A, int BitPos>
        using Read = Register::Action<ROBitLocT<Register::Address<A, maskFromRange(31, 0)>, BitPos>,
                                      ReadAction>;

        template<unsigned A, unsigned Offset, unsigned Priority>
        using PrioritySet
          = Register::Action<RWFieldLocT<Register::Address<A>, (Offset * 8) + 7, Offset * 8>,
                             WriteLiteralAction<((Priority << 6U) << Offset * 8)>>;
        static constexpr unsigned baseAddress = Kvasir::Peripheral::NVIC::Registers<>::baseAddr;

        template<typename InputIt,
                 typename T>
        constexpr InputIt find(InputIt  first,
                               InputIt  last,
                               T const& value) {
            for(; first != last; ++first) {
                if(*first == value) {
                    return first;
                }
            }
            return last;
        }

        template<typename InputIt>
        constexpr bool interuptIndexValid(int     I,
                                          InputIt f,
                                          InputIt l) {
            constexpr auto bd = std::begin(InterruptOffsetTraits<void>::disabled);
            constexpr auto ed = std::end(InterruptOffsetTraits<void>::disabled);
            return I < InterruptOffsetTraits<void>::end && I >= InterruptOffsetTraits<void>::begin
                && Detail::find(bd, ed, I) == ed && Detail::find(f, l, I) == l;
        }

    }   // namespace Detail

    template<int I>
        requires(I >= 0)
    struct MakeAction<Action::Enable, Index<I>> : Detail::BlindSet<Detail::baseAddress + 0x000, I> {
        static_assert(Detail::interuptIndexValid(I,
                                                 std::begin(InterruptOffsetTraits<void>::noEnable),
                                                 std::end(InterruptOffsetTraits<void>::noEnable)),
                      "Unable to enable this interrupt, index is out of range");
    };

    template<int I>
        requires(I >= 0)
    struct MakeAction<Action::Read, Index<I>> : Detail::Read<Detail::baseAddress + 0x000, I> {
        static_assert(Detail::interuptIndexValid(I,
                                                 std::begin(InterruptOffsetTraits<void>::noEnable),
                                                 std::end(InterruptOffsetTraits<void>::noEnable)),
                      "Unable to read this interrupt, index is out of range");
    };

    template<int I>
        requires(I >= 0)
    struct MakeAction<Action::Disable, Index<I>>
      : Detail::BlindSet<Detail::baseAddress + 0x080, I> {
        static_assert(Detail::interuptIndexValid(I,
                                                 std::begin(InterruptOffsetTraits<void>::noDisable),
                                                 std::end(InterruptOffsetTraits<void>::noDisable)),
                      "Unable to disable this interrupt, index is out of range");
    };

    template<int I>
        requires(I >= 0)
    struct MakeAction<Action::SetPending, Index<I>>
      : Detail::BlindSet<Detail::baseAddress + 0x100, I> {
        static_assert(
          Detail::interuptIndexValid(I,
                                     std::begin(InterruptOffsetTraits<void>::noSetPending),
                                     std::end(InterruptOffsetTraits<void>::noSetPending)),
          "Unable to set pending on this interrupt, index is out of range");
    };

    template<int I>
        requires(I >= 0)
    struct MakeAction<Action::ClearPending, Index<I>>
      : Detail::BlindSet<Detail::baseAddress + 0x180, I> {
        static_assert(
          Detail::interuptIndexValid(I,
                                     std::begin(InterruptOffsetTraits<void>::noClearPending),
                                     std::end(InterruptOffsetTraits<void>::noClearPending)),
          "Unable to clear pending on this interrupt, index is out of range");
    };

    template<int Priority, int I>
    struct PriorityDisambiguator
      : Detail::PrioritySet<Detail::baseAddress + 0x300 + (I / 4) * 4, I % 4, Priority> {};

    template<int Priority, int I>
        requires(I >= 0)
    struct MakeAction<Action::SetPriority<Priority>, Index<I>>
      : PriorityDisambiguator<Priority, I> {
        static_assert(3 >= Priority,
                      "priority on cortex_m0plus cant be only be 0-3");
        static_assert(
          Detail::interuptIndexValid(I,
                                     std::begin(InterruptOffsetTraits<void>::noSetPriority),
                                     std::end(InterruptOffsetTraits<void>::noSetPriority)),
          "Unable to set priority on this interrupt, index is out of range");
    };
}}   // namespace Kvasir::Nvic
