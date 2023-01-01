// main.c

#include "termgui.h"

int main(int argc, char** argv) {
  if (Ok(tg_init())) {
    while (Ok(tg_update())) {
      tg_box(0, 0, tg_width(), tg_height(), "termgui example");
      tg_box(1, 3, 48, 10, "here is a title");
      tg_box(50, 3, 32, 10, "here is another title");
      tg_box(1, 13, 24, 12, "yet another title");
      tg_render();
    }
    tg_free();
  }
  tg_print_error();
  return 0;
}
