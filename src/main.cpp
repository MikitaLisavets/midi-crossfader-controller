#include <Arduino.h>
#include <SPI.h>
#include <MIDIUSB.h> // Source: https://github.com/arduino-libraries/MIDIUSB
#include <GyverOLED.h> // Source: https://github.com/GyverLibs/GyverOLED
#include <EncButton.h> // Source: https://github.com/GyverLibs/EncButton
#include <EEPROM.h>

#define STR_ADDR 0

#define CLK_PIN 1
#define DT_PIN 0
#define SW_PIN A3

#define FADER_PIN A0
#define LEFT_PIN A1
#define RIGHT_PIN A2

#define NUMBER_OF_TRACKS 4
#define NUMBER_OF_PAGES 4
#define NUMBER_OF_STAGES 4

#define MAX_MENU_ROWS 20
#define SCREEN_MENU_ROWS 3

const uint8_t TRACK_PINS[NUMBER_OF_TRACKS] = {4, 5, 6, 7};
const uint8_t PAGE_PINS[NUMBER_OF_PAGES] = {8, 9, 10, 16};

GyverOLED<SSD1306_128x32, OLED_BUFFER> display;
EncButton<EB_TICK, DT_PIN, CLK_PIN, SW_PIN> encoder;

bool isMenuMode = false;
uint8_t pageIndex = 0;
int16_t potValue;
int16_t faderValue = 0;
int8_t menu_selected_row = 0;

enum menu_t : uint8_t {
  MENU_LOAD = 0,
  MENU_SAVE = 1,
  MENU_MIDI_CHANNEL = 2,
  MENU_FADER_THRESHOLD = 3,
  MENU_A1_CC = 4,
  MENU_B1_CC = 5,
  MENU_C1_CC = 6,
  MENU_D1_CC = 7,
  MENU_A2_CC = 8,
  MENU_B2_CC = 9,
  MENU_C2_CC = 10,
  MENU_D2_CC = 11,
  MENU_A3_CC = 12,
  MENU_B3_CC = 13,
  MENU_C3_CC = 14,
  MENU_D3_CC = 15,
  MENU_A4_CC = 16,
  MENU_B4_CC = 17,
  MENU_C4_CC = 18,
  MENU_D4_CC = 19,
};

char pageTitles[] = { '1', '2', '3', '4' };
char stageTitles[] = { '1', '2', '3', '4' };
char trackTitles[] = { 'A', 'B', 'C', 'D' };
struct Settings {
  uint8_t midiChannel = 0;
  uint8_t faderThreshold = 10;

  uint8_t stageLeftIndexes[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  };

  uint8_t stageRightIndexes[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  };

  uint8_t leftMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS][NUMBER_OF_STAGES] = {
    {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
  };

  uint8_t rightMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS][NUMBER_OF_STAGES] = {
    {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
    {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
    {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
    {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}}
  };

  uint8_t ccValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
    {0, 1, 2, 3},
    {4, 5, 6, 7},
    {8, 9, 10, 11},
    {12, 13, 14, 15}
  };
};

Settings settings;

uint8_t midiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

uint8_t previousMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

// === MIDI ===

void control_change(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, static_cast<uint8_t>(0xB0 | channel), control, value};
  MidiUSB.sendMIDI(event);
}

// ============

// === Display ===

void clear_dispay() {
  display.clear();
  display.setScale(1);
  display.invertText(false);
  display.setCursor(0, 0);
}

void refresh_dispay() {
  display.update();
}

void render_filled_number(uint8_t num) {
  display.print(num);

  if (num < 10) {
    display.print(F(".."));
  } else if (num >= 10 && num < 100) {
    display.print(F("."));
  }
}

void render_init() {
  clear_dispay();

  display.setScale(2);
  display.setCursor(25, 1);
  display.print(F("X"));
  refresh_dispay();
  delay(30);
  display.print(F("-"));
  refresh_dispay();
  delay(30);
  display.print(F("F"));
  refresh_dispay();
  delay(30);
  display.print(F("a"));
  refresh_dispay();
  delay(30);
  display.print(F("d"));
  refresh_dispay();
  delay(30);
  display.print(F("e"));
  refresh_dispay();
  delay(30);
  display.print(F("r"));
  refresh_dispay();
  delay(300);
}

