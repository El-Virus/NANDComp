#include "computer.hpp"
#include <iostream>
#include <fstream>
#include <bitset>
#include <string>

int main(int argc, char **argv) {
    std::string filename;
    if (argc < 2) {
        printf("Usage: %s code.bit", argv[0]);
        return 1;
    } else {
        filename = argv[1];
    }

    std::fstream code;
    code.open(filename);
    if (!code.is_open()) {
        printf("Could not open code file %s.", filename.c_str());
        return 1;
    }
    std::vector<MEMORY::Register> ROMMem;
    std::string line;
    while(std::getline(code, line)){  //read data from file object and put it into string.
        ROMMem.push_back(atoi(line.c_str()));
    }
    code.close();

    MEMORY::ROM ROM(ROMMem);
    COMPUTER::Computer computer(ROM);

    computer.run();
    printf("A Register: %i", computer.getLastA());
}