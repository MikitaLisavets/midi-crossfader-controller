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

const byte FADER_PIN = A1;

const byte POT_CLK = A2;
const byte POT_DT = A3;
const byte POT_SW = 1;

const byte LEFT_PIN = 8;
const byte RIGHT_PIN = 9;

const byte NUMBER_OF_TRACKS = 4;
const byte TRACK_PINS[NUMBER_OF_TRACKS] = {4, 5, 6, 7};

const byte NUMBER_OF_PAGES = 4;
const byte PAGE_PINS[NUMBER_OF_PAGES] = {10, 16, A0, 15};

const byte NUMBER_OF_STAGES = 4;

int currentStateCLK;
int lastStateCLK;

int potStep = 1;
int potValue;

byte midiChannel = 0;
byte pageIndex = 0;

byte stageLeftIndexes[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

byte stageRightIndexes[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

int faderValue = 0;

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

String pageTitles[] = { "I", "II", "III", "IV" };
String stageTitles[] = { "1", "2", "3", "4" };
String trackTitles[] = { "A", "B", "C", "D" };

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
  display.println("=== Page: "+ pageTitles[pageIndex] + " ===");

  for (int i = 0; i < NUMBER_OF_TRACKS; i++) {
    display.print(trackTitles[i] + " | ");
    display.print("(" + stageTitles[stageLeftIndexes[pageIndex][i]] + ")");
    display.setCursor(27 - String(leftMidiValues[pageIndex][i][stageLeftIndexes[pageIndex][i]]).length(), display.getCursorY());
    display.print(String(leftMidiValues[pageIndex][i][stageLeftIndexes[pageIndex][i]]));
    display.setCursor(40, display.getCursorY());
    display.print(F("<"));
    display.setCursor(55 - String(midiValues[pageIndex][i]).length(), display.getCursorY());
    display.print(String(midiValues[pageIndex][i]));
    display.setCursor(68, display.getCursorY());
    display.print(F(">"));
    display.setCursor(78 - String(rightMidiValues[pageIndex][i][stageRightIndexes[pageIndex][i]]).length(), display.getCursorY());
    display.print(String(rightMidiValues[pageIndex][i][stageRightIndexes[pageIndex][i]]));
    display.setCursor(92, display.getCursorY());
    display.print("(" + stageTitles[stageRightIndexes[pageIndex][i]] + ")");
    display.println();
  }

  refresh_dispay();
}

void render_page_change() {
  clear_dispay();
  display.setCursor(20, 15);
  display.setTextSize(3);
  display.println("Page: " + pageTitles[pageIndex]);
  refresh_dispay();
}

void render_left_stage_change(int trackIndex) {
  clear_dispay();
  display.setCursor(0, 10);
  display.setTextSize(2);
  if (trackIndex >= 0) {
    display.println("Stage Left: " + String(stageTitles[stageLeftIndexes[pageIndex][trackIndex]]));
    display.println("Track: " + trackTitles[trackIndex]);
  } else {
    display.println("Stage Left: " + String(stageTitles[stageLeftIndexes[pageIndex][0]]));
    display.println("All tracks");
  }

  refresh_dispay();
}

void render_right_stage_change(int trackIndex) {
  clear_dispay();
  display.setCursor(0, 10);
  display.setTextSize(2);
  if (trackIndex >= 0) {
    display.println("Stage Right: " + String(stageTitles[stageRightIndexes[pageIndex][trackIndex]]));
    display.println("Track: " + trackTitles[trackIndex]);
  } else {
    display.println("Stage Right: " + String(stageTitles[stageRightIndexes[pageIndex][0]]));
    display.println("All tracks");
  }

  refresh_dispay();
}

void render_midi_learn_signal(byte trackIndex) {
  clear_dispay();
  display.setCursor(0, 10);
  display.setTextSize(2);
  display.println("Sending MIDI ...");
  display.println("Track: " + trackTitles[trackIndex] + " | " + "Page:  " + pageTitles[pageIndex]);

  refresh_dispay();
}

void render_left_midi_value_change(byte trackIndex) {
  clear_dispay();
  display.setCursor(0, 15);
  display.setTextSize(3);

  display.println("Left value:");
  display.setCursor(50, display.getCursorY());
  display.println(String(leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]]));
  refresh_dispay();
}

