#ifndef LIBRARY_H
#define LIBRARY_H

#include <ncurses.h>

extern const char *library_items[];
extern const int library_count;

void render_library_with_selector(WINDOW *win, const char **items, int count, int selected);
void do_library_action(int index);
void do_search(const char *input);

#endif