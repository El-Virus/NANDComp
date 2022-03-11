#pragma once

#include "processor.hpp"

namespace COMPUTER {
    class Computer {
        private:
            PROCESSOR::Processor processor;
            MEMORY::Counter pc;
            MEMORY::ROM ROM;
            WORD lastA;
        public:
            void run();
            void tick();
            WORD getLastA();
            Computer(MEMORY::ROM NROM);
    };
    void comTickReport();
}