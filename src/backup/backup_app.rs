#[derive(PartialEq)]
pub enum Focus {
    Search,
    Library,
    Playlist,
    Main,
    Playing,
}

pub struct App {
    pub focus: Focus,
    pub library_scroll: usize,
    pub library_scroll_mode: bool,
    pub should_exit: bool,
    pub show_popup: bool,
}

impl App {
    pub fn new() -> App {
        App {
            focus: Focus::Main,
            library_scroll: 0,
            library_scroll_mode: false,
            should_exit: false,
            show_popup: false,
        }
    }

    pub fn previous_focus(&mut self) {
        self.focus = match self.focus {
            Focus::Playing => Focus::Playlist,
            Focus::Playlist => Focus::Library,
            Focus::Library => Focus::Search,
            Focus::Main => Focus::Search,
            Focus::Search => Focus::Search,
        };
    }

    pub fn next_focus(&mut self) {
        self.focus = match self.focus {
            Focus::Search => Focus::Library,
            Focus::Library => Focus::Playlist,
            Focus::Playlist => Focus::Playing,
            Focus::Main => Focus::Playing,
            Focus::Playing => Focus::Playing,
        };
    }
}
