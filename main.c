// main.c

#include "termgui.h"

int main(int argc, char** argv) {
  if (Ok(tg_init())) {
    tg_render();
    while (Ok(tg_update())) {
      tg_render();
    }
    tg_free();
  }
  tg_print_error();
  return 0;
}
