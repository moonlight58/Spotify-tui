#include <ncurses.h>
#include <string.h>
#include "tui-window.h"

static TuiWindow windows[MAX_WINDOWS];
static int window_count = 5;

int get_window_count(void) { return window_count; }
TuiWindow *get_windows(void) { return windows; }
TuiWindow *get_window(int index) { return &windows[index]; }

void init_windows(WINDOW *search, WINDOW *help, WINDOW *library, WINDOW *playlist, WINDOW *main, WINDOW *progress)
{
    window_count = 6;
    windows[0] = (TuiWindow){search, 0, 0, 0, 0, 0, 0, true, 15, 1, "Search"};
    windows[1] = (TuiWindow){help, 0, 0, 0, 0, -1, -1, false, 16, 2, "Help"};
    windows[2] = (TuiWindow){library, 0, 0, 0, 0, 0, 1, true, 17, 3, "Library"};
    windows[3] = (TuiWindow){playlist, 0, 0, 0, 0, 0, 2, true, 18, 4, "Playlists"};
    windows[4] = (TuiWindow){main, 0, 0, 0, 0, 1, 1, true, 19, 5, "Welcome!"};
    windows[5] = (TuiWindow){progress, 0, 0, 0, 0, -1, -1, false, 20, 6, "Progress Bar"};
}

void render_windows_with_focus(int focused_index)
{
    for (int i = 0; i < get_window_count(); ++i)
    {
        TuiWindow *win = get_window(i);
        int color = (i == focused_index) ? win->focus_color_pair : win->unfocus_color_pair;
        wattron(win->window, COLOR_PAIR(color));
        box(win->window, 0, 0);
        if (win->title)
        {
            int max_x = getmaxx(win->window);
            int title_len = strlen(win->title);
            mvwprintw(win->window, 0, 1, "%-*s", title_len, win->title); // overwrite only title area
        }
        wattroff(win->window, COLOR_PAIR(color));
        wrefresh(win->window);
    }
}

int find_window_by_grid(int grid_x, int grid_y)
{
    for (int i = 0; i < get_window_count(); ++i)
    {
        TuiWindow *win = get_window(i);
        if (win->grid_x == grid_x && win->grid_y == grid_y)
            return i;
    }
    return -1;
}