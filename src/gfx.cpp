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

const auto not_a_color = char(0xff);
const auto line_width = 160u;
const auto line_start = 68u;

using std::cout;

namespace {
    unsigned hor_cnt;
    unsigned ver_cnt;
    bool vsyncing;
    bool wsync_next_line;

    char background_color;
    char resmp[2];
}

class t_object {
protected:
    std::vector<unsigned> decoders;
    unsigned width;
    unsigned width_cnt;
    unsigned pos_cnt;
    char graphics;
    char delayed_graphics;
    unsigned delay_cnt;
    bool delayed;
    char offset;
    char color;
    bool reflected;

    void increment() {
        pos_cnt = (pos_cnt + 1) % line_width;
    }

    virtual char get_color() {
        auto idx = 8 * (width_cnt - 1) / width;

        if (reflected) {
            idx = 7 - idx;
        }

        auto val = graphics;
        if (delayed) {
            val = delayed_graphics;
        }
        if (get_bit(val, int(idx)) == 1) {
            return color;
        }
        return not_a_color;
    }

public:
    virtual void init() {
        decoders = {0};
        width = 0;
        width_cnt = 0;
        pos_cnt = 0;
        offset = 0;
        color = 0;
        reflected = false;
        delay_cnt = 0;
        delayed = false;
        delayed_graphics = 0;
        graphics = 0;
    }

    void set_decoders(const std::vector<unsigned>& val) {
        decoders = val;
    }

    void reset() {
        auto cc = 3 * machine::get_cycle_counter() - 1;
        pos_cnt = cc;
        if (hor_cnt + cc >= line_start && hor_cnt < line_start) {
            pos_cnt += hor_cnt;
            pos_cnt -= line_start;
        }
        pos_cnt = line_width - pos_cnt;
    }

    void set_width(unsigned val) {
        width = val;
    }

    void set_graphics(char val)  {
        graphics = val;
        delay_cnt = line_width;
    }

    void set_enabled(bool val) {
        if (val == true) {
            set_graphics(0xff);
        } else {
            set_graphics(0x00);
        }
    }

    void set_offset(char val) {
        offset = val;
    }

    void set_color(char val) {
        color = val;
    }

    void set_delayed(bool val) {
        delayed = val;
    }

    void set_reflected(bool val) {
        reflected = val;
    }

    void move() {
        offset >>= 4;
        if (offset < 8u) {
            pos_cnt = (pos_cnt + offset) % line_width;
        } else {
            offset = ~offset;
            offset = offset & 0x0fu;
            offset++;
            pos_cnt = (pos_cnt + line_width - offset) % line_width;
        }
    }

    char color_cycle() {
        if (delay_cnt != 0) {
            delay_cnt--;
            if (delay_cnt == 0) {
                delayed_graphics = graphics;
            }
        }

        auto& v = decoders;
        if (std::find(v.begin(), v.end(), pos_cnt) != v.end()) {
            width_cnt = width;
        }

        char ret = not_a_color;
        if (width_cnt != 0) {
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
};

class t_playfield : public t_object {
    std::array<char, 3> reg;
    bool score_mode;
    char score_mode_left_color;
    char score_mode_right_color;
    bool priority;

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
        char res = not_a_color;
        if (pf0 || pf1 || pf2) {
            if (score_mode) {
                if (pos_cnt < line_width / 2) {
                    res = score_mode_left_color;
                } else {
                    res = score_mode_right_color;
                }
            } else {
                res = color;
            }
        }
        return res;
    }

public:
    void init() {
        t_object::init();
        std::fill(reg.begin(), reg.end(), 0);
        score_mode = false;
        score_mode_left_color = 0;
        score_mode_right_color = 0;
        priority = false;
    }

    void set_register(unsigned idx, char val) {
        if (idx < 3) {
            reg[idx] = val;
        }
    }

    void set_score_mode(bool val) {
        score_mode = val;
    }

    void set_score_mode_left_color(char val) {
        score_mode_left_color = val;
    }

    void set_score_mode_right_color(char val) {
        score_mode_right_color = val;
    }

    void set_priority(bool val) {
        priority = val;
    }
};

namespace {
    t_player plr[2];
    t_missile msl[2];
    t_playfield plf;
    bool playfield_priority;
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

