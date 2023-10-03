#define OLED_SPI_SPEED 4000000ul
#include <GyverOLED.h> // Source: https://github.com/GyverLibs/GyverOLED
#include <display.h>

GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> display;
#define SCREEN_MENU_ROWS 7

const char PROGMEM trackTitles[ALL_TRACKS] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P'};

void clear_display() {
  display.clear();
}

void reset_display() {
  display.home();
  display.setScale(1);
  display.invertText(false);
}

void render_filled_number(uint8_t num) {
  display.print(num);

  if (num < 10) {
    display.print(F(".."));
  } else if (num >= 10 && num < 100) {
    display.print(F("."));
  }
}

void render_init_screen() {
  clear_display();

  display.setScale(1);
  display.setCursor(0, 7);
  display.print(F("Version: "));
  display.print(VERSION);

  display.setScale(2);
  display.setCursor(25, 1);
  display.print(F("X"));
  delay(100);
  display.print(F("-"));
  delay(100);
  display.print(F("F"));
  delay(100);
  display.print(F("a"));
  delay(100);
  display.print(F("d"));
  delay(100);
  display.print(F("e"));
  delay(100);
  display.print(F("r"));
  delay(300);
}

void render_main(
  StateEvent &stateEvent
) {
  reset_display();

  for (int8_t i = 0; i < NUMBER_OF_TRACKS; i++) {
    int8_t indexWithOffset = (trackOffset + i) % ALL_TRACKS;

    if (stateEvent.trackIndex == indexWithOffset && !stateEvent.stageChanged && !stateEvent.midiValuesChanged && !stateEvent.midiValuesSwap) {
      display.invertText(true);
    }
    display.print((char)pgm_read_byte(&trackTitles[indexWithOffset]));
    if (stateEvent.pageChanged) {
      display.invertText(true);
    }
    display.print(pageIndex + 1);
    display.invertText(false);
    display.print(F(": "));
    if ((stateEvent.trackIndex == indexWithOffset || stateEvent.trackIndex < 0) && stateEvent.stageChanged && stateEvent.side == SIDE_LEFT) {
      display.invertText(true);
    }
    display.print(settings.stageIndexes[SIDE_LEFT][pageIndex][indexWithOffset] + 1);
    display.invertText(false);
    display.print(F("| "));
    if (stateEvent.trackIndex == indexWithOffset && ((stateEvent.midiValuesChanged && stateEvent.side == SIDE_LEFT) || stateEvent.midiValuesSwap)) {
      display.invertText(true);
    }
    render_filled_number(settings.midiValues[SIDE_LEFT][pageIndex][indexWithOffset][settings.stageIndexes[SIDE_LEFT][pageIndex][indexWithOffset]]);
    display.invertText(false);
    display.print(F("<"));
    if (stateEvent.trackIndex == indexWithOffset) {
      render_filled_number(midiValues[pageIndex][indexWithOffset]);
    } else {
      display.print(F("-|-"));
    }
    display.print(F(">"));
    if (stateEvent.trackIndex == indexWithOffset && ((stateEvent.midiValuesChanged && stateEvent.side == SIDE_RIGHT) ||stateEvent.midiValuesSwap)) {
      display.invertText(true);
    }
    render_filled_number(settings.midiValues[SIDE_RIGHT][pageIndex][indexWithOffset][settings.stageIndexes[SIDE_RIGHT][pageIndex][indexWithOffset]]);
    display.invertText(false);
    display.print(F(" |"));
    if ((stateEvent.trackIndex == indexWithOffset || stateEvent.trackIndex < 0) && stateEvent.stageChanged && stateEvent.side == SIDE_RIGHT) {
      display.invertText(true);
    }
    display.print(settings.stageIndexes[SIDE_RIGHT][pageIndex][indexWithOffset] + 1);
    display.invertText(false);
    display.println(" ");
    if (i < NUMBER_OF_TRACKS - 1) {
      display.println(F("----------------------"));
    } else {
      display.print(F("_________"));
      for (int8_t j = 0; j < ALL_TRACKS / NUMBER_OF_TRACKS; j++) {
        if (j ==  trackOffset / NUMBER_OF_TRACKS) {
          display.print(F("*"));
        } else {
          display.print(F("."));
        }
      }
      display.println(F("_________"));
    }
  }

}

// === Menu ===

void render_row_load() {
  display.print(F("Load Settings"));
}

void render_row_save() {
  display.print(F("Save Settings"));
}

void render_row_reset() {
  display.print(F("Reset Settings"));
}

void render_row_midi_channel(bool hasActiveSubMenu) {
  display.print(F("MIDI Channel: "));
  if (hasActiveSubMenu) {
    display.invertText(true);
  }
  display.print(settings.midiChannel);

}

