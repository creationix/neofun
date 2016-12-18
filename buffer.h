#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stdlib.h>

// This is a generic buffer for holding arbitrary binary data.
// The data value is a pointer so that it can be resized.
typedef struct {
  size_t len;
  uint8_t *data;
} buffer_t;

// Create a new buffer with given initial capacity
buffer_t buffer_create(size_t initial_size);

// Resize buffer to a new capacity.  Resize to zero to free memory when done.
// Notice that you'll need to pass in a reference here.
void buffer_resize(buffer_t *buffer, size_t new_size);

#endif
