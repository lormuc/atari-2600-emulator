#pragma once

#include <chrono>

class t_millisecond_timer {
    std::chrono::time_point<std::chrono::steady_clock> t0;
public:
    void reset() {
        t0 = std::chrono::steady_clock::now();
    }
    long long int get_ticks() {
        auto t1 = std::chrono::steady_clock::now();
        auto dt = t1 - t0;
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(dt);
        return dur.count();
    }
};

bool get_bit(char, int);
void set_bit(char&, int, bool);
void print_hex(char);
void print_hex(unsigned long);
