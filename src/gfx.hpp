#pragma once

namespace gfx {
    bool init();
    bool is_running();
    bool is_waiting();
    void set(char, char);
    char get(char);
    void poll();
    void cycle();
    void print_info();
    void set_frames_per_second(unsigned);
    void close();
}