void render_main() {
  clear_dispay();

  for (int i = 0; i < NUMBER_OF_TRACKS; i++) {
    display.print(trackTitles[i]);
    display.print(pageTitles[pageIndex]);
    display.print(F("|"));
    display.print(stageTitles[settings.stageLeftIndexes[pageIndex][i]]);
    display.print(F("| "));
    render_filled_number(settings.leftMidiValues[pageIndex][i][settings.stageLeftIndexes[pageIndex][i]]);
    display.print(F("<"));
    render_filled_number(midiValues[pageIndex][i]);
    display.print(F(">"));
    render_filled_number(settings.rightMidiValues[pageIndex][i][settings.stageRightIndexes[pageIndex][i]]);
    display.print(F(" |"));
    display.print(stageTitles[settings.stageRightIndexes[pageIndex][i]]);
    display.print(F("|"));
    display.println();
  }

  refresh_dispay();
}

void render_page_press() {
  clear_dispay();
  display.setScale(3);
  display.print(F("Page:"));
  display.println(pageTitles[pageIndex]);
  refresh_dispay();
}

void render_left_stage_change(int trackIndex) {
  clear_dispay();
  display.setScale(2);
  if (trackIndex >= 0) {
    display.print(F("Stage L: "));
    display.println(stageTitles[settings.stageLeftIndexes[pageIndex][trackIndex]]);
    display.print(F("Track: "));
    display.println(trackTitles[trackIndex]);
  } else {
    display.print(F("Stage L: "));
    display.println(stageTitles[settings.stageLeftIndexes[pageIndex][0]]);
    display.println(F("All tracks"));
  }

  refresh_dispay();
}

void render_right_stage_change(int trackIndex) {
  clear_dispay();
  display.setScale(2);
  if (trackIndex >= 0) {
    display.print(F("Stage R: "));
    display.println(stageTitles[settings.stageRightIndexes[pageIndex][trackIndex]]);
    display.print(F("Track: "));
    display.println(trackTitles[trackIndex]);
  } else {
    display.print(F("Stage R: "));
    display.println(stageTitles[settings.stageRightIndexes[pageIndex][0]]);
    display.println(F("All tracks"));
  }

  refresh_dispay();
}

void render_track_press(byte trackIndex) {
  clear_dispay();
  display.println(F("Sending MIDI ..."));
  display.print(F("Page:  "));
  display.println(pageTitles[pageIndex]);
  display.print(F("Track: "));
  display.println(trackTitles[trackIndex]);

  refresh_dispay();
}

void render_left_midi_value_change(byte trackIndex) {
  clear_dispay();
  display.setScale(2);

  display.println(F("Value L:"));
  display.println(settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]]);
  refresh_dispay();
}

void render_right_midi_value_change(byte trackIndex) {
  clear_dispay();
  display.setScale(2);

  display.println(F("Value R:"));
  display.println(settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]]);
  refresh_dispay();
}

void render_midi_values_swap(byte trackIndex) {
  clear_dispay();
  display.println(F("Swap L and R values:"));
  display.print(F("Value L: "));
  display.println(settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]]);
  display.print(F("Value R: "));
  display.println(settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]]);
  display.print(F("Track: "));
  display.println(trackTitles[trackIndex]);
  refresh_dispay();
}

// ===============

// === Menu ===

void render_row_load() {
  display.println(F("Load Settings"));
}

void render_row_save() {
  display.println(F("Save Settings"));
}

void render_row_midi_channel() {
  display.print(F("MIDI Channel: "));
  display.println(settings.midiChannel);
}

void render_row_fader_threshold() {
  display.print(F("Fader Threshold: "));
  display.println(settings.faderThreshold);
}

void render_row_track_cc(uint8_t pageIndex, uint8_t trackIndex) {
  display.print(F("Track "));
  display.print(trackTitles[trackIndex]);
  display.print(pageTitles[pageIndex]);
  display.print(F(" CC: "));
  display.println(settings.ccValues[pageIndex][trackIndex]);
}

