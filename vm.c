#include "vm.h"

#include <string.h>
#include <stdio.h>

const char **names = (const char*[]){
  "INT8", "INT16", "INT32",
  "CALL",
  "CALL0-0", "CALL0-1", "CALL0-2", "CALL0-3", "CALL0-4",
  "CALL1-0", "CALL1-1", "CALL1-2", "CALL1-3", "CALL1-4",
  "CALL2-0", "CALL2-1", "CALL2-2", "CALL2-3", "CALL2-4",
  "CALL3-0", "CALL3-1", "CALL3-2", "CALL3-3", "CALL3-4",
  "CALL4-0", "CALL4-1", "CALL4-2", "CALL4-3", "CALL4-4",
  "CALL5-0", "CALL5-1", "CALL5-2", "CALL5-3", "CALL5-4",
  "RUN",
  "RUN0-0", "RUN0-1", "RUN0-2", "RUN0-3", "RUN0-4",
  "RUN1-0", "RUN1-1", "RUN1-2", "RUN1-3", "RUN1-4",
  "RUN2-0", "RUN2-1", "RUN2-2", "RUN2-3", "RUN2-4",
  "RUN3-0", "RUN3-1", "RUN3-2", "RUN3-3", "RUN3-4",
  "RUN4-0", "RUN4-1", "RUN4-2", "RUN4-3", "RUN4-4",
  "RUN5-0", "RUN5-1", "RUN5-2", "RUN5-3", "RUN5-4",
  "SET",
  "SET0", "SET1", "SET2", "SET3", "SET4",
  "SET5", "SET6", "SET7", "SET8", "SET9",
  "GET",
  "GET0", "GET1", "GET2", "GET3", "GET4",
  "GET5", "GET6", "GET7", "GET8", "GET9",
  "INCR", "DECR", "INCRMOD", "DECRMOD",
  "INCR1", "DECR1", "INCR1MOD", "DECR1MOD",
  "ADD", "SUB", "MUL", "DIV", "MOD", "NEG",
  "LT", "LTE", "GT", "GTE", "EQ", "NEQ",
  "AND", "OR", "XOR", "NOT",
  "IF", "THEN", "ELSE",
  "DO", "DOI", "LOOP",
  "GOSUB", "RETURN",

};

void push(context_t *ctx, int32_t value) {
  // Resize stack if we need more room.
  if (ctx->top == ctx->size) {
    ctx->size += 4;
    ctx->data = realloc(
      ctx->data, sizeof(*ctx->data) * ctx->size);
  }
  ctx->data[ctx->top++] = value;
}

int32_t pop(context_t *ctx) {
  assert(ctx->top > 0);
  return ctx->data[--ctx->top];
}

int32_t decode_num(uint8_t **pc) {
  if (**pc < 128) {
    return *(*pc)++;
  }
  assert(0); // TODO: parse larger numbers
  return -1;
}

char* decode_string(uint8_t **pc) {
  uint8_t *start = *pc;
  while (*(*pc)++);
  return (char*)start;
}

buffer_t decode_buffer(uint8_t **pc) {
  int32_t len = decode_num(pc);
  buffer_t buf = {
    .len = len,
    .data = *pc
  };
  (*pc) += len;
  return buf;
}

int32_t read_config(program_t *prog, const char *key, int32_t fallback) {
  for (int i = 0; i < prog->num_confs; i++) {
    if (strcmp(prog->confs[i].name, key)) continue;
    return prog->confs[i].value;
  }
  return fallback;
}

int get_func(program_t *prog, const char *key) {
  for (int i = 0; i < prog->num_funcs; i++) {
    if (strcmp(prog->funcs[i].name, key)) continue;
    return i;
  }
  return -1;
}

context_t create_context(program_t *prog) {
  context_t ctx = (context_t){
    .prog = prog,
    .size = prog->num_locals,
    .top = prog->num_locals,
    .data = malloc(sizeof(int32_t) * prog->num_locals)
  };
  memset(ctx.data, 0, sizeof(int32_t) * prog->num_locals);
  return ctx;
}

