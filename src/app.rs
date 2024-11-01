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
    pub cursor_x: usize, // For horizontal position
    pub cursor_y: usize, // For vertical position
}

impl App {
    pub fn new() -> App {
        App {
            focus: Focus::Main,
            library_scroll: 0,
            library_scroll_mode: false,
            should_exit: false,
            show_popup: false,
            cursor_x: 0,
            cursor_y: 0,
        }
    }
}
