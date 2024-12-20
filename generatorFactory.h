#ifndef FLUIDFINALVER_GENERATORFACTORY_H
#define FLUIDFINALVER_GENERATORFACTORY_H

#include "typeHolder.h"

namespace FluidPhysics {
    constexpr auto GenerateAllCombo() {
        constexpr std::pair<int, int> givenSizes[] = {DSIZES, {-1, -1}};
        constexpr int sizesCnt = sizeof(givenSizes) / sizeof(std::pair<int, int>);

        constexpr auto givenTypes = std::array{DTYPES};
        constexpr std::array<std::pair<int, int>, sizesCnt> sizes = {std::pair<int, int>(-1, -1), DSIZES};
        std::array<std::tuple<int, int, int, int, int>,
                givenTypes.size() * givenTypes.size() * givenTypes.size() * sizesCnt> res = {};

        int idx = 0;
        for (int pType: givenTypes) {
            for (int vType: givenTypes) {
                for (int vfType: givenTypes) {
                    for (auto simulationGrid: sizes) {
                        res[idx] = {pType, vType, vfType, simulationGrid.first, simulationGrid.second};
                        ++idx;
                    }
                }
            }
        }
        return res;
    }

    constexpr auto allCombos = GenerateAllCombo();
    std::array<std::shared_ptr<FluidPhysics::IEngine>(*)(), allCombos.size()> generateEngine;


    template<int idx>
    struct EngineGeneratorIdx {
        EngineGeneratorIdx<idx - 1> x;

        EngineGeneratorIdx() {
            generateEngine[idx - 1] = generate;
        }

        static std::shared_ptr<FluidPhysics::IEngine> generate() {
            return std::make_shared<FluidPhysics::FluidEngine<getType<std::get<0>(allCombos[idx - 1])>,
                    getType<std::get<1>(allCombos[idx - 1])>,
                    getType<std::get<2>(allCombos[idx - 1])>,
                    std::get<3>(allCombos[idx - 1]),
                    std::get<4>(allCombos[idx - 1])>>();
        }
    };

    template<>
    struct EngineGeneratorIdx<0> {
        EngineGeneratorIdx() = default;
    };

    EngineGeneratorIdx<allCombos.size()> generator{};
}

std::shared_ptr<FluidPhysics::IEngine> ProduceEngine(int pType, int vType, int vfType, int n, int m) {
    auto itr = std::find(FluidPhysics::allCombos.begin(),
                         FluidPhysics::allCombos.end(),
                         std::tuple(pType, vType, vfType, n, m));
    std::shared_ptr<FluidPhysics::IEngine> engine;
    if (itr != FluidPhysics::allCombos.end()) {
        engine = FluidPhysics::generateEngine[itr - FluidPhysics::allCombos.begin()]();
    } else {
        itr = std::find(FluidPhysics::allCombos.begin(),
                        FluidPhysics::allCombos.end(),
                        std::tuple(pType, vType, vfType, -1, -1));
        if (itr == FluidPhysics::allCombos.end()) {
            throw std::invalid_argument("unknown types");
        }
        engine = FluidPhysics::generateEngine[itr - FluidPhysics::allCombos.begin()]();
    }
    return engine;
}

#endif //FLUIDFINALVER_GENERATORFACTORY_H
