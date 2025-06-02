#include "event.h"
#include "tui.h"
#include "tui-window.h"
#include "library.h"

#include <ncurses.h>
#include <string.h>
#include <ctype.h>

typedef enum
{
    MODE_NORMAL,
    MODE_SEARCH,
    MODE_LIBRARY,
    MODE_PLAYLIST,
    MODE_SEARCH_SONG,
    MODE_SEARCH_ALBUM,
    MODE_SEARCH_ARTIST,
    MODE_SEARCH_PLAYLIST,
} AppMode;

typedef struct
{
    AppMode mode;
    int focused_window;
    int selector_mode;
    int selector_index;
    char search_input[256];
} AppState;

// --- Prototypes des handlers par mode ---
void handle_normal_mode(AppState *state, int ch, WINDOW **search_bar, WINDOW **help_bar, WINDOW **library_win, WINDOW **playlist_win, WINDOW **main_win, WINDOW **progress_bar);
void handle_search_mode(AppState *state, int ch);
void handle_library_mode(AppState *state, int ch);

// --- Boucle principale ---
void handle_events(WINDOW **search_bar, WINDOW **help_bar,
                   WINDOW **library_win, WINDOW **playlist_win,
                   WINDOW **main_win, WINDOW **progress_bar)
{
    AppState state = {MODE_NORMAL, 4, 0, 0, ""};
    int ch;
    while ((ch = getch()) != 'q')
    {
        switch (state.mode)
        {
            case MODE_NORMAL:
                handle_normal_mode(&state, ch, search_bar, help_bar, library_win, playlist_win, main_win, progress_bar);
                break;
            case MODE_SEARCH:
                handle_search_mode(&state, ch);
                break;
            case MODE_LIBRARY:
                handle_library_mode(&state, ch);
                break;
            case MODE_PLAYLIST:
                // handle_playlist_mode(&state, ch);
                break;
            // Ajoute d'autres modes ici si besoin
            default:
                break;
        }
    }
}

// --- Handler du mode normal ---
void handle_normal_mode(AppState *state, int ch, WINDOW **search_bar, WINDOW **help_bar, WINDOW **library_win, WINDOW **playlist_win, WINDOW **main_win, WINDOW **progress_bar)
{
    TuiWindow *current = get_window(state->focused_window);
    int nx = current->grid_x, ny = current->grid_y;
    int height, width;
    WindowLayout layouts[6];

    switch (ch)
    {
        case KEY_RESIZE:
            getmaxyx(stdscr, height, width);
            calculate_layout(height, width, layouts);
            delwin(*search_bar);
            delwin(*help_bar);
            delwin(*library_win);
            delwin(*playlist_win);
            delwin(*main_win);
            delwin(*progress_bar);
            *search_bar = create_window_with_layout(layouts[0], 1, "Search");
            *help_bar = create_window_with_layout(layouts[5], 2, "Help");
            *library_win = create_window_with_layout(layouts[1], 3, "Library");
            *playlist_win = create_window_with_layout(layouts[2], 4, "Playlists");
            *main_win = create_window_with_layout(layouts[3], 5, "Welcome !");
            *progress_bar = create_window_with_layout(layouts[4], 1, "Progress Bar");
            mvwprintw(*help_bar, 1, 1, "Type ?");
            init_windows(*search_bar, *help_bar, *library_win, *playlist_win, *main_win, *progress_bar);
            break;
        case '?':
            wrefresh(*help_bar);
            break;
        case 's':
            state->mode = MODE_SEARCH;
            state->focused_window = 0;
            
            state->search_input[0] = '\0';
            wrefresh(get_window(0)->window);
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
        case '\n':
        case KEY_ENTER:
            switch (state->focused_window)
            {
                case 0: // Search bar
                    state->mode = MODE_SEARCH;
                    state->search_input[0] = '\0';
                    mvwprintw(get_window(0)->window, 1, 2, "Type your search...");
                    wrefresh(get_window(0)->window);
                    break;
                case 2: // Library
                    state->mode = MODE_LIBRARY;
                    state->selector_index = 0;
                    render_windows_with_focus(state->focused_window);
                    render_library_with_selector(get_window(2)->window, library_items, library_count, state->selector_index);
                    break;
                // Ajoute d'autres fenêtres si besoin
            }
            return;
        default:
            break;
    }

    int next = find_window_by_grid(nx, ny);
    while (next != -1 && !get_window(next)->is_focusable)
    {
        if (ch == KEY_UP)
            ny--;
        else if (ch == KEY_DOWN)
            ny++;
        else if (ch == KEY_LEFT)
            nx--;
        else if (ch == KEY_RIGHT)
            nx++;
        next = find_window_by_grid(nx, ny);
    }
    if (next != -1)
        state->focused_window = next;

    render_windows_with_focus(state->focused_window);
}

// --- Handler du mode search ---
void handle_search_mode(AppState *state, int ch)
{
    if (ch == 27) // ESC
    {
        state->mode = MODE_NORMAL;
        mvwprintw(get_window(0)->window, 1, 2, "%*s", 50, " ");
        wrefresh(get_window(0)->window);
    }
    else if (ch == '\n' || ch == KEY_ENTER)
    {
        do_search(state->search_input);
        state->mode = MODE_NORMAL;
    }
    else if (ch == KEY_BACKSPACE || ch == 127)
    {
        int len = strlen(state->search_input);
        if (len > 0)
            state->search_input[len - 1] = '\0';
    }
    else if (isprint(ch) && strlen(state->search_input) < sizeof(state->search_input) - 1)
    {
        int len = strlen(state->search_input);
        state->search_input[len] = ch;
        state->search_input[len + 1] = '\0';
    }
    wattron(get_window(0)->window, COLOR_PAIR(201));
    mvwprintw(get_window(0)->window, 1, 2, "%-48s", state->search_input);
    wattroff(get_window(0)->window, COLOR_PAIR(201));
    wrefresh(get_window(0)->window);
}

// --- Handler du mode library ---
void handle_library_mode(AppState *state, int ch)
{
    if (ch == 27) // ESC
    {
        state->mode = MODE_NORMAL;
    }
    else if (ch == KEY_UP)
    {
        if (state->selector_index > 0)
            state->selector_index--;
    }
    else if (ch == KEY_DOWN)
    {
        if (state->selector_index < library_count - 1)
            state->selector_index++;
    }
    else if (ch == '\n' || ch == KEY_ENTER)
    {
        do_library_action(state->selector_index);
        state->mode = MODE_NORMAL;
    }
    render_windows_with_focus(state->focused_window);
    render_library_with_selector(get_window(2)->window, library_items, library_count, state->selector_index);
}