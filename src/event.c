#include "event.h"
#include "tui.h"
#include "tui-window.h"
#include "library.h"

#include <ncurses.h>
#include <string.h>
#include <ctype.h>

// Prototypes
void do_search(const char *query);
void do_library_action(int index);
void render_library_with_selector(WINDOW *win, const char **items, int count, int selected);

static int focused_window = 4;
static int selector_mode = 0; // 0: normal, 1: search input, 2: library select, 3: playlist select, etc.
static int selector_index = 0; // For list selection (library, playlist)
static char search_input[256] = {0}; // For search bar input

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

            mvwprintw(*help_bar, 1, 1, "Type ?");

            // Met à jour les pointeurs dans windows[]
            init_windows(*search_bar, *help_bar, *library_win, *playlist_win, *main_win, *progress_bar);
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

        if (!selector_mode && (ch == '\n' || ch == KEY_ENTER)) {
            switch (focused_window) {
                case 0: // Search bar
                    selector_mode = 1;
                    search_input[0] = '\0';
                    // Optionally, show a prompt
                    mvwprintw(get_window(0)->window, 1, 2, "Search: ");
                    wrefresh(get_window(0)->window);
                    break;
                case 2: // Library
                    selector_mode = 2;
                    selector_index = 0;
                    // Highlight first item
                    break;
                case 3: // Playlist
                    selector_mode = 3;
                    selector_index = 0;
                    // Highlight first playlist
                    break;
            }
        }

        if (selector_mode == 1) {
            if (ch == 27) { // ESC
                selector_mode = 0;
                // Clear prompt
                mvwprintw(get_window(0)->window, 1, 2, "%*s", 50, " ");
                wrefresh(get_window(0)->window);
            } else if (ch == '\n' || ch == KEY_ENTER) {
                // Call your search function
                do_search(search_input);
                selector_mode = 0;
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                int len = strlen(search_input);
                if (len > 0) search_input[len-1] = '\0';
            } else if (isprint(ch) && strlen(search_input) < sizeof(search_input)-1) {
                int len = strlen(search_input);
                search_input[len] = ch;
                search_input[len+1] = '\0';
            }
            // Update display
            mvwprintw(get_window(0)->window, 1, 2, "Search: %-48s", search_input);
            wrefresh(get_window(0)->window);
            continue; 
        }

        if (selector_mode == 2) {
            if (ch == 27) { // ESC
                selector_mode = 0;
            } else if (ch == KEY_UP) {
                if (selector_index > 0) selector_index--;
            } else if (ch == KEY_DOWN) {
                if (selector_index < library_count-1) selector_index++;
            } else if (ch == '\n' || ch == KEY_ENTER) {
                do_library_action(selector_index);
                selector_mode = 0;
            }
            render_library_with_selector(get_window(2)->window, library_items, library_count, selector_index);
            continue;
        }

        render_windows_with_focus(focused_window);
    }
}

void render_library_with_selector(WINDOW *win, const char **items, int count, int selected) {
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 1, "Library");
    for (int i = 0; i < count; ++i) {
        if (i == selected)
            wattron(win, A_REVERSE);
        mvwprintw(win, i+1, 2, "%s", items[i]);
        if (i == selected)
            wattroff(win, A_REVERSE);
    }
    wrefresh(win);
}

void do_search(const char *query) {
    // TODO: Call Spotify API with query
    mvprintw(0, 0, "Searching for: %s", query);
    refresh();
}

void do_library_action(int index) {
    // TODO: Handle library item selection
    mvprintw(1, 0, "Selected library item: %d", index);
    refresh();
}