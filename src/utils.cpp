#include <EEPROM.h>
#include <EncButton.h> // Source: https://github.com/GyverLibs/EncButton
#include <utils.h>

#define STR_ADDR 0

int64_t encoder_click_timer = 0;

EncButton<EB_TICK, DT_PIN, CLK_PIN, SW_PIN> encoder;

void load_settings(Settings &settings) {
  EEPROM.get(STR_ADDR, settings);
}

void save_settings(Settings &settings) {
  EEPROM.put(STR_ADDR, settings);
}

void isr() {
  encoder.tickISR();
}

void init_encoder() {
  attachInterrupt(2, isr, CHANGE); // Interrupt pin (INT2) arduino pro micro D1 TX()
  attachInterrupt(3, isr, CHANGE); // Interrupt pin (INT3) arduino pro micro D0 (RX)
}

void encoder_tick() {
  encoder.tick();
}

bool is_encoder_turned_right() {
  return encoder.isRight();
}

bool is_encoder_turned_left() {
  return encoder.isLeft();
}

bool is_encoder_turned_fast() {
  return encoder.isFast();
}

void start_encoder_click_timeout() {
  encoder_click_timer = millis();
}

bool is_encoder_click_timeout_ended() {
  return millis() - encoder_click_timer >= CLICK_TIMEOUT;
}

bool is_encoder_clicked() {
  if (digitalRead(SW_PIN) == LOW && is_encoder_click_timeout_ended()) {
    start_encoder_click_timeout();
    return true;
  }
  return false;
}

bool is_left_button_pressed() {
  return digitalRead(LEFT_PIN) == LOW;
}

bool is_right_button_pressed() {
  return digitalRead(RIGHT_PIN) == LOW;
}

bool is_button_pressed(int8_t buttonIndex) {
  return buttonIndex >= 0;
}

int8_t get_pressed_track_button() {
  for (int8_t trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (digitalRead(TRACK_PINS[trackIndex]) == LOW ) {
      return trackIndex;
    }
  }
  return -1;
}

int8_t get_pressed_page_button() {
  for (int8_t pageIndex = 0; pageIndex < NUMBER_OF_PAGES; pageIndex++) {
    if (digitalRead(PAGE_PINS[pageIndex]) == LOW ) {
      return pageIndex;
    }
  }
  return -1;
}

uint8_t safe_midi_value(int16_t unsafe_midi_value) {
  if (unsafe_midi_value > 127) {
    return 127;
  } else if (unsafe_midi_value < 0) {
    return 0;
  } else {
    return unsafe_midi_value;
  }
}

void increase_cc_value(int8_t pageIndex, int8_t trackIndex) {
  settings.ccValues[pageIndex][trackIndex] = safe_midi_value(settings.ccValues[pageIndex][trackIndex] + 1);
}

void decrease_cc_value(int8_t pageIndex, int8_t trackIndex) {
  settings.ccValues[pageIndex][trackIndex] = safe_midi_value(settings.ccValues[pageIndex][trackIndex] - 1);
}