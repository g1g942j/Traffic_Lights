int second_count, minute_count, hour_count;

void timer() {
  if ((millis() - start_time) >= 1000) {
    second_count++;
    start_time = millis();
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

void setup() {
}

void loop() {
  timer();
}
