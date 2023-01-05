// edit.c

#include "termgui.h"

typedef struct Buffer {
  char* data;
  u32 count;
  u32 size;
} Buffer;

void on_input(Element* e, void* userdata, char input);
void buffer_init(Buffer* buffer, char* data, u32 size);
void buffer_push(Buffer* buffer, char ch);
void buffer_pop(Buffer* buffer);

i32 main(i32 argc, char** argv) {
  if (Ok(tg_init())) {
    i32 fd = tg_log_fd();
    Element* grid = NULL;
    Element* containers[2] = {NULL, NULL};
    Element* text[2] = {NULL, NULL};

    Element grid_element;
    tg_grid_init(&grid_element, 2, 0);
    grid_element.padding = 0;
    grid_element.focusable = 0;

    grid = tg_attach_element(NULL, &grid_element); // attach grid to root

    Element text_container;
    tg_container_init(&text_container, 1);
    text_container.border = 1;
    text_container.padding = 1;
    text_container.focusable = 1;
    containers[0] = tg_attach_element(grid, &text_container);
    containers[1] = tg_attach_element(grid, &text_container);

    #define BUFFER_SIZE 512
    char buffer_data[BUFFER_SIZE] = {0};
    Buffer buffer;
    buffer_init(&buffer, &buffer_data[0], BUFFER_SIZE);

    {
      Element text_element;
      tg_text_init(&text_element, &buffer.data[0]);
      text_element.border = 0;
      text_element.focusable = 0;
      text_element.input_callback = on_input;
      text_element.userdata = &buffer;
      text[0] = tg_attach_element(containers[0], &text_element);
    }
    {
      Element text_element;
      tg_text_init(&text_element, "mode: insert");
      text_element.border = 0;
      text_element.focusable = 0;
      text[1] = tg_attach_element(containers[1], &text_element);
    }

    while (Ok(tg_update())) {
      tg_render();
    }
    tg_free();
  }
  tg_print_error();
  return 0;
}

void on_input(Element* e, void* userdata, char input) {
  Buffer* buffer = (Buffer*)userdata;
  switch (input) {
    case ASCII_DEL: {
      buffer_pop(buffer);
      break;
    }
    case ASCII_LF: {
      buffer_push(buffer, input);
      break;
    }
    default: {
      if (input >= 32 && input <= 126) {
        buffer_push(buffer, input);
      }
      break;
    }
  }
}

void buffer_init(Buffer* buffer, char* data, u32 size) {
  buffer->data = data;
  buffer->count = 0;
  buffer->size = size;
}

void buffer_push(Buffer* buffer, char ch) {
  if (buffer->count < buffer->size) {
    buffer->data[buffer->count++] = ch;
  }
}

void buffer_pop(Buffer* buffer) {
  if (buffer->count > 0) {
    buffer->data[--buffer->count] = 0;
  }
}
