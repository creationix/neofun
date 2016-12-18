#include "buffer.h"

buffer_t buffer_create(size_t initial_size) {
  return (buffer_t){
    .len = initial_size,
    .data = malloc(initial_size)
  };
}

void buffer_resize(buffer_t *buffer, size_t new_size) {
  buffer->len = new_size;
  buffer->data = realloc(buffer->data, new_size);
}
