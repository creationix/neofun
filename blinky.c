#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <time.h>
#include "vm.c"
#include "buffer.c"

static uint32_t deadbeef_rand() {
	deadbeef_seed = (deadbeef_seed << 7) ^ ((deadbeef_seed >> 25) + deadbeef_beef);
	deadbeef_beef = (deadbeef_beef << 7) ^ ((deadbeef_beef >> 25) + 0xdeadbeef);
	return deadbeef_seed;
}

static void deadbeef_srand(uint32_t x) {
	deadbeef_seed = x;
	deadbeef_beef = 0xdeadbeef;
}

static int32_t bt_fade(context_t *ctx, int32_t *argv, size_t argc) {
  printf("fade(");
  for (size_t i = 0; i < argc; i++) {
    if (i) printf(", ");
    printf("%d", argv[i]);
  }
  printf(")\n");
  return 0;
}

static int32_t bt_rand(context_t *ctx, int32_t *argv, size_t argc) {
  printf("rand(");
  for (size_t i = 0; i < argc; i++) {
    if (i) printf(", ");
    printf("%d", argv[i]);
  }
  int out = deadbeef_rand() % argv[0];
  printf(") = %d\n", out);
  return out;
}

static int32_t bt_hue(context_t *ctx, int32_t *argv, size_t argc) {
  printf("hue(");
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
  deadbeef_srand((int)time(NULL));
  program_t *prog = link_stdin((def_t[]) {
    { "fade", bt_fade },
    { "rand", bt_rand },
    { "hue", bt_hue },
    { NULL, NULL }
  });
  int length = read_config(prog, "length", 30);
  int bpp = read_config(prog, "bpp", 3);
  int delay = read_config(prog, "delay", 33);
  int num_sprites = read_config(prog, "sprites", 0);
  int fizz = read_config(prog, "fizz", num_sprites);

  printf("prog=%p length=%d bpp=%d delay=%d sprites=%d fizz=%d\n",
    prog, length, bpp, delay, num_sprites, fizz);
  int setup = get_func(prog, "setup");
  int loop = get_func(prog, "loop");
  int spawn = get_func(prog, "spawn");
  int update = get_func(prog, "update");

  context_t ctx = create_context(prog);
  context_t *sprites = NULL;
  call_func(&ctx, setup);
  if (num_sprites) {
    sprites = malloc(sizeof(*sprites) * num_sprites);
    for (int i = 0; i < num_sprites; i++) {
      sprites[i] = create_context(prog);
      call_func(sprites + i, spawn);
    }
  }
  int sprite_index = 0;
  while (true) {
    call_func(&ctx, loop);
    for (int i = 0; i < fizz; i++) {
      call_func(sprites + sprite_index, update);
      sprite_index = (sprite_index + 1) % num_sprites;
    }
    select(0, NULL, NULL, NULL, &(struct timeval){
      .tv_sec = delay / 1000,
      .tv_usec = (delay % 1000) * 1000
    });
  }
}
