#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 8
#define BRIGHTNESS 16
#define LED_TYPE WS2812

int i, j, h;

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (j = 1; j <= 10; j++) {
    for (h = 0; h <= 255; h++) {
      for (i = 0; i <= NUM_LEDS - 1; i++) {
        leds[i] = CHSV(h + i * 2 * j, 255, 255);
      }
      FastLED.show();
      FastLED.delay(10);
    }
  }
}
