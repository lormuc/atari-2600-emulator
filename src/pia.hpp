#pragma once

#include "machine.hpp"

namespace pia {
    void init();
    void set(t_addr, char);
    char get(t_addr);
    void cycle();
}
