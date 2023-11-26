#pragma once
#include <string_view>

namespace Kvasir::Core::Fault {

static constexpr std::pair<std::string_view, std::uint32_t> info() {
    using namespace std::literals;

    return {"HardFault"sv, 0};
}

};   // namespace Kvasir::Core::Fault
