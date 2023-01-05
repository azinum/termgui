// main.c

#include "termgui.h"

void on_click(Element* e, void* userdata) {
  for (u32 i = 0; i < e->count; ++i) {
    Element* inner = &e->items[i];
    inner->border = !inner->border;
  }
}

i32 main(i32 argc, char** argv) {
  if (Ok(tg_init())) {
    u32 rows = 2;
    u32 cols = 2;
    u32 count = rows * cols;

    Element grid_element;
    tg_grid_init(&grid_element, cols, 0);
    grid_element.focusable = 0;
    Element* grid = tg_attach_element(NULL, &grid_element);

    char* text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Blandit cursus risus at ultrices mi tempus imperdiet.";
    Element text_element;
    tg_text_init(&text_element, text);
    text_element.border = 1;
    text_element.focusable = 0;

    for (u32 i = 0; i < count; ++i) {
      Element container_element;
      tg_container_init(&container_element, 1);
      container_element.padding = 4;
      container_element.callback = on_click;
      container_element.border = 1;

      Element* container = tg_attach_element(grid, &container_element);
      tg_attach_element(container, &text_element);
    }
    while (Ok(tg_update())) {
      char input = 0;
      if (tg_input(&input)) {
        if (input == 'c') {
          tg_colors_toggle();
        }
      }
      tg_render();
    }
    tg_free();
  }
  tg_print_error();
  return 0;
}
