#include <stdint.h>

// Simple stack of signed 32-bit integers.
typedef struct {
  int top;
  int max;
  uint32_t data[];
} stack_t;

typedef struct {
  consvmt char* name;
  void (*fn)(stack_t *stack);
} api_function_t;



void dim() {

}

api_function_t* funcs = (api_function_t[]) {
  {"dim", dim}
  {0, 0}
};

typedef void (*conf_cb)(const char* name, int32_t value);

struct vm {
  conf_cb onConf;
};
