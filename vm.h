#ifndef VM_H
#define VM_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "buffer.h"

// Type signature for native functions.  Basically these receive a list of
// 32-bit integers and return a single 32-bit integer.
typedef int32_t (*native_t)(int32_t *argv, size_t argc);

// This is a pair of name with function pointer. An array of this is used
// for linking in native functions when creating a program.
typedef struct {
  const char *name;
  native_t native;
} def_t;

typedef struct {
  const char *name;
  int32_t value;
} config_t;

typedef struct {
  const char *name;
  buffer_t code;
} func_t;

// This stores all shared program state between contexts in the same program.
typedef struct {
  buffer_t bytecode;
  int num_locals;
  native_t *natives;
  func_t *funcs;
  config_t *conf;
} program_t;

typedef struct {
  program_t *prog;
  size_t size;
  size_t top;
  int32_t *data;
} stack_t;

#endif
