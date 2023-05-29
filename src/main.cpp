#include <Arduino.h>
#include <SPI.h>
#include <MIDIUSB.h> // Source: https://github.com/arduino-libraries/MIDIUSB
#include <Adafruit_GFX.h> // Source: https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h> // Source: https://github.com/adafruit/Adafruit_SSD1306
#include <Fonts/TomThumb.h>
#include <Fonts/Picopixel.h>

const uint8_t SCREEN_WIDTH = 128;
const uint8_t SCREEN_HEIGHT = 32;
const uint8_t SCREEN_ADDRESS = 0x3C;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const byte FADER_PIN = A0;

const byte POT_PIN = A1;

const byte LEFT_PIN = 8;
const byte RIGHT_PIN = 9;

const byte NUMBER_OF_TRACKS = 4;
const byte TRACK_PINS[NUMBER_OF_TRACKS] = {4, 5, 6, 7};

const byte NUMBER_OF_PAGES = 4;
const byte PAGE_PINS[NUMBER_OF_PAGES] = {10, 16, A2, 15};

const byte NUMBER_OF_STAGES = 4;

byte midiChannel = 0;
byte pageIndex = 0;

byte stageLeftIndexes[NUMBER_OF_STAGES] = {0, 0, 0, 0};
byte stageRightIndexes[NUMBER_OF_STAGES] = {0, 0, 0, 0};

int faderValue = 0;
int potValue = 0;

byte midiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

byte previousMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

byte leftMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS][NUMBER_OF_STAGES] = {
  {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
};

byte rightMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS][NUMBER_OF_STAGES] = {
  {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
  {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
  {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
  {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}}
};

byte ccValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {0, 1, 2, 3},
  {4, 5, 6, 7},
  {8, 9, 10, 11},
  {12, 13, 14, 15}
};

void control_change(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, static_cast<uint8_t>(0xB0 | channel), control, value};
  MidiUSB.sendMIDI(event);
}

void clear_dispay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
}

void refresh_dispay() {
  display.display();
}

void render_main() {
  clear_dispay();
  display.setCursor(30, display.getCursorY());
  display.println("=== Page: "+ String(pageIndex) + " ===");

  // Display A Track
  display.print(F("A | "));
  display.print("(" + String(stageLeftIndexes[pageIndex]) + ")");
  display.setCursor(27 - String(leftMidiValues[pageIndex][0][stageLeftIndexes[pageIndex]]).length(), display.getCursorY());
  display.print(String(leftMidiValues[pageIndex][0][stageLeftIndexes[pageIndex]]));
  display.setCursor(40, display.getCursorY());
  display.print(F("<"));
  display.setCursor(55 - String(midiValues[pageIndex][0]).length(), display.getCursorY());
  display.print(String(midiValues[pageIndex][0]));
  display.setCursor(68, display.getCursorY());
  display.print(F(">"));
  display.setCursor(78 - String(rightMidiValues[pageIndex][0][stageRightIndexes[pageIndex]]).length(), display.getCursorY());
  display.print(String(rightMidiValues[pageIndex][0][stageRightIndexes[pageIndex]]));
  display.setCursor(92, display.getCursorY());
  display.print("(" + String(stageRightIndexes[pageIndex]) + ")");
  display.println();

  // Display B Track
  display.print(F("B | "));
  display.print("(" + String(stageLeftIndexes[pageIndex]) + ")");
  display.setCursor(27 - String(leftMidiValues[pageIndex][1][stageLeftIndexes[pageIndex]]).length(), display.getCursorY());
  display.print(String(leftMidiValues[pageIndex][1][stageLeftIndexes[pageIndex]]));
  display.setCursor(40, display.getCursorY());
  display.print(F("<"));
  display.setCursor(55 - String(midiValues[pageIndex][1]).length(), display.getCursorY());
  display.print(String(midiValues[pageIndex][1]));
  display.setCursor(68, display.getCursorY());
  display.print(F(">"));
  display.setCursor(78 - String(rightMidiValues[pageIndex][1][stageRightIndexes[pageIndex]]).length(), display.getCursorY());
  display.print(String(rightMidiValues[pageIndex][1][stageRightIndexes[pageIndex]]));
  display.setCursor(92, display.getCursorY());
  display.print("(" + String(stageRightIndexes[pageIndex]) + ")");
  display.println();

  // Display C Track
  display.print(F("C | "));
  display.print("(" + String(stageLeftIndexes[pageIndex]) + ")");
  display.setCursor(27 - String(leftMidiValues[pageIndex][2][stageLeftIndexes[pageIndex]]).length(), display.getCursorY());
  display.print(String(leftMidiValues[pageIndex][2][stageLeftIndexes[pageIndex]]));
  display.setCursor(40, display.getCursorY());
  display.print(F("<"));
  display.setCursor(55 - String(midiValues[pageIndex][2]).length(), display.getCursorY());
  display.print(String(midiValues[pageIndex][2]));
  display.setCursor(68, display.getCursorY());
  display.print(F(">"));
  display.setCursor(78 - String(rightMidiValues[pageIndex][2][stageRightIndexes[pageIndex]]).length(), display.getCursorY());
  display.print(String(rightMidiValues[pageIndex][2][stageRightIndexes[pageIndex]]));
  display.setCursor(92, display.getCursorY());
  display.print("(" + String(stageRightIndexes[pageIndex]) + ")");
  display.println();

  // Display D Track
  display.print(F("D | "));
  display.print("(" + String(stageLeftIndexes[pageIndex]) + ")");
  display.setCursor(27 - String(leftMidiValues[pageIndex][3][stageLeftIndexes[pageIndex]]).length(), display.getCursorY());
  display.print(String(leftMidiValues[pageIndex][3][stageLeftIndexes[pageIndex]]));
  display.setCursor(40, display.getCursorY());
  display.print(F("<"));
  display.setCursor(55 - String(midiValues[pageIndex][3]).length(), display.getCursorY());
  display.print(String(midiValues[pageIndex][3]));
  display.setCursor(68, display.getCursorY());
  display.print(F(">"));
  display.setCursor(78 - String(rightMidiValues[pageIndex][3][stageRightIndexes[pageIndex]]).length(), display.getCursorY());
  display.print(String(rightMidiValues[pageIndex][3][stageRightIndexes[pageIndex]]));
  display.setCursor(92, display.getCursorY());
  display.print("(" + String(stageRightIndexes[pageIndex]) + ")");
  display.println();
  refresh_dispay();
}

