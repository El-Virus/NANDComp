#include "arithmetics.hpp"
#include "logic.hpp"
#include "misc.hpp"

using namespace BITMAN;
using namespace LOGIC_GATES;
using namespace TYPES;

namespace ARITHMETICS {

    HLBits hadd(BIT a, BIT b) {
        HLBits s;
        s.h = AND(a, b);
        s.l = AND(NOT(s.h), OR(a, b));
        return s;
    }

    HLBits add(BIT a, BIT b, BIT c) {
        HLBits s;
        HLBits apb = hadd(a, b);
        HLBits apbppc = hadd(apb.h, c);
        return {OR(AND(a, c), OR(apb.h, AND(b, c))), XOR(OR(apb.l, apbppc.h), AND(NAND(a, b), apbppc.l))};
    }

    WORDWCarry add16(WORD a, WORD b, BIT c) {
        BitArr<16> s;
        BIT carry = c;
        HLBits work;

        for (SU i = 0; i <= 15; i++) {
            work = add(getBitFromWord(a, i), getBitFromWord(b, i), carry);
            s.b[i] = work.l;
            carry = work.h;
        }
        return {composeWord(s), carry};
    }

    WORD inc16(WORD a) {
        return add16(a, 0b0000000000000000, 1).word;
    }

    WORD inv16(WORD a) {
        BitArr<16> s;

        for (SU i = 0; i <= 15; i++) {
            s.b[i] = NOT(getBitFromWord(a, i));
        }
        return composeWord(s);
    }

    WORD bitTo16(WORD a, WORD b, bitfunc func) {
        BitArr<16> s;
        
        for (SU i = 0; i <= 15; i++) {
            s.b[i] = func(getBitFromWord(a, i), getBitFromWord(b, i));
        }
        return composeWord(s);
    }

    WORD sub16(WORD a, WORD b) {
        return add16(a, inc16(inv16(b)), 0).word;
    }

    BIT eq16(WORD a) {
        BitArr<16> bits = decomposeWord(a);
        return NOT(OR(OR(OR(OR(bits.b[0], bits.b[1]), OR(bits.b[2], bits.b[3])), OR(OR(bits.b[4], bits.b[5]), OR(bits.b[6], bits.b[7]))), OR(OR(OR(bits.b[8], bits.b[9]), OR(bits.b[10], bits.b[11])), OR(OR(bits.b[12], bits.b[13]), OR(bits.b[14], bits.b[15])))));
    }

    BIT lz16(WORD a) {
        return getBitFromWord(a, 15);
    }
}