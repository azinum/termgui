// termgui.h
//
// TODO:
// - limit the window size
// - validate utf-8
// - configurable colors for elements
// - element padding
// - render n*n grid with m items where m<n*n
// - fix terminal flickering
// - add ascii only mode for terminals with no utf-8 support

#ifndef _TERMGUI_H
#define _TERMGUI_H

#include <stdlib.h> // malloc, realloc, free
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
#include <math.h> // floorf, ceilf

#define TERMGUI_API static
#define Ok(err) (err == NoError)
#define Err(message) (err_string = message, term_gui.status = Error, Error)

#ifdef CUSTOM_ALLOCATOR
  #define MALLOC
  #define FREE
  #define REALLOC
#else
  #define MALLOC malloc
  #define FREE free
  #define REALLOC realloc
#endif

// internal macros
#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, x_min, x_max) MIN(MAX(x_min, x), x_max)
#define UTF8_SIZE (4)
#define COLOR_CODE_SIZE 7
#define MAX_CELL_COUNT (300 * 80)
#define MAX_TERM_BUFFER_SIZE (UTF8_SIZE * MAX_CELL_COUNT * COLOR_CODE_SIZE)
#define LENGTH(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#define IN_BOUNDS(x, a, b) ((x >= a) && (x <= b))

#define INIT_ITEMS_SIZE 32
#define list_init(list, desired_size) \
  if ((list)->size < desired_size) { \
    (list)->size = desired_size; \
    (list)->items = REALLOC((list)->items, (list)->size * sizeof(*(list)->items)); \
    assert((list)->items != NULL && "out of memory"); \
  }

#define list_push(list, item) \
  if ((list)->count >= (list)->size) { \
    if ((list)->size == 0) { \
      (list)->size = INIT_ITEMS_SIZE; \
    } \
    else { \
      (list)->size *= 2; \
    } \
    (list)->items = REALLOC((list)->items, (list)->size * sizeof(*(list)->items)); \
    assert((list)->items != NULL && "out of memory"); \
  } \
  (list)->items[(list)->count++] = (item)

#define list_free(list) FREE((list)->items)
// end of internal macros

typedef int32_t i32;
typedef uint32_t u32;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;
typedef float f32;

static const char* log_file_name = "log.txt";

// https://www.asciitable.com/
enum Ascii_code {
  ASCII_NUL = 0, // null
  ASCII_SOH, // start of heading
  ASCII_STX, // start of text
  ASCII_ETX, // end of text
  ASCII_EOT, // end of transmission
  ASCII_ENQ, // enquiry
  ASCII_ACK, // acknowledge
  ASCII_BEL, // bell
  ASCII_BS,  // backspace
  ASCII_TAB, // horizontal tab
  ASCII_LF,  // line feed
  ASCII_VT,  // vertical tab
  ASCII_FF,  // form feed, new page
  ASCII_CR,  // carriage return
  ASCII_SO,  // shift out
  ASCII_SI,  // shift in
  ASCII_DLE, // data link escape
  ASCII_DC1, // device control 1
  ASCII_DC2, // device control 2
  ASCII_DC3, // device control 3
  ASCII_DC4, // device control 4
  ASCII_NAK, // negative acknowledge
  ASCII_SYN, // synchronous idle
  ASCII_ETB, // end of transmission block
  ASCII_CAN, // cancel
  ASCII_EM,  // end of medium
  ASCII_SUB, // substitute
  ASCII_ESC, // escape
  ASCII_FS,  // file separator
  ASCII_GS,  // group separator
  ASCII_RS,  // record separator
  ASCII_US,  // unit separator

  ASCII_DEL = 127
};

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

static Color color_default = COLOR_NONE;
static Color color_focus = COLOR_BOLD_RED;
static Color color_text = COLOR_NONE;

static char KEY_EXIT         = 4; // ctrl+d
static char KEY_SELECT       = 10; // enter
static char KEY_SWITCH_FOCUS = '\t';

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

typedef char Item;

enum Border_cell_orientation {
  BORDER_CELL_TOP_LEFT = 0,
  BORDER_CELL_TOP_RIGHT,
  BORDER_CELL_BOTTOM_LEFT,
  BORDER_CELL_BOTTOM_RIGHT,

