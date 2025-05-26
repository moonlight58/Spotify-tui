#include <ncurses.h>
#include "tui.h"

int main() {
    initscr();
    noecho();
    cbreak();
    curs_set(0);

    setup_colors();

    int height, width;
    getmaxyx(stdscr, height, width);

    WindowLayout layouts[5];
    calculate_layout(height, width, layouts);

    WINDOW *search_bar = create_window_with_layout(layouts[0], 1, "Search");
    WINDOW *library_win = create_window_with_layout(layouts[1], 2, "Library");
    WINDOW *playlist_win = create_window_with_layout(layouts[2], 3, "Playlists");
    WINDOW *main_win = create_window_with_layout(layouts[3], 4, "Main Content Area");
    WINDOW *progress_bar = create_window_with_layout(layouts[4], 5, "Progress Bar");

    refresh();
    wrefresh(search_bar);
    wrefresh(library_win);
    wrefresh(playlist_win);
    wrefresh(main_win);
    wrefresh(progress_bar);

    getch();

    delwin(search_bar);
    delwin(library_win);
    delwin(playlist_win);
    delwin(main_win);
    delwin(progress_bar);
    endwin();
    return 0;
}