void render_right_midi_value_change(byte trackIndex) {
  clear_dispay();
  display.setCursor(0, 15);
  display.setTextSize(3);

  display.println("Right value:");
  display.setCursor(50, display.getCursorY());
  display.println(String(rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]]));
  refresh_dispay();
}

void setup() {
  pinMode(POT_CLK,INPUT);
  pinMode(POT_DT,INPUT);

  pinMode(POT_SW, INPUT_PULLUP);

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

  lastStateCLK = digitalRead(POT_CLK);
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
        for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
          if (digitalRead(TRACK_PINS[trackIndex]) == LOW ) {
            /*
              PAGE button and A button pressed:
                change A stage for pressed track on current page
            */
              stageLeftIndexes[pageIndex][trackIndex] = i;
              render_left_stage_change(trackIndex);
              return;
          }
        }
        /*
          PAGE button and A button pressed:
            change A stage for all tracks on current page
        */
        for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
          stageLeftIndexes[pageIndex][trackIndex] = i;
        }
        render_left_stage_change(-1);
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == LOW) {
        for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
          if (digitalRead(TRACK_PINS[trackIndex]) == LOW ) {
            /*
              PAGE button and B button pressed:
                change B stage for pressed track on current page
            */
              stageRightIndexes[pageIndex][trackIndex] = i;
              render_right_stage_change(trackIndex);
              return;

          }
          /*
            PAGE button and B button pressed:
              change B stage for all tracks on current page
          */

          for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
            stageRightIndexes[pageIndex][trackIndex] = i;
          }
          render_right_stage_change(-1);
          return;
        }
      }
    }
  }

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
      } else if (digitalRead(LEFT_PIN) == LOW && digitalRead(RIGHT_PIN) == HIGH && digitalRead(POT_SW) == HIGH) {
        /*
          TRACK button and A button pressed:
            listen to POT_PIN and change min midi value
        */

        currentStateCLK = digitalRead(POT_CLK);

        if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
          if (digitalRead(POT_DT) != currentStateCLK) {
            potValue = leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]] - potStep;
            if (potValue < 0) {
              potValue = 0;
            }

          } else {
            potValue = leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]] + potStep;
            if (potValue > 127) {
              potValue = 127;
            }
          }

          leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]] = potValue;
        }

        render_left_midi_value_change(trackIndex);

        lastStateCLK = currentStateCLK;
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == LOW && digitalRead(POT_SW) == HIGH) {
        /*
          TRACK button and A button pressed:
            listen to POT_PIN and change min midi value
        */
        currentStateCLK = digitalRead(POT_CLK);

        if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
          if (digitalRead(POT_DT) != currentStateCLK) {
            potValue = rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]] - potStep;
            if (potValue < 0) {
              potValue = 0;
            }

          } else {
            potValue = rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]] + potStep;
            if (potValue > 127) {
              potValue = 127;
            }
          }

          rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]] = potValue;
        }

        render_right_midi_value_change(trackIndex);
        lastStateCLK = currentStateCLK;
        return;
      }  else if ((digitalRead(LEFT_PIN) == LOW || digitalRead(RIGHT_PIN) == LOW) && digitalRead(POT_SW) == LOW) {
        /*
          TRACK button and A or B button pressed and Pot SW pressed:
            swap min and max midi values
        */
        byte temp = leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]];
        leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]] = rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]];
        rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]] = temp;
        delay(150);
      }
    }
  }

  faderValue = analogRead(FADER_PIN);

  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]] < leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]]) {
      midiValues[pageIndex][trackIndex] = map(faderValue, 1023, 0, rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]], leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]]);
    } else {
      midiValues[pageIndex][trackIndex] = map(faderValue, 0, 1023, leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]], rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]]);
    }

    if (previousMidiValues[pageIndex][trackIndex] != midiValues[pageIndex][trackIndex]) {
      control_change(midiChannel, ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
      previousMidiValues[pageIndex][trackIndex] = midiValues[pageIndex][trackIndex];
      MidiUSB.flush();
    }
  }

  render_main();
}