  BORDER_CELL_CONN_LEFT,
  BORDER_CELL_CONN_RIGHT,
  BORDER_CELL_CONN_TOP,
  BORDER_CELL_CONN_BOTTOM,

  BORDER_CELL_VERTICAL,
  BORDER_CELL_HORIZONTAL,

  MAX_BORDER_CELL
};

static Cell border_cells[MAX_BORDER_CELL] = {
  { .code = { 0xe2, 0x95, 0xad, 0x00 }, .fg = COLOR_NONE }, // top left
  { .code = { 0xe2, 0x95, 0xae, 0x00 }, .fg = COLOR_NONE }, // top right
  { .code = { 0xe2, 0x95, 0xb0, 0x00 }, .fg = COLOR_NONE }, // bottom left
  { .code = { 0xe2, 0x95, 0xaf, 0x00 }, .fg = COLOR_NONE }, // bottom right

  { .code = { 0xe2, 0x94, 0x9c, 0x00 }, .fg = COLOR_NONE }, // left connection
  { .code = { 0xe2, 0x94, 0xa4, 0x00 }, .fg = COLOR_NONE }, // right connection
  { .code = { 0xe2, 0x94, 0xac, 0x00 }, .fg = COLOR_NONE }, // top connection
  { .code = { 0xe2, 0x94, 0xb4, 0x00 }, .fg = COLOR_NONE }, // bottom connection

  { .code = { 0xe2, 0x94, 0x82, 0x00 }, .fg = COLOR_NONE }, // vertical
  { .code = { 0xe2, 0x94, 0x80, 0x00 }, .fg = COLOR_NONE }, // horizontal
};

typedef enum Result { NoError, Error, Done } Result;
static char* err_string = "";

typedef enum Element_type {
  ELEM_NONE = 0,
  ELEM_CONTAINER,
  ELEM_GRID,
  ELEM_TEXT,

  MAX_ELEMENT_TYPE
} Element_type;

const char* element_type_str[MAX_ELEMENT_TYPE] = {
  "none",
  "container",
  "grid",
  "text",
};

typedef enum Focus_state {
  FOCUS_NONE = 0,
  FOCUS,
  FOCUS_SWITCH,
} Focus_state;

const char* bool_str[] = { "false", "true" };

struct Element;
typedef void (*element_callback)(struct Element* element, void* userdata);
typedef void (*element_input_callback)(struct Element* element, void* userdata, char input);

typedef struct Box {
  u32 x;
  u32 y;
  u32 w;
  u32 h;
} Box;

#define BOX(X, Y, W, H) ((Box) { .x = X, .y = Y, .w = W, .h = H })

typedef union Element_data {
  struct {
    u32 cols;
  } grid;
  struct {
    char* string;
  } text;
} Element_data;

typedef struct Element {
  struct Element* items; // children
  u32 count;
  u32 size;

  u32 id;
  Box box;
  Element_type type;

  element_callback select_callback;
  element_input_callback input_callback;
  void* userdata;

  Element_data data;
  u32 padding;
  u8 render; // render this element?
  u8 border; // render a border of this element? only works if render is set to true
  u8 focus; // is the element in focus?
  u8 focusable; // can this element be focused?
} Element;

typedef struct Ui_state {
  Element root;
  u32 id_counter;
} Ui_state;

typedef struct Termgui {
  i32 tty;
  struct termios term_copy;
  struct termios term;
  u32 width;
  u32 height;
  u32 size; // width * height
  Cell cells[MAX_CELL_COUNT];
  char buffer[MAX_TERM_BUFFER_SIZE];
  i32 cursor_x;
  i32 cursor_y;
  i32 render_event;
  Focus_state switch_focus;
  i32 use_colors;
  i32 initialized;
  Result status;
  i32 fd; // fd for logging
  i32 input_event;
  char input_code;
  Ui_state ui;
} Termgui;

static Termgui term_gui = {0};

