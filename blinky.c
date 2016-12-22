#include <unistd.h>
#include <stdio.h>
#include "vm.c"
#include "buffer.c"

static int32_t bt_fade(int32_t *argv, size_t argc) {
  printf("fade(");
  for (size_t i = 0; i < argc; i++) {
    if (i) printf(", ");
    printf("%d", argv[i]);
  }
  printf(")\n");
  return 0;
}

static int32_t bt_rand(int32_t *argv, size_t argc) {
  printf("rand(");
  for (size_t i = 0; i < argc; i++) {
    if (i) printf(", ");
    printf("%d", argv[i]);
  }
  printf(")\n");
  return 0;
}

static int32_t bt_hue(int32_t *argv, size_t argc) {
  printf("hue(");
  for (size_t i = 0; i < argc; i++) {
    if (i) printf(", ");
    printf("%d", argv[i]);
  }
  printf(")\n");
  return 0;
}

static int32_t bt_respawn(int32_t *argv, size_t argc) {
  printf("respawn(");
  for (size_t i = 0; i < argc; i++) {
    if (i) printf(", ");
    printf("%d", argv[i]);
  }
  printf(")\n");
  return 0;
}

static program_t *link_stdin(def_t *natives) {
  // Read from stdin to a buffer.
  buffer_t buf = buffer_create(1024);
  size_t length = 0;
  while (true) {
    ssize_t b = read(0, buf.data + length, buf.len - length);
    if (b < 0) {
      printf("Problem reading stdin");
      exit(-1);
    }
    length += b;
    if (b < 1024) break;
    buffer_resize(&buf, buf.len + 1024);
  }
  buffer_resize(&buf, length);

  // Parse and link bytecode
  program_t *prog = program_create(buf, natives);

  return prog;
}

int main() {
  program_t *prog = link_stdin((def_t[]) {
    { "fade", bt_fade },
    { "rand", bt_rand },
    { "hue", bt_hue },
    { "respawn", bt_respawn },
    { NULL, NULL }
  });
  int length = read_config(prog, "length", 30);
  int bpp = read_config(prog, "bpp", 3);
  int delay = read_config(prog, "delay", 33);
  int sprites = read_config(prog, "bpp", 0);
  int fizz = read_config(prog, "fizz", 0);

  printf("prog=%p length=%d bpp=%d delay=%d sprites=%d fizz=%d\n",
    prog, length, bpp, delay, sprites, fizz);
  int setup = get_func(prog, "setup");
  int update = get_func(prog, "update");

  context_t ctx = create_context(prog);
  call_func(&ctx, setup);
  call_func(&ctx, update);
  printf("setup=%d update=%d\n", setup, update);
  // prog->natives[0](pp)
}