void render_row(int8_t row_index) {
  bool is_selected = row_index == menu_selected_row;
  if (is_selected) {
    display.invertText(true);
  } else {
    display.invertText(false);
  }

  switch(row_index) {
    case MENU_LOAD: return render_row_load();
    case MENU_SAVE: return render_row_save();
    case MENU_MIDI_CHANNEL: return render_row_midi_channel();
    case MENU_FADER_THRESHOLD: return render_row_fader_threshold();
    case MENU_A1_CC: return render_row_track_cc(0, 0);
    case MENU_B1_CC: return render_row_track_cc(0, 1);
    case MENU_C1_CC: return render_row_track_cc(0, 2);
    case MENU_D1_CC: return render_row_track_cc(0, 3);
    case MENU_A2_CC: return render_row_track_cc(1, 0);
    case MENU_B2_CC: return render_row_track_cc(1, 1);
    case MENU_C2_CC: return render_row_track_cc(1, 2);
    case MENU_D2_CC: return render_row_track_cc(1, 3);
    case MENU_A3_CC: return render_row_track_cc(2, 0);
    case MENU_B3_CC: return render_row_track_cc(2, 1);
    case MENU_C3_CC: return render_row_track_cc(2, 2);
    case MENU_D3_CC: return render_row_track_cc(2, 3);
    case MENU_A4_CC: return render_row_track_cc(3, 0);
    case MENU_B4_CC: return render_row_track_cc(3, 1);
    case MENU_C4_CC: return render_row_track_cc(3, 2);
    case MENU_D4_CC: return render_row_track_cc(3, 3);
  }
}

void render_menu() {
  clear_dispay();
  display.setCursor(30, 0);
  display.println(F("=== Menu ==="));
  display.setCursor(0, 1);
  for (byte i = 0; i < SCREEN_MENU_ROWS; i++) {
    if (menu_selected_row < SCREEN_MENU_ROWS) {
      render_row(i);
    } else {
      render_row(menu_selected_row - SCREEN_MENU_ROWS + 1 + i);
    }
  }
  refresh_dispay();
}

void render_loading() {
  clear_dispay();
  display.setScale(2);
  display.println(F("Loading..."));
  refresh_dispay();
}

void render_saving() {
  clear_dispay();
  display.setScale(2);
  display.println(F("Saving..."));
  refresh_dispay();
}

// ============

// === Handlers ===

void handle_page_press(byte newPageIndex) {
  /*
    only PAGE button pressed:
      - change page index
  */

  pageIndex = newPageIndex;
  render_page_press();
}

void handle_left_stage_change(byte newStageIndex) {
  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (digitalRead(TRACK_PINS[trackIndex]) == LOW ) {
      /*
        PAGE button and LEFT button and TRACK button are pressed:
          - change left stage for track on current page
      */
        settings.stageLeftIndexes[pageIndex][trackIndex] = newStageIndex;
        render_left_stage_change(trackIndex);
        return;
    }
  }
  /*
    PAGE button and LEFT button pressed:
      - change left stage for all tracks on current page
  */
  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    settings.stageLeftIndexes[pageIndex][trackIndex] = newStageIndex;
  }
  render_left_stage_change(-1);
}

void handle_right_stage_change(byte newStageIndex) {
  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (digitalRead(TRACK_PINS[trackIndex]) == LOW ) {
      /*
        PAGE button and RIGHT button and TRACK button are pressed:
          - change right stage for track on current page
      */
        settings.stageRightIndexes[pageIndex][trackIndex] = newStageIndex;
        render_right_stage_change(trackIndex);
        return;
    }
  }
  /*
    PAGE button and RIGHT button pressed:
      - change right stage for all tracks on current page
  */
  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    settings.stageRightIndexes[pageIndex][trackIndex] = newStageIndex;
  }
  render_right_stage_change(-1);
}

