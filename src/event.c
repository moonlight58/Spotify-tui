#include "event.h"
#include "tui.h"
#include <ncurses.h>

void handle_events(WINDOW **search_bar, WINDOW **help_bar,
                   WINDOW **library_win, WINDOW **playlist_win,
                   WINDOW **main_win, WINDOW **progress_bar) {
    int ch;
    int height, width;
    WindowLayout layouts[6];

    while ((ch = getch()) != 'q') {
        switch (ch) {
        case KEY_RESIZE:
            // Get new terminal size
            getmaxyx(stdscr, height, width);
            // Recalculate layout
            calculate_layout(height, width, layouts);

            // Delete old windows
            delwin(*search_bar);
            delwin(*help_bar);
            delwin(*library_win);
            delwin(*playlist_win);
            delwin(*main_win);
            delwin(*progress_bar);

            // Create new windows
            *search_bar   = create_window_with_layout(layouts[0], 1, "Search");
            *help_bar     = create_window_with_layout(layouts[5], 2, NULL);
            *library_win  = create_window_with_layout(layouts[1], 3, "Library");
            *playlist_win = create_window_with_layout(layouts[2], 4, "Playlists");
            *main_win     = create_window_with_layout(layouts[3], 5, "Main Content Area");
            *progress_bar = create_window_with_layout(layouts[4], 1, "Progress Bar");

            mvwprintw(*help_bar, 1, 1, "Type h for help...");
            // Redraw all
            refresh();
            wrefresh(*search_bar);
            wrefresh(*help_bar);
            wrefresh(*library_win);
            wrefresh(*playlist_win);
            wrefresh(*main_win);
            wrefresh(*progress_bar);;

            break;
        case '?':
            // Open help window with the entire list of commands
            wrefresh(*help_bar);
            break;
        case 's':
            mvwprintw(*search_bar, 1, 1, "Search: ");
            wrefresh(*search_bar);
            break;
        default:
            break;
        }
    }
}