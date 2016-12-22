#ifndef VM_H
#define VM_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "buffer.h"

typedef enum {
  INT8=128, INT16, INT32,
  CALL=131,
  // CALL0-0, CALL0-1, CALL0-2, CALL0-3, CALL0-4,
  // CALL1-0, CALL1-1, CALL1-2, CALL1-3, CALL1-4,
  // CALL2-0, CALL2-1, CALL2-2, CALL2-3, CALL2-4,
  // CALL3-0, CALL3-1, CALL3-2, CALL3-3, CALL3-4,
  // CALL4-0, CALL4-1, CALL4-2, CALL4-3, CALL4-4,
  // CALL5-0, CALL5-1, CALL5-2, CALL5-3, CALL5-4,
  RUN=162,
  // RUN0-0, RUN0-1, RUN0-2, RUN0-3, RUN0-4,
  // RUN1-0, RUN1-1, RUN1-2, RUN1-3, RUN1-4,
  // RUN2-0, RUN2-1, RUN2-2, RUN2-3, RUN2-4,
  // RUN3-0, RUN3-1, RUN3-2, RUN3-3, RUN3-4,
  // RUN4-0, RUN4-1, RUN4-2, RUN4-3, RUN4-4,
  // RUN5-0, RUN5-1, RUN5-2, RUN5-3, RUN5-4,
  SET=193,
  // SET0, SET1, SET2, SET3, SET4,
  // SET5, SET6, SET7, SET8, SET9,
  GET=204,
  // GET0, GET1, GET2, GET3, GET4,
  // GET5, GET6, GET7, GET8, GET9,
  INCR=215, DECR, INCRMOD, DECRMOD,
  INCR1, DECR1, INCR1MOD, DECR1MOD,
  ADD, SUB, MUL, DIV, MOD, NEG,
  LT, LTE, GT, GTE, EQ, NEQ,
  AND, OR, XOR, NOT,
  IF, THEN, ELSE,
  DO, DOI, LOOP,
  GOSUB, RETURN,
} opcode_t;

typedef struct context_s context_t;

// Type signature for native functions.  Basically these receive a list of
// 32-bit integers and return a single 32-bit integer.
typedef int32_t (*native_t)(context_t *ctx, int32_t *argv, size_t argc);

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
  int num_funcs;
  func_t *funcs;
  int num_confs;
  config_t *confs;
} program_t;

struct context_s {
  program_t *prog;
  size_t size;
  size_t top;
  int32_t *data;
};

#endif
