#include "alu.hpp"

using namespace ARITHMETICS;
using namespace PLUMBING;
using namespace LOGIC_GATES;
using namespace TYPES;

namespace ALU {
    //class UnaryALU
        WORD UnaryALU::negate(WORD x) {
            return inv16(x);
        }

        WORD UnaryALU::zero(WORD x) {
            return 0;
        }

        WORD UnaryALU::zero() {
            return 0;
        }

        WORD UnaryALU::operate(BIT z, BIT n, WORD x) {
            WORD work;
            work = select16(z, x, zero());
            return select16(n, work, inv16(work));
        }

    //class ALU
        WORD ALU::funct(BIT f, WORD x, WORD y) {
            return select16(f, and16(x, y), add16(x, y, 0).word);
        }

        WORD ALU::operate(ALUOp op, WORD x, WORD y) {
            WORD work = funct(op.f, UnaryALU::operate(op.zx, op.nx, x), UnaryALU::operate(op.zy, op.ny, y));
            return select16(op.no, work, inv16(work));
        }

    //condition
        BIT condition(Conditions cd, WORD x) {
            return OR(AND(cd.lt, lz16(x)), OR(AND(cd.eq, eq16(x)), AND(AND(cd.gt, NOT(lz16(x))), NOT(eq16(x)))));
        }
}