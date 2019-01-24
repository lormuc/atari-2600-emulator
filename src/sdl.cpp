#include <array>
#include <iostream>
#include <cstdio>

#include <SDL2/SDL.h>

#include "misc.hpp"
#include "sdl.hpp"

const auto frames_per_second = 60;
const auto monochrome = false;

const auto out_scr_width = 640u;
const auto out_scr_height = 384u;

// const auto out_scr_width = 320u;
// const auto out_scr_height = 192u;

const auto in_scr_width = 160u;
const auto in_scr_height = 192u;

const int sdl::key_right = SDL_SCANCODE_KP_6;
const int sdl::key_left = SDL_SCANCODE_KP_4;
const int sdl::key_down = SDL_SCANCODE_KP_5;
const int sdl::key_up = SDL_SCANCODE_KP_8;

std::array<bool, 1024> keyboard_state;

const char palette[0x80][3] = {
    // 0
    {0x00, 0x00, 0x00},
    {0x1a, 0x1a, 0x1a},
    {0x39, 0x39, 0x39},
    {0x5b, 0x5b, 0x5b},
    {0x7e, 0x7e, 0x7e},
    {0xa2, 0xa2, 0xa2},
    {0xc7, 0xc7, 0xc7},
    {0xed, 0xed, 0xed},

    // 1
    {0x19, 0x02, 0x00},
    {0x3a, 0x1f, 0x00},
    {0x5d, 0x41, 0x00},
    {0x82, 0x64, 0x00},
    {0xa7, 0x88, 0x00},
    {0xcc, 0xad, 0x00},
    {0xf2, 0xd2, 0x19},
    {0xfe, 0xfa, 0x40},

    // 2
    {0x37, 0x00, 0x00},
    {0x5e, 0x08, 0x00},
    {0x83, 0x27, 0x00},
    {0xa9, 0x49, 0x00},
    {0xcf, 0x6c, 0x00},
    {0xf5, 0x8f, 0x00},
    {0xfe, 0xb4, 0x38},
    {0xfe, 0xdf, 0x6f},

    // 3
    {0x47, 0x00, 0x00},
    {0x73, 0x00, 0x00},
    {0x98, 0x13, 0x00},
    {0xbe, 0x32, 0x16},
    {0xe4, 0x53, 0x35},
    {0xfe, 0x76, 0x57},
    {0xfe, 0x9c, 0x81},
    {0xfe, 0xc6, 0xbb},

    // 4
    {0x44, 0x00, 0x00},
    {0x6f, 0x00, 0x1f},
    {0x96, 0x06, 0x40},
    {0xbb, 0x24, 0x62},
    {0xe1, 0x45, 0x85},
    {0xfe, 0x67, 0xaa},
    {0xfe, 0x8c, 0xd6},
    {0xfe, 0xb7, 0xf6},

    // 5
    {0x2d, 0x00, 0x4a},
    {0x57, 0x00, 0x67},
    {0x7d, 0x05, 0x8c},
    {0xa1, 0x22, 0xb1},
    {0xc7, 0x43, 0xd7},
    {0xed, 0x65, 0xfe},
    {0xfe, 0x8a, 0xf6},
    {0xfe, 0xb5, 0xf7},

    // 6
    {0x0d, 0x00, 0x82},
    {0x33, 0x00, 0xa2},
    {0x55, 0x0f, 0xc9},
    {0x78, 0x2d, 0xf0},
    {0x9c, 0x4e, 0xfe},
    {0xc3, 0x72, 0xfe},
    {0xeb, 0x98, 0xfe},
    {0xfe, 0xc0, 0xf9},

    // 7
    {0x00, 0x00, 0x91},
    {0x0a, 0x05, 0xbd},
    {0x28, 0x22, 0xe4},
    {0x48, 0x42, 0xfe},
    {0x6b, 0x64, 0xfe},
    {0x90, 0x8a, 0xfe},
    {0xb7, 0xb0, 0xfe},
    {0xdf, 0xd8, 0xfe},

    // 8
    {0x00, 0x00, 0x72},
    {0x0a, 0x1c, 0xab},
    {0x03, 0x3c, 0xd6},
    {0x20, 0x5e, 0xfd},
    {0x40, 0x81, 0xfe},
    {0x64, 0xa6, 0xfe},
    {0x89, 0xce, 0xfe},
    {0xb0, 0xf6, 0xfe},

    // 9
    {0x00, 0x10, 0x3a},
    {0x00, 0x31, 0x6e},
    {0x00, 0x55, 0xa2},
    {0x05, 0x79, 0xc8},
    {0x23, 0x9d, 0xee},
    {0x44, 0xc2, 0xfe},
    {0x68, 0xe9, 0xfe},
    {0x8f, 0xfe, 0xfe},

    // a
    {0x00, 0x10, 0x02},
    {0x00, 0x43, 0x26},
    {0x00, 0x69, 0x57},
    {0x00, 0x8d, 0x7a},
    {0x1b, 0xb1, 0x9e},
    {0x3b, 0xd7, 0xc3},
    {0x5d, 0xfe, 0xe9},
    {0x86, 0xfe, 0xfe},

    // b
    {0x00, 0x24, 0x03},
    {0x00, 0x4a, 0x05},
    {0x00, 0x70, 0x0c},
    {0x09, 0x95, 0x2b},
    {0x28, 0xba, 0x4c},
    {0x49, 0xe0, 0x6e},
    {0x6c, 0xfe, 0x92},
    {0x97, 0xfe, 0xb5},

    // c
    {0x00, 0x21, 0x02},
    {0x00, 0x46, 0x04},
    {0x08, 0x6b, 0x00},
    {0x28, 0x90, 0x00},
    {0x49, 0xb5, 0x09},
    {0x6b, 0xdb, 0x28},
    {0x8f, 0xfe, 0x49},
    {0xbb, 0xfe, 0x69},

    // d
    {0x00, 0x15, 0x01},
    {0x10, 0x36, 0x00},
    {0x30, 0x59, 0x00},
    {0x53, 0x7e, 0x00},
    {0x76, 0xa3, 0x00},
    {0x9a, 0xc8, 0x00},
    {0xbf, 0xee, 0x1e},
    {0xe8, 0xfe, 0x3e},

    // e
    {0x1a, 0x02, 0x00},
    {0x3b, 0x1f, 0x00},
    {0x5e, 0x41, 0x00},
    {0x83, 0x64, 0x00},
    {0xa8, 0x88, 0x00},
    {0xce, 0xad, 0x00},
    {0xf4, 0xd2, 0x18},
    {0xfe, 0xfa, 0x40},

    // f
    {0x38, 0x00, 0x00},
    {0x5f, 0x08, 0x00},
    {0x84, 0x27, 0x00},
    {0xaa, 0x49, 0x00},
    {0xd0, 0x6b, 0x00},
    {0xf6, 0x8f, 0x18},
    {0xfe, 0xb4, 0x39},
    {0xfe, 0xdf, 0x70},
};

