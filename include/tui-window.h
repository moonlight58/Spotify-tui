#ifndef TUI_WINDOW_H
#define TUI_WINDOW_H

#include <ncurses.h>

typedef struct
{
    WINDOW *window;
    int x, y, w, h;
    int grid_x, grid_y;
    bool is_focusable;
    int focus_color_pair;
    int unfocus_color_pair;
    const char *title;
} TuiWindow;

#define MAX_WINDOWS 10

int get_window_count(void);
TuiWindow* get_windows(void);
TuiWindow* get_window(int index);
void render_windows_with_focus(int focused_index);
int find_window_by_grid(int grid_x, int grid_y);
void init_windows(WINDOW *search, WINDOW *help, WINDOW *library, WINDOW *playlist, WINDOW *main, WINDOW *progress);

#endif /* TUI_WINDOW_H */