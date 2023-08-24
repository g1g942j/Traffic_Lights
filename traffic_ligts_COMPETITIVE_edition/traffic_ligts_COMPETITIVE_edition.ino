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
UniorBLEModule emg2("D7:C9:C8:AE:99:C1");

EMG_Raw_Processing emg1_int;
EMG_Raw_Processing emg2_int;
CRGB leds[NUM_LEDS];

int i, j, saturation, zero_index, goal, goal_hue, calibration_start_time, check_state;
int hue_1, data_count_1, state_1, last_state_1, current_state_1, first_player_current_state, first_player_state, first_player_last_state, timer_first_player_start_time, last_check_first_player_position, first_player_start_time, fill_first_player_start_time, first_player_fill_state, first_player_fill_last_state, first_player_second_count, first_player_check_position, first_player_last_check_position;
int hue_2, data_count_2, state_2, last_state_2, current_state_2, second_player_current_state, second_player_state, second_player_last_state, timer_second_player_start_time, last_check_second_player_position, second_player_start_time, fill_second_player_start_time, second_player_fill_state, second_player_fill_last_state, second_player_second_count, second_player_check_position, second_player_last_check_position;
int animation_step = 100;
int goal_time = 3;
int calibration_time = 5 * 1000;
int duration = goal_time * 1000;
float emg1_value, emg1_int_value, emg2_value, emg2_int_value, sum_data_1, sum_data_2;
float emg1_min = 999999999999;
float emg2_min = 999999999999;
float emg1_max = -1;
float emg2_max = -1;
bool first_player_animation_doing = false;
bool first_player_fill_doing = false;
bool first_player_check_state = false;
bool second_player_animation_doing = false;
bool second_player_fill_doing = false;
bool second_player_check_state = false;
bool first_player_win = false;
bool get_win = false;

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
  if (emg2.read(emg2_value)) {
    if (emg2_int.process(emg2_value, emg2_int_value)) {
    }
  }
}

void calibration() {
  while ((millis() - calibration_start_time) < calibration_time) {
    get_data();
    if (emg1_max < emg1_int_value) {
      emg1_max = emg1_int_value;
    }
    if (emg1_min > emg1_int_value && emg1_int_value != 0) {
      emg1_min = emg1_int_value;
    }
    if (emg2_max < emg2_int_value) {
      emg2_max = emg2_int_value;
    }
    if (emg2_min > emg2_int_value && emg2_int_value != 0) {
      emg2_min = emg2_int_value;
    }
    first_player_current_state = map((millis() - calibration_start_time), 0, calibration_time, 0, 8 * 255);
    zero_index = 0;
    draw(first_player_current_state, hue_1);
    zero_index = 56;
    second_player_current_state = map((millis() - calibration_start_time), 0, calibration_time, 0, 8 * 255);
    draw(second_player_current_state, hue_2);
    FastLED.show();
  }
  Serial.print("Calibration result for emg1:");
  Serial.print(emg1_min);
  Serial.print(" ");
  Serial.println(emg1_max);
  Serial.print("Calibration result for emg2:");
  Serial.print(emg2_min);
  Serial.print(" ");
  Serial.println(emg2_max);
}

void draw(int draw_state, int hue) {
  for (i = zero_index; i <= zero_index + 7; i++) {
    if (i - zero_index < (draw_state / 255)) {
      leds[reverse(i)] = CHSV(hue, saturation, 255);
    }
    if (i - zero_index == (draw_state / 255)) {
      leds[reverse(i)] = CHSV(hue, saturation, draw_state % 255);
    }
    if (i - zero_index > (draw_state / 255)) {
      leds[reverse(i)] = CHSV(hue, saturation, 0);
    }
  }
  // FastLED.show();
}

