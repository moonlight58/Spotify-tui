#ifndef TUI_H
#define TUI_H

#include <ncurses.h>

typedef struct {
    int height;
    int width;
    int start_y;
    int start_x;
} WindowLayout;

void setup_colors();
void calculate_layout(int screen_height, int screen_width, WindowLayout layouts[5]);
WINDOW* create_window_with_layout(WindowLayout layout, int color_pair, const char* title);
void render_welcome(WINDOW *main_win);

#endif