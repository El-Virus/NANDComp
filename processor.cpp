#include "processor.hpp"
#include <iostream>

using namespace PLUMBING;
using namespace TYPES;
using namespace MISC;
using namespace LOGIC_GATES;

namespace PROCESSOR {
    //class CombinedMemory
        void CombinedMemory::store(TYPES::BitArr<3> dst, WORD X) {
            A.set(select16(dst.b[0], A.get(), X));
            D.set(select16(dst.b[1], D.get(), X));
            RAM.set(A.get(), select16(dst.b[2], RAM.get(A.get()), X));
        }

        TriWORD CombinedMemory::getValues() {
            return {A.get(), D.get(), RAM.get(A.get())};
        }
    //class Processor
        ProcessResult Processor::process(WORD I) {
            DecodedInstruction di = decodeInstruction(I);
            TriWORD OMV = cm.getValues();
            WORD ALURes = ALU::ALU::operate(di.operation, OMV.b, select16(di.source, OMV.a, OMV.c));
            cm.store(di.destination, select16(di.computation, di.W, ALURes));
            return {condition(di.condition, ALURes), cm.getValues().a};
        }
    //decode Instruction
    DecodedInstruction decodeInstruction(WORD I) {
        DecodedInstruction di;
        BitArr<16> work = decomposeWord(I);
        di.W = select16(work.b[CI], I, 0x0000000000000000);
        work = decomposeWord(select16(work.b[CI], 0x0000000000100000, I));
        di.computation = work.b[CI];
        di.source = work.b[SM];
        di.operation = {work.b[U], work.b[OP1], work.b[OP0], work.b[ZX], work.b[SW]};
        di.destination.b[2] = work.b[AM]; di.destination.b[1] = work.b[D]; di.destination.b[0] = OR(work.b[A], NOT(work.b[CI]));
        di.condition = {work.b[LT], work.b[EQ], work.b[GT]};
        return di;
    };
}