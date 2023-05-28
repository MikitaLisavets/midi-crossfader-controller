#include <Arduino.h>
#include "MIDIUSB.h" // Source: https://github.com/arduino-libraries/MIDIUSB

const byte FADER_PIN = A0;

const byte POT_PIN = A1;

const byte A_PIN = 6;
const byte B_PIN = 7;

const byte NUMBER_OF_TRACKS = 4;
const byte TRACK_PINS[NUMBER_OF_TRACKS] = {2, 3, 4, 5};

const byte NUMBER_OF_PAGES = 4;
const byte PAGE_PINS[NUMBER_OF_PAGES] = {8, 9, 10, 16};

const byte NUMBER_OF_STAGES = 4;

byte midiChannel = 0;
byte pageIndex = 0;

byte stageAIndexes[NUMBER_OF_STAGES] = {0, 0, 0, 0};
byte stageBIndexes[NUMBER_OF_STAGES] = {0, 0, 0, 0};

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

byte minMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS][NUMBER_OF_STAGES] = {
  {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
};

byte maxMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS][NUMBER_OF_STAGES] = {
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

byte previousMidiValue = 0;



void control_change(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void setup() {
  pinMode(A_PIN, INPUT_PULLUP);
  pinMode(B_PIN, INPUT_PULLUP);

  for (int i = 0; i < NUMBER_OF_TRACKS; i++) {
    pinMode(TRACK_PINS[i], INPUT_PULLUP);
  }

  for (int i = 0; i < NUMBER_OF_PAGES; i++) {
    pinMode(PAGE_PINS[i], INPUT_PULLUP);
  }

  Serial.begin(9600);
}

void loop() {
  for (int i = 0; i < NUMBER_OF_PAGES; i++) {

    if (digitalRead(PAGE_PINS[i]) == LOW) {
      if (digitalRead(A_PIN) == HIGH && digitalRead(B_PIN) == HIGH && pageIndex != i) {
        /*
          only PAGE button pressed:
            change page index
        */

        Serial.println("Page index was changed from: " + String(pageIndex) + " to: " + String(i));

        pageIndex = i;
        return;
      } else if (digitalRead(A_PIN) == LOW && digitalRead(B_PIN) == HIGH && stageAIndexes[pageIndex] != i) {
        /*
          PAGE button and A button pressed:
            change A stage on current page
        */

        Serial.println("Stage A for page: " + String(pageIndex) + " was changed from: " + String(stageAIndexes[pageIndex]) + " to: " + String(i));

        stageAIndexes[pageIndex] = i;
        return;
      } else if (digitalRead(A_PIN) == HIGH && digitalRead(B_PIN) == LOW && stageBIndexes[pageIndex] != i) {
        /*
          PAGE button and B button pressed:
            change B stage on current page
        */

        Serial.println("Stage B for page: " + String(pageIndex) + " was changed from: " + String(stageBIndexes[pageIndex]) + " to: " + String(i));

        stageBIndexes[pageIndex] = i;
        return;
      }
    }
  }

  potValue = analogRead(FADER_PIN);

  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (digitalRead(TRACK_PINS[trackIndex]) == LOW ) {
      if (digitalRead(A_PIN) == HIGH && digitalRead(B_PIN) == HIGH) {
        /*
          only TRACK button pressed:
            - send midi event to learn with current value
        */


        Serial.println("Send MIDI learn for track: " + String(trackIndex) + " on page: " + String(pageIndex));
        control_change(midiChannel, ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
        MidiUSB.flush();
        delay(50);
        return;
      } else if (digitalRead(A_PIN) == LOW && digitalRead(B_PIN) == HIGH) {
        /*
          TRACK button and A button pressed:
            listen to POT_PIN and change min midi value
        */

        Serial.print("Min MIDI value was changed from: " + String(minMidiValues[pageIndex][trackIndex][stageAIndexes[pageIndex]]));
        Serial.print(" to: " + String(map(potValue, 0, 1023, 0, 127)));
        Serial.print(" for track: " + String(trackIndex));
        Serial.print(" on stange: " + String(stageAIndexes[pageIndex]));
        Serial.print(" on page: " + String(pageIndex));
        Serial.println();

        minMidiValues[pageIndex][trackIndex][stageAIndexes[pageIndex]] = map(potValue, 0, 1023, 0, 127);
        return;
      } else if (digitalRead(A_PIN) == HIGH && digitalRead(B_PIN) == LOW) {
        /*
          TRACK button and A button pressed:
            listen to POT_PIN and change min midi value
        */

        Serial.print("Max MIDI value was changed from: " + String(maxMidiValues[pageIndex][trackIndex][stageBIndexes[pageIndex]]));
        Serial.print(" to: " + String(map(potValue, 0, 1023, 0, 127)));
        Serial.print(" for track: " + String(trackIndex));
        Serial.print(" on stange: " + String(stageBIndexes[pageIndex]));
        Serial.print(" on page: " + String(pageIndex));
        Serial.println();

        maxMidiValues[pageIndex][trackIndex][stageBIndexes[pageIndex]] = map(potValue, 0, 1023, 0, 127);
        return;
      }
    }
  }

  faderValue = analogRead(FADER_PIN);

  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (maxMidiValues[pageIndex][trackIndex][stageBIndexes[pageIndex]] < minMidiValues[pageIndex][trackIndex][stageAIndexes[pageIndex]]) {
      midiValues[pageIndex][trackIndex] = map(faderValue, 1023, 0, maxMidiValues[pageIndex][trackIndex][stageBIndexes[pageIndex]], minMidiValues[pageIndex][trackIndex][stageAIndexes[pageIndex]]);
    } else {
      midiValues[pageIndex][trackIndex] = map(faderValue, 0, 1023, minMidiValues[pageIndex][trackIndex][stageAIndexes[pageIndex]], maxMidiValues[pageIndex][trackIndex][stageBIndexes[pageIndex]]);
    }

    if (previousMidiValues[pageIndex][trackIndex] != midiValues[pageIndex][trackIndex]) {
      Serial.println("MIDI value was sent: " + String(midiValues[pageIndex][trackIndex]) + " with CC value: " + String(ccValues[pageIndex][trackIndex]));

      control_change(midiChannel, ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
      previousMidiValues[pageIndex][trackIndex] = midiValues[pageIndex][trackIndex];
    }
  }

  MidiUSB.flush();
}

