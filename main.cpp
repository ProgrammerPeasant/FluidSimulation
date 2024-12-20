#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <signal.h>
#include <cstdio>
#include <filesystem>

#include "generatorFactory.h"

namespace fs = std::filesystem;

bool isSave = false;

void handle_sigquit(int signum) {
    isSave = true;
}

void validateFile(const fs::path& filePath) {
    if (!fs::exists(filePath)) {
        throw std::invalid_argument("File does not exist: " + filePath.string());
    }
}

int main(int argc, char* argv[]) {
    OptionsParser optsParser(argc, argv);
    int n = 0, m = 0;
    std::string inputFile;
    try {
        inputFile = optsParser.getOptVal("--input");
    } catch (const std::exception& ex) {
        throw std::invalid_argument("Missing required --input argument: " + std::string(ex.what()));
    }
    const auto saveFileName = optsParser.getOptVal("--output");
    const int pTypeCode = GetTypeCode(optsParser.getOptVal("--p-type"));
    const int vTypeCode = GetTypeCode(optsParser.getOptVal("--v-type"));
    const int vFlowTypeCode = GetTypeCode(optsParser.getOptVal("--v-flow-type"));


    const fs::path inputFilePath = inputFile;
    validateFile(inputFilePath);

    std::ifstream input(inputFilePath);
    if (!input.is_open()) {
        throw std::runtime_error("Failed to open input file: " + inputFilePath.string());
    }

    if (!(input >> n >> m)) {
        throw std::runtime_error("Failed to read 'n' and 'm' from input file: " + inputFilePath.string());
    }

    input.clear();
    input.seekg(0, std::ios::beg);

    signal(SIGQUIT, handle_sigquit);

    auto engine = ProduceEngine(pTypeCode, vTypeCode, vFlowTypeCode, n, m);

    std::ofstream saveFile(saveFileName);
    if (!saveFile.is_open()) {
        throw std::invalid_argument("Can't open save file: " + saveFileName);
    }

    engine->load(input);

    const int T = 10000;
    for (int i = 0; i < T; ++i) {
        if (isSave) {
            std::cout << "\nSaving\n";
            engine->save(saveFile);
            isSave = false;
            std::cout << "Saved in file " << saveFileName << std::endl;
        }
        engine->next(std::cout);
    }

    return 0;
}