void handle_track_press(byte trackIndex) {
  /*
    TRACK button pressed:
      - send midi event to learn with current value
  */

  control_change(settings.midiChannel, settings.ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
  MidiUSB.flush();
  render_track_press(trackIndex);

  delay(100);
}

uint8_t safe_midi_value(int16_t unsafe_midi_value) {
  if (potValue > 127) {
    return 127;
  } else if (potValue < 0) {
    return 0;
  } else {
    return unsafe_midi_value;
  }
}

void handle_left_midi_value_change(byte trackIndex) {
  /*
    TRACK button and LEFT button pressed:
      listen to POT and change left midi value
  */
  uint8_t speed;

  if (encoder.isRight()) {
      speed = encoder.isFast() ? 5 : encoder.isRightH() ? 10 : 1;
      potValue = safe_midi_value(settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]] + speed);
      settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]] = potValue;
    }
    if (encoder.isLeft()) {
      speed = encoder.isFast() ? 5 : encoder.isLeftH() ? 10 : 1;
      potValue = safe_midi_value(settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]] - speed);
      settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]] = potValue;
    }

    render_left_midi_value_change(trackIndex);

}

void handle_right_midi_value_change(byte trackIndex) {
  uint8_t speed;
  /*
    TRACK button and RIGHT button pressed:
      listen to POT and change right midi value
  */

  if (encoder.isRight()) {
    speed = encoder.isFast() ? 5 : encoder.isRightH() ? 10 : 1;

    potValue = safe_midi_value(settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]] + speed);
    settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]] = potValue;
  }
  if (encoder.isLeft()) {
    speed = encoder.isFast() ? 5 : encoder.isLeftH() ? 10 : 1;

    potValue = safe_midi_value(settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]] - speed);
    settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]] = potValue;
  }

  render_right_midi_value_change(trackIndex);
}

void handle_midi_values_swap(byte trackIndex) {
  /*
    TRACK button and LEFT or RIGHT button pressed and POT button are pressed:
      - swap left and right midi values
  */
  byte temp = settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]];
  settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]] = settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]];
  settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]] = temp;
  render_midi_values_swap(trackIndex);

  delay(300);
}

