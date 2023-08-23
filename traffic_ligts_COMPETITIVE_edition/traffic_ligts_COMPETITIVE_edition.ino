#include <unior.h>
#include <unior_data_processing.h>
#include <unior_filters.h>
#include <FastLED.h>
#include <stdlib.h>

#define LED_PIN 12
#define NUM_LEDS 64
#define BRIGHTNESS 32
#define LED_TYPE WS2812

UniorBLEModule emg1("D0:44:97:62:C3:6A");
UniorBLEModule emg2("D0:44:97:62:C3:6A");

EMG_Raw_Processing emg1_int;
EMG_Raw_Processing emg2_int;
CRGB leds[NUM_LEDS];

int start_time, fill_start_time, state, last_state, fill_state, fill_last_state, i, j, hue, saturation;
int data_count_1, data_count_2 current_state, timer_first_player_start_time, timer_second_player_start_time, second_count, zero_index, goal, last_check_position;
int animation_step = 100;
int goal_time = 3;
int calibration_time = 5 * 1000;
int duration = goal_time * 1000;
int check_position = 0;
float emg1_value, emg1_int_value, emg1_value, emg1_int_value, sum_data_1, sum_data_2;
float emg1_min = emg2_min = 999999999999;
float emg1_max = emg2_max = -1;
bool animation_doing = false;
bool fill_doing = false;
bool check_state = false;
bool player_id = false;
bool first_player_win = false;

int goal_color[3]{ 0, 60, 100 };
int first[10]{ 18, 25, 32, 33, 34, 35, 36, 37, 38, 39 };
int second[16]{ 14, 17, 18, 21, 22, 24, 28, 30, 32, 35, 38, 40, 42, 46, 49, 54 };

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
  if (emg2.read(emg1_value)) {
    if (emg2_int.process(emg2_value, emg2_int_value)) {
    }
  }
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
    zero_index = 0;  // Если calibration() будет использоваться ещё в программе, для верного построения.
    draw(current_state);
    FastLED.show();
  }
  Serial.print("Calibration result:");
  Serial.print(emg_min);
  Serial.print(" ");
  Serial.println(emg_max);
}

void draw(int draw_state) {
  for (i = zero_index; i <= zero_index + 7; i++) {
    if (i < (draw_state / 255)) {
      leds[reverse(i)] = CHSV(hue, saturation, 255);
    }
    if (i == (draw_state / 255)) {
      leds[reverse(i)] = CHSV(hue, saturation, draw_state % 255);
    }
    if (i > (draw_state / 255)) {
      leds[reverse(i)] = CHSV(hue, saturation, 0);
    }
  }
}

void fill(bool fill_doing, int fill_last_state, int fill_state, int fill_start_time, int last_check_position, int check_position) {
  if (player_id == false) {
    zero_index = 16;
  } else {
    zero_index = 40;
  }
  saturation = 0;
  if (fill_doing == false) {
    fill_start_time = millis();
    fill_doing = true;
  }
  fill_last_state = fill_state;
  fill_state = map((millis() - fill_start_time), 0, duration, 8 * 255, 2 * 8 * 255);
  if ((millis() - start_time) < duration && last_state != state) {
    draw(fill_state);
  }
  // if ((millis() - fill_start_time) >= duration) { // ||
  if (last_check_position != check_position || goal_color[goal] != hue) {
    fill_doing = false;
  }
}

