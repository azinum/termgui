// dynamic_grid.c

#include "termgui.h"

void on_input_event(Element* e, void* userdata, const char* input, u32 size);

i32 main(void) {
  if (Ok(tg_init())) {
    u32 cols = 1;
    Element grid_element;
    tg_grid_init(&grid_element, cols, false);
    grid_element.focusable = 0;
    grid_element.input_callback = on_input_event;
    tg_attach_element(NULL, &grid_element);

    while (Ok(tg_update())) {
      tg_render();
    }
    tg_free();
  }
  tg_print_error();
  return 0;
}

void on_input_event(Element* e, void* userdata, const char* input, u32 size) {
  if (!size) {
    return;
  }
  char ch = *input;
  switch (ch) {
    case 'e': {
      // initialize container
      Element container_element;
      tg_container_init(&container_element, true /* render */);
      container_element.padding = 2;
      container_element.data.text.string = "title";

      // initialize text element
      char* text = "Blandit cursus risus at ultrices mi tempus imperdiet.";
      Element text_element;
      tg_text_init(&text_element, text);
      text_element.border = false;
      text_element.focusable = false;

      // attach container to grid
      Element* container = tg_attach_element(e, &container_element);
      // attach text element to container
      tg_attach_element(container, &text_element);
      break;
    }
    case 'w': {
      e->data.grid.cols++;
      break;
    }
    case 'q': {
      if (e->data.grid.cols > 1) {
        e->data.grid.cols--;
      }
      break;
    }
    default:
      break;
  }
}
