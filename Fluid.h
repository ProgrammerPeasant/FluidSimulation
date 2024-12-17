#ifndef FLUIDFINALVER_FLUID_H
#define FLUIDFINALVER_FLUID_H

#include <cstdint>
#include <iostream>
#include <type_traits>
#include <utility>
#include <random>
#include <array>
#include <cassert>
#include <algorithm>

#include "specialArr.h"

namespace FluidPhysics {

    template<int N, bool isFast>
    struct realType {
        using type = std::conditional_t<
                (N <= 8),
                std::conditional_t<isFast, int_fast8_t, int8_t>,
                std::conditional_t<
                        (N <= 16),
                        std::conditional_t<isFast, int_fast16_t, int16_t>,
                        std::conditional_t<
                                (N <= 32),
                                std::conditional_t<isFast, int_fast32_t, int32_t>,
                                std::conditional_t<
                                        (N <= 64),
                                        std::conditional_t<isFast, int_fast64_t, int64_t>,
                                        void
                                >
                        >
                >
        >;
    };

    template<int N, bool isFast>
    using real_t = typename realType<N, isFast>::type;

    template<int N, int K, bool fast = false>
    struct Fixed {
        static_assert(N > K, "N must be greater than K");
        using value_t = real_t<N, fast>;

        static constexpr value_t scale = (static_cast<value_t>(1) << K);
        value_t v = 0;

        static constexpr int k = K;

        constexpr Fixed() noexcept = default;

        static constexpr Fixed from_raw(value_t raw) noexcept {
            Fixed f;
            f.v = raw;
            return f;
        }

        template<int otherN, int otherK, bool otherIsFast>
        constexpr explicit Fixed(const Fixed<otherN, otherK, otherIsFast>& other) noexcept {
            if constexpr (otherK > K) {
                v = other.v >> (otherK - K);
            } else {
                v = other.v << (K - otherK);
            }
        }

        constexpr explicit Fixed(int64_t val) noexcept : v(static_cast<value_t>(val) << K) {}
        constexpr explicit Fixed(float f) noexcept : v(static_cast<value_t>(f * scale)) {}
        constexpr explicit Fixed(double f) noexcept : v(static_cast<value_t>(f * scale)) {}

        explicit constexpr operator float() const noexcept { return static_cast<float>(v) / scale; }
        explicit constexpr operator double() const noexcept { return static_cast<double>(v) / scale; }

        constexpr Fixed operator+(const Fixed& other) const noexcept {
            return from_raw(v + other.v);
        }

        constexpr Fixed operator-(const Fixed& other) const noexcept {
            return from_raw(v - other.v);
        }

        constexpr Fixed operator*(const Fixed& other) const noexcept {
            return from_raw(static_cast<int64_t>(v) * other.v >> K);
        }

        constexpr Fixed operator/(const Fixed& other) const noexcept {
            return from_raw((static_cast<int64_t>(v) << K) / other.v);
        }

        constexpr Fixed& operator+=(const Fixed& other) noexcept {
            v += other.v;
            return *this;
        }

        constexpr Fixed& operator-=(const Fixed& other) noexcept {
            v -= other.v;
            return *this;
        }

        constexpr Fixed& operator*=(const Fixed& other) noexcept {
            v = static_cast<value_t>((static_cast<int64_t>(v) * other.v) >> K);
            return *this;
        }

        constexpr Fixed& operator/=(const Fixed& other) noexcept {
            v = static_cast<value_t>((static_cast<int64_t>(v) << K) / other.v);
            return *this;
        }

        constexpr Fixed operator-() const noexcept {
            return from_raw(-v);
        }

        constexpr Fixed abs() const noexcept {
            return from_raw(v < 0 ? -v : v);
        }

        // Comparison operators
        constexpr bool operator<(const Fixed& other) const noexcept { return v < other.v; }
        constexpr bool operator<=(const Fixed& other) const noexcept { return v <= other.v; }
        constexpr bool operator>(const Fixed& other) const noexcept { return v > other.v; }
        constexpr bool operator>=(const Fixed& other) const noexcept { return v >= other.v; }
        constexpr bool operator==(const Fixed& other) const noexcept { return v == other.v; }
        constexpr bool operator!=(const Fixed& other) const noexcept { return v != other.v; }

        // Output stream operator
        friend std::ostream& operator<<(std::ostream& out, const Fixed& x) {
            return out << (static_cast<double>(x.v) / scale);
        }
    };

    constexpr std::array<std::pair<int, int>, 4> deltas{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

    template<typename T>
    T g() { return 0.1; }


    std::mt19937 rnd(1337);

    template<typename T>
    T random01() {
        if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
            return T(rnd()) / T(std::mt19937::max());
        } else {
            return T::from_raw((rnd() & ((1ll << T::k) - 1ll)));
        }
    }

    template<typename T, int N, int M>
    struct VectorField {
    private:
        static const int D = deltas.size();
        Array<std::array<T, D>, N, M> values_;
        std::array<std::pair<int, int>, D> deltas_;

    public:
        T& add(int x, int y, int dx, int dy, T dv) {
            return get(x, y, dx, dy) += dv;
        }

        T& get(int x, int y, int dx, int dy) {
            auto it = std::find_if(deltas_.begin(), deltas_.end(),
                                   [dx, dy](const std::pair<int, int>& p) {
                                       return p.first == dx && p.second == dy;
                                   });
            //auto it = std::find(deltas_.begin(), deltas_.end(), std::pair(dx, dy));

            if (it == deltas_.end()) {
                throw std::invalid_argument("Delta pair not found");
            }

            std::size_t index = std::distance(deltas_.begin(), it);
            return values_[x][y][index];
        }

        explicit VectorField(const std::array<std::pair<int, int>, D>& temp_deltas)
                : deltas_(temp_deltas) {}
    };

} // namespace FluidPhysics

#endif //FLUIDFINALVER_FLUID_H