// api functions
TERMGUI_API Result tg_init();
TERMGUI_API Result tg_update();
TERMGUI_API Result tg_render();
TERMGUI_API i32 tg_input(char* input);
TERMGUI_API u32 tg_width();
TERMGUI_API u32 tg_height();
TERMGUI_API void tg_cursor_move(i32 delta_x, i32 delta_y);
TERMGUI_API void tg_exit();
TERMGUI_API void tg_colors_toggle();
TERMGUI_API char* tg_err_string();
TERMGUI_API void tg_print_error();
TERMGUI_API i32 tg_log_fd();
TERMGUI_API void tg_free();

TERMGUI_API void tg_empty_init(Element* e);
TERMGUI_API void tg_container_init(Element* e, u8 render);
TERMGUI_API void tg_grid_init(Element* e, u32 cols, u8 render);
TERMGUI_API void tg_text_init(Element* e, char* text);
TERMGUI_API Element* tg_attach_element(Element* target, Element* e);

static u8 utf8_decode_byte(u8 byte, u32* size);

static void tg_cells_init(Termgui* tg, Cell* cell);
static void tg_cell_init(Cell* cell);
static void tg_cell_init_ascii(Cell* cell, char ascii);

static i32 tg_handle_input(Termgui* tg);
static void tg_prepare_frame(Termgui* tg);
static void tg_plot(Termgui* tg, u32 x, u32 y, Item item);
static void tg_plot_cell(Termgui* tg, u32 x, u32 y, Cell* cell);
static void tg_plot_cell_color(Termgui* tg, u32 x, u32 y, Cell* cell, Color fg_color);
static void tg_render_box(Termgui* tg, Box box, Color fg_color);
static void tg_render_text_ascii(Termgui* tg, Box box, char* text);
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

static void ui_state_init(Termgui* tg);
static void ui_element_init(Termgui* tg, Element* e);
static void ui_update_elements(Termgui* tg, Element* e);
static void ui_update(Termgui* tg);
static void ui_render_elements(Termgui* tg, Element* e);
static void ui_render(Termgui* tg);
static void ui_print_element(Termgui* tg, Element* e);
static void ui_print_elements(Termgui* tg, Element* e, u32 level);
static void ui_elements_free(Element* e);
static void ui_state_free(Termgui* tg);

static void tabs(u32 fd, const u32 count);

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
    tg->switch_focus = FOCUS_SWITCH; // set to FOCUS_SWITCH to find the first focusable element
    tg->use_colors = 1;
    tg->initialized = 1;
    tg->status = NoError;
    tg->fd = open(log_file_name, O_CREAT | O_TRUNC | O_WRONLY, 0662);
    tg->input_event = 0;
    tg->input_code = 0;
    ui_state_init(tg);

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
  // make sure to clear all events
  tg->render_event = 0;
  tg->input_code = 0;
  tg->input_event = 0;
  tg_handle_sig_events(tg);
  tg_handle_input(tg);
  if (!Ok(tg->status)) {
    return tg->status;
  }
  ui_update(tg);
  return tg->status;
}

