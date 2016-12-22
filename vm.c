#include "vm.h"

#include <string.h>
#include <stdio.h>

static uint32_t deadbeef_seed;
static uint32_t deadbeef_beef = 0xdeadbeef;

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

static void push(context_t *ctx, int32_t value) {
  // Resize stack if we need more room.
  if (ctx->top == ctx->size) {
    ctx->size += 4;
    ctx->data = realloc(
      ctx->data, sizeof(*ctx->data) * ctx->size);
  }
  ctx->data[ctx->top++] = value;
}

static int32_t pop(context_t *ctx) {
  assert(ctx->top > 0);
  return ctx->data[--ctx->top];
}

static int32_t decode_num(uint8_t **pc) {
  if (**pc < 128) {
    return *(*pc)++;
  }
  switch (*(*pc)++) {
    int32_t val;
    case INT8:
      val = (int8_t)**pc;
      (*pc)++;
      return val;
    case INT16:
      val = (int16_t)(**pc << 8 | *((*pc) + 1));
      (*pc) += 2;
      return val;
    case INT32:
      val = (int16_t)(
        *(*pc) << 24 |
        *((*pc) + 1) << 16 |
        *((*pc) + 2) << 8 |
        *((*pc) + 3));
      (*pc) += 4;
      return val;
    default:
      assert(0);
      return -1;
  }
}

static char* decode_string(uint8_t **pc) {
  uint8_t *start = *pc;
  while (*(*pc)++);
  return (char*)start;
}

static buffer_t decode_buffer(uint8_t **pc) {
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

context_t create_context(program_t *prog, void *user) {
  context_t ctx = (context_t){
    .prog = prog,
    .size = prog->num_locals,
    .top = prog->num_locals,
    .data = malloc(sizeof(int32_t) * prog->num_locals),
    .user = user
  };
  memset(ctx.data, 0, sizeof(int32_t) * prog->num_locals);
  return ctx;
}

#define binop(op) { \
  int r = pop(ctx); \
  int l = pop(ctx); \
  push(ctx, l op r); \
  continue; \
}

#define MAX_LOOPS 10

typedef struct {
  int slot;
  int index;
  int limit;
  uint8_t *pc;
} loop_t;

