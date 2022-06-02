#include "computer.hpp"
#include "logic.hpp"
#include "misc.hpp"

using namespace BITMAN;
using namespace LOGIC_GATES;
using namespace PROCESSOR;

namespace COMPUTER {
    //class Computer
        void Computer::run()  {
            while ((AND(getBitFromWord(ROM.get(pc.get()), 14), getBitFromWord(ROM.get(pc.get()), MISC::CI)) != true)) {
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
        }

        WORD Computer::getLastA() {
            return lastA;
        }

        Computer::Computer(MEMORY::ROM NROM) {
            ROM = NROM;
        }
}