void call_func(context_t *ctx, int index) {
  buffer_t code = ctx->prog->funcs[index].code;
  uint8_t *pc = code.data;
  while (*pc != RETURN) {
    if (*pc < 128) {
      printf("Running %p:%d\n", pc, *pc);
      push(ctx, *pc++);
      continue;
    }
    printf("Running %p:%s\n", pc, names[*pc - 128]);
    if (*pc > SET && *pc < GET) {
      int index = *pc++ - (SET + 1);
      ctx->data[index] = pop(ctx);
      continue;
    }
    else if (*pc > CALL && *pc < RUN) {
      printf("TODO: callx-y\n");
    }
    switch ((opcode_t)*pc++) {
      case INT8:
        push(ctx, *((int8_t*)pc));
        pc += 1; continue;
      case INT16:
        push(ctx, *((int16_t*)pc));
        pc += 2; continue;
      case INT32:
        push(ctx, *((int32_t*)pc));
        pc += 4; continue;
      case INCR:
      case DECR:
      case INCRMOD: {
        int index = pop(ctx);
        int mod = pop(ctx);
        int delta = pop(ctx);
        ctx->data[index] = (ctx->data[index] + delta) % mod;
        continue;
      }
      case DECRMOD:
      case INCR1:
      case DECR1:
        printf("TODO: Implement %s\n", names[*(pc - 1) - 128]);
        exit(-2);
      case INCR1MOD: {
        int index = pop(ctx);
        int mod = pop(ctx);
        ctx->data[index] = (ctx->data[index] + 1) % mod;
        continue;
      }
      case DECR1MOD:

      default:
        printf("TODO: Implement %s\n", names[*(pc - 1) - 128]);
        exit(-2);

    }
  }
}

// Takes bytecode and a list of named native function pointers and returns a
// fully linked VM context.
program_t *program_create(buffer_t bytecode, def_t natives[]) {
  program_t *prog = malloc(sizeof(*prog));

  uint8_t *pc = bytecode.data;

  // Verify the magic header bytes from bytecode
  assert(*pc++ == 'B' && *pc++ == 't' && *pc++ == 0);

  prog->num_locals = decode_num(&pc);

  // Decode Configurations
  prog->num_confs = decode_num(&pc);
  prog->confs = malloc(sizeof(config_t) * prog->num_confs);
  for (int i = 0; i < prog->num_confs; i++) {
    const char* name = decode_string(&pc);
    int32_t value = decode_num(&pc);
    prog->confs[i] = (config_t){
      .name = name,
      .value = value
    };
    printf("CONF %s=%d\n", name, value);
  }

  // Link native functions to slots in bytecode
  int num_natives = decode_num(&pc);
  prog->natives = malloc(sizeof(native_t) * num_natives);
  // Link in function pointers from the user to slots for actual calls from code
  for (int i = 0; i < num_natives; i++) {
    const char* name = decode_string(&pc);
    for (int j = 0; natives[j].name; j++) {
      if (strcmp(name, natives[j].name) == 0) {
        prog->natives[i] = natives[j].native;
        goto found;
      }
    }
    printf("Missing native definition %s\n", name);
    exit(-2);
    found:
    printf("NATIVE %s\n", name);
  }

  // Read the lengths from the bytecode header
  prog->num_funcs = decode_num(&pc);
  prog->funcs = malloc(sizeof(func_t) * prog->num_funcs);
  for (int i = 0; i < prog->num_funcs; i++) {
    const char* name = decode_string(&pc);
    buffer_t buf = decode_buffer(&pc);
    prog->funcs[i] = (func_t){
      .name = name,
      .code = buf
    };
    printf("FUNC %s\n", name);
  }

  assert(pc == bytecode.data + bytecode.len);

  return prog;
}
