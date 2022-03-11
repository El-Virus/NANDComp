#include "memory.hpp"

using namespace ARITHMETICS;
using namespace TYPES;
using namespace PLUMBING;

namespace MEMORY {
    //class Latch
        void Latch::set(BIT d) {
            status = d;
        }

        BIT Latch::get() {
            return status;
        }

        Latch::Latch() {
            status = 0;
        }
        
        Latch::Latch(BIT d) {
            status = d;
        }
    //class Register
        void Register::set(WORD d) {
            status = d;
        }

        WORD Register::get() {
            return status;
        }

        Register::Register() {
            status = 0b0000000000000000;
        }

        Register::Register(WORD d) {
            status = d;
        }
    //class Counter
        void Counter::tick() {
            status = inc16(status);
        }
    //class ROM
        WORD ROM::get(WORD addr) {
            return registers[switch16(addr)].get();
        }

        ROM::ROM() {}

        ROM::ROM(std::vector<Register> regs) {
            for (WORD i = 0; i < regs.size(); ++i) {
                registers[i].set(regs[i].get());
            }
        }
    //class RAM
        void RAM::set(WORD addr, WORD d) {
            registers[switch16(addr)].set(d);
        }

        RAM::RAM() : ROM() {}

        RAM::RAM(std::vector<Register> regs) : ROM(regs) {}
}