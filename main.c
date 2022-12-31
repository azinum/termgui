// main.c

#include "termgui.h"

int main(int argc, char** argv) {
  if (Ok(tg_init())) {
    tg_render();
    while (Ok(tg_update())) {
      tg_box(0, 2, 48, 10, "here is a title");
      tg_box(49, 2, 48, 10, "here is a title");
      tg_box(10, 5, 24, 12, "title here");
      // tg_box_with_title(tg, (tg->width >> 1) - 6, (tg->height >> 1) - 3, 28, 10, "tg_box");
      tg_render();
    }
    tg_free();
  }
  tg_print_error();
  return 0;
}
