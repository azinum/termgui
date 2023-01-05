// edit.c

#include "termgui.h"

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

    #define MAX_BUFFER_SIZE 512
    char buffer[MAX_BUFFER_SIZE] = {0};
    char* begin = &buffer[0];
    char* end = &buffer[MAX_BUFFER_SIZE - 1];
    char* it = begin;
    {
      Element text_element;
      tg_text_init(&text_element, buffer);
      text_element.border = 0;
      text_element.focusable = 0;
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
      char input = 0;
      if (tg_input(&input)) {
        if (input >= 32 && input <= 126) {
          if (it < end) {
            *it++ = input;
          }
        }
        else {
          switch (input) {
            case ASCII_DEL: {
              if (it > begin) {
                *--it = 0;
              }
              break;
            }
            case ASCII_LF: {
              if (it < end) {
                *it++ = input;
              }
              break;
            }
            default:
              // TODO(lucas): handle more escape codes
              break;
          }
        }
      }
      tg_render();
    }
    tg_free();
  }
  tg_print_error();
  return 0;
}
