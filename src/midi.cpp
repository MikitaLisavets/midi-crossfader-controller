#include <MIDIUSB.h> // Source: https://github.com/arduino-libraries/MIDIUSB
#include <midi.h>

void control_change(uint8_t channel, uint8_t control, uint8_t value) {
  midiEventPacket_t event = {0x0B, static_cast<uint8_t>(0xB0 | channel), control, value};
  MidiUSB.sendMIDI(event);
}

void send_midi() {
  MidiUSB.flush();
};