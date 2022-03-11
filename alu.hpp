#include "plumbing.hpp"

namespace ALU {
    struct ALUOp
    {
        BIT zx;
        BIT nx;
        BIT zy;
        BIT ny;
        BIT f;
        BIT no;
    };

    struct Conditions
    {
        BIT lt;
        BIT eq;
        BIT gt;
    };
    class UnaryALU {
        public:
            static WORD negate(WORD x);
            static WORD zero(WORD x);
            static WORD zero();
            static WORD operate(BIT z, BIT n, WORD x);
    };
    class ALU {
        public:
            static WORD funct(BIT f, WORD x, WORD y);
            static WORD operate(ALUOp op, WORD x, WORD y);
    };
    BIT condition(Conditions cd, WORD x);
}