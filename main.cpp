#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdio>

#include "generatorFactory.h"

bool isSave = false;

int main(int argc, char* argv[]) {
    OptionsParser optsParser(argc, argv);
    auto inputFile = optsParser.getOptVal("--input");
    auto saveFileName = optsParser.getOptVal("--output");
    int pTypeCode = GetTypeCode(optsParser.getOptVal("--p-type"));
    int vTypeCode = GetTypeCode(optsParser.getOptVal("--v-type"));
    int vFlowTypeCode = GetTypeCode(optsParser.getOptVal("--v-flow-type"));

    std::ifstream input(inputFile);
    if (!input.is_open()) {
        throw std::invalid_argument("Can't open input file: " + inputFile);
    }
    int a;
    int n, m;
    input >> n >> m;
    input.seekg(0, std::ios::beg);

    auto engine = ProduceEngine(pTypeCode, vTypeCode, vFlowTypeCode, n, m);

    std::ofstream saveFile(saveFileName);
    if (!saveFile.is_open()) {
        throw std::invalid_argument("Can't open save file: " + saveFileName);
    }

    engine->load(input);

    const int T = 1'000'000;
    for (int i = 0; i < T; ++i) {
        if (isSave) {
            std::cout << "\nSaving\n";
            engine->save(saveFile);
            isSave = false;
            std::cout << "Saved in file " << saveFileName << std::endl;
            getchar();
        }
        engine->next(std::cout);
    }

    return 0;
}