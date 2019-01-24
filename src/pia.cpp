#include "misc.hpp"
#include "pia.hpp"
#include "sdl.hpp"

class t_timer {
    unsigned interval;
    unsigned interval_cnt;
    char cnt;

public:
    void set(char new_cnt, unsigned new_interval) {
        cnt = new_cnt;
        interval = new_interval;
        interval_cnt = 0;
    }

    char read() {
        return cnt;
    }

    void cycle() {
        interval_cnt++;
        if (interval_cnt == interval) {
            interval_cnt = 0;
            if (cnt == 0) {
                interval = 1;
            }
            cnt--;
        }
    }
};

namespace {
    t_timer timer;
}

void pia::init() {
    timer.set(0, 1);
}

void pia::set(t_addr addr, char val) {
    switch (addr) {

    case 0x294:
        timer.set(val, 1);
        break;

    case 0x295:
        timer.set(val, 8);
        break;

    case 0x296:
        timer.set(val, 64);
        break;

    case 0x297:
        timer.set(val, 1024);
        break;

    }
}

char pia::get(t_addr addr) {
    char res;
    switch (addr) {

    case 0x280:
        res = 0xff;
        set_bit(res, 7, not sdl::get_key(sdl::key_right));
        set_bit(res, 6, not sdl::get_key(sdl::key_left));
        set_bit(res, 5, not sdl::get_key(sdl::key_down));
        set_bit(res, 4, not sdl::get_key(sdl::key_up));
        break;

    case 0x284:
        res = timer.read();
        break;

    default:
        res = 0x00;
        break;

    }
    return res;
}

void pia::cycle() {
    timer.cycle();
}