void render_row_fader_threshold(bool hasActiveSubMenu) {
  display.print(F("Fader Threshold: "));
  if (hasActiveSubMenu) {
    display.invertText(true);
  }
  display.print(settings.faderThreshold);
}

void render_row_scroll_fast_speed(bool hasActiveSubMenu) {
  display.print(F("Scroll fast speed: "));
  if (hasActiveSubMenu) {
    display.invertText(true);
  }
  display.print(settings.scrollFastSpeed);
}

void render_row_auto_load_settings(bool hasActiveSubMenu) {
  display.print(F("Auto-Load: "));
  if (hasActiveSubMenu) {
    display.invertText(true);
  }
  if (settings.autoLoadSettings) {
    display.print(F("Yes"));
  } else {
    display.print(F("No"));
  }
}

void render_row_track_cc(uint8_t pageIndex, uint8_t trackIndex, bool hasActiveSubMenu) {
  display.print(F("Control Change "));
  display.print((char)pgm_read_byte(&trackTitles[trackIndex]));
  display.print(pageIndex + 1);
  display.print(F(": "));
  if (hasActiveSubMenu) {
    display.invertText(true);
  }
  display.print(settings.ccValues[pageIndex][trackIndex]);
}

void render_row(int8_t rowIndex) {
  bool isSelected = rowIndex == menuSelectedRow;
  bool hasActiveSubMenu = isSelected && isSubMenuActive;
  if (isSelected && !hasActiveSubMenu) {
    display.invertText(true);
  } else {
    display.invertText(false);
  }

  switch(rowIndex) {
    case MENU_LOAD: return render_row_load();
    case MENU_SAVE: return render_row_save();
    case MENU_RESET: return render_row_reset();
    case MENU_MIDI_CHANNEL: return render_row_midi_channel(hasActiveSubMenu);
    case MENU_FADER_THRESHOLD: return render_row_fader_threshold(hasActiveSubMenu);
    case MENU_AUTO_LOAD_SETTINGS: return render_row_auto_load_settings(hasActiveSubMenu);
    case MENU_SCROLL_FAST_SPEED: return render_row_scroll_fast_speed(hasActiveSubMenu);
    case MENU_A1_CC: return render_row_track_cc(0, 0, hasActiveSubMenu);
    case MENU_B1_CC: return render_row_track_cc(0, 1, hasActiveSubMenu);
    case MENU_C1_CC: return render_row_track_cc(0, 2, hasActiveSubMenu);
    case MENU_D1_CC: return render_row_track_cc(0, 3, hasActiveSubMenu);
    case MENU_E1_CC: return render_row_track_cc(0, 4, hasActiveSubMenu);
    case MENU_F1_CC: return render_row_track_cc(0, 5, hasActiveSubMenu);
    case MENU_G1_CC: return render_row_track_cc(0, 6, hasActiveSubMenu);
    case MENU_H1_CC: return render_row_track_cc(0, 7, hasActiveSubMenu);
    case MENU_I1_CC: return render_row_track_cc(0, 8, hasActiveSubMenu);
    case MENU_J1_CC: return render_row_track_cc(0, 9, hasActiveSubMenu);
    case MENU_K1_CC: return render_row_track_cc(0, 10, hasActiveSubMenu);
    case MENU_L1_CC: return render_row_track_cc(0, 11, hasActiveSubMenu);
    case MENU_M1_CC: return render_row_track_cc(0, 12, hasActiveSubMenu);
    case MENU_N1_CC: return render_row_track_cc(0, 13, hasActiveSubMenu);
    case MENU_O1_CC: return render_row_track_cc(0, 14, hasActiveSubMenu);
    case MENU_P1_CC: return render_row_track_cc(0, 15, hasActiveSubMenu);
    case MENU_A2_CC: return render_row_track_cc(1, 0, hasActiveSubMenu);
    case MENU_B2_CC: return render_row_track_cc(1, 1, hasActiveSubMenu);
    case MENU_C2_CC: return render_row_track_cc(1, 2, hasActiveSubMenu);
    case MENU_D2_CC: return render_row_track_cc(1, 3, hasActiveSubMenu);
    case MENU_E2_CC: return render_row_track_cc(1, 4, hasActiveSubMenu);
    case MENU_F2_CC: return render_row_track_cc(1, 5, hasActiveSubMenu);
    case MENU_G2_CC: return render_row_track_cc(1, 6, hasActiveSubMenu);
    case MENU_H2_CC: return render_row_track_cc(1, 7, hasActiveSubMenu);
    case MENU_I2_CC: return render_row_track_cc(1, 8, hasActiveSubMenu);
    case MENU_J2_CC: return render_row_track_cc(1, 9, hasActiveSubMenu);
    case MENU_K2_CC: return render_row_track_cc(1, 10, hasActiveSubMenu);
    case MENU_L2_CC: return render_row_track_cc(1, 11, hasActiveSubMenu);
    case MENU_M2_CC: return render_row_track_cc(1, 12, hasActiveSubMenu);
    case MENU_N2_CC: return render_row_track_cc(1, 13, hasActiveSubMenu);
    case MENU_O2_CC: return render_row_track_cc(1, 14, hasActiveSubMenu);
    case MENU_P2_CC: return render_row_track_cc(1, 15, hasActiveSubMenu);
    case MENU_A3_CC: return render_row_track_cc(2, 0, hasActiveSubMenu);
    case MENU_B3_CC: return render_row_track_cc(2, 1, hasActiveSubMenu);
    case MENU_C3_CC: return render_row_track_cc(2, 2, hasActiveSubMenu);
    case MENU_D3_CC: return render_row_track_cc(2, 3, hasActiveSubMenu);
    case MENU_E3_CC: return render_row_track_cc(2, 4, hasActiveSubMenu);
    case MENU_F3_CC: return render_row_track_cc(2, 5, hasActiveSubMenu);
    case MENU_G3_CC: return render_row_track_cc(2, 6, hasActiveSubMenu);
    case MENU_H3_CC: return render_row_track_cc(2, 7, hasActiveSubMenu);
    case MENU_I3_CC: return render_row_track_cc(2, 8, hasActiveSubMenu);
    case MENU_J3_CC: return render_row_track_cc(2, 9, hasActiveSubMenu);
    case MENU_K3_CC: return render_row_track_cc(2, 10, hasActiveSubMenu);
    case MENU_L3_CC: return render_row_track_cc(2, 11, hasActiveSubMenu);
    case MENU_M3_CC: return render_row_track_cc(2, 12, hasActiveSubMenu);
    case MENU_N3_CC: return render_row_track_cc(2, 13, hasActiveSubMenu);
    case MENU_O3_CC: return render_row_track_cc(2, 14, hasActiveSubMenu);
    case MENU_P3_CC: return render_row_track_cc(2, 15, hasActiveSubMenu);
    case MENU_A4_CC: return render_row_track_cc(3, 0, hasActiveSubMenu);
    case MENU_B4_CC: return render_row_track_cc(3, 1, hasActiveSubMenu);
    case MENU_C4_CC: return render_row_track_cc(3, 2, hasActiveSubMenu);
    case MENU_D4_CC: return render_row_track_cc(3, 3, hasActiveSubMenu);
    case MENU_E4_CC: return render_row_track_cc(3, 4, hasActiveSubMenu);
    case MENU_F4_CC: return render_row_track_cc(3, 5, hasActiveSubMenu);
    case MENU_G4_CC: return render_row_track_cc(3, 6, hasActiveSubMenu);
    case MENU_H4_CC: return render_row_track_cc(3, 7, hasActiveSubMenu);
    case MENU_I4_CC: return render_row_track_cc(3, 8, hasActiveSubMenu);
    case MENU_J4_CC: return render_row_track_cc(3, 9, hasActiveSubMenu);
    case MENU_K4_CC: return render_row_track_cc(3, 10, hasActiveSubMenu);
    case MENU_L4_CC: return render_row_track_cc(3, 11, hasActiveSubMenu);
    case MENU_M4_CC: return render_row_track_cc(3, 12, hasActiveSubMenu);
    case MENU_N4_CC: return render_row_track_cc(3, 13, hasActiveSubMenu);
    case MENU_O4_CC: return render_row_track_cc(3, 14, hasActiveSubMenu);
    case MENU_P4_CC: return render_row_track_cc(3, 15, hasActiveSubMenu);

  }
}

void render_menu() {
  reset_display();
  display.println(F("======== Menu ========"));
  for (byte i = 0; i < SCREEN_MENU_ROWS; i++) {
    if (menuSelectedRow < SCREEN_MENU_ROWS) {
      render_row(i);
    } else {
      render_row(menuSelectedRow - SCREEN_MENU_ROWS + 1 + i);
    }
    display.println(F("                    "));
  }
}

void render_loading() {
  clear_display();
  reset_display();
  display.setScale(2);
  display.setCursor(0, 3);
  display.println(F("Loading..."));
}

void render_saving() {
  clear_display();
  reset_display();
  display.setScale(2);
  display.setCursor(0, 3);
  display.println(F("Saving..."));
}

void render_resetting() {
  clear_display();
  reset_display();
  display.setScale(2);
  display.setCursor(0, 3);
  display.println(F("Reset..."));
}

void init_display() {
  display.init();
  render_init_screen();
  clear_display();
  reset_display();
}