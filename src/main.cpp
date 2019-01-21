#include <iostream>
#include <chrono>
#include <cstdio>

#include "gfx.hpp"
#include "machine.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "invalid arguments\n";
        return 1;
    }

    machine::init();

    auto ret = machine::load_program_from_file(argv[1], 0xf000);
    if (ret < 0) {
        std::cout << "could not load file\n";
        return 1;
    }

    gfx::init();
    while (gfx::is_running()) {
        gfx::poll();

        if (gfx::is_waiting() == false) {
            gfx::cycle();
            gfx::cycle();
            gfx::cycle();
            machine::cycle();
        }
    }

    gfx::close();
}
