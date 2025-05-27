#include "tui.h"
#include <stdlib.h>
#include <stdio.h>

#define COLOR_BG 8
#define COLOR_FG 9
#define COLOR_FG_ACTIVE 10

void setup_colors() {
    if (!has_colors()) {
        endwin();
        fprintf(stderr, "Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    use_default_colors();

    init_color(COLOR_BG, (4*1000)/255, (37*1000)/255, (46*1000)/255);
    init_color(COLOR_FG, (61*1000)/255, (94*1000)/255, (103*1000)/255);
    init_color(COLOR_FG_ACTIVE, (25*1000)/255, (255*1000)/255, (255*1000)/255);

    init_pair(1, COLOR_FG, COLOR_BG); // search_bar
    init_pair(2, COLOR_FG, COLOR_BG); // help_bar  
    init_pair(3, COLOR_FG, COLOR_BG); // library window
    init_pair(4, COLOR_FG, COLOR_BG); // playlist window
    init_pair(5, COLOR_FG, COLOR_BG); // main window
    init_pair(6, COLOR_FG, COLOR_BG); // progress bar
}

void calculate_layout(int screen_height, int screen_width, WindowLayout layouts[6]) {
    const int TOP_HEIGHT = 3;
    const int BOTTOM_HEIGHT = 6;
    const int HELP_WIDTH = 20; // help_bar width 
    const int LEFT_WIDTH = screen_width / 4;
    int available_height = screen_height - TOP_HEIGHT - BOTTOM_HEIGHT;
    int library_height = available_height / 2;
    int playlist_height = available_height - library_height;
    int main_width = screen_width - LEFT_WIDTH;

    layouts[0] = (WindowLayout){TOP_HEIGHT, screen_width - HELP_WIDTH, 0, 0};                   // search_bar
    layouts[1] = (WindowLayout){library_height, LEFT_WIDTH, TOP_HEIGHT, 0};                     // library window
    layouts[2] = (WindowLayout){playlist_height, LEFT_WIDTH, TOP_HEIGHT + library_height, 0};   // playlist window
    layouts[3] = (WindowLayout){available_height, main_width, TOP_HEIGHT, LEFT_WIDTH};          // main window
    layouts[4] = (WindowLayout){BOTTOM_HEIGHT, screen_width, screen_height - BOTTOM_HEIGHT, 0}; // progress_bar
    layouts[5] = (WindowLayout){TOP_HEIGHT, HELP_WIDTH, 0, screen_width - HELP_WIDTH};          // help_bar
}

WINDOW* create_window_with_layout(WindowLayout layout, int color_pair, const char* title) {
    WINDOW *win = newwin(layout.height, layout.width, layout.start_y, layout.start_x);
    wbkgd(win, COLOR_PAIR(color_pair));
    box(win, 0, 0);
    if (title) {
        mvwaddstr(win, 0, 1, title);
    }
    return win;
}