Result tg_render() {
  Termgui* tg = &term_gui;
  if (tg->render_event) {
    ui_render(tg);
    tg_term_clear(tg);
    // 1) decode each cell
    // 2) write to buffer
    // 3) write buffer to tty
    i32 decoded_size = 0;
    i32 write_index = 0;
    for (u32 i = 0; i < tg->size; ++i) {
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
    assert(decoded_size <= MAX_TERM_BUFFER_SIZE);
    i32 write_size = write(tg->tty, &tg->buffer[0], decoded_size);
    (void)write_size;
    tg_cursor_update(tg);
    fsync(tg->tty);
  }
  return tg->status;
}

i32 tg_input(char* input) {
  Termgui* tg = &term_gui;
  i32 input_event = tg->input_event;
  if (input && input_event) {
    *input = tg->input_code;
  }
  return input_event;
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
    if (input == KEY_EXIT) {
      tg->status = Done;
    }
    else if (input == KEY_SWITCH_FOCUS) {
      tg->switch_focus = FOCUS;
    }
    tg->render_event = 1;
    tg->input_event = 1;
    tg->input_code = input;
  }
  else {
    tg->input_event = 0;
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

void tg_plot_cell_color(Termgui* tg, u32 x, u32 y, Cell* cell, Color fg_color) {
  Cell c = *cell;
  c.fg = fg_color;
  tg_plot_cell(tg, x, y, &c);
}

void tg_render_box(Termgui* tg, Box box, Color fg_color) {
  box.x = CLAMP(box.x, 0, tg->width - 1);
  box.y = CLAMP(box.y, 0, tg->height - 1);
  box.w = CLAMP(box.w, 0, tg->width - box.x);
  box.h = CLAMP(box.h, 0, tg->height - box.y);

  if (!box.w || !box.h) {
    return;
  }

  for (u32 y = 1; y < box.h - 1; ++y) {
    tg_plot_cell_color(tg, box.x, box.y + y, &border_cells[BORDER_CELL_VERTICAL], fg_color);
    tg_plot_cell_color(tg, box.x + box.w - 1, box.y + y, &border_cells[BORDER_CELL_VERTICAL], fg_color);
  }
  for (u32 x = 1; x < box.w - 1; ++x) {
    tg_plot_cell_color(tg, box.x + x, box.y, &border_cells[BORDER_CELL_HORIZONTAL], fg_color);
    tg_plot_cell_color(tg, box.x + x, box.y + box.h - 1, &border_cells[BORDER_CELL_HORIZONTAL], fg_color);
  }
  tg_plot_cell_color(tg, box.x, box.y, &border_cells[BORDER_CELL_TOP_LEFT], fg_color);
  tg_plot_cell_color(tg, box.x + box.w - 1, box.y, &border_cells[BORDER_CELL_TOP_RIGHT], fg_color);
  tg_plot_cell_color(tg, box.x, box.y + box.h - 1, &border_cells[BORDER_CELL_BOTTOM_LEFT], fg_color);
  tg_plot_cell_color(tg, box.x + box.w - 1, box.y + box.h - 1, &border_cells[BORDER_CELL_BOTTOM_RIGHT], fg_color);
}

void tg_render_text_ascii(Termgui* tg, Box box, char* text) {
  if (!text) {
    return;
  }
  if (!box.w || !box.h) {
    return;
  }
  u32 x = 0;
  u32 y = 0;
  for (;;) {
    char ch = *text++;
    if (ch == 0) {
      break;
    }
    if (ch == '\n') {
      x = 0;
      ++y;
      continue;
    }
    if (x >= box.w) {
      x = 0;
      ++y;
    }
    if (y >= box.h) {
      break;
    }
    Cell cell;
    tg_cell_init_ascii(&cell, ch);
    cell.fg = color_text;
    tg_plot_cell(tg, box.x + x, box.y + y, &cell);
    ++x;
  }
}

void tg_render_horizontal_line(Termgui* tg, u32 x_pos, u32 y_pos, u32 length) {
  x_pos = CLAMP(x_pos, 0, tg->width - 1);
  y_pos = CLAMP(y_pos, 0, tg->height - 1);
  tg_plot_cell(tg, x_pos, y_pos, &border_cells[BORDER_CELL_CONN_LEFT]);
  tg_plot_cell(tg, x_pos + length - 1, y_pos, &border_cells[BORDER_CELL_CONN_RIGHT]);
  if (length > 2) {
    for (u32 i = 1; i < length - 1; ++i) {
      tg_plot_cell(tg, x_pos + i, y_pos, &border_cells[BORDER_CELL_HORIZONTAL]);
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

void tg_colors_toggle() {
  Termgui* tg = &term_gui;
  tg->use_colors = !tg->use_colors;
}

char* tg_err_string() {
  return err_string;
}

void tg_print_error() {
  if (term_gui.status == Error) {
    dprintf(term_gui.fd, "[termgui-error]: %s\n", tg_err_string());
    dprintf(STDERR_FILENO, "[termgui-error]: %s\n", tg_err_string());
  }
}

i32 tg_log_fd() {
  Termgui* tg = &term_gui;
  return tg->fd;
}

// https://git.suckless.org/st/
u8 utf8_decode_byte(u8 byte, u32* size) {
  for (*size = 0; *size < LENGTH(utf8_mask); ++(*size)) {
    if ((byte & utf8_mask[*size]) == utf8_byte[*size]) {
      return byte & ~utf8_mask[*size];
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

void tg_cell_init_ascii(Cell* cell, char ascii) {
  tg_cell_init(cell);
  cell->code[0] = ascii;
}

void tg_free() {
  Termgui* tg = &term_gui;
  tcsetattr(tg->tty, TCSANOW, &tg->term_copy); // reset tty state
  ui_print_elements(tg, &tg->ui.root, 0);
  close(tg->fd);
  ui_state_free(tg);
}

void tg_empty_init(Element* e) {
  Termgui* tg = &term_gui;
  ui_element_init(tg, e);
  e->render = 0;
}

void tg_container_init(Element* e, u8 render) {
  Termgui* tg = &term_gui;
  ui_element_init(tg, e);
  e->type = ELEM_CONTAINER;
  e->render = render;
}

void tg_grid_init(Element* e, u32 cols, u8 render) {
  Termgui* tg = &term_gui;
  ui_element_init(tg, e);
  e->data.grid.cols = cols;
  e->type = ELEM_GRID;
  e->render = render;
}

void tg_text_init(Element* e, char* text) {
  Termgui* tg = &term_gui;
  ui_element_init(tg, e);
  e->data.text.string = text;
  e->type = ELEM_TEXT;
}

Element* tg_attach_element(Element* target, Element* e) {
  Termgui* tg = &term_gui;
  // if no target was specified, use the root
  if (!target) {
    target = &tg->ui.root;
  }
  u32 index = target->count;
  e->id = tg->ui.id_counter++;
  list_push(target, *e);
  return &target->items[index];
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
  const u8 clear_code[] = { 27, 91, 50, 74, 27, 91, 72 };
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
  for (u32 i = 0; i < signal_event_count; ++i) {
    signal_callback callback = signal_event_callbacks[signal_events[i]];
    callback(tg);
  }
  signal_event_count = 0;
}

void tg_sig_event_winch(Termgui* tg) {
  tg_term_fetch_size(tg);
}

void tg_sig_event_int(Termgui* tg) {
  tg->status = Done;
}

void ui_state_init(Termgui* tg) {
  Element* root = &tg->ui.root;
  tg->ui.id_counter = 1;
  ui_element_init(tg, &tg->ui.root);
  root->type = ELEM_CONTAINER;
  root->render = 0;
  root->focusable = 0;
}

void ui_element_init(Termgui* tg, Element* e) {
  e->items = NULL;
  e->count = 0;
  e->size = 0;

  e->id = 0;
  e->box = BOX(0, 0, 0, 0);
  e->type = ELEM_NONE;

  e->select_callback = NULL;
  e->input_callback = NULL;
  e->userdata = NULL;

  memset(&e->data, 0, sizeof(Element_data));
  e->padding = 0;
  e->render = 1;
  e->border = 1;
  e->focus = 0;
  e->focusable = 1;
}

void ui_update_elements(Termgui* tg, Element* e) {
  if (!e) {
    return;
  }
  if (tg->switch_focus == FOCUS_SWITCH && !e->focus && e->focusable) {
    e->focus = 1;
    tg->switch_focus = FOCUS_NONE;
    tg->render_event = 1;
  }
  if (tg->switch_focus == FOCUS && e->focus && e->focusable) {
    e->focus = 0;
    tg->switch_focus = FOCUS_SWITCH;
    tg->render_event = 1;
  }
  if (tg->input_event) {
    if (tg->input_code == KEY_SELECT && e->focus) {
      if (e->select_callback) {
        e->select_callback(e, e->userdata);
      }
    }
    if (e->input_callback) {
      e->input_callback(e, e->userdata, tg->input_code);
    }
  }
  for (u32 i = 0; i < e->count; ++i) {
    Element* item = &e->items[i];
    switch (e->type) {
      case ELEM_CONTAINER: {
        Box outer_box = e->box;
        item->box = BOX(
          outer_box.x + e->padding,
          outer_box.y + e->padding,
          outer_box.w - 2 * e->padding,
          outer_box.h - 2 * e->padding
        );
        break;
      }
      case ELEM_GRID: {
        Box pbox = e->box; // parent box
        if (e->data.grid.cols == 0) {
          item->box = BOX(
            pbox.x + floorf((f32)pbox.w/e->count * i) + e->padding,
            pbox.y + e->padding,
            ceilf((f32)pbox.w/e->count) - 2 * e->padding,
            pbox.h - 2 * e->padding
          );
        }
        else {
          u32 cols = e->data.grid.cols;
          u32 rows = e->count / e->data.grid.cols;
          u32 w = ceilf((f32)pbox.w / cols);
          u32 h = ceilf((f32)pbox.h / rows);
          u32 x = i % cols;
          u32 y = (u32)floorf((f32)i / cols);
          item->box = BOX(
            pbox.x + x * w + e->padding,
            pbox.y + y * h + e->padding,
            w - 2 * e->padding,
            h - 2 * e->padding
          );
        }
        break;
      }
      default:
        break;
    }
    ui_update_elements(tg, item);
  }
}

void ui_update(Termgui* tg) {
  Element* root = &tg->ui.root;
  root->box = BOX(0, 0, tg->width, tg->height);
  ui_update_elements(tg, root);
}

void ui_render_elements(Termgui* tg, Element* e) {
  if (!e) {
    return;
  }
  if (e->render) {
    Color fg_color = color_default;
    if (e->focus) {
      fg_color = color_focus;
    }
    switch (e->type) {
      case ELEM_TEXT: {
        tg_render_text_ascii(tg, e->box, e->data.text.string);
        break;
      }
      default:
        break;
    }
    if (e->border) {
      tg_render_box(tg, e->box, fg_color);
    }
  }
  for (u32 i = 0; i < e->count; ++i) {
    Element* item = &e->items[i];
    ui_render_elements(tg, item);
  }
}

void ui_render(Termgui* tg) {
  Element* root = &tg->ui.root;
  ui_render_elements(tg, root);
}

void ui_print_element(Termgui* tg, Element* e) {
  dprintf(
    tg->fd,
    "Element {\n"
    "  id: %d\n"
    "  box: {%d, %d, %d, %d}\n"
    "  focus: %d\n"
    "}\n"
    ,
    e->id,
    e->box.x,
    e->box.y,
    e->box.w,
    e->box.h,
    e->focus
  );
}

void ui_print_elements(Termgui* tg, Element* e, u32 level) {
  dprintf(tg->fd, "{\n");
  tabs(tg->fd, level + 1); dprintf(tg->fd, "type: %s\n", element_type_str[e->type]);
  tabs(tg->fd, level + 1); dprintf(tg->fd, "id: %d\n", e->id);
  tabs(tg->fd, level + 1); dprintf(tg->fd, "render: %s\n", bool_str[e->render == 1]);
  tabs(tg->fd, level + 1); dprintf(tg->fd, "focus: %s\n", bool_str[e->focus == 1]);
  tabs(tg->fd, level + 1); dprintf(tg->fd, "box: {%d, %d, %d, %d}\n", e->box.x, e->box.y, e->box.w, e->box.h);

  for (u32 i = 0; i < e->count; ++i) {
    tabs(tg->fd, level + 1);
    ui_print_elements(tg, &e->items[i], level + 1);
  }
  tabs(tg->fd, level);
  dprintf(tg->fd, "}\n");
}

void ui_elements_free(Element* e) {
  if (!e) {
    return;
  }
  for (u32 i = 0; i < e->count; ++i) {
    ui_elements_free(&e->items[i]);
  }
  list_free(e);
}

void ui_state_free(Termgui* tg) {
  ui_elements_free(&tg->ui.root);
}

void tabs(u32 fd, const u32 count) {
  const char* tab = "   ";
  for (u32 i = 0; i < count; ++i) {
    dprintf(fd, "%s", tab);
  }
}

#undef MIN
#undef MAX
#undef CLAMP
#undef UTF8_SIZE
#undef COLOR_CODE_SIZE
#undef MAX_CELL_COUNT
#undef MAX_TERM_BUFFER_SIZE
#undef LENGTH
#undef IN_BOUNDS

#undef INIT_ITEMS_SIZE
#undef list_init
#undef list_push
#undef list_free

#endif // _TERMGUI_H
