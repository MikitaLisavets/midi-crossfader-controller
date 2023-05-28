#include <Arduino.h>
#include "MIDIUSB.h" // Source: https://github.com/arduino-libraries/MIDIUSB

const byte FADER_PIN = A0;

const byte POT_PIN = A1;

const byte LEFT_PIN = 6;
const byte RIGHT_PIN = 7;

const byte NUMBER_OF_TRACKS = 4;
const byte TRACK_PINS[NUMBER_OF_TRACKS] = {2, 3, 4, 5};

const byte NUMBER_OF_PAGES = 4;
const byte PAGE_PINS[NUMBER_OF_PAGES] = {8, 9, 10, 16};

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

void setup() {
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);

  for (int i = 0; i < NUMBER_OF_TRACKS; i++) {
    pinMode(TRACK_PINS[i], INPUT_PULLUP);
  }

  for (int i = 0; i < NUMBER_OF_PAGES; i++) {
    pinMode(PAGE_PINS[i], INPUT_PULLUP);
  }

  Serial.begin(9600);
}

void clear_dispay() {
  /*
    Clear display
  */
}

void render_main() {
  clear_dispay();
  Serial.println("MIDI value A: " + String(midiValues[pageIndex][0]) + " with CC value: " + String(ccValues[pageIndex][0]));
  Serial.println("MIDI value B: " + String(midiValues[pageIndex][1]) + " with CC value: " + String(ccValues[pageIndex][1]));
  Serial.println("MIDI value C: " + String(midiValues[pageIndex][2]) + " with CC value: " + String(ccValues[pageIndex][2]));
  Serial.println("MIDI value D: " + String(midiValues[pageIndex][3]) + " with CC value: " + String(ccValues[pageIndex][3]));
}

void render_page_change() {
  clear_dispay();
  Serial.println("Page index was changed: " + String(pageIndex));
}

void render_left_stage_change() {
  clear_dispay();
  Serial.println("Stage A for page '" + String(pageIndex) + "' was changed: " + String(stageLeftIndexes[pageIndex]));
}

void render_right_stage_change() {
  clear_dispay();
  Serial.println("Stage B for page '" + String(pageIndex) + "' was changed: " + String(stageRightIndexes[pageIndex]));
}

void render_midi_learn_signal(byte trackIndex) {
  clear_dispay();
  Serial.println("Send MIDI learn for track: " + String(trackIndex) + " on page: " + String(pageIndex));
}

void render_left_midi_value_change(byte trackIndex) {
  clear_dispay();
  Serial.print("Left MIDI value was changed: " + String(leftMidiValues[pageIndex][trackIndex][stageLeftIndexes[pageIndex]]));
  Serial.print(" for track: " + String(trackIndex));
  Serial.print(" on stage: " + String(stageLeftIndexes[pageIndex]));
  Serial.print(" on page: " + String(pageIndex));
  Serial.println();
}

void render_right_midi_value_change(byte trackIndex) {
  clear_dispay();
  Serial.print("Right MIDI value was changed: " + String(rightMidiValues[pageIndex][trackIndex][stageRightIndexes[pageIndex]]));
  Serial.print(" for track: " + String(trackIndex));
  Serial.print(" on stage: " + String(stageRightIndexes[pageIndex]));
  Serial.print(" on page: " + String(pageIndex));
  Serial.println();
}


void refresh_dispay() {
  /*
    Render display
  */
}

void loop() {
  for (int i = 0; i < NUMBER_OF_PAGES; i++) {
    if (digitalRead(PAGE_PINS[i]) == LOW) {
      if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == HIGH && pageIndex != i) {
        /*
          only PAGE button pressed:
            change page index
        */
        pageIndex = i;

        render_page_change();
        return;
      } else if (digitalRead(LEFT_PIN) == LOW && digitalRead(RIGHT_PIN) == HIGH && stageLeftIndexes[pageIndex] != i) {
        /*
          PAGE button and A button pressed:
            change A stage on current page
        */

        stageLeftIndexes[pageIndex] = i;

        render_left_stage_change();
        return;
      } else if (digitalRead(LEFT_PIN) == HIGH && digitalRead(RIGHT_PIN) == LOW && stageRightIndexes[pageIndex] != i) {
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

  refresh_dispay();
}

