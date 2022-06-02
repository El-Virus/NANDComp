#pragma once
#include <stdbool.h>

typedef unsigned short WORD;
typedef unsigned char SU;
typedef bool BIT;

namespace TYPES {
    struct HLBits {
        bool h;
        bool l;
    };

    template <int T>
    struct BitArr {
        BIT b[T];
    };

    struct WORDWCarry {
        WORD word;
        BIT carry;
    };
    
}