void render_page_change() {
  clear_dispay();
  display.setCursor(20, 15);
  display.setTextSize(3);
  display.println("Page: " + String(pageIndex));
  refresh_dispay();
}

void render_left_stage_change() {
  clear_dispay();
  display.setCursor(0, 15);
  display.setTextSize(3);
  display.println("Stage Left:");
  display.println(String(stageLeftIndexes[pageIndex]));
  refresh_dispay();
}

void render_right_stage_change() {
  clear_dispay();
  display.setCursor(0, 15);
  display.setTextSize(3);
  display.println("Stage Right:");
  display.println(String(stageRightIndexes[pageIndex]));
  refresh_dispay();
}

void render_midi_learn_signal(byte trackIndex) {
  clear_dispay();
  display.println("Send MIDI signal");
  display.println("Track: " + String(trackIndex));
  display.println("Page:  " + String(pageIndex));

  refresh_dispay();
}

void render_left_midi_value_change(byte trackIndex) {
  clear_dispay();
  display.setCursor(0, 15);
  display.setTextSize(3);

  display.println("Left value:");
  display.println(String(leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex]]));
  refresh_dispay();
}

void render_right_midi_value_change(byte trackIndex) {
  clear_dispay();
  display.setCursor(0, 15);
  display.setTextSize(3);

  display.println("Right value:");
  display.println(String(rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex]]));
  refresh_dispay();
}

void setup() {
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);

  for (int i = 0; i < NUMBER_OF_TRACKS; i++) {
    pinMode(TRACK_PINS[i], INPUT_PULLUP);
  }

  for (int i = 0; i < NUMBER_OF_PAGES; i++) {
    pinMode(PAGE_PINS[i], INPUT_PULLUP);
  }

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setFont(&TomThumb);
  clear_dispay();
}


void loop() {
  for (int i = 0; i < NUMBER_OF_PAGES; i++) {
    if (digitalRead(PAGE_PINS[i]) == LOW) {
      if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == HIGH) {
        /*
          only PAGE button pressed:
            change page index
        */
        pageIndex = i;

        render_page_change();
        return;
      } else if (digitalRead(LEFT_PIN) == LOW && digitalRead(RIGHT_PIN) == HIGH) {
        /*
          PAGE button and A button pressed:
            change A stage on current page
        */

        stageLeftIndexes[pageIndex] = i;

        render_left_stage_change();
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == LOW) {
        /*
          PAGE button and B button pressed:
            change B stage on current page
        */

        stageRightIndexes[pageIndex] = i;

        render_right_stage_change();
        return;
      }
    }
  }

  potValue = analogRead(FADER_PIN);

  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (digitalRead(TRACK_PINS[trackIndex]) == LOW ) {
      if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == HIGH) {
        /*
          only TRACK button pressed:
            - send midi event to learn with current value
        */

        control_change(midiChannel, ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
        MidiUSB.flush();
        delay(50);

        render_midi_learn_signal(trackIndex);
        return;
      } else if (digitalRead(LEFT_PIN) == LOW && digitalRead(RIGHT_PIN) == HIGH) {
        /*
          TRACK button and A button pressed:
            listen to POT_PIN and change min midi value
        */

        leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex]] = map(potValue, 0, 1023, 0, 127);

        render_left_midi_value_change(trackIndex);
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == LOW) {
        /*
          TRACK button and A button pressed:
            listen to POT_PIN and change min midi value
        */

        rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex]] = map(potValue, 0, 1023, 0, 127);

        render_right_midi_value_change(trackIndex);
        return;
      }
    }
  }

  faderValue = analogRead(FADER_PIN);

  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex]] < leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex]]) {
      midiValues[pageIndex][trackIndex] = map(faderValue, 1023, 0, rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex]], leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex]]);
    } else {
      midiValues[pageIndex][trackIndex] = map(faderValue, 0, 1023, leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex]], rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex]]);
    }

    if (previousMidiValues[pageIndex][trackIndex] != midiValues[pageIndex][trackIndex]) {
      control_change(midiChannel, ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
      previousMidiValues[pageIndex][trackIndex] = midiValues[pageIndex][trackIndex];
    }
  }

  render_main();

  MidiUSB.flush();
}

