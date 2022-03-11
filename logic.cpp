//#include "bit/bit.hpp"
#include "logic.hpp"
#include <assert.h>

namespace LOGIC_GATES {

    //Logic Exception
    BIT NAND(BIT a, BIT b) {
        assert((a == 0 || a == 1) && (b == 0 || b == 1));
        if (a & b) {
            return false;
        } else {
            return true;
        }
    }

    BIT NOT(BIT a) {
        return NAND(a, a);
    }

    BIT AND(BIT a, BIT b) {
        return NOT(NAND(a, b));
    }

    BIT OR(BIT a, BIT b) {
        return NAND(NOT(a), NOT(b));
    }

    BIT XOR(BIT a, BIT b) {
        return AND(OR(a, b), NAND(a, b));
    }
}

