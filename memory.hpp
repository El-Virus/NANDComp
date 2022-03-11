#pragma once
#define MEM_SIZE 0xffff
#include "plumbing.hpp"
#include <vector>

namespace MEMORY {
    class Latch {
        private:
            BIT status;
        public:
            void set(BIT d);
            BIT get();
            Latch();
            Latch(BIT d);
    };
    //class DFF
    class Register {
        protected:
            WORD status;
        public:
            void set(WORD d);
            WORD get();
            Register();
            Register(WORD d);
    };
    class Counter : public Register {
        public:
            void tick();
    };
    class ROM {
        protected:
            Register registers[MEM_SIZE];
        public:
            WORD get(WORD addr);
            ROM();
            ROM(std::vector<Register> regs);
    };
    class RAM : public ROM {
        public:
            void set(WORD addr, WORD d);
            RAM();
            RAM(std::vector<Register> regs);
    };
}