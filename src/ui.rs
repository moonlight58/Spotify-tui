use crate::app::App;
use ratatui::style::{Color, Modifier, Style};
use ratatui::text::{Line, Span};
use ratatui::{
    layout::{Constraint, Direction, Layout, Rect},
    widgets::{Block, Borders, Clear, Paragraph},
    Frame,
};

pub fn draw (frame: &mut Frame, app: &mut App) {
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
    draw_help_bar(frame, top_area_chunks[1]);
    draw_main_section(frame, app, main_area_chunks[1]);
    draw_library(frame, app, music_area_chunks[0]);
    draw_playlist(frame, app, music_area_chunks[1]);
    draw_player(frame, app, chunk[2]);

    draw_popup(frame, app);
}

fn draw_search_bar(frame: &mut Frame, app: &App, top_area: Rect) {
    let search_bar =
        Paragraph::new("Search").block(
            Block::default()
            .borders(Borders::ALL)
            .title("Search Bar")
            .border_style(active_style(app.cursor_y == 0)),
    );
    frame.render_widget(search_bar, top_area);
}

fn draw_help_bar(frame: &mut Frame, top_area: Rect) {
    let help_line =
        vec![Line::from(vec![
        Span::styled("Type ", Style::default()),
        Span::styled("h", Style::default().add_modifier(Modifier::UNDERLINED)),
    ])];

    let help_bar =
        Paragraph::new(help_line)
        .block(Block::default().borders(Borders::ALL).title("Help Bar"))
        .style(Style::default().fg(Color::White));
    frame.render_widget(help_bar, top_area);
}

fn draw_main_section(frame: &mut Frame, app: &App, main_area: Rect) {
    let main_section =
        Paragraph::new("Main").block(
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
    let library_section =
        Paragraph::new("Library").block(
            Block::default()
            .borders(Borders::ALL)
            .title("Library Section")
            .border_style(active_style(app.cursor_x == 0 && app.cursor_y == 1)),
    );
    frame.render_widget(library_section, main_area);
}

fn draw_playlist(frame: &mut Frame, app: &App, main_area: Rect) {
    let playlist_section =
        Paragraph::new("Playlist").block(
            Block::default()
            .borders(Borders::ALL)
            .title("Playlist Section")
            .border_style(active_style(app.cursor_x == 0 && app.cursor_y == 2)),
    );
    frame.render_widget(playlist_section, main_area);
}

fn draw_player(frame: &mut Frame, app: &App, bottom_area: Rect) {
    let player_section =
        Paragraph::new("Player").block(
            Block::default()
            .borders(Borders::ALL)
            .title("Player Section")
            .border_style(active_style(app.cursor_y == 3)),
        );
    frame.render_widget(player_section, bottom_area);
}

fn draw_popup(frame: &mut Frame, app: &App) {
    if app.show_popup {
        let area = frame.size();
        let popup_width = 50;
        let popup_height = (area.height * 90) / 100; // 90% de la hauteur totale
        let popup_area = Rect::new(
            (area.width - popup_width) / 2,   // centré horizontalement
            (area.height - popup_height) / 2, // centré verticalement
            popup_width,
            popup_height,
        );


        let command_help_text = vec![
            Line::from(vec![
                Span::styled("Commands List", Style::default().add_modifier(Modifier::BOLD)),
            ]),
            Line::from(""),
            Line::from(vec![
                Span::styled("q", Style::default().fg(Color::Yellow)),
                Span::styled(" - Quit the application", Style::default().fg(Color::Green)),
            ]),
            Line::from(vec![
                Span::styled("h", Style::default().fg(Color::Yellow)),
                Span::styled(" - Toggle this help popup", Style::default().fg(Color::Green)),
            ]),
            Line::from(vec![
                Span::styled("Up/Down", Style::default().fg(Color::Yellow)),
                Span::styled(" - Navigate between sections", Style::default().fg(Color::Green)),
            ]),
            Line::from(vec![
                Span::styled("Enter", Style::default().fg(Color::Yellow)),
                Span::styled(" - Select an item", Style::default().fg(Color::Green)),
            ]),
        ];

        let command_help = Paragraph::new(command_help_text)
            .block(Block::default().title("Commands").borders(Borders::ALL)
                       .border_style(Style::default().fg(Color::Green)))
            .style(Style::default().fg(Color::White));


        frame.render_widget(Clear, popup_area); // Efface l'arrière-plan
        frame.render_widget(command_help, popup_area);
    }
}


fn active_style(active: bool) -> Style {
    if active { Style::default().fg(Color::Cyan) }
    else { Style::default() }
}