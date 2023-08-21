#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 64
#define BRIGHTNESS 32
#define LED_TYPE WS2812

int pre_state, state, i, h, j, b;

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();
  Serial.begin(9600);
  Serial.setTimeout(10);
}

int reverse(int r) {
  if ((r / 8) % 2 == 0) {
    return r;
  }
  r = 8 * (r / 8 + 1) - (r % 8) - 1;
  return r;
}

void d_draw() {
  if (i <= 7) {
    for (j = 0; j <= i; j++) {
      leds[reverse(i + j * 7)] = CHSV(h, 255, b);
    }
  } else {
    for (j = 0; j <= (14 - i); j++) {
      leds[reverse((i - 6) * 8 - 1 + j * 7)] = CHSV(h, 255, b);  // (i+1)*H-1; i=id - D/2; (id - d/2 + 1)*M - 1
    }
  }
}

void draw() {
  if (state <= 15 * 255) {
    for (i = 0; i <= 14; i++) {
      if (i < (state / 255)) {
        h = map(state, 0, 15 * 255, 255, 0);
        b = 255;
      }
      if (i == (state / 255)) {
        h = map(state, 0, 15 * 255, 255, 0);
        b = state % 255;
      }
      if (i > (state / 255)) {
        h = 0;
        b = 0;
      }
      d_draw();
    }
  } else {
    for (i = 0; i <= 14; i++) {
      if (i < ((state - 15 * 255) / 255)) {
        h = 0;
        b = 0;
      }
      if (i == ((state - 15 * 255) / 255)) {
        h = map(state, 15 * 255, 2 * 15 * 255, 0, 255);
        b = ((15 * 255 - state) % 255);
      }
      if (i > ((state - 15 * 255) / 255)) {
        h = map(state, 15 * 255, 2 * 15 * 255, 0, 255);
        b = 255;
      }
      d_draw();
    }
  }
}

void loop() {
  for (state = 0; state <= 2 * 15 * 255; state += 20) {
    draw();
    FastLED.delay(10);
  }
}
