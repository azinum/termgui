// termgui.h
// TODO(lucas): limit the window size

#ifndef _TERMGUI_H
#define _TERMGUI_H

#include <stdint.h>
#include <string.h> // memset
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <assert.h>
#include <locale.h> // setlocale

#define foreach(_i, _count) for (typeof(_count) _i = 0; _i < _count; ++_i)
#define TERMGUI_API static
#define Ok(err) (err == NoError)
#define Err(message) (err_string = message, term_gui.status = Error, Error)
#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, x_min, x_max) MIN(MAX(x_min, x), x_max)
#define UTF8_SIZE (4)
#define COLOR_CODE_SIZE 7
#define MAX_CELL_COUNT (200 * 80)
#define MAX_BUFFER_SIZE (UTF8_SIZE * MAX_CELL_COUNT * COLOR_CODE_SIZE)
#define LENGTH(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#define IN_BOUNDS(x, a, b) ((x >= a) && (x <= b))

typedef int32_t i32;
typedef uint32_t u32;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;

static const char* log_file_name = "log.txt";

typedef enum Color {
  COLOR_NONE = 0,

  COLOR_BLACK,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW,
  COLOR_BLUE,
  COLOR_PURPLE,
  COLOR_CYAN,
  COLOR_WHITE,

  COLOR_BOLD_BLACK,
  COLOR_BOLD_RED,
  COLOR_BOLD_GREEN,
  COLOR_BOLD_YELLOW,
  COLOR_BOLD_BLUE,
  COLOR_BOLD_PURPLE,
  COLOR_BOLD_CYAN,
  COLOR_BOLD_WHITE,

  MAX_COLOR
} Color;

static const char* color_code_str[MAX_COLOR] = {
  "\033[0;00m",
  "\033[0;30m",
  "\033[0;31m",
  "\033[0;32m",
  "\033[0;33m",
  "\033[0;34m",
  "\033[0;35m",
  "\033[0;36m",
  "\033[0;37m",

  "\033[1;30m",
  "\033[1;31m",
  "\033[1;32m",
  "\033[1;33m",
  "\033[1;34m",
  "\033[1;35m",
  "\033[1;36m",
  "\033[1;37m",
};

static u8 utf8_mask[] = { 0xc0, 0x80, 0xe0, 0xf0, 0xf8 };
static u8 utf8_byte[] = { 0x80, 0x00, 0xc0, 0xe0, 0xf0 };

typedef struct Cell {
  u8 code[UTF8_SIZE];
  u16 fg;
} Cell;

typedef i8 Item;

static Cell border_cell_vertical =   { .code = { 0xe2, 0x94, 0x82, 0x00 }, .fg = COLOR_NONE };
static Cell border_cell_horizontal = { .code = { 0xe2, 0x94, 0x80, 0x00 }, .fg = COLOR_NONE };

enum Border_cell_orientation {
  BORDER_CELL_TOP_LEFT = 0,
  BORDER_CELL_TOP_RIGHT,
  BORDER_CELL_BOTTOM_LEFT,
  BORDER_CELL_BOTTOM_RIGHT,

  BORDER_CELL_CONN_LEFT,
  BORDER_CELL_CONN_RIGHT,
  BORDER_CELL_CONN_TOP,
  BORDER_CELL_CONN_BOTTOM,

  MAX_BORDER_CELL
};

static Cell border_cell_corners[MAX_BORDER_CELL] = {
  { .code = {	0xe2, 0x94, 0x8c, 0x00 }, .fg = COLOR_NONE}, // top left
  { .code = {	0xe2, 0x94, 0x90, 0x00 }, .fg = COLOR_NONE}, // top right
  { .code = { 0xe2, 0x94, 0x94, 0x00 }, .fg = COLOR_NONE}, // bottom left
  { .code = { 0xe2, 0x94, 0x98, 0x00 }, .fg = COLOR_NONE}, // bottom right

  { .code = { 0xe2, 0x94, 0x9c, 0x00 }, .fg = COLOR_NONE}, // left connection
  { .code = { 0xe2, 0x94, 0xa4, 0x00 }, .fg = COLOR_NONE}, // right connection
  { .code = { 0xe2, 0x94, 0xac, 0x00 }, .fg = COLOR_NONE}, // top connection
  { .code = { 0xe2, 0x94, 0xb4, 0x00 }, .fg = COLOR_NONE}, // bottom connection
};

typedef enum Result { NoError, Error, Done } Result;
static char* err_string = "";

typedef struct Termgui {
  i32 tty;
  struct termios term_copy;
  struct termios term;
  u32 width;
  u32 height;
  u32 size; // width * height
  Cell cells[MAX_CELL_COUNT];
  i8 buffer[MAX_BUFFER_SIZE];
  i32 cursor_x;
  i32 cursor_y;
  i32 render_event;
  i32 use_colors;
  i32 initialized;
  Result status;
  i32 fd; // fd for logging
} Termgui;

