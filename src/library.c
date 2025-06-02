#include "library.h"
#include <ncurses.h>

const char *library_items[] = {
    "Made For You",
    "Recently Played",
    "Liked Songs",
    "Albums",
    "Artists",
    "Podcasts"
};
const int library_count = sizeof(library_items) / sizeof(library_items[0]);

void render_library_with_selector(WINDOW *win, const char **items, int count, int selected) {
    werase(win);

    wattron(win, COLOR_PAIR(103));
    box(win, 0, 0);
    mvwprintw(win, 0, 1, "Library");
    wattroff(win, COLOR_PAIR(103));

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    for (int i = 0; i < count && i < max_y - 2; ++i) {
        if (i == selected) {
            wattron(win, COLOR_PAIR(203) | A_BOLD); 
            mvwprintw(win, i + 1, 2, "%s", items[i]);
            wattroff(win, COLOR_PAIR(203) | A_BOLD);
        } else {
            mvwprintw(win, i + 1, 2, "%s", items[i]);
        }
    }
    wrefresh(win);
}

void do_library_action(int index) {}
void do_search(const char *input) {}