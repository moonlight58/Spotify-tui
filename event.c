#include "event.h"

void handle_events(WINDOW *search_bar, WINDOW *help_bar) {
    int ch;
    while ((ch = getch()) != 'q') {
        switch (ch) {
        case '?':
            mvwprintw(help_bar, 1, 1, "Help: Press q to exit");
            wrefresh(help_bar);
            break;
        case 's':
            mvwprintw(search_bar, 1, 1, "Search: ");
            wrefresh(search_bar);
            break;
        default:
            break;
        }
    }
}