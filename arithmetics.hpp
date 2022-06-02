#include "logic.hpp"
#include "misc.hpp"

namespace ARITHMETICS {
    typedef BIT (*bitfunc)(BIT a, BIT b);
    TYPES::HLBits add(bool a, bool b, bool c);
    TYPES::WORDWCarry add16(WORD a, WORD b, BIT c);
    WORD inc16(WORD a);
    WORD inv16(WORD a);
    WORD sub16(WORD substractee, WORD substractor);
    WORD bitTo16(WORD a, WORD b, bitfunc func);
    BIT eq16(WORD a);
    BIT lz16(WORD a);
}