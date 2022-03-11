#include "computer.hpp"

using namespace PROCESSOR;

namespace COMPUTER {
    //class Computer
        void Computer::run()  {
            while (true) {
                tick();
            }
        }

        void Computer::tick() {
            ProcessResult pr = processor.process(ROM.get(pc.get()));
            lastA = pr.A;
            if (pr.jump) {
                pc.set(pr.A);
            } else {
                pc.tick();
            }
            comTickReport();
        }

        WORD Computer::getLastA() {
            return lastA;
        }

        Computer::Computer(MEMORY::ROM NROM) {
            ROM = NROM;
        }
    //comTickReport
        void comTickReport() {}
}