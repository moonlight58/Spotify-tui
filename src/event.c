#include "event.h"
#include "tui.h"
#include "tui-window.h"

#include <ncurses.h>

static int focused_window = 4;

void handle_events(WINDOW **search_bar, WINDOW **help_bar,
                   WINDOW **library_win, WINDOW **playlist_win,
                   WINDOW **main_win, WINDOW **progress_bar)
{
    int ch;
    int height, width;
    WindowLayout layouts[6];

    while ((ch = getch()) != 'q')
    {
        TuiWindow *current = get_window(focused_window);
        int nx = current->grid_x, ny = current->grid_y;

        switch (ch)
        {
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
            *search_bar = create_window_with_layout(layouts[0], 1, "Search");
            *help_bar = create_window_with_layout(layouts[5], 2, "Help");
            *library_win = create_window_with_layout(layouts[1], 3, "Library");
            *playlist_win = create_window_with_layout(layouts[2], 4, "Playlists");
            *main_win = create_window_with_layout(layouts[3], 5, "Welcome !");
            *progress_bar = create_window_with_layout(layouts[4], 1, "Progress Bar");

            mvwprintw(*help_bar, 1, 1, "Type h for help...");
            break;
        case '?':
            // Open help window with the entire list of commands
            wrefresh(*help_bar);
            break;
        case 's':
            mvwprintw(*search_bar, 1, 1, "Search: ");
            wrefresh(*search_bar);
            break;

        case KEY_UP:
            ny--;
            break;
        case KEY_DOWN:
            ny++;
            break;
        case KEY_LEFT:
            nx--;
            break;
        case KEY_RIGHT:
            nx++;
            break;
        default:
            break;
        }

        int next = find_window_by_grid(nx, ny);
        // Skip non-focusable windows
        while (next != -1 && !get_window(next)->is_focusable) {
            // Continue in the same direction
            // Example for KEY_UP:
            if (ch == KEY_UP) ny--;
            else if (ch == KEY_DOWN) ny++;
            else if (ch == KEY_LEFT) nx--;
            else if (ch == KEY_RIGHT) nx++;
            next = find_window_by_grid(nx, ny);
        }
        if (next != -1)
            focused_window = next;

        render_windows_with_focus(focused_window);
    }
}