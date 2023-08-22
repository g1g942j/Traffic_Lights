#include <unior.h>
#include <unior_data_processing.h>
#include <unior_filters.h>
#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 8
#define BRIGHTNESS 32
#define LED_TYPE WS2812

UniorBLEModule emg1("D0:44:97:62:C3:6A");

EMG_Raw_Processing emg1_int;
CRGB leds[NUM_LEDS];

int start_time, state, last_state, i, hue, saturation, data_count, current_state;
int animation_step = 100;  // 50-125; Необходимо играться со значением, для определения плавной анимации.
int calibration_time = 5000;
float emg1_value, emg1_int_value, data;
float emg_min = 999999999999;
float emg_max = -1;
bool animation_doing = false;

int reverse(int r) {
  if ((r / 8) % 2 == 0) {
    return r;
  }
  r = 8 * (r / 8 + 1) - (r % 8) - 1;
  return r;
}

void get_data() {
  if (emg1.read(emg1_value)) {
    if (emg1_int.process(emg1_value, emg1_int_value)) {
    }
  }
}

void animation() {
  if (animation_doing == false) {
    start_time = millis();
    state = map(data / data_count, emg_min, emg_max, 0, NUM_LEDS * 255);
    data = data_count = 0;
    animation_doing = true;
  }

  if ((millis() - start_time) < animation_step) {
    get_data();
    data += emg1_int_value;
    data_count++;

    last_state = current_state;
    current_state = map((millis() - start_time), 0, animation_step, last_state, state);

    if (last_state != state) {
      draw();
    }
  } else {
    animation_doing = false;
  }
}

void draw() {
  for (i = 0; i <= NUM_LEDS - 1; i++) {
    if (i < (current_state / 255)) {
      leds[reverse(i)] = CHSV(hue, 255, 255);
    }
    if (i == (current_state / 255)) {
      leds[reverse(i)] = CHSV(hue, 255, state % 255);
    }
    if (i > (current_state / 255)) {
      leds[reverse(i)] = CHSV(hue, 255, 0);
    }
  }
  FastLED.show();
}

void calibration() {
  // Нет других действий, поэтому можно использовать while.
  while ((millis() - start_time) < calibration_time) {
    get_data();
    if (emg_max < emg1_int_value) {
      emg_max = emg1_int_value;
    }
    if (emg_min > emg1_int_value) {
      emg_min = emg1_int_value;
    }
    current_state = map((millis() - start_time), 0, calibration_time, 0, NUM_LEDS * 255);
    draw();
  }
  Serial.print("Calibration result:");
  Serial.print(emg_min);
  //Serial.print(map(emg1_int_value, emg_min, emg_max, 0, NUM_LEDS * 255));
  Serial.print(" ");
  Serial.println(emg_max);
  // Serial.println(map(emg1_int_value, emg_min, emg_max, 0, NUM_LEDS * 255));
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
    saturation = 255;
    calibration();
    saturation = 0;
  }
}

void check() {
  if (emg1_int_value <= (emg_min + (emg_max - emg_min) / 3)) {
    hue = 80;
  }
  if (emg1_int_value >= (emg_min + (emg_max - emg_min) / 3) && emg1_int_value >= (emg_min + 2 * (emg_max - emg_min) / 3)) {
    hue = 30;
  }
  if (emg1_int_value >= (emg_min + 2 * (emg_max - emg_min) / 3)) {
    hue = 0;
  }
}

void loop() {
  get_data();
  check();
  animation();
}