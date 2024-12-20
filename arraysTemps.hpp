#ifndef FLUIDFINALVER_ARRAYSTEMPS_H
#define FLUIDFINALVER_ARRAYSTEMPS_H

#include <array>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <cassert>

#include "specialArr.hpp"

namespace FluidPhysics {

    constexpr std::array<std::pair<int, int>, 4> deltas{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

    template<typename T>
    T g() { return 0.1; }

    template<typename T, int gridWidth, int gridHeight>
    struct VectorField {
        Array <std::array<T, deltas.size()>, gridWidth, gridHeight> v;

        T &add(int x, int y, int dx, int dy, T dv) {
            return get(x, y, dx, dy) += dv;
        }

        T &get(int x, int y, int dx, int dy) {
            size_t i = std::ranges::find(deltas, std::pair(dx, dy)) - deltas.begin();
            assert(i < deltas.size());
            return v[x][y][i];
        }
    };

//    template<typename T, int gridWidth, int gridHeight>
//    struct TemplateArray {
//    private:
//        int D = deltas.size();
//
//        Array <std::vector<T>, gridWidth, gridHeight> values_;
//        std::vector<std::pair<int, int>> deltas_;
////        Array<std::array<T, D>, gridWidth, gridHeight> values_;
////        std::array<std::pair<int, int>, D> deltas_;
//
//    public:
//        T &add(int x, int y, int dx, int dy, T dv) {
//            return get(x, y, dx, dy) += dv;
//        }
//
//        T &get(int x, int y, int dx, int dy) {
//            auto it = std::find_if(deltas_.begin(), deltas_.end(),
//                                   [dx, dy](const std::pair<int, int> &p) {
//                                       return p.first == dx && p.second == dy;
//                                   });
//            //auto it = std::find(deltas_.begin(), deltas_.end(), std::pair(dx, dy));
//
//            if (it == deltas_.end()) {
//                throw std::invalid_argument("Delta pair not found");
//            }
//
//            std::size_t index = std::distance(deltas_.begin(), it);
//            return values_[x][y][index];
//        }
//
//        explicit TemplateArray(std::vector<std::pair<int, int>> &temp_deltas)
//                : deltas_(temp_deltas) {}
//    };
}

#endif //FLUIDFINALVER_ARRAYSTEMPS_H
