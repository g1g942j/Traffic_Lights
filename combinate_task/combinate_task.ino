#include <unior.h>
#include <unior_data_processing.h>
#include <unior_filters.h>
#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 64
#define BRIGHTNESS 32
#define LED_TYPE WS2812

UniorBLEModule emg1("D0:44:97:62:C3:6A");

EMG_Raw_Processing emg1_int;
CRGB leds[NUM_LEDS];

int start_time, fill_start_time, state, last_state, fill_state, fill_last_state, i, saturation, data_count, current_state;
int animation_step = 100;  // 50-125; Необходимо играться со значением, для определения плавной анимации.
int calibration_time = 5000;
int duration = 10000;
float emg1_value, emg1_int_value, data;
float emg_min = 999999999999;
float emg_max = -1;
bool animation_doing = false;
bool fill_doing = false;
bool mode = false;

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
  mode = true;
  saturation = 0;
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
      draw(current_state);
    }
  } else {
    animation_doing = false;
  }
}

void draw(int draw_state) {
  if (mode == true) {
    for (i = 0; i <= 8 - 1; i++) {
      if (i < (draw_state / 255)) {
        leds[reverse(i)] = CHSV(0, saturation, 255);
      }
      if (i == (draw_state / 255)) {
        leds[reverse(i)] = CHSV(0, saturation, draw_state % 255);
      }
      if (i > (draw_state / 255)) {
        leds[reverse(i)] = CHSV(0, saturation, 0);
      }
    }
  }
  if (mode == false) {
    for (i = 8; i <= 16 - 1; i++) {
      if (i < (draw_state / 255)) {
        leds[reverse(i)] = CHSV(0, saturation, 255);
      }
      if (i == (draw_state / 255)) {
        leds[reverse(i)] = CHSV(0, saturation, draw_state % 255);
      }
      if (i > (draw_state / 255)) {
        leds[reverse(i)] = CHSV(0, saturation, 0);
      }
    }
  }
  FastLED.show();
}

void calibration() {
  // Так ка программа выполняется один раз и нет других действий - используется while.
  while ((millis() - start_time) < calibration_time) {
    get_data();
    if (emg_max < emg1_int_value) {
      emg_max = emg1_int_value;
    }
    if (emg_min > emg1_int_value && emg1_int_value != 0) {
      emg_min = emg1_int_value;
    }
    current_state = map((millis() - start_time), 0, calibration_time, 0, 8 * 255);
    mode = true;  // Если calibration() будет использоваться ещё в программе, для верного построения.
    draw(current_state);
  }
  Serial.print("Calibration result:");
  Serial.print(emg_min);
  Serial.print(" ");
  Serial.println(emg_max);
}

void fill() {
  mode = false;
  saturation = 255;
  if (fill_doing == false) {
    fill_start_time = millis();
    fill_doing = true;
  }
  fill_last_state = fill_state;
  fill_state = map((millis() - fill_start_time), 0, duration, 8 * 255, 2 * 8 * 255);
  if ((millis() - start_time) < duration && last_state != state) {
    draw(fill_state);
  }
  if ((millis() - start_time) >= duration) {
    fill_doing = false;
  }
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

void loop() {
  // if (emg1.connected() == false) {
  //   emg1.start();
  // }
  get_data();
  fill();
  animation();
}
