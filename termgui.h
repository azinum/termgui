// termgui.h

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

#define foreach(_i, _count) for (typeof(_count) _i = 0; _i < _count; ++_i)
#define MAX_BUFFER_SIZE (200 * 80)
#define TERMGUI_API static
#define Ok(err) (err == NoError)
#define Err(message) (err_string = message, term_gui.status = Error, Error)
#define MIN(x, y)(x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, x_min, x_max) MIN(MAX(x_min, x), x_max)

typedef int32_t i32;
typedef uint32_t u32;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;

typedef enum Result { NoError, Error, Done } Result;
static char* err_string = "";

typedef struct Termgui {
  i32 tty;
  struct termios term_copy;
  struct termios term;
  u32 width;
  u32 height;
  i8 buffer[MAX_BUFFER_SIZE];
  i32 cursor_x;
  i32 cursor_y;
  i32 render_event;
  i32 initialized;
  Result status;
  i32 fd; // fd for logging
} Termgui;

static Termgui term_gui = {0};

// api functions
TERMGUI_API Result tg_init();
TERMGUI_API Result tg_update();
TERMGUI_API Result tg_render();
TERMGUI_API u32 tg_width();
TERMGUI_API u32 tg_height();
TERMGUI_API void tg_cursor_move(i32 delta_x, i32 delta_y);
TERMGUI_API void tg_render_column(u32 column, i8 item);
TERMGUI_API void tg_exit();
TERMGUI_API char* tg_err_string();
TERMGUI_API void tg_print_error();
TERMGUI_API void tg_free();

i32 tg_handle_input(Termgui* tg);
static void tg_prepare_frame(Termgui* tg);
static void tg_plot(Termgui* tg, u32 x, u32 y, i8 item);
static void tg_box(Termgui* tg, u32 x_pox, u32 y_pos, u32 w, u32 h, i8 border_item);
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
    memset(&tg->buffer[0], ' ', sizeof(tg->buffer));
    tg->cursor_x = 0;
    tg->cursor_y = 0;
    tg->render_event = 1;
    tg->initialized = 1;
    tg->status = NoError;
    tg->fd = open("log.txt", O_CREAT | O_TRUNC | O_WRONLY, 0662);

    signal(SIGWINCH, sigwinch);
    signal(SIGINT, sigint);
    tg_queue_sig_event(tg, SIG_EVENT_WINCH);
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
  tg_box(tg, (tg->width >> 1) - 6, (tg->height >> 1) - 3, 12, 6, '+');
  return tg->status;
}

Result tg_render() {
  Termgui* tg = &term_gui;
  if (tg->render_event) {
    tg_term_clear(tg);
    i32 write_size = write(tg->tty, &tg->buffer[0], tg->width * tg->height);
    tg_cursor_update(tg);
    fsync(tg->tty);
    (void)write_size;
  }
  return tg->status;
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
  memset(&tg->buffer[0], ' ', tg->width * tg->height);
}

void tg_plot(Termgui* tg, u32 x, u32 y, i8 item) {
  x = CLAMP(x, 0, tg->width - 1);
  y = CLAMP(y, 0, tg->height - 1);
  tg->buffer[y * tg->width + x] = item;
}

// TODO(lucas): wrap to border (to terminal border and user defined border)
void tg_box(Termgui* tg, u32 x_pos, u32 y_pos, u32 w, u32 h, i8 border_item) {
  x_pos = CLAMP(x_pos, 0, tg->width - 1);
  y_pos = CLAMP(y_pos, 0, tg->height - 1);
  w = CLAMP(w, 0, w);
  h = CLAMP(w, 0, h);

  for (u32 y = 0; y < h; ++y) {
    tg_plot(tg, x_pos, y_pos + y, border_item);
    tg_plot(tg, x_pos + w - 1, y_pos + y, border_item);
  }
  for (u32 x = 0; x < w; ++x) {
    tg_plot(tg, x_pos + x, y_pos, border_item);
    tg_plot(tg, x_pos + x, y_pos + h - 1, border_item);
  }
}

void tg_cursor_move(i32 delta_x, i32 delta_y) {
  Termgui* tg = &term_gui;
  tg->cursor_x += delta_x;
  tg->cursor_y += delta_y;
  tg->render_event = 1;
}

void tg_render_column(u32 column, i8 item) {
  Termgui* tg = &term_gui;
  if (column >= tg->width) {
    column = tg->width - 1;
  }
  for (u32 y = 0; y < tg->height; ++y) {
    tg_plot(tg, column, y, item);
  }
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

void tg_free() {
  Termgui* tg = &term_gui;
  tcsetattr(tg->tty, TCSANOW, &tg->term_copy); // reset tty state
  close(tg->fd);
}

void tg_cursor_update(Termgui* tg) {
  dprintf(tg->tty, "\033[%d;%df", tg->cursor_x, tg->cursor_y);
}

void tg_term_fetch_size(Termgui* tg) {
  struct winsize win;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
  tg->width = win.ws_col;
  tg->height = win.ws_row;
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
