use crate::app::App;
use ratatui::style::{Color, Modifier, Style};
use ratatui::text::{Line, Span};
use ratatui::{
    layout::{Constraint, Direction, Layout, Rect},
    widgets::{Block, Borders, Clear, Paragraph},
    Frame,
};
use ratatui::backend::Backend;

pub fn draw<B: Backend>(frame: &mut Frame, app: &mut App) {
    let chunk = Layout::default()
        .direction(Direction::Vertical)
        .constraints(
            [
                Constraint::Length(3),
                Constraint::Fill(1),
                Constraint::Length(5),
            ]
                .as_ref(),
        )
        .split(frame.size());

    // Split top_area into 80/20 sections
    let top_area_chunks = Layout::default()
        .direction(Direction::Horizontal)
        .constraints([Constraint::Percentage(90), Constraint::Percentage(10)].as_ref())
        .split(chunk[0]);

    let main_area_chunks = Layout::default()
        .direction(Direction::Horizontal)
        .constraints([Constraint::Percentage(30), Constraint::Percentage(70)].as_ref())
        .split(chunk[1]);

    let music_area_chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([Constraint::Percentage(50), Constraint::Percentage(50)].as_ref())
        .split(main_area_chunks[0]);

    draw_search_bar(frame, app, top_area_chunks[0]);
    draw_help_bar(frame, app, top_area_chunks[1]);
    draw_main_section(frame, app, main_area_chunks[1]);
    draw_library(frame, app, music_area_chunks[0]);
    draw_playlist(frame, app, music_area_chunks[1]);
    draw_player(frame, app, chunk[2]);

    // Ajoutez cet appel
    draw_popup(frame, app);
}

fn draw_search_bar(frame: &mut Frame, app: &App, top_area: Rect) {
    let search_bar = Paragraph::new("Search").block(
        Block::default()
            .borders(Borders::ALL)
            .title("Search Bar")
            .border_style(active_style(app.cursor_y == 0)),
    );
    frame.render_widget(search_bar, top_area);
}

fn draw_help_bar(frame: &mut Frame, app: &App, top_area: Rect) {
    let help_line = vec![Line::from(vec![
        Span::styled("Type ", Style::default()),
        Span::styled("h", Style::default().add_modifier(Modifier::UNDERLINED)),
    ])];

    let help_bar = Paragraph::new(help_line)
        .block(Block::default().borders(Borders::ALL).title("Help Bar"))
        .style(Style::default().fg(Color::White));
    frame.render_widget(help_bar, top_area);
}

fn draw_main_section(frame: &mut Frame, app: &App, main_area: Rect) {
    let main_section = Paragraph::new("Main").block(
        Block::default()
            .borders(Borders::ALL)
            .title("Main Section")
            .border_style(active_style(
                (app.cursor_x == 1 || app.cursor_x == 2) && app.cursor_y == 1,
            )),
    );
    frame.render_widget(main_section, main_area);
}

fn draw_library(frame: &mut Frame, app: &App, main_area: Rect) {
    let library_section = Paragraph::new("Library").block(
        Block::default()
            .borders(Borders::ALL)
            .title("Library Section")
            .border_style(active_style(app.cursor_x == 0 && app.cursor_y == 1)),
    );
    frame.render_widget(library_section, main_area);
}

fn draw_playlist(frame: &mut Frame, app: &App, main_area: Rect) {
    let playlist_section = Paragraph::new("Playlist").block(
        Block::default()
            .borders(Borders::ALL)
            .title("Playlist Section")
            .border_style(active_style(app.cursor_x == 0 && app.cursor_y == 2)),
    );
    frame.render_widget(playlist_section, main_area);
}

fn draw_player(frame: &mut Frame, app: &App, bottom_area: Rect) {
    let player_section = Paragraph::new("Player").block(
        Block::default()
            .borders(Borders::ALL)
            .title("Player Section")
            .border_style(active_style(app.cursor_y == 3)),
    );
    frame.render_widget(player_section, bottom_area);
}

fn active_style(active: bool) -> Style {
    if active {
        Style::default().fg(Color::Cyan)
    } else {
        Style::default()
    }
}

fn draw_popup<B: Backend>(frame: &mut Frame, app: &App) {
    if app.show_popup {
        let size = frame.size();
        let popup_layout = Rect {
            x: (size.width - size.width * 60 / 100) / 2,
            y: (size.height - size.height * 80 / 100) / 2,
            width: size.width * 60 / 100,
            height: size.height * 80 / 100,
        };

        let popup_block = Block::default()
            .title("Popup")
            .borders(Borders::ALL)
            .border_style(Style::default().fg(Color::Red));

        frame.render_widget(Clear::default(), popup_layout);  // Cette ligne permet de rendre la popup sur une zone vide
        frame.render_widget(popup_block, popup_layout);
    }
}