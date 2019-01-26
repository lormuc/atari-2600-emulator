#include <iostream>
#include <chrono>
#include <string>
#include <cstdio>

#include "gfx.hpp"
#include "pia.hpp"
#include "machine.hpp"

int main(int argc, char** argv) {
    if (argc < 2 || argc >= 4) {
        std::cout << "invalid arguments\n";
        return 1;
    }
    unsigned long fps = 60;
    if (argc == 3) {
        fps = std::stoul(argv[2]);
    }

    machine::init();

    auto ret = machine::load_program_from_file(argv[1], 0xf000);
    if (ret < 0) {
        std::cout << "could not load file\n";
        return 1;
    }

    pia::init();
    gfx::init();
    gfx::set_frames_per_second(fps);
    while (gfx::is_running()) {
        gfx::poll();

        if (gfx::is_waiting() == false) {
            gfx::cycle();
            gfx::cycle();
            gfx::cycle();
            machine::cycle();
            pia::cycle();
        }
    }

    gfx::close();
}
