#include <Arduino.h>
#include "MIDIUSB.h" // Source: https://github.com/arduino-libraries/MIDIUSB

int potPin = A0;

int APin = 10;
int BPin = 9;

int CC1Pin = 7;
int CC2Pin = 8;

int numberOfCC = 2;

int channel = 0;

int index = 0;

int cc[] = {0, 1};
int potVal = 0;
int prevVal[] = {0, 0};
int midiVal[] = {0, 0};

int minVal[] = {0, 0};
int maxVal[] = {127, 127};


void control_change(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void setup() {
  pinMode(APin, INPUT_PULLUP);
  pinMode(BPin, INPUT_PULLUP);
  pinMode(CC1Pin, INPUT_PULLUP);
  pinMode(CC2Pin, INPUT_PULLUP);
  Serial.begin(9600);
}

void sub_func(int potVal, int ind) {
  index = ind;

  if (digitalRead(APin) == LOW) {
    minVal[index] = map(potVal, 0, 1023, 0, 127);
  }
  if (digitalRead(BPin) == LOW) {
    maxVal[index] = map(potVal, 0, 1023, 0, 127);
  }

  control_change(channel, cc[index], prevVal[index]);
  MidiUSB.flush();
}

void loop() {
  potVal = analogRead(potPin);

  if (digitalRead(CC1Pin) == LOW) {
    sub_func(potVal, 0);
  } else if (digitalRead(CC2Pin) == LOW) {
    sub_func(potVal, 1);
  } else {
    for (int i = 0; i < numberOfCC; i++) {
      if (maxVal[i] < minVal[i]) {
        midiVal[i] = map(potVal, 1023, 0, maxVal[i], minVal[i]);
      } else {
        midiVal[i] = map(potVal, 0, 1023, minVal[i], maxVal[i]);
      }

      if (prevVal[i] != midiVal[i]) {
        control_change(channel, cc[i], midiVal[i]);
        prevVal[i] = midiVal[i];
      }
      MidiUSB.flush();
    }
  }
}

