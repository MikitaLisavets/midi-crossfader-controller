#include <Arduino.h>
#include <SPI.h>
#include <MIDIUSB.h> // Source: https://github.com/arduino-libraries/MIDIUSB
#include <GyverOLED.h> // Source: https://github.com/GyverLibs/GyverOLED
#include <EncButton.h> // Source: https://github.com/GyverLibs/EncButton

#define CLK_PIN 1
#define DT_PIN 0
#define SW_PIN A3

#define FADER_PIN A0
#define LEFT_PIN A1
#define RIGHT_PIN A2

#define NUMBER_OF_TRACKS 4
#define NUMBER_OF_PAGES 4
#define NUMBER_OF_STAGES 4

const uint8_t TRACK_PINS[NUMBER_OF_TRACKS] = {4, 5, 6, 7};
const uint8_t PAGE_PINS[NUMBER_OF_PAGES] = {8, 9, 10, 16};

GyverOLED<SSD1306_128x32, OLED_BUFFER> display;
EncButton<EB_TICK, DT_PIN, CLK_PIN, SW_PIN> encoder;

uint8_t pageIndex = 0;

int16_t potValue;

int16_t faderValue = 0;

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

void handle_left_midi_value_change(byte trackIndex) {
  /*
    TRACK button and LEFT button pressed:
      listen to POT and change left midi value
  */

  if (encoder.isRight()) {
      potValue = settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]] + (encoder.isFast() ? 5 : 1);
      if (potValue > 127) {
        potValue = 127;
      }
      settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]] = potValue;
    }
    if (encoder.isLeft()) {
      potValue = settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]] - (encoder.isFast() ? 5 : 1);
      if (potValue < 0) {
        potValue = 0;
      }
      settings.leftMidiValues[pageIndex][trackIndex][settings.stageLeftIndexes[pageIndex][trackIndex]] = potValue;
    }

    render_left_midi_value_change(trackIndex);

}

void handle_right_midi_value_change(byte trackIndex) {
  /*
    TRACK button and RIGHT button pressed:
      listen to POT and change right midi value
  */

 Serial.println(settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]]);
  if (encoder.isRight()) {
    potValue = settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]] + (encoder.isFast() ? 5 : 1);
    if (potValue > 127) {
      potValue = 127;
    }
    settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]] = potValue;
  }
  if (encoder.isLeft()) {
    potValue = settings.rightMidiValues[pageIndex][trackIndex][settings.stageRightIndexes[pageIndex][trackIndex]] - (encoder.isFast() ? 5 : 1);
    if (potValue < 0) {
      potValue = 0;
    }
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

// ================

void isr() {
  encoder.tickISR();
}

void setup() {
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

  render_main();
}

