#pragma once
#include "kvasir/Common/Interrupt.hpp"

namespace Kvasir {
template<int I>
using Type = ::Kvasir::Nvic::Index<I>;

struct CoreInterrupts {
    static constexpr Type<-14> nonMaskableInt{};
    static constexpr Type<-13> hardFault{};
    static constexpr Type<-5>  sVCall{};
    static constexpr Type<-2>  pendSV{};
    static constexpr Type<-1>  systick{};
};
}   // namespace Kvasir
