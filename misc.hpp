#pragma once

#include "types.hpp"

namespace MISC {
    enum InstructionMap {
        GT,
        EQ,
        LT,
        SW,
        ZX,
        OP0,
        OP1,
        U,
        AM,
        D,
        A,
        __NULL1__,
        SM,
        __NULL2__,
        __NULL3__,
        CI
    };

    struct TriWORD {
        WORD a;
        WORD b;
        WORD c;
    };
}

namespace BITMAN {
    bool getBitFromWord(WORD word, SU i);
    TYPES::BitArr<16> decomposeWord(WORD word);
    WORD composeWord(TYPES::BitArr<16> bword);
}