void handle_menu() {
  if (encoder.isRight()) {
    menu_selected_row++;
    if (menu_selected_row >= MAX_MENU_ROWS) {
      menu_selected_row = 0;
    }
  }

  if (encoder.isLeft()) {
    menu_selected_row--;
    if (menu_selected_row < 0) {
      menu_selected_row = MAX_MENU_ROWS - 1;
    }
  }

  if (encoder.isClick()) {
    if (menu_selected_row == MENU_LOAD) {
      render_loading();
      EEPROM.get(STR_ADDR, settings);
      delay(300);
    }
    if (menu_selected_row == MENU_SAVE) {
      render_saving();
      EEPROM.put(STR_ADDR, settings);
      delay(300);
    }
  }

  if (digitalRead(LEFT_PIN) == LOW) {
    if (menu_selected_row == MENU_MIDI_CHANNEL) {
      settings.midiChannel = safe_midi_value(settings.midiChannel - 1);
    }
    if (menu_selected_row == MENU_FADER_THRESHOLD) {
      int16_t newValue = settings.faderThreshold - 1;
      if (newValue < 0) {
        newValue = 0;
      }
      settings.faderThreshold = newValue;
    }
    if (menu_selected_row == MENU_A1_CC) {
      settings.ccValues[0][0] = safe_midi_value(settings.ccValues[0][0] - 1);
    }
    if (menu_selected_row == MENU_B1_CC) {
      settings.ccValues[0][1] = safe_midi_value(settings.ccValues[0][1] - 1);
    }
    if (menu_selected_row == MENU_C1_CC) {
      settings.ccValues[0][2] = safe_midi_value(settings.ccValues[0][2] - 1);
    }
    if (menu_selected_row == MENU_D1_CC) {
      settings.ccValues[0][3] = safe_midi_value(settings.ccValues[0][3] - 1);
    }
    if (menu_selected_row == MENU_A2_CC) {
      settings.ccValues[1][0] = safe_midi_value(settings.ccValues[1][0] - 1);
    }
    if (menu_selected_row == MENU_B2_CC) {
      settings.ccValues[1][1] = safe_midi_value(settings.ccValues[1][1] - 1);
    }
    if (menu_selected_row == MENU_C2_CC) {
      settings.ccValues[1][2] = safe_midi_value(settings.ccValues[1][2] - 1);
    }
    if (menu_selected_row == MENU_D2_CC) {
      settings.ccValues[1][3] = safe_midi_value(settings.ccValues[1][3] - 1);
    }
    if (menu_selected_row == MENU_A3_CC) {
      settings.ccValues[2][0] = safe_midi_value(settings.ccValues[2][0] - 1);
    }
    if (menu_selected_row == MENU_B3_CC) {
      settings.ccValues[2][1] = safe_midi_value(settings.ccValues[2][1] - 1);
    }
    if (menu_selected_row == MENU_C3_CC) {
      settings.ccValues[2][2] = safe_midi_value(settings.ccValues[2][2] - 1);
    }
    if (menu_selected_row == MENU_D3_CC) {
      settings.ccValues[2][3] = safe_midi_value(settings.ccValues[2][3] - 1);
    }
    if (menu_selected_row == MENU_A4_CC) {
      settings.ccValues[3][0] = safe_midi_value(settings.ccValues[3][0] - 1);
    }
    if (menu_selected_row == MENU_B4_CC) {
      settings.ccValues[3][1] = safe_midi_value(settings.ccValues[3][1] - 1);
    }
    if (menu_selected_row == MENU_C4_CC) {
      settings.ccValues[3][2] = safe_midi_value(settings.ccValues[3][2] - 1);
    }
    if (menu_selected_row == MENU_D4_CC) {
      settings.ccValues[3][3] = safe_midi_value(settings.ccValues[3][3] - 1);
    }
    delay(150);
  }

  if (digitalRead(RIGHT_PIN) == LOW) {
    if (menu_selected_row == MENU_MIDI_CHANNEL) {
      settings.midiChannel = safe_midi_value(settings.midiChannel + 1);
    }
    if (menu_selected_row == MENU_FADER_THRESHOLD) {
      int16_t newValue = settings.faderThreshold + 1;
      if (newValue > 1023) {
        newValue = 1023;
      }
      settings.faderThreshold = newValue;
    }
    if (menu_selected_row == MENU_A1_CC) {
      settings.ccValues[0][0] = safe_midi_value(settings.ccValues[0][0] + 1);
    }
    if (menu_selected_row == MENU_B1_CC) {
      settings.ccValues[0][1] = safe_midi_value(settings.ccValues[0][1] + 1);
    }
    if (menu_selected_row == MENU_C1_CC) {
      settings.ccValues[0][2] = safe_midi_value(settings.ccValues[0][2] + 1);
    }
    if (menu_selected_row == MENU_D1_CC) {
      settings.ccValues[0][3] = safe_midi_value(settings.ccValues[0][3] + 1);
    }
    if (menu_selected_row == MENU_A2_CC) {
      settings.ccValues[1][0] = safe_midi_value(settings.ccValues[1][0] + 1);
    }
    if (menu_selected_row == MENU_B2_CC) {
      settings.ccValues[1][1] = safe_midi_value(settings.ccValues[1][1] + 1);
    }
    if (menu_selected_row == MENU_C2_CC) {
      settings.ccValues[1][2] = safe_midi_value(settings.ccValues[1][2] + 1);
    }
    if (menu_selected_row == MENU_D2_CC) {
      settings.ccValues[1][3] = safe_midi_value(settings.ccValues[1][3] + 1);
    }
    if (menu_selected_row == MENU_A3_CC) {
      settings.ccValues[2][0] = safe_midi_value(settings.ccValues[2][0] + 1);
    }
    if (menu_selected_row == MENU_B3_CC) {
      settings.ccValues[2][1] = safe_midi_value(settings.ccValues[2][1] + 1);
    }
    if (menu_selected_row == MENU_C3_CC) {
      settings.ccValues[2][2] = safe_midi_value(settings.ccValues[2][2] + 1);
    }
    if (menu_selected_row == MENU_D3_CC) {
      settings.ccValues[2][3] = safe_midi_value(settings.ccValues[2][3] + 1);
    }
    if (menu_selected_row == MENU_A4_CC) {
      settings.ccValues[3][0] = safe_midi_value(settings.ccValues[3][0] + 1);
    }
    if (menu_selected_row == MENU_B4_CC) {
      settings.ccValues[3][1] = safe_midi_value(settings.ccValues[3][1] + 1);
    }
    if (menu_selected_row == MENU_C4_CC) {
      settings.ccValues[3][2] = safe_midi_value(settings.ccValues[3][2] + 1);
    }
    if (menu_selected_row == MENU_D4_CC) {
      settings.ccValues[3][3] = safe_midi_value(settings.ccValues[3][3] + 1);
    }
    delay(150);
  }
  
  render_menu();
}

