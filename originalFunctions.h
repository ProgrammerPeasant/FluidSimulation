#ifndef FLUIDFINALVER_ORIGINALFUNCTIONS_H
#define FLUIDFINALVER_ORIGINALFUNCTIONS_H

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <tuple>
#include <algorithm>
#include <sstream>
#include <optional>
#include <fstream>

#include "Fixed.h"
#include "specialArr.h"
#include "arraysTemps.h"
#include "random.h"

using namespace std;


namespace FluidPhysics {
    class IEngine {
    public:
        virtual void next(std::optional<std::reference_wrapper<std::ostream>> out) = 0;
        virtual void load(std::ifstream& file) = 0;
        virtual void save(std::ofstream& file) = 0;
        virtual ~IEngine() = default;

//        virtual void writeToStream(std::ostream& out) = 0;
//        virtual void readFromStream(std::istream& in);
//        virtual void storeToFileVelocity(const std::string& filename);

    };


    template <typename PressureType, typename VelocityType, typename FlowVelocityType, int Width, int Height>
    class FluidEngine : public IEngine {
    private:
        int gridWidth = Width;
        int gridHeight = Height;

        // implement static assert for the types

        Array<char, Width, Height> simulationGrid{};
        VectorField<VelocityType, Width, Height> velocityField{};
        VectorField<FlowVelocityType, Width, Height> flowVelocityField{};
        Array<PressureType, Width, Height> pressure{};
        Array<PressureType, Width, Height> previousPressure{};
        Array<int64_t, Height, Height> directionMatrix{};
        Array<int, Height, Height> lastUsage{};
        int updateTimestamp = 0;
        PressureType densityLevels[256];


        std::tuple<FlowVelocityType, bool, pair<int, int>> propagate_flow(int x, int y, FlowVelocityType lim) {
            lastUsage[x][y] = updateTimestamp - 1;
            FlowVelocityType ret{};
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (nx < 0 || nx >= gridWidth || ny < 0 || ny >= gridHeight) continue;

                if (simulationGrid[nx][ny] != '#' && lastUsage[nx][ny] < updateTimestamp) {
                    VelocityType cap = velocityField.get(x, y, dx, dy);
                    FlowVelocityType flow = flowVelocityField.get(x, y, dx, dy);
                    if (fabs(flow - FlowVelocityType(cap)) <= 0.0001) {
                        continue;
                    }
                    // assert(v >= flowVelocityField.get(x, y, dx, dy));
                    FlowVelocityType vp = std::min(lim, FlowVelocityType(cap) - flow);
                    if (lastUsage[nx][ny] == updateTimestamp - 1) {
                        flowVelocityField.add(x, y, dx, dy, vp);
                        lastUsage[x][y] = updateTimestamp;
                        // cerr << x << " " << y << " -> " << nx << " " << ny << " " << vp << " / " << lim << "\n";
                        return {vp, true, {nx, ny}};
                    }
                    auto [t, prop, end] = propagate_flow(nx, ny, vp);
                    ret += t;
                    if (prop) {
                        flowVelocityField.add(x, y, dx, dy, t);
                        lastUsage[x][y] = updateTimestamp;
                        // cerr << x << " " << y << " -> " << nx << " " << ny << " " << t << " / " << lim << "\n";
                        return {t, prop && end != pair(x, y), end};
                    }
                }
            }
            lastUsage[x][y] = updateTimestamp;
            return {ret, false, {0, 0}};
        }

