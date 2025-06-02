#include "tui.h"
#include "utils.h"
#include "banner.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#define COLOR_BG                     8
#define COLOR_FG                     9

#define COLOR_CUSTOM_SEARCH          10
#define COLOR_CUSTOM_LIBRARY         11
#define COLOR_CUSTOM_PLAYLIST        12
#define COLOR_CUSTOM_MAIN            13
#define COLOR_CUSTOM_PROGRESS        14

#define COLOR_CUSTOM_ACTIVE          15
#define COLOR_CUSTOM_ACTIVE_SEARCH   16
#define COLOR_CUSTOM_ACTIVE_LIBRARY  17
#define COLOR_CUSTOM_ACTIVE_PLAYLIST 18
#define COLOR_CUSTOM_ACTIVE_MAIN     19

void setup_colors()
{
    if (!has_colors())
    {
        endwin();
        error_window("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    use_default_colors();

    init_color(COLOR_BG,                     (4 * 1000) / 255,   (37 * 1000) / 255,  (46 * 1000) / 255);
    init_color(COLOR_FG,                     (61 * 1000) / 255,  (94 * 1000) / 255,  (103 * 1000) / 255);
    init_color(COLOR_CUSTOM_SEARCH,          (0 * 1000) / 255,   (255 * 1000) / 255, (255 * 1000) / 255);
    init_color(COLOR_CUSTOM_LIBRARY,         (226 * 1000) / 255, (140 * 1000) / 255, (236 * 1000) / 255);
    init_color(COLOR_CUSTOM_PLAYLIST,        (255 * 1000) / 255, (121 * 1000) / 255, (198 * 1000) / 255);
    init_color(COLOR_CUSTOM_MAIN,            (0 * 1000) / 255,   (255 * 1000) / 255, (255 * 1000) / 255);
    init_color(COLOR_CUSTOM_PROGRESS,        (61 * 1000) / 255,  (255 * 1000) / 255, (255 * 1000) / 255);

    init_color(COLOR_CUSTOM_ACTIVE,          (183 * 1000) / 255, (211 * 1000) / 255, (230 * 1000) / 255);
    init_color(COLOR_CUSTOM_ACTIVE_SEARCH,   (200 * 1000) / 255, (224 * 1000) / 255, (237 * 1000) / 255);
    init_color(COLOR_CUSTOM_ACTIVE_LIBRARY,  (226 * 1000) / 255, (140 * 1000) / 255, (236 * 1000) / 255);
    init_color(COLOR_CUSTOM_ACTIVE_PLAYLIST, (255 * 1000) / 255, (121 * 1000) / 255, (198 * 1000) / 255);
    init_color(COLOR_CUSTOM_ACTIVE_MAIN,     (200 * 1000) / 255, (224 * 1000) / 255, (237 * 1000) / 255);

    init_pair(1, COLOR_FG, COLOR_BG); // search_bar
    init_pair(2, COLOR_FG, COLOR_BG); // help_bar
    init_pair(3, COLOR_FG, COLOR_BG); // library window
    init_pair(4, COLOR_FG, COLOR_BG); // playlist window
    init_pair(5, COLOR_FG, COLOR_BG); // main window
    init_pair(6, COLOR_FG, COLOR_BG); // progress bar

    // Active color pairs
    init_pair(101, COLOR_CUSTOM_SEARCH,          COLOR_BG); // search_bar active
    init_pair(102, COLOR_FG,                     COLOR_BG); // help_bar active
    init_pair(103, COLOR_CUSTOM_LIBRARY,         COLOR_BG); // library window active
    init_pair(104, COLOR_CUSTOM_PLAYLIST,        COLOR_BG); // playlist window active
    init_pair(105, COLOR_CUSTOM_MAIN,            COLOR_BG); // main window active
    init_pair(106, COLOR_CUSTOM_PROGRESS,        COLOR_BG); // progress bar active

    init_pair(201, COLOR_CUSTOM_ACTIVE_SEARCH,   COLOR_BG); // search_bar active
    init_pair(202, COLOR_CUSTOM_ACTIVE,          COLOR_BG); // active color
    init_pair(203, COLOR_CUSTOM_ACTIVE_LIBRARY,  COLOR_BG); // library window active
    init_pair(204, COLOR_CUSTOM_ACTIVE_PLAYLIST, COLOR_BG); // playlist window active
    init_pair(205, COLOR_CUSTOM_ACTIVE_MAIN,     COLOR_BG); // main window active
    init_pair(206, COLOR_CUSTOM_ACTIVE_MAIN,     COLOR_BG); // progress bar active
}

void calculate_layout(int screen_height, int screen_width, WindowLayout layouts[6])
{
    const int TOP_HEIGHT = 3;
    const int BOTTOM_HEIGHT = 6;
    const int HELP_WIDTH = 20; 
    const int LEFT_WIDTH = screen_width / 4;
    int available_height = screen_height - TOP_HEIGHT - BOTTOM_HEIGHT;
    int library_height = available_height / 4;
    int playlist_height = available_height - library_height;
    int main_width = screen_width - LEFT_WIDTH;

    layouts[0] = (WindowLayout){TOP_HEIGHT, screen_width - HELP_WIDTH, 0, 0};                   // search_bar
    layouts[1] = (WindowLayout){library_height, LEFT_WIDTH, TOP_HEIGHT, 0};                     // library window
    layouts[2] = (WindowLayout){playlist_height, LEFT_WIDTH, TOP_HEIGHT + library_height, 0};   // playlist window
    layouts[3] = (WindowLayout){available_height, main_width, TOP_HEIGHT, LEFT_WIDTH};          // main window
    layouts[4] = (WindowLayout){BOTTOM_HEIGHT, screen_width, screen_height - BOTTOM_HEIGHT, 0}; // progress_bar
    layouts[5] = (WindowLayout){TOP_HEIGHT, HELP_WIDTH, 0, screen_width - HELP_WIDTH};          // help_bar
}

WINDOW *create_window_with_layout(WindowLayout layout, int color_pair, const char *title)
{
    WINDOW *win = newwin(layout.height, layout.width, layout.start_y, layout.start_x);
    wbkgd(win, COLOR_PAIR(color_pair));
    box(win, 0, 0);
    if (title)
    {
        mvwaddstr(win, 0, 1, title);
    }
    return win;
}

void render_welcome(WINDOW *main_win)
{
    wattron(main_win, COLOR_PAIR(202)); // Use custom main color
    int max_y, max_x;
    getmaxyx(main_win, max_y, max_x);

    // Split the banner into lines
    char banner_copy[512];
    strncpy(banner_copy, BANNER, sizeof(banner_copy));
    banner_copy[sizeof(banner_copy) - 1] = '\0';

    char *line = strtok(banner_copy, "\n");
    int row = 1;
    while (line) {
        int len = strlen(line);
        int col = (max_x - len) / 2;
        if (col < 1) col = 1; // Avoid border overwrite
        mvwprintw(main_win, row++, col, "%s", line);
        line = strtok(NULL, "\n");
    }
    int banner_lines = row - 1;

    // Now print the welcome text below the banner
    FILE *file = fopen("welcome.txt", "r");
    if (!file)
    {
        mvwprintw(main_win, row, 1, "Welcome file not found.");
        wrefresh(main_win);
        return;
    }
    char text_line[512];
    int usable_width = max_x - 2; // leave space for borders

    while (fgets(text_line, sizeof(text_line), file))
    {
        size_t len = strlen(text_line);
        if (len > 0 && text_line[len - 1] == '\n')
            text_line[len - 1] = '\0';

        char *ptr = text_line;
        while ((int)strlen(ptr) > usable_width)
        {
            int wrap = usable_width;
            // Try to wrap at the last space before usable_width
            for (int i = usable_width; i > 0; --i)
            {
                if (ptr[i] == ' ')
                {
                    wrap = i;
                    break;
                }
            }
            // If no space found, force wrap at usable_width
            char saved = ptr[wrap];
            ptr[wrap] = '\0';
            mvwprintw(main_win, row++, 2, "%s", ptr);
            ptr[wrap] = saved;
            ptr += wrap;
            // Skip leading spaces
            while (*ptr == ' ') ptr++;
            if (row >= max_y - 1)
                break;
        }
        if (row < max_y - 1)
        {
            mvwprintw(main_win, row++, 2, "%s", ptr);
        }
        if (row >= max_y - 1)
            break;
    }
    fclose(file);
    wattroff(main_win, COLOR_PAIR(202)); // Reset color
    wrefresh(main_win);
}

void render_library(WINDOW *library_win, const char **items, int count)
{
    werase(library_win);                     // Clear previous content
    box(library_win, 0, 0);                  // Redraw border
    mvwprintw(library_win, 0, 1, "Library"); // Title

    int max_y, max_x;
    getmaxyx(library_win, max_y, max_x);

    for (int i = 0; i < count && i < max_y - 2; ++i)
    {
        mvwprintw(library_win, i + 1, 2, "%s", items[i]);
    }
    wrefresh(library_win);
}

void render_all_windows(WINDOW *search_bar, WINDOW *help_bar, WINDOW *library_win,
                        WINDOW *playlist_win, WINDOW *main_win, WINDOW *progress_bar)
{
    refresh();
    wrefresh(search_bar);
    wrefresh(help_bar);
    wrefresh(library_win);
    wrefresh(playlist_win);
    wrefresh(main_win);
    wrefresh(progress_bar);
}