// ================

void isr() {
  encoder.tickISR();
}

void setup() {
  Serial.begin(9600);

  // Turn off system leds
  pinMode(LED_BUILTIN_TX, INPUT);
  pinMode(LED_BUILTIN_RX, INPUT);


  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);

  for (int i = 0; i < NUMBER_OF_TRACKS; i++) {
    pinMode(TRACK_PINS[i], INPUT_PULLUP);
  }

  for (int i = 0; i < NUMBER_OF_PAGES; i++) {
    pinMode(PAGE_PINS[i], INPUT_PULLUP);
  }

  display.init();
  render_init();
  clear_dispay();

  attachInterrupt(2, isr, CHANGE);
  attachInterrupt(3, isr, CHANGE);
}

void loop() {
  encoder.tick();

  for (byte pIndex = 0; pIndex < NUMBER_OF_PAGES; pIndex++) {
    if (digitalRead(PAGE_PINS[pIndex]) == LOW) {
      if (digitalRead(LEFT_PIN) == LOW && digitalRead(RIGHT_PIN) == HIGH) {
        handle_left_stage_change(pIndex);
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == LOW) {
        handle_right_stage_change(pIndex);
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == HIGH) {
        handle_page_press(pIndex);
        return;
      } 
    }
  }

  for (int tIndex = 0; tIndex < NUMBER_OF_TRACKS; tIndex++) {
    if (digitalRead(TRACK_PINS[tIndex]) == LOW) {
      if (digitalRead(LEFT_PIN) == LOW && digitalRead(RIGHT_PIN) == HIGH) {
        handle_left_midi_value_change(tIndex);
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == LOW) {
        handle_right_midi_value_change(tIndex);
        return;
      }  else if (digitalRead(LEFT_PIN) == LOW && digitalRead(RIGHT_PIN) == LOW) {
        handle_midi_values_swap(tIndex);
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == HIGH) {
        handle_track_press(tIndex);
        return;
      } 
    }
  }

  faderValue = analogRead(FADER_PIN);
  if (faderValue > 1023 - settings.faderThreshold) {
    faderValue = 1023 - settings.faderThreshold;
  } else if (faderValue < 0 + settings.faderThreshold) {
    faderValue = 0 + settings.faderThreshold;
  }

  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]] < settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]]) {
      midiValues[pageIndex][trackIndex] = map(faderValue, 1023 - settings.faderThreshold, 0 + settings.faderThreshold, settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]], settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]]);
    } else {
      midiValues[pageIndex][trackIndex] = map(faderValue, 0 + settings.faderThreshold, 1023 - settings.faderThreshold, settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]], settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]]);
    }

    if (previousMidiValues[pageIndex][trackIndex] != midiValues[pageIndex][trackIndex]) {
      control_change(settings.midiChannel, settings.ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
      previousMidiValues[pageIndex][trackIndex] = midiValues[pageIndex][trackIndex];
      MidiUSB.flush();
    }
  }
  
  if (!isMenuMode && encoder.isClick()) {
    isMenuMode = true;
  }

  if (isMenuMode) {
    handle_menu();

    for (byte pIndex = 0; pIndex < NUMBER_OF_PAGES; pIndex++) {
      for (int tIndex = 0; tIndex < NUMBER_OF_TRACKS; tIndex++) {
        if (digitalRead(PAGE_PINS[pIndex]) == LOW || digitalRead(TRACK_PINS[tIndex]) == LOW) {
          isMenuMode = false;
          delay(100);
        }
      }
    }
  } else {
    render_main();
  }
}

