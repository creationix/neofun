#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "mjs/mjs.c"
#include "buffer.c"

static volatile int alive = 1;
static int count;
static int width;
static int height;
static uint8_t* pixels;

static void fade(uint8_t keep) {
  for (int i = 0; i < count * 3; i++) {
    pixels[i] = pixels[i] * keep >> 8;
  }
}

static void update() {
  bool empty = false;
  printf("\e[H");
  for (int y = 0; y < height; y += 2) {
    if (y) printf("\r\n");
    for (int x = 0; x < width; x++) {
      int i = (y * width + x) * 3;
      if (pixels[i] || pixels[i + 1] || pixels[i + 2]) {
        if (pixels[i + width*3] || pixels[i+width*3+1] || pixels[i+width*3+2]) {
          empty = false;
          printf("\e[38;2;%d;%d;%dm\e[48;2;%d;%d;%dm▀",
            pixels[i], pixels[i+1], pixels[i+2],
            pixels[i + width*3], pixels[i+width*3+1], pixels[i+width*3+2]);
        }
        else {
          printf("\e[0m\e[38;2;%d;%d;%dm▀",
            pixels[i], pixels[i+1], pixels[i+2]);
        }
      }
      else {
        if (pixels[i + width*3] || pixels[i+width*3+1] || pixels[i+width*3+2]) {
          printf("\e[0m\e[38;2;%d;%d;%dm▄",
            pixels[i + width*3], pixels[i+width*3+1], pixels[i+width*3+2]);
        }
        else {
          if (empty) {
            printf(" ");
          }
          else {
            empty = true;
            printf("\e[0m ");
          }
        }
      }
    }
  }
}

static void rgb(int p, int r, int g, int b) {
  int i = ((p + count) % count) * 3;
  pixels[i] = r;
  pixels[++i] = g;
  pixels[++i] = b;
}

static void rgbl(int p, uint8_t r, uint8_t g, uint8_t b, uint8_t keep) {
  int i = ((p + count) % count) * 3;
  pixels[i] = (r * keep + pixels[i] * (255 - keep)) >> 8;
  i++;
  pixels[i] = (g * keep + pixels[i] * (255 - keep)) >> 8;
  i++;
  pixels[i] = (b * keep + pixels[i] * (255 - keep)) >> 8;
}

// R- /^^\__
// G- ^\__/^
// B- __/^^\_
static void hue(int p, int h, uint8_t keep) {
  h = (h + 1536) % 1536;
  if (h < 256) return rgbl(p, h, 255, 0, keep);
  if ((h -= 256) < 256) return rgbl(p, 255, 255-h, 0, keep);
  if ((h -= 256) < 256) return rgbl(p, 255, 0, h, keep);
  if ((h -= 256) < 256) return rgbl(p, 255-h, 0, 255, keep);
  if ((h -= 256) < 256) return rgbl(p, 0, h, 255, keep);
  if ((h -= 256) < 256) return rgbl(p, 0, 255, 255-h, keep);
}

static void msleep(int delay) {
  select(0, NULL, NULL, NULL, &(struct timeval){
    .tv_sec = delay / 1000,
    .tv_usec = (delay % 1000) * 1000
  });
}

static int get_width() { return width; }
static int get_height() { return height; }
static int get_alive() { return alive; }

static void *my_dlsym(void *handle, const char *name) {
  if (strcmp(name, "rand") == 0) return rand;
  if (strcmp(name, "srand") == 0) return srand;
  if (strcmp(name, "fade") == 0) return fade;
  if (strcmp(name, "update") == 0) return update;
  if (strcmp(name, "rgb") == 0) return rgb;
  if (strcmp(name, "rgbl") == 0) return rgbl;
  if (strcmp(name, "hue") == 0) return hue;
  if (strcmp(name, "msleep") == 0) return msleep;
  if (strcmp(name, "get_alive") == 0) return get_alive;
  if (strcmp(name, "get_width") == 0) return get_width;
  if (strcmp(name, "get_height") == 0) return get_height;
  return NULL;
  (void) handle;
}


static void intHandler(int dummy) {
  alive = 0;
  printf("\e[?25h\n");
}

static buffer_t read_stdin() {
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
  return buf;
}

int main() {
  struct winsize sz;
  ioctl(1, TIOCGWINSZ, &sz);
  width = sz.ws_col;
  height = sz.ws_row * 2;
  count = width * height;
  pixels = malloc(count * 3);

  buffer_t code = read_stdin();

  struct mjs *mjs = mjs_create();

  // Register library functions
  mjs_set_ffi_resolver(mjs, my_dlsym);
  mjs_err_t err = mjs_exec(mjs,
    "let getWidth = ffi('int get_width()');"
    "let getHeight = ffi('int get_height()');"
    "let getAlive = ffi('int get_alive()');"
    "let rand = ffi('int rand()');"
    "let srand = ffi('void srand(int)');"
    "let fade = ffi('void fade(int)');"
    "let update = ffi('void update()');"
    "let rgb = ffi('void rgb(int, int, int, int)');"
    "let rgbl = ffi('void rgbl(int, int, int, int, int)');"
    "let hue = ffi('void hue(int, int, int)');"
    "let msleep = ffi('void msleep(int)');"
  , NULL);
  if (err != MJS_OK) {
    printf("%s\n",mjs_strerror(mjs, err));
    return -1;
  }

  // Clear screen and hide cursor
  printf("\e[2J\e[?25l");
  // Register sigint handler
  signal(SIGINT, intHandler);

  // Run user program
  err = mjs_exec_buf(mjs, (char*)code.data, code.len, NULL);

  // Show cursor again
  printf("\e[?25h\n");

  // Handle any user error
  if (err != MJS_OK) {
    printf("%s\n",mjs_strerror(mjs, err));
    return -1;
  }

  return 0;
}
