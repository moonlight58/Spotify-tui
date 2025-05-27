#include <ncurses.h>

#include "tui.h"
#include "event.h"

int main()
{
    initscr();
    keypad(stdscr, TRUE); // Enable function keys and arrow keys
    noecho();
    cbreak();
    curs_set(0);

    setup_colors();

    int height, width;
    getmaxyx(stdscr, height, width);

    WindowLayout layouts[6];
    calculate_layout(height, width, layouts);

    WINDOW *search_bar = create_window_with_layout(layouts[0], 1, "Search");
    WINDOW *help_bar = create_window_with_layout(layouts[5], 2, "Help");
    WINDOW *library_win = create_window_with_layout(layouts[1], 3, "Library");
    WINDOW *playlist_win = create_window_with_layout(layouts[2], 4, "Playlists");
    WINDOW *main_win = create_window_with_layout(layouts[3], 5, "Welcome!");
    WINDOW *progress_bar = create_window_with_layout(layouts[4], 6, "Progress Bar");

    // Affiche le texte d'aide dans la help_bar
    mvwprintw(help_bar, 1, 1, "Type ?");

    refresh();
    wrefresh(search_bar);
    wrefresh(help_bar);
    wrefresh(library_win);
    wrefresh(playlist_win);
    wrefresh(main_win);
    wrefresh(progress_bar);

    // Render welcome message
    render_welcome(main_win);

    handle_events(&search_bar, &help_bar, &library_win, &playlist_win, &main_win, &progress_bar);

    delwin(search_bar);
    delwin(help_bar);
    delwin(library_win);
    delwin(playlist_win);
    delwin(main_win);
    delwin(progress_bar);
    endwin();
    return 0;
}