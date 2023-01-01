// utf8code.c
// a tool that prints the hexadecimal representation of utf-8 characters

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

#define EXTRACTBIT(ith, byte) (byte & (1 << ith))
#define LENGTH(ARR) (sizeof(ARR) / sizeof(ARR[0]))
void print_bits(char byte) {
  for (int bit = 7; bit >= 0; --bit) {
    printf("%d", EXTRACTBIT(bit, byte) != 0);
  }
}

int main(int argc, char** argv) {
  setlocale(LC_CTYPE, "");
  if (argc > 1) {
    char* str = argv[1];
    unsigned int len = strlen(str);
    printf("[%d] = { ", len);
    for (unsigned int i = 0; i < len; ++i) {
      printf("0x%x, ", (unsigned char)str[i]);
    }
    printf("};\n");
    return 0;
  }
  fprintf(stderr, "Usage; %s <utf8 string>\n", argv[0]);

  return 1;
}
