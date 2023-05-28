#include <Arduino.h>
#include "MIDIUSB.h" // Source: https://github.com/arduino-libraries/MIDIUSB

const byte FADER_PIN = A0;

const byte POT_PIN = A1;

const byte A_PIN = 9;
const byte B_PIN = 8;

const byte NUMBER_OF_TRACKS = 4;
const byte TRACK_PINS[NUMBER_OF_TRACKS] = {4, 5, 6, 7};

const byte NUMBER_OF_PAGES = 4;
const byte PAGE_PINS[NUMBER_OF_PAGES] = {10, 14, 15, 16};

byte midiChannel = 0;
byte pageIndex = 0;

byte faderValue = 0;
byte potValue = 0;

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

byte minMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

byte maxMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {127, 127, 127, 127},
  {127, 127, 127, 127},
  {127, 127, 127, 127},
  {127, 127, 127, 127}
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

  // Serial.begin(9600);
}

void loop() {
  for (int pIndex = 0; pIndex < NUMBER_OF_PAGES; pIndex++) {
    if (digitalRead(PAGE_PINS[pIndex]) == LOW ) {
      /*
        PAGE button pressed:
          change page index
      */

      pageIndex = pIndex;
      return;
    }
  }

  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (digitalRead(TRACK_PINS[trackIndex]) == LOW ) {
      if (digitalRead(A_PIN) == HIGH && digitalRead(B_PIN) == HIGH) {
        /*
          only TRACK button pressed:
            - send midi event to learn with current value
        */

        control_change(midiChannel, ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
        MidiUSB.flush();
        return;
      } else if (digitalRead(A_PIN) == LOW && digitalRead(B_PIN) == HIGH) {
        /*
          TRACK button and A button pressed:
            listen to POT_PIN and change min midi value
        */
        potValue = analogRead(FADER_PIN);
        minMidiValues[pageIndex][trackIndex] = map(potValue, 0, 1023, 0, 127);
        return;
      } else if (digitalRead(A_PIN) == HIGH && digitalRead(B_PIN) == LOW) {
        /*
          TRACK button and A button pressed:
            listen to POT_PIN and change min midi value
        */
        potValue = analogRead(FADER_PIN);
        maxMidiValues[pageIndex][trackIndex] = map(potValue, 0, 1023, 0, 127);
        return;
      }
    }
  }

  faderValue = analogRead(FADER_PIN);

  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
    if (maxMidiValues[pageIndex][trackIndex] < minMidiValues[pageIndex][trackIndex]) {
      midiValues[pageIndex][trackIndex] = map(faderValue, 1023, 0, maxMidiValues[pageIndex][trackIndex], minMidiValues[pageIndex][trackIndex]);
    } else {
      midiValues[pageIndex][trackIndex] = map(faderValue, 0, 1023, minMidiValues[pageIndex][trackIndex], maxMidiValues[pageIndex][trackIndex]);
    }

    if (previousMidiValues[pageIndex][trackIndex] != midiValues[pageIndex][trackIndex]) {
      control_change(midiChannel, ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
      previousMidiValues[pageIndex][trackIndex] = midiValues[pageIndex][trackIndex];
    }
  }
  MidiUSB.flush();
}