        void propagate_stop(int x, int y, bool force = false) {
            if (!force) {
                for (auto [dx, dy] : deltas) {
                    int nx = x + dx, ny = y + dy;
                    if (nx < 0 || nx >= gridWidth || ny < 0 || ny >= gridHeight) continue;

                    if (simulationGrid[nx][ny] != '#' && lastUsage[nx][ny] < updateTimestamp - 1 && velocityField.get(x, y, dx, dy) > 0ll) {
                        return;
                    }
                }
            }
            lastUsage[x][y] = updateTimestamp;
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (nx < 0 || nx >= gridWidth || ny < 0 || ny >= gridHeight) continue;

                if (simulationGrid[nx][ny] == '#' || lastUsage[nx][ny] == updateTimestamp || velocityField.get(x, y, dx, dy) > 0ll) {
                    continue;
                }
                propagate_stop(nx, ny);
            }
        }

        VelocityType move_prob(int x, int y) {
            VelocityType sum{};
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (nx < 0 || nx >= gridWidth || ny < 0 || ny >= gridHeight) continue;

                if (simulationGrid[nx][ny] == '#' || lastUsage[nx][ny] == updateTimestamp) {
                    continue;
                }
                VelocityType v = velocityField.get(x, y, dx, dy);
                if (v < 0ll) {
                    continue;
                }
                sum += v;
            }
            return sum;
        }

        void swap(int x1, int y1, int x2, int y2) {
            std::swap(simulationGrid[x1][y1], simulationGrid[x2][y2]);
            std::swap(pressure[x1][y1], pressure[x2][y2]);
            std::swap(velocityField.v[x1][y1], velocityField.v[x2][y2]);
        }

        bool propagate_move(int x, int y, bool is_first) {
            lastUsage[x][y] = updateTimestamp - is_first;
            bool ret = false;
            int nx = -1, ny = -1;
            do {
                std::array<VelocityType, deltas.size()> tres;
                VelocityType sum{};
                for (size_t i = 0; i < deltas.size(); ++i) {
                    auto [dx, dy] = deltas[i];
                    int fx = x + dx, fy = y + dy;
                    if (fx < 0 || fx >= gridWidth || fy < 0 || fy >= gridHeight) continue;

                    if (simulationGrid[fx][fy] == '#' || lastUsage[fx][fy] == updateTimestamp) {
                        tres[i] = sum;
                        continue;
                    }
                    VelocityType v = velocityField.get(x, y, dx, dy);
                    if (v < 0ll) {
                        tres[i] = sum;
                        continue;
                    }
                    sum += v;
                    tres[i] = sum;
                }

                if (sum == 0ll) {
                    break;
                }

                VelocityType randNum = random01<VelocityType>() * sum;
                size_t d = std::ranges::upper_bound(tres, randNum) - tres.begin();

                auto [dx, dy] = deltas[d];
                nx = x + dx;
                ny = y + dy;
                assert(velocityField.get(x, y, dx, dy) > 0ll && simulationGrid[nx][ny] != '#' && lastUsage[nx][ny] < updateTimestamp);

                ret = (lastUsage[nx][ny] == updateTimestamp - 1 || propagate_move(nx, ny, false));
            } while (!ret);
            lastUsage[x][y] = updateTimestamp;
            for (auto [dx, dy] : deltas) {
                int fx = x + dx, fy = y + dy;
                if (fx < 0 || fx >= gridWidth || fy < 0 || fy >= gridHeight) continue;

                if (simulationGrid[fx][fy] != '#' && lastUsage[fx][fy] < updateTimestamp - 1 && velocityField.get(x, y, dx, dy) < 0ll) {
                    propagate_stop(nx, ny);
                }
            }
            if (ret && !is_first) {
                swap(x, y, nx, ny);
            }
            return ret;
        }

        void init() {
            flowVelocityField.v.init(gridWidth, gridHeight);
            directionMatrix.init(gridWidth, gridHeight);
            previousPressure.init(gridWidth, gridHeight);

            densityLevels[' '] = 0.01;
            densityLevels['.'] = 1000ll;
            for (int x = 0; x < gridWidth; ++x) {
                for (int y = 0; y < gridHeight; ++y) {
                    if (simulationGrid[x][y] == '#')
                        continue;
                    for (auto [dx, dy]: deltas) {
                        directionMatrix[x][y] += (simulationGrid[x + dx][y + dy] != '#');
                    }
                }
            }
        }

    public:
        FluidEngine() = default;

        void next(std::optional<std::reference_wrapper<std::ostream>> out) override {
            PressureType total_delta_p = 0ll;

            // Apply external forces
            for (size_t x = 0; x < gridWidth; ++x) {
                for (size_t y = 0; y < gridHeight; ++y) {
                    if (simulationGrid[x][y] == '#')
                        continue;
                    if (simulationGrid[x + 1][y] != '#')
                        velocityField.add(x, y, 1, 0, g<VelocityType>());
                }
            }

            // Apply forces from pressure
            previousPressure = pressure;
            for (size_t x = 0; x < gridWidth; ++x) {
                for (size_t y = 0; y < gridHeight; ++y) {
                    if (simulationGrid[x][y] == '#')
                        continue;
                    for (auto [dx, dy] : deltas) {
                        int nx = x + dx, ny = y + dy;
                        if (simulationGrid[nx][ny] != '#' && previousPressure[nx][ny] < previousPressure[x][y]) {
                            PressureType delta_p = previousPressure[x][y] - previousPressure[nx][ny];
                            PressureType force = delta_p;
                            VelocityType &contr = velocityField.get(nx, ny, -dx, -dy);
                            if (PressureType(contr) * densityLevels[(int) simulationGrid[nx][ny]] >= force) {
                                contr -= VelocityType(force / densityLevels[(int) simulationGrid[nx][ny]]);
                                continue;
                            }
                            force -= PressureType(contr) * densityLevels[(int) simulationGrid[nx][ny]];
                            contr = 0ll;
                            velocityField.add(x, y, dx, dy, VelocityType(force / densityLevels[(int) simulationGrid[x][y]]));
                            pressure[x][y] -= force / directionMatrix[x][y];
                            total_delta_p -= force / directionMatrix[x][y];
                        }
                    }
                }
            }

            // Make flow from velocities
            flowVelocityField = {};
            bool prop = false;
            do {
                updateTimestamp += 2;
                prop = false;
                for (size_t x = 0; x < gridWidth; ++x) {
                    for (size_t y = 0; y < gridHeight; ++y) {
                        if (simulationGrid[x][y] != '#' && lastUsage[x][y] != updateTimestamp) {
                            auto [t, local_prop, _] = propagate_flow(x, y, 1ll);
                            if (t > 0ll) {
                                prop = true;
                            }
                        }
                    }
                }
            } while (prop);

            // Recalculate pressure with kinetic energy
            for (size_t x = 0; x < gridWidth; ++x) {
                for (size_t y = 0; y < gridHeight; ++y) {
                    if (simulationGrid[x][y] == '#')
                        continue;
                    for (auto [dx, dy] : deltas) {
                        VelocityType old_v = velocityField.get(x, y, dx, dy);
                        FlowVelocityType new_v = flowVelocityField.get(x, y, dx, dy);
                        if (old_v > 0ll) {
                            assert(VelocityType(new_v) <= old_v);

                            velocityField.get(x, y, dx, dy) = VelocityType(new_v);
                            auto force = PressureType(old_v - VelocityType(new_v)) * densityLevels[(int) simulationGrid[x][y]];
                            if (simulationGrid[x][y] == '.')
                                force *= 0.8;
                            if (simulationGrid[x + dx][y + dy] == '#') {
                                pressure[x][y] += force / directionMatrix[x][y];
                                total_delta_p += force / directionMatrix[x][y];
                            } else {
                                pressure[x + dx][y + dy] += force / directionMatrix[x + dx][y + dy];
                                total_delta_p += force / directionMatrix[x + dx][y + dy];
                            }
                        }
                    }
                }
            }

            updateTimestamp += 2;
            prop = false;
            for (size_t x = 0; x < gridWidth; ++x) {
                for (size_t y = 0; y < gridHeight; ++y) {
                    if (simulationGrid[x][y] != '#' && lastUsage[x][y] != updateTimestamp) {
                        if (random01<VelocityType>() < move_prob(x, y)) {
                            prop = true;
                            propagate_move(x, y, true);
                        } else {
                            propagate_stop(x, y, true);
                        }
                    }
                }
            }

            if (prop && out) {
                for (int i = 0; i < gridWidth; ++i) {
                    for (int j = 0; j < gridHeight; ++j) {
                        out->get() << simulationGrid[i][j];
                    }
                    out->get() << std::endl;
                }
                std::cout.flush();
            }
        }

        void load(std::ifstream& file) override {
            auto loadArray = [&]<typename T, int gridWidth, int gridHeight>(Array<T, gridWidth, gridHeight>& arr, int n, int m) {
                arr.init(n, m);
                for (int i = 0; i < arr.gridWidth; ++i) {
                    for (int j = 0; j < arr.gridHeight; ++j) {
                        if constexpr (std::is_same_v<T, char>) {
                            char tmp;
                            do {
                                file.get(tmp);
                            } while (tmp == '\n');
//                            file >> tmp;
                            arr[i][j] = static_cast<char>(tmp);
                        } else {
                            double tmp = 0;
                            file >> tmp;
                            arr[i][j] = tmp;
                        }
                    }
                }
            };

            auto loadArrayOfArrays = [&]<typename T, int gridWidth, int gridHeight>(Array<std::array<T, 4>, gridWidth, gridHeight>& arr, int n, int m){
                arr.init(n, m);
                for (int i = 0; i < arr.gridWidth; ++i) {
                    for (int j = 0; j < arr.gridHeight; ++j) {
                        double tmp[4] = {0};
                        file >> tmp[0] >> tmp[1] >> tmp[2] >> tmp[3];
                        arr[i][j] = {T(tmp[0]), T(tmp[1]), T(tmp[2]), T(tmp[3])};
                    }
                }
            };


            if (!file.is_open()) {
                throw std::invalid_argument("File is not opened");
            }
            file >> gridWidth >> gridHeight >> updateTimestamp;
            loadArray(simulationGrid, gridWidth, gridHeight);
            loadArray(lastUsage, gridWidth, gridHeight);
            loadArray(pressure, gridWidth, gridHeight);
            loadArrayOfArrays(velocityField.v, gridWidth, gridHeight);
            init();
        }

        void save(std::ofstream &file) override {
            auto saveArray = [&]<typename T, int gridWidth, int gridHeight>(Array<T, gridWidth, gridHeight>& arr) {
                for (int i = 0; i < arr.gridWidth; ++i) {
                    for (int j = 0; j < arr.gridHeight; ++j) {
                        if constexpr (std::is_same_v<T, char>) {
                            file << arr[i][j];
                        } else {
                            file << arr[i][j] << " ";
                        }
                    }
                    file << std::endl;
                }
            };

            auto saveArrayOfArrays = [&]<typename T, int gridWidth, int gridHeight>(Array<std::array<T, 4>, gridWidth, gridHeight>& arr){
                for (int i = 0; i < arr.gridWidth; ++i) {
                    for (int j = 0; j < arr.gridHeight; ++j) {
                        file << arr[i][j][0] << " " << arr[i][j][1] << " " << arr[i][j][2] << " " << arr[i][j][3];
                    }
                    file << std::endl;
                }
            };

            if (!file.is_open()) {
                throw std::invalid_argument("File is not opened");
            }
            file << gridWidth << " " << gridHeight << " " << updateTimestamp << std::endl;
            saveArray(simulationGrid);
            saveArray(lastUsage);
            saveArray(pressure);
            saveArrayOfArrays(velocityField.v);
        }
    };
}

#endif //FLUIDFINALVER_ORIGINALFUNCTIONS_H
