#pragma once

namespace sdl {
    bool init();
    bool is_running();
    bool is_waiting();
    void render();
    void poll();
    void begin_drawing();
    void send_pixel(char);
    void close();
}
