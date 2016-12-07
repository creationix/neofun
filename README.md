## API

- fade(d) - fade the entire strip by 2^d
- rgb(p, r, g, b)
- rgbw(p, r, g, b, w)
- hue(p, h, d)
- rand(n) - integer from 0 to n-1
- callable from within update block only
  - respawn() - reuse this sprite slot, but re-run setup
  - die() - kill this sprite slot
  - spawn() - create a new sprite slot

## Configuration

The top of the program is configuration.  The following is a sample using all
allowed keys.

```js
length: 60    // length of neopixels in strip. default is `30`
type: grb     // byte order. default is `grb` or Green/Red/Blue,
              // others are `rgb`, `grbw`, and `rgbw`
delay: 100 // number of ms between animation loops. Default 100
```

Code blocks are also configuration but contain code.

### Setup Code

This code is run once at program startup.

```js
setup:
  // code goes here...
```

### Loop Code

This code is run every loop iteration.

```js
loop:
  // code goes here...
```

### Spawn Code

This code is run when a sprite is spawned.  Here we setup internal variables
for the sprite.

```js
spawn:
  // code goes here...
```

### Update Code

This code is run every animation frame for each sprite.

```js
update:
  // code goes here...
```

## Language

- name= value - set a variable
- name - retrieve a variable
-
