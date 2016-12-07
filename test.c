#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

static volatile int alive = 1;

#define width 64
#define height 64
#define count (width*height)
uint8_t pixels[count * 3];

void fade(uint8_t keep) {
  for (int i = 0; i < count * 3; i++) {
    pixels[i] = pixels[i] * keep >> 8;
  }
}
void update() {
  printf("\e[H");
  for (int y = 0; y < height; y += 2) {
    for (int x = 0; x < width; x++) {
      int i = (y * width + x) * 3;
      printf("\e[48;2;%d;%d;%dm\e[38;2;%d;%d;%dm▄",
        pixels[i], pixels[i+1], pixels[i+2],
        pixels[i + width*3], pixels[i+width*3+1], pixels[i+width*3+2]);
    }
    printf("\r\n");
  }
}
void rgb(int p, int r, int g, int b) {
  int i = ((p + count) % count) * 3;
  pixels[i] = r;
  pixels[++i] = g;
  pixels[++i] = b;
}

static const uint16_t squares[] = {
    0, 1, 4, 9,
    16, 25, 36, 49,
    64, 81, 100, 121,
    144, 169, 196, 225,
    256, 289, 324, 361,
    400, 441, 484, 529,
    576, 625, 676, 729,
    784, 841, 900, 961,
    1024, 1089, 1156, 1225,
    1296, 1369, 1444, 1521,
    1600, 1681, 1764, 1849,
    1936, 2025, 2116, 2209,
    2304, 2401, 2500, 2601,
    2704, 2809, 2916, 3025,
    3136, 3249, 3364, 3481,
    3600, 3721, 3844, 3969,
    4096, 4225, 4356, 4489,
    4624, 4761, 4900, 5041,
    5184, 5329, 5476, 5625,
    5776, 5929, 6084, 6241,
    6400, 6561, 6724, 6889,
    7056, 7225, 7396, 7569,
    7744, 7921, 8100, 8281,
    8464, 8649, 8836, 9025,
    9216, 9409, 9604, 9801,
    10000, 10201, 10404, 10609,
    10816, 11025, 11236, 11449,
    11664, 11881, 12100, 12321,
    12544, 12769, 12996, 13225,
    13456, 13689, 13924, 14161,
    14400, 14641, 14884, 15129,
    15376, 15625, 15876, 16129,
    16384, 16641, 16900, 17161,
    17424, 17689, 17956, 18225,
    18496, 18769, 19044, 19321,
    19600, 19881, 20164, 20449,
    20736, 21025, 21316, 21609,
    21904, 22201, 22500, 22801,
    23104, 23409, 23716, 24025,
    24336, 24649, 24964, 25281,
    25600, 25921, 26244, 26569,
    26896, 27225, 27556, 27889,
    28224, 28561, 28900, 29241,
    29584, 29929, 30276, 30625,
    30976, 31329, 31684, 32041,
    32400, 32761, 33124, 33489,
    33856, 34225, 34596, 34969,
    35344, 35721, 36100, 36481,
    36864, 37249, 37636, 38025,
    38416, 38809, 39204, 39601,
    40000, 40401, 40804, 41209,
    41616, 42025, 42436, 42849,
    43264, 43681, 44100, 44521,
    44944, 45369, 45796, 46225,
    46656, 47089, 47524, 47961,
    48400, 48841, 49284, 49729,
    50176, 50625, 51076, 51529,
    51984, 52441, 52900, 53361,
    53824, 54289, 54756, 55225,
    55696, 56169, 56644, 57121,
    57600, 58081, 58564, 59049,
    59536, 60025, 60516, 61009,
    61504, 62001, 62500, 63001,
    63504, 64009, 64516, 65025
};

static int isqrt(uint16_t x) {
    const uint16_t *p = squares;

    if (p[128] <= x) p += 128;
    if (p[ 64] <= x) p +=  64;
    if (p[ 32] <= x) p +=  32;
    if (p[ 16] <= x) p +=  16;
    if (p[  8] <= x) p +=   8;
    if (p[  4] <= x) p +=   4;
    if (p[  2] <= x) p +=   2;
    if (p[  1] <= x) p +=   1;

    return p - squares;
}

void rgbl(int p, int r, int g, int b, int keep) {
  // return rgb(p,
  //   r * keep / 255,
  //   g * keep / 255,
  //   b * keep / 255);
  int dist = isqrt(r*r+g*g+b*b);
  return rgb(p,
    r * keep / dist,
    g * keep / dist,
    b * keep / dist);
}
void hue(int p, int h, uint8_t keep) {
  h = (h + 768) % 768;
  if (h < 256) return rgbl(p, h, 255 - h, 0, keep);
  if (h < 512) return rgbl(p, 511 - h, 0, h - 256, keep);
  return rgbl(p, 0, h - 512, 767 - h, keep);
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
  bug->h = rand() % 768;
  bug->l = rand() % 100;
}
void update_bug(bug_t* bug) {
  switch(bug->d) {
    case 0: bug->x = (bug->x + width - 1) % width; break;
    case 1: bug->x = (bug->x + 1) % width; break;
    case 2: bug->y = (bug->y + height - 1) % height; break;
    case 3: bug->y = (bug->y + 1) % height; break;
  }
  bug->h = (bug->h + 13) % 768;
  if ((bug->l % 10) == 0) bug->d = rand() % 4;
  int p = bug->y * width + bug->x;
  hue(p, bug->h, 255);
  if (!--bug->l) {
    int b = 255;
    for (int i = 1; i < 4; i++) {
      b = b * 250 >> 8;
      hue(p + i, bug->h, b);
      hue(p - i, bug->h, b);
      hue(p + i * width, bug->h, b);
      hue(p - i * width, bug->h, b);
      bug->h += 27;
      b = b * 250 >> 8;
      hue(p + i * (width + 1), bug->h, b);
      hue(p + i * (width - 1), bug->h, b);
      hue(p - i * (width + 1), bug->h, b);
      hue(p - i * (width - 1), bug->h, b);
    }
    init_bug(bug);
  }

}

int main() {
  bug_t bugs[15];
  for (int i = 0; i < 15; i++) {
    init_bug(bugs + i);
  }
  // Clear screen and hide cursor
  printf("\e[2J\e[?25l");
  // Register sigint handler
  signal(SIGINT, intHandler);

  while (alive) {
    fade(240);
    for (int i = 0; i < 15; i++) {
      update_bug(bugs + i);
    }
    update();
    usleep(1000*16);
  }
  // Show cursor again
  printf("\e[?25h\n");
  return 0;
}
