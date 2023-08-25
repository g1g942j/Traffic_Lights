#include <unior.h>
#include <unior_data_processing.h>
#include <unior_filters.h>
#include <FastLED.h>
#include <stdlib.h>
//#include <ctime> // Библиотека для ключа srand();, рандомной генерации не было, ключом стало значение ЭМГ.

#define LED_PIN 12
#define NUM_LEDS 64
#define BRIGHTNESS 32
#define LED_TYPE WS2812

UniorBLEModule emg1("D0:44:97:62:C3:6A");
UniorBLEModule emg2("D7:C9:C8:AE:99:C1");

EMG_Raw_Processing emg1_int;
EMG_Raw_Processing emg2_int;
CRGB leds[NUM_LEDS];

int start_time, fill_start_time, state, last_state, fill_state, fill_last_state, i, j, hue, saturation, data_count, current_state, timer_start_time, second_count, zero_index, goal, last_check_position;
int animation_step = 100;  // 50-125; Необходимо играться со значением, для определения плавной анимации. В миллисекундах.
int goal_time = 3;         // В секундах
int calibration_time = 5 * 1000;
int duration = goal_time * 1000;
int check_position = 0;
float emg1_value, emg1_int_value, emg2_value, emg2_int_value, data;
float emg1_min = 999999999999;
float emg1_max = -1;
float emg2_min = 999999999999;
float emg2_max = -1;
bool animation_doing = false;
bool fill_doing = false;
bool check_state = false;

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
    second_player_current_state = map((millis() - calibration_start_time), 0, calibration_time, 56 * 255, 64 * 255);
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
  FastLED.show();
}

void fill(int zero_index, int fill_doing) {
  saturation = 0;
  if (fill_doing == false) {
    fill_start_time = millis();
    fill_doing = true;
  }
  fill_last_state = fill_state;
  fill_state = map((millis() - fill_start_time), 0, duration, zero_index * 255, (zero_index + 8) * 255);
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

void get_reward() {
  saturation = 255;
  hue = 60;
  for (i = 0; i <= 63; i++) {
    for (j = 0; j <= 13; j++) {
      if (i == smile[j]) {
        leds[reverse(i)] = CHSV(60, 255, 128);
        break;
      } else {
        leds[reverse(i)] = CHSV(0, 0, 0);
      }
    }
  }
  FastLED.show();
  FastLED.delay(10 * 1000);
  for (i = 0; i <= 63; i++) {
    leds[reverse(i)] = CHSV(0, 0, 0);
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
    start_time = millis();
    saturation = 0;
    calibration();
    saturation = 255;
  }
}

void loop() {
  get_data();
  timer();
  fill();
  check();
  animation();
}