#ifndef FLUIDFINALVER_SPECIALARR_H
#define FLUIDFINALVER_SPECIALARR_H

#include <vector>
#include <array>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace FluidPhysics {

    template<typename T, int Width, int Height>
    struct Array {
        std::array<std::array<T, Height>, Width> arr{};
        static constexpr int gridWidth = Width;
        static constexpr int gridHeight = Height;

        Array() = default;

        void init(int, int) {
            static_assert(Width != -1 && Height != -1, "init() can only be called on dynamic Arrays");
        }

        T* operator[](int n) noexcept {
            if (n < 0 || n >= gridWidth) {
                throw std::out_of_range("Index out of range");
            }
            return arr[n].data();
        }

        const T* operator[](int n) const noexcept {
            if (n < 0 || n >= gridWidth) {
                throw std::out_of_range("Index out of range");
            }
            return arr[n].data();
        }

        Array& operator=(const Array& other) = default;

    };

    template<typename T>
    struct Array<T, -1, -1> {
        std::vector<std::vector<T>> arr{};
        int gridWidth = 0;
        int gridHeight = 0;

        Array() = default;

        void init(int n, int m) {
            if (n <= 0 || m <= 0) {
                throw std::invalid_argument("Dimensions must be positive");
            }
            gridWidth = n;
            gridHeight = m;
            arr.assign(gridWidth, std::vector<T>(gridHeight, T{}));
        }

        std::vector<T>& operator[](int n) {
            if (n < 0 || n >= gridWidth) {
                throw std::out_of_range("Index out of range");
            }
            return arr[n];
        }

        const std::vector<T>& operator[](int n) const {
            if (n < 0 || n >= gridWidth) {
                throw std::out_of_range("Index out of range");
            }
            return arr[n];
        }

        Array& operator=(const Array& other) = default;

    };

}

#endif //FLUIDFINALVER_SPECIALARR_H
