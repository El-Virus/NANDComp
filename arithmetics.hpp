#include "logic.hpp"
#include "misc.hpp"

namespace ARITHMETICS {
    TYPES::HLBits add(bool a, bool b, bool c);
    TYPES::WORDWCarry add16(WORD a, WORD b, BIT c);
    WORD inc16(WORD a);
    WORD inv16(WORD a);
    WORD and16(WORD a, WORD b);
    WORD sub16(WORD substractee, WORD substractor);
    BIT eq16(WORD a);
    BIT lz16(WORD a);
}