void fill() {
  saturation = 0;
  if (first_player_fill_doing == false) {
    fill_first_player_start_time = millis();
    first_player_fill_doing = true;
  }
  if (second_player_fill_doing == false) {
    fill_second_player_start_time = millis();
    second_player_fill_doing = true;
  }

  first_player_fill_last_state = first_player_fill_state;
  first_player_fill_state = map((millis() - fill_first_player_start_time), 0, duration, 0, 8 * 255);
  if ((millis() - timer_first_player_start_time) >= duration) {
    first_player_win = true;
    get_win = true;
  }
  if ((millis() - timer_first_player_start_time) < duration && first_player_last_state != first_player_state) {
    zero_index = 8;
    draw(first_player_fill_state, hue_1);
  }
  second_player_fill_last_state = second_player_fill_state;
  second_player_fill_state = map((millis() - fill_second_player_start_time), 0, duration, 0, 8 * 255);
  if ((millis() - timer_second_player_start_time) >= duration) {
    first_player_win = false;
    get_win = true;
  }
  if ((millis() - timer_second_player_start_time) < duration && second_player_last_state != second_player_state) {
    zero_index = 48;
    draw(second_player_fill_state, hue_2);
  }

  if (first_player_last_check_position != first_player_check_position || goal_color[goal] != goal_hue) {
    first_player_fill_doing = false;
  }
  if (second_player_last_check_position != second_player_check_position || goal_color[goal] != goal_hue) {
    second_player_fill_doing = false;
  }
}

void animation() {
  saturation = 255;

  if (first_player_animation_doing == false) {
    first_player_start_time = millis();
    first_player_state = map(sum_data_1 / data_count_1, emg1_min, emg1_max, 0, 8 * 255);
    sum_data_1 = 0;
    data_count_1 = 0;
    first_player_animation_doing = true;
  }
  if (second_player_animation_doing == false) {
    second_player_start_time = millis();
    second_player_state = map(sum_data_2 / data_count_2, emg2_min, emg2_max, 0, 8 * 255);
    sum_data_2 = 0;
    data_count_2 = 0;
    second_player_animation_doing = true;
  }

  if ((millis() - first_player_start_time) < animation_step || (millis() - second_player_start_time) < animation_step) {
    get_data();
    sum_data_1 += emg1_int_value;
    data_count_1++;
    sum_data_2 += emg2_int_value;
    data_count_2++;

    first_player_last_state = first_player_current_state;
    first_player_current_state = map((millis() - first_player_start_time), 0, animation_step, first_player_last_state, first_player_state);
    second_player_last_state = second_player_current_state;
    second_player_current_state = map((millis() - second_player_start_time), 0, animation_step, second_player_last_state, second_player_state);

    // if (first_player_last_state != first_player_state) {
      zero_index = 0;
      // Serial.print("FIRST PLAYER.");
      // Serial.println(first_player_current_state);
      draw(first_player_current_state, hue_1);
    // } else {
    //   first_player_animation_doing = false;
    // }
    // if (second_player_last_state != second_player_state) {
      zero_index = 56;
      // Serial.println("SECOND PLAYER.");
      draw(second_player_current_state, hue_2);
    // } else {
    //   second_player_animation_doing = false;
    // }
  }
}

