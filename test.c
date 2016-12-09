#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

static volatile int alive = 1;

#define width 122
#define height 70
#define count (width*height)
uint8_t pixels[count * 3];

void fade(uint8_t keep) {
  for (int i = 0; i < count * 3; i++) {
    pixels[i] = pixels[i] * keep >> 8;
  }
}
void update() {
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
void rgb(int p, int r, int g, int b) {
  int i = ((p + count) % count) * 3;
  pixels[i] = r;
  pixels[++i] = g;
  pixels[++i] = b;
}

void rgbl(int p, uint8_t r, uint8_t g, uint8_t b, uint8_t keep) {
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
void hue(int p, int h, uint8_t keep) {
  h = (h + 1536) % 1536;
  if (h < 256) return rgbl(p, h, 255, 0, keep);
  if ((h -= 256) < 256) return rgbl(p, 255, 255-h, 0, keep);
  if ((h -= 256) < 256) return rgbl(p, 255, 0, h, keep);
  if ((h -= 256) < 256) return rgbl(p, 255-h, 0, 255, keep);
  if ((h -= 256) < 256) return rgbl(p, 0, h, 255, keep);
  if ((h -= 256) < 256) return rgbl(p, 0, 255, 255-h, keep);
}

void intHandler(int dummy) {
  alive = 0;
}
typedef struct {
  int x;
  int y;
  int d;
  int h;
  int l;
} bug_t;

void init_bug(bug_t* bug) {
  bug->x = rand() % width;
  bug->y = rand() % height;
  bug->d = rand() % 4;
  bug->h = rand() % 1536;
  bug->l = rand() % 1500;
}
void update_bug(bug_t* bug) {
  switch(bug->d) {
    case 0: bug->x = (bug->x + width - 1) % width; break;
    case 1: bug->y = (bug->y + height - 1) % height; break;
    case 2: bug->x = (bug->x + 1) % width; break;
    case 3: bug->y = (bug->y + 1) % height; break;
  }
  bug->h = (bug->h + 13) % 1536;
  if (bug->l < 0 || (bug->l % 2) == 0) bug->d = (bug->d + (rand() % 2) * 2 + 3) % 4;
  int p = bug->y * width + bug->x;
  hue(p, bug->h, 255);
  if (!--bug->l) {
    // for (int i = 0; i < 50; i++) {
    //   update_bug(bug);
    // }
    // printf("\a");
    // int b = 255;
    // for (int i = 1; i < 5; i++) {
    //   b = b * 210 >> 8;
    //   hue(p + i, bug->h, b);
    //   hue(p - i, bug->h, b);
    //   hue(p + i * width, bug->h, b);
    //   hue(p - i * width, bug->h, b);
    //   bug->h += 51;
    //   b = b * 210 >> 8;
    //   hue(p + i * (width + 1), bug->h, b);
    //   hue(p + i * (width - 1), bug->h, b);
    //   hue(p - i * (width + 1), bug->h, b);
    //   hue(p - i * (width - 1), bug->h, b);
    // }
    init_bug(bug);
  }

}

#define NUM_BUGS 1

int main() {
  bug_t bugs[NUM_BUGS];
  for (int i = 0; i < NUM_BUGS; i++) {
    init_bug(bugs + i);
  }
  // Clear screen and hide cursor
  printf("\e[2J\e[?25l");
  // Register sigint handler
  signal(SIGINT, intHandler);

  while (alive) {
    fade(250);
    for (int i = 0; i < NUM_BUGS; i++) {
      update_bug(bugs + i);
      update_bug(bugs + i);
      update_bug(bugs + i);
      update_bug(bugs + i);
      update_bug(bugs + i);
    }
    update();
    usleep(1000*16);
  }
  // Show cursor again
  printf("\e[?25h\n");
  return 0;
}
