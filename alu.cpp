#include "alu.hpp"
#include "arithmetics.hpp"
#include "logic.hpp"
#include "plumbing.hpp"

using namespace ARITHMETICS;
using namespace LOGIC_GATES;
using namespace PLUMBING;

namespace ALU {
    //class LogicUnit
        WORD LogicUnit::operate(BIT op1, BIT op0, WORD x, WORD y) {
            return select16(op1, select16(op0, bitTo16(x, y, AND), bitTo16(x, y, OR)), select16(op0, bitTo16(x, y, XOR), inv16(x)));
        }

    //class ArithmeticUnit
        WORD ArithmeticUnit::operate(BIT op1, BIT op0, WORD x, WORD y) {
            return select16(op0, select16(op1, add16(x, y, 0).word, sub16(x, y)), select16(op1, add16(x, 1, 0).word, sub16(x, 1)));
        }

    //class ALU
        WORD ALU::operate(ALUOp op, WORD x, WORD y) {
            WORD X = select16(op.zx, select16(op.sw, x, y), 0);
            WORD Y = select16(op.sw, y, x);
            return (select16(op.u, LogicUnit::operate(op.op1, op.op0, X, Y),ArithmeticUnit::operate(op.op1, op.op0, X, Y)));
        }

    //condition
        BIT condition(Conditions cd, WORD x) {
            return OR(AND(cd.lt, lz16(x)), OR(AND(cd.eq, eq16(x)), AND(AND(cd.gt, NOT(lz16(x))), NOT(eq16(x)))));
        }
}