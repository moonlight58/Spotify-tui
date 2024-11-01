mod app;
mod ui;
mod events;

use color_eyre::Result;
use crossterm::{terminal::{self}, event::{self, Event}};
use std::time::{Duration, Instant};
use std::io::{self};
use color_eyre::eyre::WrapErr;
use ratatui::{backend::CrosstermBackend, Terminal};

fn main() -> Result<()> {
    // Install color_eyre for error handling
    color_eyre::install()?;
    
    // Initialize terminal and enable raw mode
    terminal::enable_raw_mode().wrap_err("Failed to enter raw mode")?;
    let stdout = io::stdout();
    let mut terminal = Terminal::new(CrosstermBackend::new(stdout))?;
    terminal.clear()?;

    let tick_rate = Duration::from_millis(100);
    let mut last_tick = Instant::now();
    let mut app = app::App::new();  // Initialize App

    loop {
        // Only redraw the UI if enough time has passed
        if last_tick.elapsed() >= tick_rate {
            terminal.draw(|frame| ui::draw(frame, &app))?;
            last_tick = Instant::now();
        }

        // Poll for keyboard events
        if event::poll(Duration::from_millis(100))? {
            if let Event::Key(key_event) = event::read()? {
                events::handle_key_events(&mut app, key_event);
                if app.should_exit {
                    break;
                }
            }
        }
    }

    // Restore terminal to normal mode
    terminal.clear()?;
    terminal::disable_raw_mode()?;
    terminal.show_cursor()?;
    std::process::Command::new("clear").status()?;
    Ok(())
}