// main.c

#include "termgui.h"

i32 main(i32 argc, char** argv) {
  if (Ok(tg_init())) {
    Element grid;
    tg_grid_init(&grid, 4, 0);

    Element* g = tg_attach_element(NULL, &grid);
    tg_grid_init(&grid, 4, 1);
    tg_attach_element(g, &grid);
    tg_attach_element(g, &grid);
    tg_attach_element(g, &grid);
    tg_attach_element(g, &grid);
    tg_attach_element(g, &grid);
    tg_attach_element(g, &grid);
    tg_attach_element(g, &grid);
    tg_grid_init(&grid, 2, 0);
    Element* g2 = tg_attach_element(g, &grid);
    grid.render = 1;
    tg_attach_element(g2, &grid);
    tg_attach_element(g2, &grid);
    tg_attach_element(g2, &grid);
    tg_attach_element(g2, &grid);

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