static Termgui term_gui = {0};

// api functions
TERMGUI_API Result tg_init();
TERMGUI_API Result tg_update();
TERMGUI_API Result tg_render();
TERMGUI_API void tg_box(u32 x_pos, u32 y_pos, u32 w, u32 h, char* title);
TERMGUI_API u32 tg_width();
TERMGUI_API u32 tg_height();
TERMGUI_API void tg_cursor_move(i32 delta_x, i32 delta_y);
TERMGUI_API void tg_exit();
TERMGUI_API char* tg_err_string();
TERMGUI_API void tg_print_error();
TERMGUI_API void tg_free();

static i8 utf8_decode_byte(i8 byte, u32* size);

static void tg_cells_init(Termgui* tg, Cell* cell);
static void tg_cell_init(Cell* cell);
static void tg_cell_init_ascii(Cell* cell, i8 ascii);

static i32 tg_handle_input(Termgui* tg);
static void tg_prepare_frame(Termgui* tg);
static void tg_plot(Termgui* tg, u32 x, u32 y, Item item);
static void tg_plot_cell(Termgui* tg, u32 x, u32 y, Cell* cell);
static void tg_render_box(Termgui* tg, u32 x_pox, u32 y_pos, u32 w, u32 h);
static void tg_render_box_with_title(Termgui* tg, u32 x_pos, u32 y_pos, u32 w, u32 h, char* title);
static void tg_render_text(Termgui* tg, u32 x_pos, u32 y_pos, char* text, u32 length);
static void tg_render_horizontal_line(Termgui* tg, u32 x_pos, u32 y_pos, u32 length);
static void tg_render_vertical_line(Termgui* tg, u32 x_pos, u32 y_pos, u32 length);
static void tg_cursor_update(Termgui* tg);
static void tg_term_fetch_size(Termgui* tg);
static void tg_term_clear(Termgui* tg);
static Result tg_term_init(Termgui* tg);

// signal handlers
static void sigwinch();
static void sigint(i32 sig);

typedef enum Signal_event {
  SIG_EVENT_WINCH,
  SIG_EVENT_INT,

  MAX_SIG_EVENT,
} Signal_event;

static void tg_queue_sig_event(Termgui* tg, Signal_event event);
static void tg_handle_sig_events(Termgui* tg);
static void tg_sig_event_winch(Termgui* tg);
static void tg_sig_event_int(Termgui* tg);

typedef void (*signal_callback)(Termgui*);

#define MAX_SIGNAL 10
static volatile Signal_event signal_events[MAX_SIGNAL] = {0};
static volatile u32 signal_event_count = 0;

static signal_callback signal_event_callbacks[MAX_SIG_EVENT] = {
  tg_sig_event_winch,
  tg_sig_event_int,
};

Result tg_init() {
  Termgui* tg = &term_gui;
  memset(tg, 0, sizeof(Termgui));
  tg->tty = STDOUT_FILENO;
  tg->status = tg_term_init(tg);
  if (Ok(tg->status)) {
    Cell cell;
    tg_cell_init_ascii(&cell, ' ');
    tg_cells_init(tg, &cell);
    tg->cursor_x = 0;
    tg->cursor_y = 0;
    tg->render_event = 1;
    tg->use_colors = 1;
    tg->initialized = 1;
    tg->status = NoError;
    tg->fd = open(log_file_name, O_CREAT | O_TRUNC | O_WRONLY, 0662);

    signal(SIGWINCH, sigwinch);
    signal(SIGINT, sigint);
    tg_queue_sig_event(tg, SIG_EVENT_WINCH);

    if (!setlocale(LC_CTYPE, "")) {
      dprintf(tg->fd, "[warning]: no locale support\n");
    }

  }
  return tg->status;
}

Result tg_update() {
  Termgui* tg = &term_gui;
  tg_prepare_frame(tg);
  tg->render_event = 0;
  tg_handle_sig_events(tg);
  tg_handle_input(tg);
  if (!Ok(tg->status)) {
    return tg->status;
  }
  return tg->status;
}

