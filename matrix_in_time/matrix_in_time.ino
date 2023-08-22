#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 16
#define BRIGHTNESS 32
#define LED_TYPE WS2812

CRGB leds[NUM_LEDS];

int state, i, h;
int step = 5;
int duration, last_time, start_time;
int last_state = 0;

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();
  Serial.begin(9600);
  Serial.setTimeout(10);
  //Serial.println("Write time in milles.");
}

void draw() {
  for (i = 0; i <= NUM_LEDS - 1; i++) {
    if (i < (state / 255)) {
      leds[i] = CHSV(0, 0, 255);
    }
    if (i == (state / 255)) {
      leds[i] = CHSV(0, 0, state % 255);
    }
    if (i > (state / 255)) {
      leds[i] = CHSV(0, 0, 0);
    }
  }
  FastLED.show();
}

void loop() {
  if (Serial.available()) {
    duration = Serial.readString().toInt();
    start_time = millis();
    Serial.println(duration);
  }
  last_state = state;
  state = map((millis() - start_time), 0, duration, 8 * 255, 2 * 8 * 255);
  if ((millis() - start_time) < duration && last_state != state) {
    draw();
  } else {
    Serial.println(millis() - start_time);
  }
}
