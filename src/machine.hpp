#pragma once

#include <array>
#include <vector>

using t_addr = unsigned long;

namespace machine {
    void init();
    void set_program_counter(t_addr);
    t_addr get_program_counter();
    unsigned long get_step_counter();
    unsigned long get_cycle_counter();
    void print_info();
    char read_memory(t_addr);
    void load_program(const std::vector<char>&, t_addr);
    int load_program_from_file(const std::string&, t_addr);
    void reset();
    void cycle();
    void halt();
    bool is_halted();
    void resume();
}
