#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <time.h>
#include "vm.c"
#include "buffer.c"

static volatile int alive = 1;
static void intHandler(int dummy) {
  alive = 0;
}

typedef struct {
  int len;
  int bpp;
  uint8_t bytes[];
} strip_t;

static uint32_t deadbeef_rand() {
	deadbeef_seed = (deadbeef_seed << 7) ^ ((deadbeef_seed >> 25) + deadbeef_beef);
	deadbeef_beef = (deadbeef_beef << 7) ^ ((deadbeef_beef >> 25) + 0xdeadbeef);
	return deadbeef_seed;
}

static void deadbeef_srand(uint32_t x) {
	deadbeef_seed = x;
	deadbeef_beef = 0xdeadbeef;
}

static void rgb(strip_t *strip, int p, uint8_t r, uint8_t g, uint8_t b) {
  p = ((p % strip->len) + strip->len) % strip->len;
  int i = p * strip->bpp;
  strip->bytes[i++] = r;
  strip->bytes[i++] = g;
  strip->bytes[i] = b;
}

static void hue(strip_t *strip, int p, int h, uint8_t d) {
  h = ((h % 768) + 768) % 768;
  if (h < 256) {
    return rgb(strip, p,
      (h * d) >> 8,
      ((255 - h) * d) >> 8,
      0
    );
  }
  if (h < 512) {
    return rgb(strip, p,
      ((511 - h) * d) >> 8,
      0,
      ((h - 256) * d) >> 8
    );
  }
  return rgb(strip, p,
    0,
    ((h - 512) * d) >> 8,
    ((767 - h) * d) >> 8
  );
}

static void update(strip_t *strip) {
  bool empty = true;
  printf("\e[H");
  for (int x = 0; x < strip->len; x++) {
    int i = x * strip->bpp;
    if (strip->bytes[i] || strip->bytes[i + 1] || strip->bytes[i + 2]) {
      printf("\e[48;2;%d;%d;%dm ",
        strip->bytes[i], strip->bytes[i+1], strip->bytes[i+2]);
      empty = false;
    }
    else {
      if (!empty) {
        empty = true;
        printf("\e[0m");
      }
      printf(" ");
    }
  }
  printf("\n");
}

static void fade(strip_t *strip, uint8_t d) {
  for (int i = 0, l = strip->len * strip->bpp; i < l; i++) {
    strip->bytes[i] = (strip->bytes[i] * d) >> 8;
  }
}

static int32_t bt_fade(void *user, int32_t *argv, size_t argc) {
  fade((strip_t*)user, argv[0]);
  return 0;
}

static int32_t bt_rand(void *user, int32_t *argv, size_t argc) {
  return deadbeef_rand() % argv[0];
}


static int32_t bt_hue(void *user, int32_t *argv, size_t argc) {
  hue((strip_t*)user, argv[0], argv[1], argv[2]);
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

  strip_t *strip = malloc(sizeof(strip_t) + length * bpp);
  strip->len = length;
  strip->bpp = bpp;
  memset(strip->bytes, 0, length * bpp);

  printf("prog=%p length=%d bpp=%d delay=%d sprites=%d fizz=%d\n",
    prog, length, bpp, delay, num_sprites, fizz);

  int setup_idx = get_func(prog, "setup");
  int loop_idx = get_func(prog, "loop");
  int spawn_idx = get_func(prog, "spawn");
  int update_idx = get_func(prog, "update");

  context_t ctx = create_context(prog, strip);
  context_t *sprites = NULL;
  call_func(&ctx, setup_idx);
  if (num_sprites) {
    sprites = malloc(sizeof(*sprites) * num_sprites);
    for (int i = 0; i < num_sprites; i++) {
      sprites[i] = create_context(prog, strip);
      call_func(sprites + i, spawn_idx);
    }
  }
  // Clear screen and hide cursor
  printf("\e[2J\e[?25l");
  // Register sigint handler
  signal(SIGINT, intHandler);

  int sprite_index = 0;
  while (alive) {
    call_func(&ctx, loop_idx);
    for (int i = 0; i < fizz; i++) {
      call_func(sprites + sprite_index, update_idx);
      sprite_index = (sprite_index + 1) % num_sprites;
    }
    update(strip);
    select(0, NULL, NULL, NULL, &(struct timeval){
      .tv_sec = delay / 1000,
      .tv_usec = (delay % 1000) * 1000
    });
  }
  // Show cursor again
  printf("\e[?25h\nThanks for playing.\n");
  return 0;
}
