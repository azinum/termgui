// dynamic_grid.c

#include "termgui.h"

Element* grid = NULL;

void on_input_event(Element* e, void* userdata, char input) {
  switch (input) {
    case 'e': {
      Element empty;
      tg_empty_init(&empty);
      empty.render = 1;
      empty.border = 1;
      empty.focusable = 1;
      if (grid) {
        tg_attach_element(grid, &empty);
      }
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

i32 main(void) {
  if (Ok(tg_init())) {
    u32 rows = 3;
    u32 cols = 3;
    Element grid_element;
    tg_grid_init(&grid_element, cols, 0);
    grid_element.focusable = 0;
    grid_element.input_callback = on_input_event;
    grid = tg_attach_element(NULL, &grid_element);

    while (Ok(tg_update())) {
      tg_render();
    }
    tg_free();
  }
  tg_print_error();
  return 0;
}
