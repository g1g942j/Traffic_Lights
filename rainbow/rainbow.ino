#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 64
#define BRIGHTNESS 32
#define LED_TYPE WS2812

CRGB leds[NUM_LEDS];

int state, i, h, j;
int step = 5;
int duration, last_time, start_time;
int last_state = 0;

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();
  Serial.begin(9600);
  Serial.setTimeout(10);
}

void draw() {
  for (i = 1; i <= 8; i++) {
    for (j = i; j <= (7 - i); j += 9) {
      leds[j - 1] = CHSV(0, 255, 128);
    }
  }
  FastLED.show();
}

int reverse(int r) {
  if ((r / 8) % 2 == 0) {
    return r;
  }
  r = 8 * (r / 8 + 1) - (r % 8) - 1;
  return r;
}

void loop() {
  for (i = 56; i >= 1; i-=8) {
    for (j = 0; j <= (8 - i); j++) {
      leds[reverse(i + ((j - 1) * 9))] = CHSV(0, 255, 128);
    }
    FastLED.show();
    FastLED.delay(1000);
  }
}
