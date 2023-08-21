#include <unior.h>
#include <unior_data_processing.h>
#include <unior_filters.h>
#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 64
#define BRIGHTNESS 32
#define LED_TYPE WS2812

UniorBLEModule emg1("E2:B6:A6:B2:39:50");

EMG_Raw_Processing emg1_int;
CRGB leds[NUM_LEDS];

int start_time, state, time_draw, last_state, i, data_count;
int animation_step = 500;
float emg1_value, emg1_int_value, data;
float emg_min = 99999999999999999999999;
float emg_max = 0;
bool animation_doing = false;

int reverse(int r) {
  if ((r / 8) % 2 == 0) {
    return r;
  }
  r = 8 * (r / 8 + 1) - (r % 8) - 1;
  return r;
}

void draw() {
  for (i = 0; i <= NUM_LEDS - 1; i++) {
    if (emg1.read(emg1_value)) {
      if (emg1_int.process(emg1_value, emg1_int_value)) {
        data += emg1_int_value;
        data_count++;
      }
    }
    if (i < (state / 255)) {
      leds[reverse(i)] = CHSV(0, 0, 255);
    }
    if (i == (state / 255)) {
      leds[reverse(i)] = CHSV(0, 0, state % 255);
    }
    if (i > (state / 255)) {
      leds[reverse(i)] = CHSV(0, 0, 0);
    }
  }
}

void calibration() {
  while ((millis() - start_time) < 5000) {
    if (emg1.read(emg1_value)) {
      if (emg1_int.process(emg1_value, emg1_int_value)) {
      }
    }
    if (emg_max < emg1_int_value) {
      emg_max = emg1_int_value;
    }
    if (emg_min > emg1_int_value) {
      emg_min = emg1_int_value;
    }
    leds[0] = CHSV(0, 255, 128);
    FastLED.show();
  }
  //Serial.print(emg_min);
  Serial.print(map(emg1_int_value, emg_min, emg_max, 0, 8 * 255));
  Serial.print(" ");
  //Serial.println(emg_max);
  Serial.print(map(emg1_int_value, emg_min, emg_max, 0, 8 * 255));
}

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();

  Serial.begin(115200);
  Serial.setTimeout(10);
  emg1.connect(0);

  if (emg1.connected()) {
    emg1.start();
    start_time = millis();
    calibration();
  } else {
  }
}

void loop() {
  if (emg1.read(emg1_value)) {
    if (emg1_int.process(emg1_value, emg1_int_value)) {
      if((millis()- start_time) > animation_step){
        animation_doing = false;
      }
      if (animation_doing == false) {
        start_time = millis();
        last_state = state;
        state = map(emg1_int_value, emg_min, emg_max, 0, NUM_LEDS * 255);
        animation_doing = true;
      }
      time_draw = map((millis() - start_time), 0, animation_step, 0, state);
      draw();
      FastLED.show();
    }
  }
}