namespace {
    SDL_Window* window;
    SDL_Renderer* renderer;

    std::array<char, in_scr_width * in_scr_height> screen;
    unsigned scr_cnt;
    long frame_cnt;
    t_millisecond_timer timer;
    bool drawing;
    bool running;
    bool frame_done;
}

void sdl::render() {
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(renderer);
    for (unsigned idx = 0; idx < screen.size(); idx++) {
        auto rgb = &palette[screen[idx] >> 1][0];
        if (monochrome) {
            auto lum = (rgb[0] + rgb[1] + rgb[2]) / 3;
            SDL_SetRenderDrawColor(renderer, lum, lum, lum, 0xff);
        } else {
            SDL_SetRenderDrawColor(renderer, rgb[0], rgb[1], rgb[2], 0xff);
        }
        auto i = idx % in_scr_width;
        auto j = idx / in_scr_width;
        auto rx = int(out_scr_width * i / in_scr_width);
        auto ry = int(out_scr_height * j / in_scr_height);
        auto rw = int(out_scr_width / in_scr_width);
        auto rh = int(out_scr_height / in_scr_height);
        SDL_Rect rect = { rx, ry, rw, rh };
        SDL_RenderFillRect(renderer, &rect);
    }
    SDL_RenderPresent(renderer);

    char buf[0x10];
    std::snprintf(buf, 0x10, "%05ld", frame_cnt);
    SDL_SetWindowTitle(window, buf);

    scr_cnt = 0;
    frame_done = true;
    frame_cnt++;
}

void sdl::send_pixel(char color) {
    if (drawing == false) {
        return;
    }
    if (scr_cnt < screen.size()) {
        screen[scr_cnt] = color;
        scr_cnt++;
    }
}

bool sdl::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init fail : " << SDL_GetError() << "\n";
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    auto wu = SDL_WINDOWPOS_UNDEFINED;
    auto sw = out_scr_width;
    auto sh = out_scr_height;
    window = SDL_CreateWindow("atari", wu, wu, sw, sh, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "create window fail : " << SDL_GetError() << "\n";
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "create renderer fail : " << SDL_GetError() << "\n";
        return false;
    }
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    std::fill(screen.begin(), screen.end(), 0x00);
    scr_cnt = 0;
    frame_cnt = 0;
    frame_done = false;
    running = true;
    drawing = false;

    std::fill(keyboard_state.begin(), keyboard_state.end(), false);

    return true;
}

void sdl::poll() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        if (event.type == SDL_KEYDOWN) {
            auto sc = event.key.keysym.scancode;
            if (sc == SDL_SCANCODE_ESCAPE) {
                running = false;
            }
            keyboard_state[sc] = true;
        }
    }
}

bool sdl::is_running() {
    return running;
}

bool sdl::is_waiting() {
    if (frame_done) {
        if (frames_per_second * timer.get_ticks() < 1000 * frame_cnt) {
            return true;
        } else {
            frame_done = false;
            return false;
        }
    }
    return false;
}

void sdl::begin_drawing() {
    drawing = true;
    timer.reset();
}

void sdl::close() {
    running = false;
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
    SDL_DestroyWindow(window);
    window = nullptr;
    SDL_Quit();
}

bool sdl::get_key(int sc) {
    auto res = keyboard_state[sc];

    auto ks = SDL_GetKeyboardState(nullptr);
    keyboard_state[sc] = ks[sc];

    return res;
}