void call_func(context_t *ctx, int index) {
  // printf("\nStarting: %p %d:%s\n", ctx, index, ctx->prog->funcs[index].name);
  buffer_t code = ctx->prog->funcs[index].code;
  uint8_t *pc = code.data;
  int en = 0, co = 0; // Conditional nesting
  loop_t loops[MAX_LOOPS];
  int num_loops = 0;
  while (*pc != RETURN) {
    // printf("Stack %d:", en < co);
    // for (int i = 0; i < ctx->top; i++) {
    //   printf(" %d", ctx->data[i]);
    // }
    // printf("\n");
    if (en < co) {
      if (*pc < 128) {
        // printf("Skipping %p:%d\n", pc, *pc);
        pc++;
        continue;
      }
      // printf("Skipping %p:%s\n", pc, names[*pc - 128]);
      switch (*pc++) {
        case ELSE:
          en = co;
          continue;
        case IF: case DO:
          co++;
          continue;
        case THEN: case LOOP:
          co--;
          continue;
        case INT8: pc += 1; continue;
        case INT16: pc += 2; continue;
        case INT32: pc += 4; continue;
        default: continue;
      }
    }
    if (*pc < 128) {
      // printf("Running %p:%d\n", pc, *pc);
      push(ctx, *pc++);
      continue;
    }
    // printf("Running %p:%s\n", pc, names[*pc - 128]);
    if (*pc > CALL && *pc < RUN) {
      int index = *pc++ - (CALL + 1);
      int argc = index % 5;
      index /= 5;
      native_t fn = ctx->prog->natives[index];
      int32_t argv[5];
      for (int i = argc - 1; i >= 0; i--) {
        argv[i] = pop(ctx);
      }
      push(ctx, fn(ctx->user, argv, argc));
      continue;
    }
    if (*pc > RUN && *pc < SET) {
      int index = *pc++ - (RUN + 1);
      int argc = index % 5;
      index /= 5;
      native_t fn = ctx->prog->natives[index];
      int32_t argv[5];
      for (int i = argc - 1; i >= 0; i--) {
        argv[i] = pop(ctx);
      }
      fn(ctx->user, argv, argc);
      continue;
    }
    if (*pc > SET && *pc < GET) {
      int index = *pc++ - (SET + 1);
      ctx->data[index] = pop(ctx);
      continue;
    }
    if (*pc > GET && *pc < INCR) {
      int index = *pc++ - (GET + 1);
      push(ctx, ctx->data[index]);
      continue;
    }
    switch ((opcode_t)*pc++) {
      case INT8:
        push(ctx, (int8_t)*pc);
        pc += 1; continue;
      case INT16:
        push(ctx, (int16_t)(*pc << 8 | *(pc + 1)));
        pc += 2; continue;
      case INT32:
        push(ctx, (int16_t)(
          *pc << 24 |
          *(pc + 1) << 16 |
          *(pc + 2) << 8 |
          *(pc + 3)));
        pc += 4; continue;
      case INCR: {
        int index = pop(ctx);
        int delta = pop(ctx);
        ctx->data[index] += delta;
        continue;
      }
      case DECR: {
        int index = pop(ctx);
        int delta = pop(ctx);
        ctx->data[index] -= delta;
        continue;
      }
      case INCRMOD: {
        int index = pop(ctx);
        int mod = pop(ctx);
        int delta = pop(ctx);
        ctx->data[index] = (ctx->data[index] + delta) % mod;
        continue;
      }
      case DECRMOD: {
        int index = pop(ctx);
        int mod = pop(ctx);
        int delta = pop(ctx);
        ctx->data[index] = (ctx->data[index] - delta + mod) % mod;
        continue;
      }
      case INCR1:
        ctx->data[pop(ctx)]++;
        continue;
      case DECR1:
        ctx->data[pop(ctx)]--;
        continue;
      case INCR1MOD: {
        int index = pop(ctx);
        int mod = pop(ctx);
        ctx->data[index] = (ctx->data[index] + 1) % mod;
        continue;
      }
      case DECR1MOD: {
        int index = pop(ctx);
        int mod = pop(ctx);
        ctx->data[index] = (ctx->data[index] + mod - 1) % mod;
        continue;
      }
      case ADD: binop(+)
      case SUB: binop(-)
      case MUL: binop(*)
      case DIV: binop(/)
      case MOD: binop(%)
      case NEG:
        ctx->data[ctx->top - 1] = -ctx->data[ctx->top - 1];;
        continue;
      case LT: binop(<)
      case LTE: binop(<=)
      case GT: binop(>)
      case GTE: binop(>=)
      case EQ: binop(==)
      case NEQ: binop(!=)
      case AND: binop(&&)
      case OR: binop(||)
      case XOR: {
        int r = pop(ctx);
        int l = pop(ctx);
        push(ctx, (l && !r) || (!l && r));
        continue;
      }
      case NOT:
        ctx->data[ctx->top - 1] = !ctx->data[ctx->top - 1];;
        continue;
      case IF:
        co++;
        if (pop(ctx)) en = co;
        continue;
      case ELSE:
        en = co - 1;
        continue;
      case THEN:
        co--;
        continue;
      case GOSUB:
        call_func(ctx, pop(ctx));
        continue;
      case DO: case DOI: {
        int slot = *(pc - 1) == DOI ? pop(ctx) : -1;
        int count = pop(ctx);
        loops[num_loops++] = (loop_t){
          .slot = slot,
          .index = 0,
          .limit = count,
          .pc = pc
        };
        if (slot >= 0) ctx->data[slot] = 0;
        continue;
      }
      case LOOP: {
        int i = num_loops - 1;
        if (++loops[i].index < loops[i].limit) {
          pc = loops[i].pc;
          if (loops[i].slot >= 0) {
            ctx->data[loops[i].slot] = loops[i].index;
          }
          continue;
        }
        num_loops--;
        continue;
      }
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
