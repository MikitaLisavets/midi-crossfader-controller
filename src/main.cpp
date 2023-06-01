#include <Arduino.h>
#include <SPI.h>
#include <MIDIUSB.h> // Source: https://github.com/arduino-libraries/MIDIUSB
#include <Adafruit_GFX.h> // Source: https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h> // Source: https://github.com/adafruit/Adafruit_SSD1306

const uint8_t SCREEN_WIDTH = 128;
const uint8_t SCREEN_HEIGHT = 32;
const uint8_t SCREEN_ADDRESS = 0x3C;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const byte FADER_PIN = A0;

const byte POT_CLK = A1;
const byte POT_DT = A2;
const byte POT_SW = A3;

const byte LEFT_PIN = 10;
const byte RIGHT_PIN = 16;

const byte NUMBER_OF_TRACKS = 4;
const byte TRACK_PINS[NUMBER_OF_TRACKS] = {1, 0, 4, 5};

const byte NUMBER_OF_PAGES = 4;
const byte PAGE_PINS[NUMBER_OF_PAGES] = {6, 7, 8, 9};

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
byte faderThreshold = 10;

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

String pageTitles[] = { "1", "2", "3", "4" };
String stageTitles[] = { "1", "2", "3", "4" };
String trackTitles[] = { "A", "B", "C", "D" };

String fill_with(byte number) {
  switch(String(number).length()) {
    case 1:
      return String(number) + "..";
    case 2:
      return String(number) + ".";
    default:
      return String(number);
  }
}

// === MIDI ===

void control_change(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, static_cast<uint8_t>(0xB0 | channel), control, value};
  MidiUSB.sendMIDI(event);
}

// ============

// === Display ===

void clear_dispay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
}

void refresh_dispay() {
  display.display();
}

void render_init() {
  clear_dispay();

  display.setTextSize(2);
  display.setCursor(25, 10);
  display.print("X");
  refresh_dispay();
  delay(60);
  display.print("-");
  refresh_dispay();
  delay(60);
  display.print("F");
  refresh_dispay();
  delay(60);
  display.print("a");
  refresh_dispay();
  delay(60);
  display.print("d");
  refresh_dispay();
  delay(60);
  display.print("e");
  refresh_dispay();
  delay(60);
  display.print("r");
  refresh_dispay();
  delay(300);
}

void render_main() {
  clear_dispay();

  for (int i = 0; i < NUMBER_OF_TRACKS; i++) {
    display.print(trackTitles[i] + pageTitles[pageIndex] + + "|" + stageTitles[stageLeftIndexes[pageIndex][i]] + "| " + fill_with(leftMidiValues[pageIndex][i][stageLeftIndexes[pageIndex][i]]) + "<" + fill_with(midiValues[pageIndex][i]) + ">" + fill_with(rightMidiValues[pageIndex][i][stageRightIndexes[pageIndex][i]]) + " |" + stageTitles[stageRightIndexes[pageIndex][i]] + "|");
    display.println();
  }

  refresh_dispay();
}

void render_page_press() {
  clear_dispay();
  display.setTextSize(3);
  display.println("Page:" + pageTitles[pageIndex]);
  refresh_dispay();
}

void render_left_stage_change(int trackIndex) {
  clear_dispay();
  display.setTextSize(2);
  if (trackIndex >= 0) {
    display.println("Stage L: " + String(stageTitles[stageLeftIndexes[pageIndex][trackIndex]]));
    display.println("Track: " + trackTitles[trackIndex]);
  } else {
    display.println("Stage L: " + String(stageTitles[stageLeftIndexes[pageIndex][0]]));
    display.println("All tracks");
  }

  refresh_dispay();
}

void render_right_stage_change(int trackIndex) {
  clear_dispay();
  display.setTextSize(2);
  if (trackIndex >= 0) {
    display.println("Stage R: " + String(stageTitles[stageRightIndexes[pageIndex][trackIndex]]));
    display.println("Track: " + trackTitles[trackIndex]);
  } else {
    display.println("Stage R: " + String(stageTitles[stageRightIndexes[pageIndex][0]]));
    display.println("All tracks");
  }

  refresh_dispay();
}

void render_track_press(byte trackIndex) {
  clear_dispay();
  display.setTextSize(1);
  display.println("Sending MIDI ...");
  display.println("Page:  " + pageTitles[pageIndex]);
  display.println("Track: " + trackTitles[trackIndex]);

  refresh_dispay();
}

