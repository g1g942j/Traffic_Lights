#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 64
#define BRIGHTNESS 32
#define LED_TYPE WS2812

int state, i, h;
int step = 5;  // Скорость анимации изменяется через размер шага.

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();
  Serial.begin(9600);
}

int reverse(int r) {
  if ((r / 8) % 2 == 0) {
    return r;
  }
  r = 8 * (r / 8 + 1) - (r % 8) - 1;
  return r;
}

void loop() {
  if (state <= 64 * 255) {
    for (state = 0; state <= 64 * 255; state += step) {
      h = map(state, 0, 64 * 255, 152, 0);
      if (state / 255 >= 1) {
        for (i = 1; i <= state / 255; i++) {
          leds[reverse(i - 1)] = CHSV(h, 255, 255);
        }
      }
      leds[reverse(state / 255)] = CHSV(h, 255, state % 255);
      FastLED.delay(2);
    }
  } else {
    for (state = 64 * 255; state >= 0; state -= step) {
      h = map(state, 64 * 255, 2 * 64 * 255, 0, 152);
      if ((64 * 255 - state) / 255 >= 1) {
        for (i = 64; i >= (64 * 255 - state) / 255; i--) {
          leds[reverse(i - 1)] = CHSV(255 - h, 255, 255);
        }
        for (i = 1; i <= (64 * 255 - state) / 255; i++) {
          leds[reverse(i - 1)] = CHSV(255 - h, 255, 0);
        }
      }
      leds[reverse((64 * 255 - state) / 255)] = CHSV(255 - h, 255, 255 - ((64 * 255 - state) % 255));
      FastLED.delay(2);
    }
    //leds[56] = CHSV(h, 255, 0);  // КОСТЫЛЬ, последний светодиод(линия светодиода перевёрнута) не гаснет полностью, принуждаем его. // КОСТЫЛЬ решён(прозапас код сохранён).
  }
}
