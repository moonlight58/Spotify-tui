use crossterm::event::{KeyCode, KeyEvent};
use crate::app::App;


pub fn handle_key_events(app: &mut App, key_event: KeyEvent) {
    match key_event.code {
        KeyCode::Char('q') => app.should_exit = true,
        KeyCode::Char('h') => app.show_popup = !app.show_popup,
        KeyCode::Up => {
            if app.cursor_y > 0 {
                if app.cursor_x == 1 && app.cursor_y == 3 {
                    app.cursor_y -= 2;
                } else {
                    app.cursor_y -= 1;
                }
            }
            // info!("Cursor moved up: ({}, {})", app.cursor_x, app.cursor_y);
        },
        KeyCode::Down => {
            let max_y = 3; // Maximum Y index for 4 rows
            if app.cursor_y < max_y {
                if app.cursor_x == 1 && app.cursor_y == 1 {
                    app.cursor_y += 2;
                } else {
                    app.cursor_y += 1;
                }
            }
            // info!("Cursor moved down: ({}, {})", app.cursor_x, app.cursor_y);
        },
        KeyCode::Left => {
            if app.cursor_x > 0 {
                app.cursor_x -= 1;
            }
        },
        KeyCode::Right => {
            let max_x = 1; // Maximum X index for 2 columns
            if app.cursor_x < max_x {
                if app.cursor_x == 0 && app.cursor_y == 2 {
                    app.cursor_y -= 1;
                }
                app.cursor_x += 1;
            }
        },
        _ => {}
    }
}