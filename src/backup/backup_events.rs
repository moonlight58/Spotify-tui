use crossterm::event::{KeyCode, KeyEvent};

use crate::app::App;

pub fn handle_key_events(app: &mut App, key_event: KeyEvent) {
    match key_event.code {
        KeyCode::Char('q') => app.should_exit = true,
        KeyCode::Char('h') => {
            app.show_popup = !app.show_popup;  // Toggle popup visibility
        },
        KeyCode::Up => {
            if app.library_scroll_mode {
                if app.library_scroll > 0 {
                    app.library_scroll -= 1;
                }
            } else {
                app.previous_focus();
            }
        },
        KeyCode::Down => {
            if app.library_scroll_mode {
                app.library_scroll += 1;
            } else {
                app.next_focus();
            }
        },
        KeyCode::Left => {
            match app.focus {
                crate::app::Focus::Main => app.focus = crate::app::Focus::Library,
                _ => {}
            }
        },
        KeyCode::Right => {
            match app.focus {
                crate::app::Focus::Library => app.focus = crate::app::Focus::Main,
                crate::app::Focus::Playlist => app.focus = crate::app::Focus::Main,
                _ => {}
            }
        },
        KeyCode::Enter => {
            if app.focus == crate::app::Focus::Library {
                app.library_scroll_mode = !app.library_scroll_mode;
            }
        },
        _ => {}
    }
}
