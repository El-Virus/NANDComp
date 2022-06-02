#pragma once
#include "types.hpp"

namespace PLUMBING {
    BIT bselect(BIT s, BIT d0, BIT d1);
    WORD select16(BIT s, WORD d0, WORD d1);
}