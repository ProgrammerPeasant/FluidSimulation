#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdio>
#include <termios.h>
#include <unistd.h>

#include "generatorFactory.h"

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;

    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

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

    enableRawMode(); // Включаем неканонический режим

    const int T = 1'000'000;
    for (int i = 0; i < T; ++i) {
        // Проверяем наличие нажатия клавиши
        char c;
        int nread = read(STDIN_FILENO, &c, 1);
        if (nread == 1 && c == 4) { // ASCII код 4 соответствует Ctrl+D
            isSave = true;
        }

        if (isSave) {
            std::cout << "\nSaving\n";
            engine->save(saveFile);
            isSave = false;
            std::cout << "Saved in file " << saveFileName << std::endl;
            getchar(); // Ожидаем нажатия клавиши перед продолжением
        }
        engine->next(std::cout);
    }

    return 0;
}