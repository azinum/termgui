// grid.c

#include "termgui.h"

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("Usage; %s <num cols> <num rows>\n", argv[0]);
    return 0;
  }
  i32 cols = atoi(argv[1]);
  i32 rows = atoi(argv[2]);
  i32 count = cols * rows;
  if (Ok(tg_init())) {
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
      tg_render();
    }
    tg_free();
  }
  tg_print_error();
  return 0;
}
