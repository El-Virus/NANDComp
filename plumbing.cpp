#include "plumbing.hpp" 
#include "logic.hpp"
#include "misc.hpp"

using namespace BITMAN;
using namespace LOGIC_GATES;
using namespace TYPES;

namespace PLUMBING {
    BIT bselect(BIT s, BIT d0, BIT d1) {
        return OR(AND(s, d1), AND(NOT(s), d0));
    }

    WORD select16(BIT s, WORD d0, WORD d1) {
        BitArr<16> work;
        for (SU i = 0; i <= 15; i++) {
            work.b[i] = bselect(s, getBitFromWord(d0, i), getBitFromWord(d1, i));
        }
        return composeWord(work);
    }
}