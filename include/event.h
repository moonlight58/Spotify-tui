#ifndef EVENT_H
#define EVENT_H

#include <ncurses.h>

void handle_events(WINDOW **search_bar, WINDOW **help_bar,
                   WINDOW **library_win, WINDOW **playlist_win,
                   WINDOW **main_win, WINDOW **progress_bar);

#endif