void render_left_midi_value_change(byte trackIndex) {
  clear_dispay();
  display.setTextSize(2);

  display.println("Value L:");
  display.println(String(leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]]));
  refresh_dispay();
}

void render_right_midi_value_change(byte trackIndex) {
  clear_dispay();
  display.setTextSize(2);

  display.println("Value R:");
  display.println(String(rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]]));
  refresh_dispay();
}

void render_midi_values_swap(byte trackIndex) {
  clear_dispay();
  display.setTextSize(1);
  display.println("Swap L and R values:");
  display.print("Value L: ");
  display.println(String(leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]]));
  display.print("Value R: ");
  display.println(String(rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]]));
  display.print("Track: ");
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
        stageLeftIndexes[pageIndex][trackIndex] = newStageIndex;
        render_left_stage_change(trackIndex);
        return;
    }
  }
  /*
    PAGE button and LEFT button pressed:
      - change left stage for all tracks on current page
  */
  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    stageLeftIndexes[pageIndex][trackIndex] = newStageIndex;
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
        stageRightIndexes[pageIndex][trackIndex] = newStageIndex;
        render_right_stage_change(trackIndex);
        return;
    }
  }
  /*
    PAGE button and RIGHT button pressed:
      - change right stage for all tracks on current page
  */
  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    stageRightIndexes[pageIndex][trackIndex] = newStageIndex;
  }
  render_right_stage_change(-1);
}

void handle_track_press(byte trackIndex) {
  /*
    TRACK button pressed:
      - send midi event to learn with current value
  */

  control_change(midiChannel, ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
  MidiUSB.flush();
  render_track_press(trackIndex);

  delay(100);
}

void handle_left_midi_value_change(byte trackIndex) {
  /*
    TRACK button and LEFT button pressed:
      listen to POT and change left midi value
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
}

void handle_right_midi_value_change(byte trackIndex) {
  /*
    TRACK button and RIGHT button pressed:
      listen to POT and change right midi value
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
}

void handle_midi_values_swap(byte trackIndex) {
  /*
    TRACK button and LEFT or RIGHT button pressed and POT button are pressed:
      - swap left and right midi values
  */
  byte temp = leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]];
  leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]] = rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]];
  rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]] = temp;
  render_midi_values_swap(trackIndex);

  delay(300);
}

// ================

void setup() {
  // Turn off system leds
  pinMode(LED_BUILTIN_TX, INPUT);
  pinMode(LED_BUILTIN_RX, INPUT);

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

  lastStateCLK = digitalRead(POT_CLK);

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  render_init();
  clear_dispay();
}


void loop() {
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
    if (digitalRead(TRACK_PINS[tIndex]) == LOW ) {
      if (digitalRead(LEFT_PIN) == LOW && digitalRead(RIGHT_PIN) == HIGH && digitalRead(POT_SW) == HIGH) {
        handle_left_midi_value_change(tIndex);
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == LOW && digitalRead(POT_SW) == HIGH) {
        handle_right_midi_value_change(tIndex);
        return;
      }  else if ((digitalRead(LEFT_PIN) == LOW || digitalRead(RIGHT_PIN) == LOW) && digitalRead(POT_SW) == LOW) {
        handle_midi_values_swap(tIndex);
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == HIGH) {
        handle_track_press(tIndex);
        return;
      } 
    }
  }

  faderValue = analogRead(FADER_PIN);
  if (faderValue > 1023 - faderThreshold) {
    faderValue = 1023 - faderThreshold;
  } else if (faderValue < 0 + faderThreshold) {
    faderValue = 0 + faderThreshold;
  }

  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]] < leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]]) {
      midiValues[pageIndex][trackIndex] = map(faderValue, 1023 - faderThreshold, 0 + faderThreshold, rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]], leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]]);
    } else {
      midiValues[pageIndex][trackIndex] = map(faderValue, 0 + faderThreshold, 1023 - faderThreshold, leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex][trackIndex]], rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex][trackIndex]]);
    }

    if (previousMidiValues[pageIndex][trackIndex] != midiValues[pageIndex][trackIndex]) {
      control_change(midiChannel, ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
      previousMidiValues[pageIndex][trackIndex] = midiValues[pageIndex][trackIndex];
      MidiUSB.flush();
    }
  }

  render_main();
}

