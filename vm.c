#include "vm.h"

#include <string.h>
#include <stdio.h>

void push(stack_t *stack, int32_t value) {
  // Resize stack if we need more room.
  if (stack->top == stack->size) {
    stack->size += 4;
    stack->data = realloc(
      stack->data, sizeof(*stack->data) * stack->size);
  }
  stack->data[stack->top++] = value;
}

int32_t pop(stack_t *stack) {
  assert(stack->top > 0);
  return stack->data[--stack->top];
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