Result tg_render() {
  Termgui* tg = &term_gui;
  if (tg->render_event) {
    tg_term_clear(tg);
    // 1) decode each cell
    // 2) write to buffer
    // 3) write buffer to tty
    i32 decoded_size = 0;
    i32 write_index = 0;
    foreach(i, tg->size) {
      Cell* cell = &tg->cells[i];
      if (tg->use_colors) {
        for (u32 color_code_index = 0; color_code_index < COLOR_CODE_SIZE; ++color_code_index) {
          tg->buffer[write_index++] = color_code_str[cell->fg][color_code_index];
        }
        decoded_size += COLOR_CODE_SIZE;
      }
      u32 code_size = 0;
      utf8_decode_byte(cell->code[0], &code_size);
      if (IN_BOUNDS(code_size, 1, UTF8_SIZE)) {
        for (u32 code_index = 0; code_index < code_size; ++code_index) {
          tg->buffer[write_index++] = cell->code[code_index];
        }
        decoded_size += code_size;
        continue;
      }
      // render as ascii
      tg->buffer[write_index++] = cell->code[0];
      ++decoded_size;
    }
    assert(decoded_size <= MAX_BUFFER_SIZE);
    i32 write_size = write(tg->tty, &tg->buffer[0], decoded_size);
    (void)write_size;
    tg_cursor_update(tg);
    fsync(tg->tty);
  }
  return tg->status;
}

void tg_box(u32 x_pos, u32 y_pos, u32 w, u32 h, char* title) {
  Termgui* tg = &term_gui;
  if (title) {
    tg_render_box_with_title(tg, x_pos, y_pos, w, h, title);
    return;
  }
  tg_render_box(tg, x_pos, y_pos, w, h);
}

u32 tg_width() {
  return term_gui.width;
}

u32 tg_height() {
  return term_gui.height;
}

i32 tg_handle_input(Termgui* tg) {
  char input = 0;
  i32 read_size = read(tg->tty, &input, 1);
  if (read_size > 0) {
    switch (input) {
      case 4: { // ^D
        tg->status = Done;
        break;
      }
      default:
        break;
    }
    tg->render_event = 1;
  }
  return read_size;
}

void tg_prepare_frame(Termgui* tg) {
  Cell cell;
  tg_cell_init_ascii(&cell, ' ');
  tg_cells_init(tg, &cell);
}

void tg_plot(Termgui* tg, u32 x, u32 y, Item item) {
  Cell cell;
  tg_cell_init_ascii(&cell, item);
  tg_plot_cell(tg, x, y, &cell);
}

void tg_plot_cell(Termgui* tg, u32 x, u32 y, Cell* cell) {
  x = CLAMP(x, 0, tg->width - 1);
  y = CLAMP(y, 0, tg->height - 1);
  memcpy(&tg->cells[y * tg->width + x], cell, sizeof(Cell));
}

// TODO(lucas): wrap to border (to terminal border and user defined border)
void tg_render_box(Termgui* tg, u32 x_pos, u32 y_pos, u32 w, u32 h) {
  x_pos = CLAMP(x_pos, 0, tg->width - 1);
  y_pos = CLAMP(y_pos, 0, tg->height - 1);
  w = CLAMP(w, 0, w);
  h = CLAMP(h, 0, h);

  for (u32 y = 1; y < h - 1; ++y) {
    tg_plot_cell(tg, x_pos, y_pos + y, &border_cell_vertical);
    tg_plot_cell(tg, x_pos + w - 1, y_pos + y, &border_cell_vertical);
  }
  for (u32 x = 1; x < w - 1; ++x) {
    tg_plot_cell(tg, x_pos + x, y_pos, &border_cell_horizontal);
    tg_plot_cell(tg, x_pos + x, y_pos + h - 1, &border_cell_horizontal);
  }
  tg_plot_cell(tg, x_pos, y_pos, &border_cell_corners[BORDER_CELL_TOP_LEFT]);
  tg_plot_cell(tg, x_pos + w - 1, y_pos, &border_cell_corners[BORDER_CELL_TOP_RIGHT]);
  tg_plot_cell(tg, x_pos, y_pos + h - 1, &border_cell_corners[BORDER_CELL_BOTTOM_LEFT]);
  tg_plot_cell(tg, x_pos + w - 1, y_pos + h - 1, &border_cell_corners[BORDER_CELL_BOTTOM_RIGHT]);
}

void tg_render_box_with_title(Termgui* tg, u32 x_pos, u32 y_pos, u32 w, u32 h, char* title) {
  tg_render_box(tg, x_pos, y_pos, w, h);
  tg_render_horizontal_line(tg, x_pos, y_pos + 2, w);
  tg_render_text(tg, x_pos + 1, y_pos + 1, title, w - 2);
}

void tg_render_text(Termgui* tg, u32 x_pos, u32 y_pos, char* text, u32 length) {
  for (u32 i = 0; i < length; ++i) {
    Item item = text[i];
    if (item == 0) {
      break;
    }
    Cell cell;
    tg_cell_init_ascii(&cell, item);
    cell.fg = COLOR_RED;
    tg_plot_cell(tg, x_pos, y_pos, &cell);
    x_pos++;
  }
}

