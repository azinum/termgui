// main.c

#include "termgui.h"

i32 main(i32 argc, char** argv) {
  if (Ok(tg_init())) {
    u32 rows = 3;
    u32 cols = 3;
    u32 count = rows * cols;
    Element grid;
    tg_grid_init(&grid, cols, 0);
    Element* g = tg_attach_element(NULL, &grid);
    Element empty;
    tg_empty_init(&empty);
    empty.render = 1;
    for (u32 i = 0; i < count; ++i) {
      tg_attach_element(g, &empty);
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
