#pragma once

#include "types.hpp"

namespace MISC {
    enum InstructionMap {
        GT,
        EQ,
        LT,
        AM,
        D,
        A,
        NO,
        F,
        NY,
        ZY,
        NX,
        ZX,
        SM,
        __NULL1__,
        __NULL2__,
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
