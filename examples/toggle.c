// toggle.c

#include "termgui.h"

void on_toggle_callback(Element* e, void* userdata, u8 toggle_value);

int main(void) {
  if (Ok(tg_init())) {
    u32 rows = 3;
    u32 cols = 3;
    u32 count = rows * cols;
    Element grid_element;
    tg_grid_init(&grid_element, cols, false);
    grid_element.focusable = 0;
    Element* grid = tg_attach_element(NULL, &grid_element);

    for (u32 i = 0; i < count; ++i) {
      Element container_element;
      tg_container_init(&container_element, false /* render */);
      container_element.padding = 2;
      container_element.focusable = false;

      // initialize toggle element
      Element toggle_element;
      tg_toggle_init(&toggle_element, true, on_toggle_callback);

      // attach container to grid
      Element* container = tg_attach_element(grid, &container_element);
      // attach toggle element to container
      tg_attach_element(container, &toggle_element);
    }

    while (Ok(tg_update())) {
      tg_render();
    }
    tg_free();
  }
  tg_print_error();
  return 0;
}

void on_toggle_callback(Element* e, void* userdata, u8 toggle_value) {
  e->border = toggle_value;
}
