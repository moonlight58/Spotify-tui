use ratatui::{layout::{Constraint, Layout, Flex, Rect}, widgets::{Block, Borders, Paragraph, Clear}, Frame};
use crate::app::{App, Focus};
use ratatui::style::{Color, Style, Modifier};
use ratatui::text::{Span, Line};

pub fn draw(frame: &mut Frame, app: &App) {

    let vertical = Layout::vertical([
        Constraint::Length(3),
        Constraint::Fill(1),
        Constraint::Length(6),
    ]);

    let [header_area, main_area, footer_area] = vertical.areas(frame.area());

    let main_layout = Layout::horizontal([
        Constraint::Percentage(30),
        Constraint::Percentage(70),
    ]);

    let [music_area, welcome_area] = main_layout.areas(main_area);

    let header_layout = Layout::horizontal([
        Constraint::Percentage(90),
        Constraint::Percentage(10),
    ]);

    let [search_bar, help_bar] = header_layout.areas(header_area);

    let music_layout = Layout::vertical([
        Constraint::Percentage(30),
        Constraint::Percentage(70),
    ]);

    let [library_bar, playlist_area] = music_layout.areas(music_area);

    // Render widgets based on index
    frame.render_widget(draw_search(app.focus == Focus::Search), search_bar);
    frame.render_widget(draw_help(), help_bar);
    frame.render_widget(draw_library(app.focus == Focus::Library, app.library_scroll), library_bar);
    frame.render_widget(draw_playlist(app.focus == Focus::Playlist), playlist_area);
    frame.render_widget(draw_main(app.focus == Focus::Main), welcome_area);
    frame.render_widget(draw_footer(app.focus == Focus::Playing), footer_area);

    if app.show_popup {
        let block = Block::default().title("Help popup").borders(Borders::ALL);
        let area = popup_area(frame.area(), 60, 85);
        frame.render_widget(Clear, area); // Clear background
        frame.render_widget(block, area); // Draw popup
    }
}

fn draw_search(active: bool) -> Paragraph<'static> {
    let block = Block::default().title("Search").borders(Borders::ALL).border_style(active_style(active));
    Paragraph::new(Span::raw("Search something here...")).block(block)
}

fn draw_help() -> Paragraph<'static> {
    let block = Block::default()
        .title("Help")
        .borders(Borders::ALL)
        .border_style(Style::default());

    let help_line = vec![
        Line::from(vec![
            Span::styled("Type ", Style::default()),
            Span::styled("h", Style::default().add_modifier(Modifier::UNDERLINED)),
        ]),
    ];

    Paragraph::new(help_line).block(block)
}

fn draw_library(active: bool, scroll: usize) -> Paragraph<'static> {
    let block = Block::default().title("Library").borders(Borders::ALL).border_style(active_style(active));
    let library_items = vec!["Made For You", "Recently Played", "Liked Songs", "Albums", "Artists", "Podcasts"];
    let displayed_items = library_items.iter().skip(scroll).take(6).enumerate().map(|(i, item)| format!("{}. {}\n", i + 1 + scroll, item)).collect::<String>();
    Paragraph::new(displayed_items).block(block)
}

fn draw_playlist(active: bool) -> Paragraph<'static> {
    let block = Block::default().title("Playlist").borders(Borders::ALL).border_style(active_style(active));
    Paragraph::new(Span::raw("Playlist")).block(block)
}

fn draw_main(active: bool) -> Paragraph<'static> {
    let block = Block::default().title("Main Content").borders(Borders::ALL).border_style(active_style(active));
    Paragraph::new(Span::raw("Welcome to the Terminal App!")).block(block)
}

fn draw_footer(active: bool) -> Paragraph<'static> {
    let block = Block::default().title("Playing...").borders(Borders::ALL).border_style(active_style(active));
    Paragraph::new(Span::raw("Duration: 00:00 / 03:45")).block(block)
}

pub fn popup_area(area: Rect, percent_x: u16, percent_y: u16) -> Rect {
    let vertical = Layout::vertical([Constraint::Percentage(percent_y)]).flex(Flex::Center);
    let horizontal = Layout::horizontal([Constraint::Percentage(percent_x)]).flex(Flex::Center);
    let [area] = vertical.areas(area);
    let [area] = horizontal.areas(area);
    area
}

fn active_style(active: bool) -> Style {
    if active {
        Style::default().fg(Color::Cyan)
    } else {
        Style::default()
    }
}