void animation() {
  zero_index = 0;
  saturation = 255;
  if (animation_doing == false) {
    start_time = millis();
    state = map(data / data_count, emg_min, emg_max, 0, 8 * 255);
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

void scale() {
  zero_index = 24;
  for (i = zero_index; i <= zero_index + 7; i++) {
    if ((i - zero_index + 1) * emg1_max / 8 <= (emg1_min + (emg1_max - emg1_min) / 3)) {  // Зелёный
      hue = 100;
    }
    if ((i - zero_index + 1) * emg1_max / 8 >= (emg1_min + (emg1_max - emg1_min) / 3) && (i - 39) * emg1_max / 8 <= (emg1_min + 2 * (emg1_max - emg1_min) / 3)) {  // Жёлтый
      hue = 60;
    }
    if ((i - zero_index + 1) * emg1_max / 8 >= (emg1_min + 2 * (emg1_max - emg1_min) / 3)) {  // Красный
      hue = 0;
    }
    leds[reverse(i)] = CHSV(hue, 255, 128);
  }
  zero_index = 32;
  for (i = zero_index; i <= zero_index + 7; i++) {
    if ((i - zero_index + 1) * emg2_max / 8 <= (emg2_min + (emg2_max - emg2_min) / 3)) {  // Зелёный
      hue = 100;
    }
    if ((i - zero_index + 1) * emg2_max / 8 >= (emg2_min + (emg2_max - emg2_min) / 3) && (i - 39) * emg2_max / 8 <= (emg2_min + 2 * (emg2_max - emg2_min) / 3)) {  // Жёлтый
      hue = 60;
    }
    if ((i - zero_index + 1) * emg2_max / 8 >= (emg2_min + 2 * (emg2_max - emg2_min) / 3)) {  // Красный
      hue = 0;
    }
    leds[reverse(i)] = CHSV(hue, 255, 128);
  }
  FastLED.show();
}

void check() {
  zero_index = 56;
  saturation = 255;
  if (second_count >= goal_time && goal_color[goal] == hue) {  // Проверка достижения цели
    draw(64 * 255);
    check_state = false;
    get_reward();
  }
  if (second_count < goal_time) {  // Обработка утраты цели
    draw(0);
  }
  last_check_position = check_position;
  if (check_state == false) {  // Проверка начала цели
    check_state = true;
    timer_start_time = millis();
    second_count = 0;
    srand(emg1_int_value);
    goal = rand() % 3;
  }

  hue = goal_color[goal];
  zero_index = 48;
  draw(56 * 255);

  if (emg1_int_value < (emg_min + (emg_max - emg_min) / 3)) {  // Зелёный
    hue = 100;
    if (check_position != 2) {
      second_count = 0;
      check_position = 2;
    }
  }
  if (emg1_int_value >= (emg_min + (emg_max - emg_min) / 3) && emg1_int_value <= (emg_min + 2 * (emg_max - emg_min) / 3)) {  // Жёлтый
    hue = 60;
    if (check_position != 1) {
      second_count = 0;
      check_position = 1;
    }
  }
  if (emg1_int_value > (emg_min + 2 * (emg_max - emg_min) / 3)) {  // Красный
    hue = 0;
    if (check_position != 0) {
      second_count = 0;
      check_position = 0;
    }
  }
}

void timer(int timer_start_time, int second_count) {
  if ((millis() - timer_start_time) >= 1000) {
    second_count++;
    timer_start_time += 1000;
  }
  if (second_count >= 60) {
    second_count -= 60;
    minute_count++;
  }
  if (minute_count >= 60) {
    minute_count -= 60;
    hour_count++;
  }
}

void check_win() {
  saturation = 255;
  if (first_player_win == true) {
    hue = 60;
    for (i = 0; i <= 63; i++) {
      for (j = 0; j <= size(first); j++) {
        if (i == first[j]) {
          leds[reverse(i)] = CHSV(hue, 255, 128);
          break;
        } else {
          leds[reverse(i)] = CHSV(0, 0, 0);
        }
      }
    }
  } else {
    hue = 152;
    for (i = 0; i <= 63; i++) {
      for (j = 0; j <= size(second); j++) {
        if (i == second[j]) {
          leds[reverse(i)] = CHSV(hue, 255, 128);
          break;
        } else {
          leds[reverse(i)] = CHSV(0, 0, 0);
        }
      }
    }
  }
  FastLED.show();
  FastLED.delay(10 * 1000);
  for (i = 0; i <= 63; i++) {
    leds[reverse(i)] = CHSV(0, 0, 0);
  }
}

void one_step() {
  if (player_id == false) {
  } else {
  }
  fill();
  check();
  animation();
  player_id != player_id;
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
    saturation = 0;
    calibration();
    saturation = 255;
  }
  scale();
}

void loop() {
  get_data();
  timer();
  one_step();
  one_step();
  FastLED.show();
  check_win();
}
