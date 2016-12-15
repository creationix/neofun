#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>


typedef enum opcode {
  // 0 - 127 number literals
  // 0msxxxxx mxxxxxxx
  // `m` is more flag, `s` is sign, `x`s are variable length value.
  GGET = 128, // 16 global gets
  SGET = 144, // 16 sprite gets
  GSET = 160, // 16 global sets
  SSET = 176, // 16 sprite sets
  GMOD = 192,
  SMOD = 208,
  // Global configuration
  CONFIG = 224, // (length, bpp, delay, sprites, fizz)
  // Library functions
  FADE, // fade(d)
  RAND, // rand()
  HUE, // hue(p, h, d)
  RGB, // rgb(p, r, g, b)
  RGBW, // rgbw(p, r, g, b, w)

  // Core language
  ADD, SUB, MUL, DIV, MOD, NEG, // Arithmetic
  EQ, NEQ, GT, GTE, LT, LTE, // Comparison
  AND, OR, XOR, NOT, // Logic
  // Control flow
  IF, // if (cond) body...
  ELF, // elf (cond) body...
  ELSE, // else body...
  LOOP, // for (name start end step)
  END, // end for conditional or loop block
  GOSUB, // call subroutine
  RETURN, // return from subroutine
  RESPAWN, // start over current sprite
  RESTART, // re-run global setup function and reset sprites.
  RAW, // offset, length, bytes
} opcode_t;

uint8_t* code = (uint8_t[]){
  // length = 60 // Our strip has 60 LEDs
  // bpp = 3     // We are using normal ws2812b lights that expect 3 bytes of color.
  // delay = 33  // Delay between animation loops in ms (defaults to 100)
  // sprites = 4 // Total number of sprites to spawn at startup
  // fizz = 3    // How many sprites to update per animation loop (defaults to all)
  60, 3, 33, 4, 3, CONFIG,

  // // This is called once per animation loop before updating sprites.
  // loop {
  //   fade(1)
  1, FADE,
  RETURN,
  // }
  //
  // // This is called for each sprite when it's spawned.
  // // Variables created here are local to the sprite.
  // spawn {

  //   p:length = rand()   // Random starting position
  //   h:768 = rand()      // Random starting hue
  //   l = rand(50) + 50   // Semi-random lifetime
  //   r = rand(2) * 2 - 1 // Random direction
  // }
  //
  // // This is called once per sprite update
  // // you can access global variables as well as sprite local variables.
  // update {
  //   p += 1         // Move 1 pixel forward
  //   h += 7         // Slightly change hue
  //   hue(p, h, 100) // Draw new position
  //   l -= 1         // Decrement lifetime.
  //   if l <= 0 {
  //     explode()    // When lifetime reaches zero, explode!
  //   }
  // }
  //
  // // draw gradient that color shifts while fading away.
  // explode {
  //   b = 255
  //   loop i:14 {
  //     b -= 18
  //     hue(p + i, h, b)
  //     hue(p - i, h, b)
  //   }
  //   respawn() // And then restart the sprite.
  // }
};
