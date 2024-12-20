#ifndef FLUIDFINALVER_TYPEHOLDER_H
#define FLUIDFINALVER_TYPEHOLDER_H

#include <cinttypes>
#include <type_traits>
#include <array>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "originalFunctions.hpp"
#include "types.hpp"

namespace FluidPhysics {
    template<size_t n>
    constexpr auto select_type() {
        if constexpr (n == 1) {
            return float{};
        } else if constexpr (n == 2) {
            return double{};
        } else if constexpr (n > 100000) {
            return Fixed<n / 100000, n % 100000, true>{};
        } else if constexpr (n > 1000) {
            return Fixed<n / 1000, n % 1000>{};
        } else {
            return std::type_identity<void>{};
        }
    }

    template<size_t n>
    struct TypeHolder {
        using type = decltype(select_type<n>());
    };

    template<int n>
    using getType = TypeHolder<n>::type;

}

#endif //FLUIDFINALVER_TYPEHOLDER_H
