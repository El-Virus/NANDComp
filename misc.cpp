#include "misc.hpp"

using namespace TYPES;

//Logic Exception
namespace BITMAN {
    bool getBitFromWord(WORD word, SU i) {
        return ((word >> i) & 0b0000000000000001);
    }
    BitArr<16> decomposeWord(WORD word) {
        BitArr<16> bword;
        for (SU i = 0; i <= 15; i++) {
            bword.b[i] = getBitFromWord(word, i);
        }
        return bword;
    }
    WORD composeWord(BitArr<16> bword) {
        WORD word = 0;
        for (SU i = 15; i > 0; i--) {
            word = word + bword.b[i];
            word = word << 1;
        }
        word = word + bword.b[0];
        return word;
    }
}