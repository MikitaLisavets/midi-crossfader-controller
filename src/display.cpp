#define OLED_SPI_SPEED 4000000ul
#include <GyverOLED.h> // Source: https://github.com/GyverLibs/GyverOLED
#include <display.h>

GyverOLED<SSD1306_128x64, OLED_BUFFER> display;
#define SCREEN_MENU_ROWS 7

const char PROGMEM trackTitles[ALL_TRACKS] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };

void clear_dispay() {
  display.clear();
}

void reset_dispay() {
  display.setScale(1);
  display.invertText(false);
  display.home();
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

void render_init_screen() {
  clear_dispay();

  display.setScale(1);
  display.setCursor(0, 7);
  display.print(F("Version: "));
  display.print(VERSION);
  refresh_dispay();

  display.setScale(2);
  display.setCursor(25, 1);
  display.print(F("X"));
  refresh_dispay();
  display.print(F("-"));
  refresh_dispay();
  display.print(F("F"));
  refresh_dispay();
  display.print(F("a"));
  refresh_dispay();
  display.print(F("d"));
  refresh_dispay();
  display.print(F("e"));
  refresh_dispay();
  display.print(F("r"));
  refresh_dispay();
  delay(300);
}

void render_main(
  StateEvent stateEvent
) {
  reset_dispay();

  for (int8_t i = 0; i < NUMBER_OF_TRACKS; i++) {
    int8_t indexWithOffset = (trackOffset + i) % ALL_TRACKS;

    if (stateEvent.trackIndex == (indexWithOffset) && !stateEvent.stageChanged && !stateEvent.midiValuesChanged && !stateEvent.midiValuesSwap) {
      display.invertText(true);
    }
    display.print((char)pgm_read_byte(&trackTitles[(indexWithOffset)]));
    if (stateEvent.pageChanged) {
      display.invertText(true);
    }
    display.print(pageIndex + 1);
    display.invertText(false);
    display.print(F(": "));
    if ((stateEvent.trackIndex == (indexWithOffset) || stateEvent.trackIndex < 0) && stateEvent.stageChanged && stateEvent.side == SIDE_LEFT) {
      display.invertText(true);
    }
    display.print(settings.stageIndexes[SIDE_LEFT][pageIndex][(indexWithOffset)] + 1);
    display.invertText(false);
    display.print(F("| "));
    if (stateEvent.trackIndex == (indexWithOffset) && ((stateEvent.midiValuesChanged && stateEvent.side == SIDE_LEFT) || stateEvent.midiValuesSwap)) {
      display.invertText(true);
    }
    render_filled_number(settings.midiValues[SIDE_LEFT][pageIndex][(indexWithOffset)][settings.stageIndexes[SIDE_LEFT][pageIndex][(indexWithOffset)]]);
    display.invertText(false);
    display.print(F("<"));
    render_filled_number(midiValues[pageIndex][(indexWithOffset)]);
    display.print(F(">"));
    if (stateEvent.trackIndex == (indexWithOffset) && ((stateEvent.midiValuesChanged && stateEvent.side == SIDE_RIGHT) ||stateEvent.midiValuesSwap)) {
      display.invertText(true);
    }
    render_filled_number(settings.midiValues[SIDE_RIGHT][pageIndex][(indexWithOffset)][settings.stageIndexes[SIDE_RIGHT][pageIndex][(indexWithOffset)]]);
    display.invertText(false);
    display.print(F(" |"));
    if ((stateEvent.trackIndex == (indexWithOffset) || stateEvent.trackIndex < 0) && stateEvent.stageChanged && stateEvent.side == SIDE_RIGHT) {
      display.invertText(true);
    }
    display.print(settings.stageIndexes[SIDE_RIGHT][pageIndex][(indexWithOffset)] + 1);
    display.invertText(false);
    display.println();
    if (i < NUMBER_OF_TRACKS - 1) {
      display.println(F("----------------------"));
    } else {
      display.print(F("__________"));
      for (int8_t j = 0; j < ALL_TRACKS / NUMBER_OF_TRACKS; j++) {
        if (j ==  trackOffset / NUMBER_OF_TRACKS) {
          display.print(F("*"));
        } else {
          display.print(F("."));
        }
      }
      display.println(F("__________"));
    }
  }

  refresh_dispay();
}

// === Menu ===

void render_row_load() {
  display.println(F("Load Settings"));
}

void render_row_save() {
  display.println(F("Save Settings"));
}

void render_row_reset() {
  display.println(F("Reset Settings"));
}

void render_row_midi_channel(bool hasActiveSubMenu) {
  display.print(F("MIDI Channel: "));
  if (hasActiveSubMenu) {
    display.invertText(true);
  }
  display.println(settings.midiChannel);

}

void render_row_fader_threshold(bool hasActiveSubMenu) {
  display.print(F("Fader Threshold: "));
  if (hasActiveSubMenu) {
    display.invertText(true);
  }
  display.println(settings.faderThreshold);
}

void render_row_auto_load_settings(bool hasActiveSubMenu) {
  display.print(F("Auto-Load: "));
  if (hasActiveSubMenu) {
    display.invertText(true);
  }
  if (settings.autoLoadSettings) {
    display.println(F("Yes"));
  } else {
    display.println(F("No"));
  }
}

void render_row_track_cc(uint8_t pageIndex, uint8_t trackIndex, bool hasActiveSubMenu) {
  display.print(F("CC "));
  display.print((char)pgm_read_byte(&trackTitles[trackIndex]));
  display.print(pageIndex + 1);
  display.print(F(": "));
  if (hasActiveSubMenu) {
    display.invertText(true);
  }
  display.println(settings.ccValues[pageIndex][trackIndex]);
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
    case MENU_A1_CC: return render_row_track_cc(0, 0, hasActiveSubMenu);
    case MENU_B1_CC: return render_row_track_cc(0, 1, hasActiveSubMenu);
    case MENU_C1_CC: return render_row_track_cc(0, 2, hasActiveSubMenu);
    case MENU_D1_CC: return render_row_track_cc(0, 3, hasActiveSubMenu);
    case MENU_E1_CC: return render_row_track_cc(0, 4, hasActiveSubMenu);
    case MENU_F1_CC: return render_row_track_cc(0, 5, hasActiveSubMenu);
    case MENU_G1_CC: return render_row_track_cc(0, 6, hasActiveSubMenu);
    case MENU_H1_CC: return render_row_track_cc(0, 7, hasActiveSubMenu);
    case MENU_A2_CC: return render_row_track_cc(1, 0, hasActiveSubMenu);
    case MENU_B2_CC: return render_row_track_cc(1, 1, hasActiveSubMenu);
    case MENU_C2_CC: return render_row_track_cc(1, 2, hasActiveSubMenu);
    case MENU_D2_CC: return render_row_track_cc(1, 3, hasActiveSubMenu);
    case MENU_E2_CC: return render_row_track_cc(1, 4, hasActiveSubMenu);
    case MENU_F2_CC: return render_row_track_cc(1, 5, hasActiveSubMenu);
    case MENU_G2_CC: return render_row_track_cc(1, 6, hasActiveSubMenu);
    case MENU_H2_CC: return render_row_track_cc(1, 7, hasActiveSubMenu);
    case MENU_A3_CC: return render_row_track_cc(2, 0, hasActiveSubMenu);
    case MENU_B3_CC: return render_row_track_cc(2, 1, hasActiveSubMenu);
    case MENU_C3_CC: return render_row_track_cc(2, 2, hasActiveSubMenu);
    case MENU_D3_CC: return render_row_track_cc(2, 3, hasActiveSubMenu);
    case MENU_E3_CC: return render_row_track_cc(2, 4, hasActiveSubMenu);
    case MENU_F3_CC: return render_row_track_cc(2, 5, hasActiveSubMenu);
    case MENU_G3_CC: return render_row_track_cc(2, 6, hasActiveSubMenu);
    case MENU_H3_CC: return render_row_track_cc(2, 7, hasActiveSubMenu);
    case MENU_A4_CC: return render_row_track_cc(3, 0, hasActiveSubMenu);
    case MENU_B4_CC: return render_row_track_cc(3, 1, hasActiveSubMenu);
    case MENU_C4_CC: return render_row_track_cc(3, 2, hasActiveSubMenu);
    case MENU_D4_CC: return render_row_track_cc(3, 3, hasActiveSubMenu);
    case MENU_E4_CC: return render_row_track_cc(3, 4, hasActiveSubMenu);
    case MENU_F4_CC: return render_row_track_cc(3, 5, hasActiveSubMenu);
    case MENU_G4_CC: return render_row_track_cc(3, 6, hasActiveSubMenu);
    case MENU_H4_CC: return render_row_track_cc(3, 7, hasActiveSubMenu);
  }
}

void render_menu() {
  clear_dispay();
  reset_dispay();
  display.setCursor(30, 0);
  display.println(F("=== Menu ==="));
  display.setCursor(0, 1);
  for (byte i = 0; i < SCREEN_MENU_ROWS; i++) {
    if (menuSelectedRow < SCREEN_MENU_ROWS) {
      render_row(i);
    } else {
      render_row(menuSelectedRow - SCREEN_MENU_ROWS + 1 + i);
    }
  }
  refresh_dispay();
}

void render_loading() {
  clear_dispay();
  reset_dispay();
  display.setScale(2);
  display.println(F("Loading..."));
  refresh_dispay();
}

void render_saving() {
  clear_dispay();
  reset_dispay();
  display.setScale(2);
  display.println(F("Saving..."));
  refresh_dispay();
}

void render_resetting() {
  clear_dispay();
  reset_dispay();
  display.setScale(2);
  display.println(F("Resetting..."));
  refresh_dispay();
}

void init_display() {
  display.init();
  render_init_screen();
  clear_dispay();
  reset_dispay();
}