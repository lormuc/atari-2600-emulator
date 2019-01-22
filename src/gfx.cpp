#include <array>
#include <iostream>
#include <algorithm>
#include <string>
#include <chrono>
#include <cstdio>

#include "misc.hpp"
#include "gfx.hpp"
#include "machine.hpp"
#include "sdl.hpp"

const char not_a_color = 0xff;

class t_object {
protected:
    std::vector<unsigned> decoders;
    unsigned width;
    unsigned width_cnt;
    unsigned pos_cnt;
    bool enabled;
    char offset;
    char color;

    void increment() {
        pos_cnt = (pos_cnt + 1) % 160;
    }

    virtual char get_color() {
        return color;
    }

public:
    virtual void init() {
        decoders = {0};
        width = 0;
        width_cnt = 0;
        pos_cnt = 0;
        enabled = false;
        offset = 0;
        color = 0;
    }

    void set_decoders(const std::vector<unsigned>& val) {
        decoders = val;
    }

    void reset() {
        pos_cnt = 0;
        pos_cnt = 160 - 8;
    }

    void set_width(unsigned val) {
        width = val;
    }

    void set_enabled(bool val) {
        enabled = val;
    }

    void set_offset(char val) {
        offset = val;
    }

    void set_color(char val) {
        color = val;
    }

    void move() {
        offset >>= 4;
        if (offset < 8u) {
            pos_cnt = (pos_cnt + offset) % 160;
        } else {
            offset = ~offset;
            offset = offset & 0x0fu;
            offset++;
            pos_cnt = (pos_cnt + 160 - offset) % 160;
        }
    }

    char color_cycle() {
        auto& v = decoders;
        if (std::find(v.begin(), v.end(), pos_cnt) != v.end()) {
            width_cnt = width;
        }

        char ret = not_a_color;
        if (enabled && width_cnt != 0) {
            ret = get_color();
            width_cnt--;
        }

        increment();

        return ret;
    }
};

class t_ball : public t_object {
};

class t_missile : public t_object {
};

class t_player : public t_object {
    bool reflected;
    char graphics;

    char get_color() {
        auto idx = 8 * (width_cnt - 1) / width;

        if (reflected) {
            idx = 7 - idx;
        }

        if (get_bit(graphics, int(idx)) == 1) {
            return color;
        }
        return not_a_color;
    }
public:
    void init() {
        t_object::init();
        reflected = false;
        graphics = 0;
    }

    void set_reflected(bool val) {
        reflected = val;
    }

    void set_graphics(char val)  {
        graphics = val;
    }
};

class t_playfield : public t_object {
    bool reflected;
    char reg[3];

    char get_color() {
        auto j = pos_cnt / 4;
        if (j >= 20) {
            j -= 20;
            if (reflected) {
                j = 19 - j;
            }
        }
        bool pf0 = j < 4u && get_bit(reg[0], 4 + j) == 1;
        bool pf1 = j >= 4 && j < 12 && get_bit(reg[1], (7 - (j - 4))) == 1;
        bool pf2 = j >= 12 && get_bit(reg[2], j - 12) == 1;
        if (pf0 || pf1 || pf2) {
            return color;
        }
        return not_a_color;
    }

public:
    void init() {
        t_object::init();
        reflected = false;
        reg[0] = 0;
        reg[1] = 0;
        reg[2] = 0;
    }

    void set_reflected(bool val) {
        reflected = val;
    }

    void set_register(unsigned idx, char val) {
        if (idx < 3) {
            reg[idx] = val;
        }
    }
};

namespace {
    unsigned hor_cnt;
    unsigned ver_cnt;
    bool vsyncing;

    unsigned halt_line;

    char background_color;
    char vdelp[2];
    char vdelbl;
    char resmp[2];

    t_player plr[2];
    t_missile msl[2];
    t_playfield plf;
    t_ball ball;