void check() {
  saturation = 255;
  check_win();

  // if (first_player_second_count < goal_time) {
  //   zero_index = 0;
  //   draw(0, 0);
  // }
  // if (second_player_second_count < goal_time) {
  //   zero_index = 56;
  //   draw(0, 0);  // Возможно здесь ошибка!
  // }

  last_check_first_player_position = first_player_check_position;
  last_check_second_player_position = second_player_check_position;

  if (check_state == false) {  // Проверка начала цели
    check_state = true;
    timer_first_player_start_time = millis();
    timer_second_player_start_time = millis();
    first_player_second_count = 0;
    second_player_second_count = 0;
    srand(emg1_int_value);
    goal = rand() % 3;
  }

  goal_hue = goal_color[goal];
  zero_index = 16;
  draw(8 * 255, goal_hue);

  if (emg1_int_value < (emg1_min + (emg1_max - emg1_min) / 3)) {  // Зелёный
    hue_1 = 100;
    if (first_player_check_position != 2) {
      first_player_second_count = 0;
      first_player_check_position = 2;
    }
  }
  if (emg1_int_value >= (emg1_min + (emg1_max - emg1_min) / 3) && emg1_int_value <= (emg1_min + 2 * (emg1_max - emg1_min) / 3)) {  // Жёлтый
    hue_1 = 60;
    if (first_player_check_position != 1) {
      first_player_second_count = 0;
      first_player_check_position = 1;
    }
  }
  if (emg1_int_value > (emg1_min + 2 * (emg1_max - emg1_min) / 3)) {  // Красный
    hue_1 = 0;
    if (first_player_check_position != 0) {
      first_player_second_count = 0;
      first_player_check_position = 0;
    }
  }

  if (emg2_int_value < (emg2_min + (emg2_max - emg2_min) / 3)) {  // Зелёный
    hue_2 = 100;
    if (second_player_check_position != 2) {
      second_player_second_count = 0;
      second_player_check_position = 2;
    }
  }
  if (emg2_int_value >= (emg2_min + (emg2_max - emg2_min) / 3) && emg2_int_value <= (emg2_min + 2 * (emg2_max - emg2_min) / 3)) {  // Жёлтый
    hue_2 = 60;
    if (second_player_check_position != 1) {
      second_player_second_count = 0;
      second_player_check_position = 1;
    }
  }
  if (emg2_int_value > (emg2_min + 2 * (emg2_max - emg2_min) / 3)) {  // Красный
    hue_2 = 0;
    if (second_player_check_position != 0) {
      second_player_second_count = 0;
      second_player_check_position = 0;
    }
  }
}

void timer() {
  if ((millis() - timer_first_player_start_time) >= 1000) {
    first_player_second_count++;
    timer_first_player_start_time += 1000;
  }
  if ((millis() - timer_second_player_start_time) >= 1000) {
    second_player_second_count++;
    timer_second_player_start_time += 1000;
  }
}

void check_win() {
  saturation = 255;
  if (first_player_second_count >= goal_time && goal_color[goal] == hue_1) {
    check_state = false;
    first_player_win = true;
    get_win = true;
  }
  if (second_player_second_count >= goal_time && goal_color[goal] == hue_2) {
    check_state = false;
    first_player_win = false;
    get_win = true;
  }
  if (first_player_win == true && get_win == true) {
    goal_hue = 60;
    for (i = 0; i <= 63; i++) {
      for (j = 0; j <= 9; j++) {
        if (i == first[j]) {
          leds[reverse(i)] = CHSV(goal_hue, 255, 128);
          break;
        } else {
          leds[reverse(i)] = CHSV(0, 0, 0);
        }
      }
    }
  }
  if (first_player_win == false && get_win == true) {
    goal_hue = 152;
    for (i = 0; i <= 63; i++) {
      for (j = 0; j <= 15; j++) {
        if (i == second[j]) {
          leds[reverse(i)] = CHSV(goal_hue, 255, 128);
          break;
        } else {
          leds[reverse(i)] = CHSV(0, 0, 0);
        }
      }
    }
  }
  if (get_win == true) {
    FastLED.show();
    FastLED.delay(10 * 1000);
    for (i = 0; i <= 63; i++) {
      leds[reverse(i)] = CHSV(0, 0, 0);
    }
    get_win = false;
  }
}

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();

  Serial.begin(115200);
  Serial.setTimeout(10);
  emg1.connect(0);
  emg2.connect(0);

  if (emg1.connected() && emg2.connected()) {
    emg1.start();
    emg2.start();
    calibration_start_time = millis();
    saturation = 0;
    calibration();
    saturation = 255;
  }
  hue_1 = -1;
  hue_2 = -1;
}

void loop() {
  get_data();
  timer();
  fill();
  check();
  // check_win();
  animation();
  FastLED.show();
}