    void set_vsync(bool on) {
        if (vsyncing == false && on) {
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
        } else if (vsyncing && on == false) {
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
    const unsigned width_table[] = { 1, 2, 4, 8 };

    auto set_number_size = [&](unsigned idx, char val) {
        msl[idx].set_width(width_table[val >> 4]);

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
        set_vsync(get_bit(val, 1));
        break;

    case 0x01:
        break;

    case 0x02:
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
        plf.set_score_mode_left_color(val);
        break;

    case 0x07:
        plr[1].set_color(val);
        plf.set_score_mode_right_color(val);
        break;

    case 0x08:
        plf.set_color(val);
        ball.set_color(val);
        break;

    case 0x09:
        background_color = val;
        break;

    case 0x0a:
        plf.set_reflected(get_bit(val, 0));
        plf.set_score_mode(get_bit(val, 1));
        playfield_priority = get_bit(val, 2);
        ball.set_width(width_table[val >> 4]);
        break;

    case 0x0b:
        plr[0].set_reflected(get_bit(val, 3));
        break;

    case 0x0c:
        plr[1].set_reflected(get_bit(val, 3));
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
        msl[0].set_enabled(get_bit(val, 1));
        break;

    case 0x1e:
        msl[1].set_enabled(get_bit(val, 1));
        break;

    case 0x1f:
        ball.set_enabled(get_bit(val, 1));
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
        plr[0].set_delayed(get_bit(val, 0));
        break;

    case 0x26:
        plr[1].set_delayed(get_bit(val, 0));
        break;

    case 0x27:
        ball.set_delayed(get_bit(val, 0));
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

    case 0x0c:
        res = sdl::get_key(sdl::key_left_trigger);
        res ^= 1;
        res <<= 7;
        break;

    }

    return res;
}

bool gfx::init() {
    hor_cnt = 0;
    ver_cnt = 0;
    vsyncing = false;

    plf.init();
    plf.set_width(line_width);
    playfield_priority = false;
    plr[0].init();
    plr[0].set_width(8);
    plr[1].init();
    plr[1].set_width(8);
    msl[0].init();
    msl[0].set_width(1);
    msl[1].init();
    msl[1].set_width(1);
    ball.init();
    ball.set_width(1);

    background_color = 0;
    resmp[0] = 0;
    resmp[1] = 0;
    cxclr();

    wsync_next_line = false;

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

void gfx::set_frames_per_second(unsigned val) {
    sdl::set_frames_per_second(val);
}

void gfx::print_info() {
}

void gfx::cycle() {
    // if (hor_cnt == 0) {
    //     std::cout << "scanline " << ver_cnt << "\n";
    // }

    if (hor_cnt >= line_start) {
        char color = background_color;
        auto add_color = [&](char new_color) {
            if (new_color != not_a_color) {
                color = new_color;
            }
        };

        auto pf = plf.color_cycle();
        auto bl = ball.color_cycle();
        auto p0 = plr[0].color_cycle();
        auto p1 = plr[1].color_cycle();
        auto m0 = msl[0].color_cycle();
        auto m1 = msl[1].color_cycle();

        auto set_cx = [&](bool& cx, char c0, char c1) {
            if (c0 != not_a_color && c1 != not_a_color) {
                cx = true;
            }
        };

        set_cx(cxm0p1, m0, p1);
        set_cx(cxm0p0, m0, p0);
        set_cx(cxm1p0, m1, p0);
        set_cx(cxm1p1, m1, p1);
        set_cx(cxp0pf, p0, pf);
        set_cx(cxp0bl, p0, bl);
        set_cx(cxp1pf, p1, pf);
        set_cx(cxp1bl, p1, bl);
        set_cx(cxm0pf, m0, pf);
        set_cx(cxm0bl, m0, bl);
        set_cx(cxm1pf, m1, pf);
        set_cx(cxm1bl, m1, bl);
        set_cx(cxblpf, bl, pf);
        set_cx(cxp0p1, p0, p1);
        set_cx(cxm0m1, m0, m1);

        if (playfield_priority) {
            add_color(p1);
            add_color(m1);
            add_color(p0);
            add_color(m0);
            add_color(pf);
            add_color(bl);
        } else {
            add_color(pf);
            add_color(bl);
            add_color(p1);
            add_color(m1);
            add_color(p0);
            add_color(m0);
        }

        if (ver_cnt >= 40) {
            sdl::send_pixel(color);
        }
    }
    hor_cnt++;
    if (hor_cnt == line_width + line_start) {
        hor_cnt = 0;
        if (machine::is_halted()) {
            wsync_next_line = true;
        }
        ver_cnt++;
    }
    if (hor_cnt == 6 && wsync_next_line) {
        machine::resume();
        wsync_next_line = false;
    }
}
