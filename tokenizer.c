#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef enum opcode {
  // global config overrides
  LENGTH = 128, // Reallocate space and set pixel count
  TYPE, // Configure pixel type
  DELAY, // Set update loop delay in ms
  // Basic math opcodes
  ADD, // a + b
  SUB, // a - b
  MUL, // a * b
  DIV, // a / b
  MOD, // a % b
  NEG, // -a
  // Comparison
  EQ, NEQ, GT, GTE, LT, LTE, // a op b
  // Logic
  AND, OR, XOR, // a op b
  NOT, // !a
  // Library functions
  FADE, // fade(d)
  RAND, // rand(a, b)
  HUE, // hue(p, h, d)
  RGB, // rgb(p, r, g, b)
  RGBW, // rgbw(p, r, g, b, w)
  // Control flow
  IF, // if (cond) body...
  FOR, // for (name start end step)
  // Variable management
  SET, // set name value
  GET, // get name
  // Variable mutation ops
  INCRMOD, // incrmod name delta limit
  DECR, // decrease by one -> returns true if zero is hit
  // Concurrency management
  SPAWN, // spawn count body...
  READY, // set loop pointer to next instruction
  RESPAWN, // start over current sprite
  DIE, // end current sprite
} opcode_t;

typedef struct sprite {
  struct sprite *next; // optional pointer to previous sprite in linked list
  struct sprite *prev; // optional pointer to next sprite
  uint8_t *start; // pointer to start of sprite setup code
  uint8_t *loop; // pointer to start of sprite loop code
  int variables[]; // local variables
} sprite_t;

typedef struct state {
  sprite_t *first_sprite; // pointer to linked list of sprites
  sprite_t *sprite; // optional pointer if running in sprite;
  uint8_t *next; // pointer to next instruction
  uint8_t *start; // pointer to start of setup code
  uint8_t *loop; // pointer to start of loop code
  int num_globals; // number of global variable slots.
  int variables[]; // global variables
} state_t;

int eval(state_t *S) {
  if (*S->next < 128) {
    // TODO: handle numbers larger than 127
    return *S->next++;
  }
  switch ((opcode_t)*S->next++) {

  }
}


#define MAX_TOKEN_LENGTH 64

int main() {
  char ch;
  char buffer[MAX_TOKEN_LENGTH];
  int len = 0;
  int stack = 0;
  bool comment = true;
  while (read(0, &ch, 1) > 0) {
    if (comment) {
      if (ch != '\n') continue;
      comment = false;
    }
    if (!(ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t' ||
          ch == ')' || ch == '(' || ch == ';')) {
      if (len < MAX_TOKEN_LENGTH) buffer[len++] = ch;
      continue;
    }
    if (len) {
      for (int i = 0; i < stack; i++) printf(" ");
      printf("'%.*s'\n", len, buffer);
      len = 0;
    }
    if (ch == '(') {
      stack++;
      continue;
    }
    if (ch == ')') {
      stack--;
      printf("\n");
      continue;
    }
    if (ch == ';') {
      comment = 1;
      continue;
    }
  }
  if (len) {
    printf("'%.*s'\n", len, buffer);
  }
}
