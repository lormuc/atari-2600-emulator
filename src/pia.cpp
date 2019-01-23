#include "pia.hpp"

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
    if (addr == 0x284) {
        return timer.read();
    }
    return 0x00;
}

void pia::cycle() {
    timer.cycle();
}