void tg_render_horizontal_line(Termgui* tg, u32 x_pos, u32 y_pos, u32 length) {
  x_pos = CLAMP(x_pos, 0, tg->width - 1);
  y_pos = CLAMP(y_pos, 0, tg->height - 1);
  tg_plot_cell(tg, x_pos, y_pos, &border_cell_corners[BORDER_CELL_CONN_LEFT]);
  tg_plot_cell(tg, x_pos + length - 1, y_pos, &border_cell_corners[BORDER_CELL_CONN_RIGHT]);
  if (length > 2) {
    for (u32 i = 1; i < length - 1; ++i) {
      tg_plot_cell(tg, x_pos + i, y_pos, &border_cell_horizontal);
    }
  }
}

void tg_render_vertical_line(Termgui* tg, u32 x_pos, u32 y_pos, u32 length) {

}

void tg_cursor_move(i32 delta_x, i32 delta_y) {
  Termgui* tg = &term_gui;
  tg->cursor_x = CLAMP(tg->cursor_x + delta_x, 0, tg->width);
  tg->cursor_y = CLAMP(tg->cursor_y + delta_y, 0, tg->height);
  tg->render_event = 1;
}

void tg_exit() {
  term_gui.status = Done;  
}

char* tg_err_string() {
  return err_string;
}

void tg_print_error() {
  if (term_gui.status == Error) {
    dprintf(term_gui.fd, "[termgui-error]: %s\n", tg_err_string());
  }
}

// https://git.suckless.org/st/
i8 utf8_decode_byte(i8 byte, u32* size) {
  for (*size = 0; *size < LENGTH(utf8_mask); ++(*size)) {
    if (((u8)byte & utf8_mask[*size]) == utf8_byte[*size]) {
      return (u8)byte & ~utf8_mask[*size];
    }
  }
  return 0;
}

void tg_cells_init(Termgui* tg, Cell* cell) {
  for (u32 i = 0; i < tg->size; ++i) {
    memcpy(&tg->cells[i], cell, sizeof(Cell));
  }
}

void tg_cell_init(Cell* cell) {
  memset(cell, 0, sizeof(Cell));
  cell->fg = COLOR_NONE;
}

void tg_cell_init_ascii(Cell* cell, i8 ascii) {
  tg_cell_init(cell);
  cell->code[0] = ascii;
}

void tg_free() {
  Termgui* tg = &term_gui;
  tcsetattr(tg->tty, TCSANOW, &tg->term_copy); // reset tty state
  close(tg->fd);
}

void tg_cursor_update(Termgui* tg) {
  dprintf(tg->tty, "\033[%d;%df", tg->cursor_y, tg->cursor_x);
}

void tg_term_fetch_size(Termgui* tg) {
  struct winsize win;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
  tg->width = win.ws_col;
  tg->height = win.ws_row;
  tg->size = tg->width * tg->height;
  tg->render_event = 1;
}

void tg_term_clear(Termgui* tg) {
  const i8 clear_code[] = { 27, 91, 50, 74, 27, 91, 72 };
  i32 write_size = write(tg->tty, &clear_code[0], sizeof(clear_code));
  (void)write_size;
}

Result tg_term_init(Termgui* tg) {
  if (!isatty(tg->tty)) {
    return Err("device is not a tty");
  }
  if (tcgetattr(tg->tty, &tg->term) < 0) {
    return Err("could not get the terminal state");
  }
  memcpy(&tg->term_copy, &tg->term, sizeof(struct termios));

  tg->term.c_lflag &= ~(ICANON | ECHO);
  tg->term.c_cc[VMIN] = 0;
  tg->term.c_cc[VTIME] = 0;
  tcsetattr(tg->tty, TCSANOW, &tg->term);
  fcntl(tg->tty, F_SETFL, fcntl(tg->tty, F_GETFL));

  tg_term_fetch_size(tg);
  return NoError;
}

void sigwinch() {
  tg_queue_sig_event(&term_gui, SIG_EVENT_WINCH);
}

void sigint(i32 sig) {
  (void)sig;
  tg_queue_sig_event(&term_gui, SIG_EVENT_INT);
}

void tg_queue_sig_event(Termgui* tg, Signal_event event) {
  if (signal_event_count < MAX_SIG_EVENT) {
    signal_events[signal_event_count++] = event;
  }
  else {
    // event queue is full, handle the events and try again
    tg_handle_sig_events(tg);
    tg_queue_sig_event(tg, event);
  }
}

void tg_handle_sig_events(Termgui* tg) {
  foreach(i, signal_event_count) {
    signal_callback callback = signal_event_callbacks[signal_events[i]];
    callback(tg);
  }
  signal_event_count = 0;
}

void tg_sig_event_winch(Termgui* tg) {
  tg_term_fetch_size(tg);
  dprintf(tg->fd, "width = %d, height = %d\n", tg->width, tg->height);
}

void tg_sig_event_int(Termgui* tg) {
  tg->status = Done;
}

#endif // _TERMGUI_H
