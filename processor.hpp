#pragma once
#include "alu.hpp"
#include "memory.hpp"

using namespace BITMAN;

namespace PROCESSOR {
    
    struct DecodedInstruction {
        BIT computation;
        BIT source;
        ALU::ALUOp operation;
        TYPES::BitArr<3> destination;
        ALU::Conditions condition;
        WORD W;
    };

    struct ProcessResult {
        BIT jump;
        WORD A;
    };

    class CombinedMemory {
        private:
            MEMORY::Register A;
            MEMORY::Register D;
            MEMORY::RAM RAM;
        public:
            void store(TYPES::BitArr<3> dst, WORD X);
            MISC::TriWORD getValues();
    };

    DecodedInstruction decodeInstruction(WORD I);

    class Processor {
        private:
            CombinedMemory cm;
        public:
            ProcessResult process(WORD I);
    };
}