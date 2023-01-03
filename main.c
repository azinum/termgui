// main.c

#include "termgui.h"

char* text[2] = {
  "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Blandit cursus risus at ultrices mi tempus imperdiet.",
  "It is important to take care of the patient, to be followed by the doctor, but it is a time of great pain and suffering. I am flattered by the course of laughter, but my time is spent in basketball."
};

void on_click(Element* e, void* userdata) {
  for (u32 i = 0; i < e->count; ++i) {
    Element* inner = &e->items[i];
    if (inner->type == ELEM_TEXT) {
      inner->data.text.string = text[1];
    }
  }
}

i32 main(i32 argc, char** argv) {
  if (Ok(tg_init())) {
    u32 rows = 3;
    u32 cols = 2;
    u32 count = rows * cols;

    Element grid_element;
    tg_grid_init(&grid_element, cols, 0);
    grid_element.focusable = 0;
    Element* grid = tg_attach_element(NULL, &grid_element);

    Element text_element;
    tg_text_init(&text_element, text[0]);
    text_element.render = 1;
    text_element.border = 0;
    text_element.focusable = 0;

    for (u32 i = 0; i < count; ++i) {
      Element container_element;
      tg_container_init(&container_element, 1);
      container_element.callback = on_click;

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