    bool cxm0p1;
    bool cxm0p0;
    bool cxm1p0;
    bool cxm1p1;
    bool cxp0pf;
    bool cxp0bl;
    bool cxp1pf;
    bool cxp1bl;
    bool cxm0pf;
    bool cxm0bl;
    bool cxm1pf;
    bool cxm1bl;
    bool cxblpf;
    bool cxp0p1;
    bool cxm0m1;

    void set_vsync(char val) {
        auto b = get_bit(val, 1);
        if (vsyncing == false && b == 1) {
            static bool initial = true;
            if (initial) {
                sdl::begin_drawing();
                initial = false;
            } else {
                sdl::render();
            }

            ver_cnt = 0;
            // plr[0].reset();
            // plr[1].reset();
            // msl[0].reset();
            // msl[1].reset();
            // ball.reset();
            // plf.reset();
            vsyncing = true;
        } else if (vsyncing == true && b == 0) {
            vsyncing = false;
        }
    }

    void cxclr() {
        cxm0p1 = 0;
        cxm0p0 = 0;
        cxm1p0 = 0;
        cxm1p1 = 0;
        cxp0pf = 0;
        cxp0bl = 0;
        cxp1pf = 0;
        cxp1bl = 0;
        cxm0pf = 0;
        cxm0bl = 0;
        cxm1pf = 0;
        cxm1bl = 0;
        cxblpf = 0;
        cxp0p1 = 0;
        cxm0m1 = 0;
    }
}

void gfx::set(char addr, char val) {
    auto set_number_size = [&](unsigned idx, char val) {
        switch (val >> 4) {
        case 0: msl[idx].set_width(1); break;
        case 1: msl[idx].set_width(2); break;
        case 2: msl[idx].set_width(4); break;
        case 3: msl[idx].set_width(8); break;
        }

        auto ns = val & 0x07u;
        std::vector<unsigned> decoders;
        if (ns == 0) {
            decoders = {0};
        } else if (ns == 1) {
            decoders = {0, 16};
        } else if (ns == 2) {
            decoders = {0, 32};
        } else if (ns == 3) {
            decoders = {0, 16, 32};
        } else if (ns == 4) {
            decoders = {0, 64};
        } else if (ns == 6) {
            decoders = {0, 32, 64};
        }

        msl[idx].set_decoders(decoders);
        plr[idx].set_decoders(decoders);

        if (ns == 5) {
            plr[idx].set_width(16);
        } else if (ns == 6) {
            plr[idx].set_width(32);
        } else {
            plr[idx].set_width(8);
        }
    };

    switch (addr) {

    case 0x00:
        set_vsync(val);
        break;

    case 0x01:
        break;

    case 0x02:
        halt_line = ver_cnt;
        machine::halt();
        break;

    case 0x03:
        hor_cnt = 0;
        break;

    case 0x04:
        set_number_size(0, val);
        break;

    case 0x05:
        set_number_size(1, val);
        break;

    case 0x06:
        plr[0].set_color(val);
        break;

    case 0x07:
        plr[1].set_color(val);
        break;

    case 0x08:
        plf.set_color(val);
        break;

    case 0x09:
        background_color = val;
        break;

    case 0x0a:
        plf.set_reflected(get_bit(val, 0));
        ball.set_width(val >> 4);
        break;

    case 0x0b:
        plr[0].set_reflected(val);
        break;

    case 0x0c:
        plr[1].set_reflected(val);
        break;

    case 0x0d:
        plf.set_register(0, val);
        break;

    case 0x0e:
        plf.set_register(1, val);
        break;

    case 0x0f:
        plf.set_register(2, val);
        break;

    case 0x10:
        plr[0].reset();
        break;

    case 0x11:
        plr[1].reset();
        break;

    case 0x12:
        msl[0].reset();
        break;

    case 0x13:
        msl[1].reset();
        break;

    case 0x14:
        ball.reset();
        break;

    case 0x1b:
        plr[0].set_graphics(val);
        break;

    case 0x1c:
        plr[1].set_graphics(val);
        break;

    case 0x1d:
        msl[0].set_enabled(val);
        break;

    case 0x1e:
        msl[1].set_enabled(val);
        break;

    case 0x1f:
        ball.set_enabled(val);
        break;

    case 0x20:
        plr[0].set_offset(val);
        break;

    case 0x21:
        plr[1].set_offset(val);
        break;

    case 0x22:
        msl[0].set_offset(val);
        break;

    case 0x23:
        msl[1].set_offset(val);
        break;

    case 0x24:
        ball.set_offset(val);
        break;

    case 0x25:
        vdelp[0] = val;
        break;

    case 0x26:
        vdelp[1] = val;
        break;

    case 0x27:
        vdelbl = val;
        break;

    case 0x28:
        resmp[0] = val;
        break;

    case 0x29:
        resmp[1] = val;
        break;

    case 0x2a:
        plr[0].move();
        plr[1].move();
        msl[0].move();
        msl[1].move();
        ball.move();
        break;

    case 0x2b:
        plr[0].set_offset(0);
        plr[1].set_offset(0);
        msl[0].set_offset(0);
        msl[1].set_offset(0);
        ball.set_offset(0);
        break;

    case 0x2c:
        cxclr();
        break;

    };
}

