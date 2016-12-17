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

typedef struct {
  int32_t version;
  int32_t num_native;
  int32_t num_func;
  int32_t num_conf;
  uint8_t *strings[];
} program_t;

typedef struct {
  size_t count;
  int32_t data[];
} context_t;

typedef struct {
  uint8_t **confs;
  uint8_t **funcs;
} program_t;

int main() {
  buffer_t code = read_stdin();
  printf("len=%lu data=%p\n", code.len, code.data);
}
