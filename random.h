#ifndef FLUIDFINALVER_RANDOM_H
#define FLUIDFINALVER_RANDOM_H

#include <random>

namespace FluidPhysics {
    std::mt19937 rnd(1337);

    template<typename T>
    T random01() {
        if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
            return T(rnd()) / T(std::mt19937::max());
        } else {
            return T::from_raw((rnd() & ((1ll << T::k) - 1ll)));
        }
    }
}

#endif //FLUIDFINALVER_RANDOM_H
