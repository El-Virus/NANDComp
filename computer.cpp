#include "computer.hpp"
#include <bitset>

using namespace PROCESSOR;
using namespace BITMAN;
using namespace LOGIC_GATES;

namespace COMPUTER {
    //class Computer
        void Computer::run()  {
            while ((AND(BITMAN::getBitFromWord(ROM.get(pc.get()), 14), BITMAN::getBitFromWord(ROM.get(pc.get()), MISC::CI)) != true)) {
                //printf("%i:%i=%s\n", BITMAN::getBitFromWord(ROM.get(pc.get()), MISC::CI), BITMAN::getBitFromWord(ROM.get(pc.get()), 14), std::bitset<16>(ROM.get(pc.get())).to_string().c_str());
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