char gfx::get(char addr) {
    auto cx = [](char d7, char d6) -> char {
        return (d7 << 7) | (d6 << 6);
    };

    char res = 0;

    switch (addr) {

    case 0x00:
        res = cx(cxm0p1, cxm0p0);
        break;

    case 0x01:
        res = cx(cxm1p0, cxm1p1);
        break;

    case 0x02:
        res = cx(cxp0pf, cxp0bl);
        break;

    case 0x03:
        res = cx(cxp1pf, cxp1bl);
        break;

    case 0x04:
        res = cx(cxm0pf, cxm0bl);
        break;

    case 0x05:
        res = cx(cxm1pf, cxm1bl);
        break;

    case 0x06:
        res = cx(cxblpf, 0);
        break;

    case 0x07:
        res = cx(cxp0p1, cxm0m1);
        break;

    }

    return res;
}

void gfx::cycle() {
    if (hor_cnt >= 68) {
        char color = background_color;
        auto add_color = [&](char new_color) {
            if (new_color != not_a_color) {
                color = new_color;
            }
        };
        add_color(plf.color_cycle());
        add_color(ball.color_cycle());
        add_color(plr[1].color_cycle());
        add_color(msl[1].color_cycle());
        add_color(plr[0].color_cycle());
        add_color(msl[0].color_cycle());

        if (ver_cnt >= 40) {
            sdl::send_pixel(color);
        }
    }
    hor_cnt++;
    if (hor_cnt == 228) {
        hor_cnt = 0;
        ver_cnt++;
    }
    if (hor_cnt == 6 && ver_cnt == halt_line + 1) {
        machine::resume();
        halt_line = -2;
    }
}

bool gfx::init() {
    hor_cnt = 0;
    ver_cnt = 0;
    vsyncing = false;

    plf.init();
    plf.set_width(160);
    plf.set_enabled(true);

    plr[0].init();
    plr[0].set_enabled(true);
    plr[0].set_width(8);
    plr[1].init();
    plr[1].set_enabled(true);
    plr[1].set_width(8);
    msl[0].init();
    msl[0].set_width(1);
    msl[1].init();
    msl[1].set_width(1);
    ball.init();
    ball.set_width(1);

    background_color = 0;
    vdelp[0] = 0;
    vdelp[1] = 0;
    vdelbl = 0;
    resmp[0] = 0;
    resmp[1] = 0;
    cxclr();

    halt_line = -2;

    auto success = sdl::init();

    return success;
}

void gfx::close() {
    sdl::close();
}

void gfx::poll() {
    sdl::poll();
}

bool gfx::is_running() {
    return sdl::is_running();
}

bool gfx::is_waiting() {
    return sdl::is_waiting();
}

void gfx::print_info() {
}
