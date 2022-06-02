#pragma once
#include "types.hpp"

namespace ALU {
    struct ALUOp {
        BIT u;
        BIT op1;
        BIT op0;
        BIT zx;
        BIT sw;
    };

    struct Conditions {
        BIT lt;
        BIT eq;
        BIT gt;
    };

    class LogicUnit {
        public:
            static WORD operate(BIT op1, BIT op0, WORD x, WORD y);
    };

    class ArithmeticUnit {
        public:
            static WORD operate(BIT op1, BIT op0, WORD x, WORD y);
    };

    class ALU {
        public:
            static WORD operate(ALUOp op, WORD x, WORD y);
    };
    BIT condition(Conditions cd, WORD x);
}