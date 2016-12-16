#include <unistd.h>
#include <stdio.h>
#include "vm.h"

static buffer_t read_stdin() {
  size_t length = 0;
  size_t max = 1024;
  uint8_t *data = malloc(1024);
  while (true) {
    ssize_t b = read(0, data + length, max - length);
    if (b < 0) {
      printf("Problem reading stdin");
      exit(-1);
    }
    length += b;
    if (b < 1024) break;
    max += 1024;
    data = realloc(data, max);
  }
  data = realloc(data, length);
  return (buffer_t){
    .len = length,
    .data = data
  };
}

int main() {
  buffer_t code = read_stdin();
  printf("len=%lu data=%p\n", code.len, code.data);
}
