#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 64
#define BRIGHTNESS 32
#define LED_TYPE WS2812

int pre_state, state, i, h;
int step = 5;  // Скорость анимации изменяется через размер шага.

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

void draw() {
  if (state <= 64 * 255) {
    for (i = 0; i <= NUM_LEDS - 1; i++) {
      if (i < (state / 255)) {
        leds[reverse(i)] = CHSV(map(state, 0, 64 * 255, 152, 0), 255, 255);
      }
      if (i == (state / 255)) {
        leds[reverse(i)] = CHSV(map(state, 0, 64 * 255, 152, 0), 255, state % 255);
      }
      if (i > (state / 255)) {
        leds[reverse(i)] = CHSV(0, 255, 0);
      }
    }
  } else {
    for (i = 0; i <= NUM_LEDS - 1; i++) {
      if (i < ((state - 64 * 255) / 255)) {
        leds[reverse(i)] = CHSV(0, 255, 0);
      }
      if (i == ((state - 64 * 255) / 255)) {
        leds[reverse(i)] = CHSV(map(state, 64 * 255, 2 * 64 * 255, 0, 152), 255, ((64 * 255 - state) % 255));
      }
      if (i > ((state - 64 * 255) / 255)) {
        leds[reverse(i)] = CHSV(map(state, 64 * 255, 2 * 64 * 255, 0, 152), 255, 255);
      }
    }
  }
}

void loop() {
  if (Serial.available()) {
    pre_state = Serial.readString().toInt();
    if (pre_state >= 0 and pre_state <= 32640) {
      state = pre_state;
    } else {
      Serial.println("ERROR: state out of range 0 - 32640.");
    }
    Serial.println(state);
  }
  draw();
}
