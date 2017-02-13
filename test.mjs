let width = getWidth();
let height = getHeight();
let speed = (width * height) >> 7;
while (getAlive()) {
  let i = speed;
  while (i--) {
    let p = rand() % (width * height);
    let h = rand() % 1536;
    hue(p, h, 255);
    hue(p + 1, h, 200);
    hue(p - 1, h, 200);
    hue(p + width, h, 200);
    hue(p - width, h, 200);
    hue(p + 1 + width, h, 100);
    hue(p + 1 - width, h, 100);
    hue(p - 1 + width, h, 100);
    hue(p - 1 - width, h, 100);
  }
  update();
  fade(210);
  msleep(16);
}
