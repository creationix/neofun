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


// Takes bytecode and a list of named native function pointers and returns a
// fully linked VM context.
program_t *program_create(buffer_t bytecode, def_t natives[]) {
  // DUMMY data
  // TODO: parse from bytecode
  int num_natives = 4;
  int num_funcs = 5;
  int num_confs = 5;
  int num_locals = 7;
  const char** native_names = (const char*[]){
    "fade",
    "rand",
    "hue",
    "respawn",
  };
  program_t *prog = malloc(sizeof(*prog));
  prog->num_locals = num_locals;
  prog->natives = malloc(sizeof(native_t) * num_natives);
  printf("%lu vs %lu\n", sizeof(prog->funcs), sizeof(*prog->funcs));
  prog->funcs = malloc(sizeof(prog->funcs) * num_funcs);
  prog->conf = malloc(sizeof(*prog->conf) * num_confs);

  // Link in function pointers from the user to slots for actual calls from code
  for (int i = 0; i < num_natives; i++) {
    const char* name = native_names[i];
    for (int j = 0; natives[j].name; j++) {
      if (strcmp(name, natives[j].name) == 0) {
        prog->natives[i] = natives[j].native;
        goto found;
      }
    }
    printf("Missing native definition %s\n", name);
    exit(-2);
    found:
    printf("FOUND\n");
  }

  
  return prog;
}
