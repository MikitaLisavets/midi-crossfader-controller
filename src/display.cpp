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

    if (stateEvent.trackIndex == indexWithOffset && !stateEvent.variantChanged && !stateEvent.midiValuesChanged && !stateEvent.midiValuesSwap) {
      display.invertText(true);
    }
    display.print((char)pgm_read_byte(&trackTitles[indexWithOffset]));
    if (stateEvent.pageChanged) {
      display.invertText(true);
    }
    display.print(pageIndex + 1);
    display.invertText(false);
    display.print(F(": "));
    if ((stateEvent.trackIndex == indexWithOffset || stateEvent.trackIndex < 0) && stateEvent.variantChanged && stateEvent.side == SIDE_LEFT) {
      display.invertText(true);
    }
    display.print(settings.variantIndexes[SIDE_LEFT][pageIndex][indexWithOffset] + 1);
    display.invertText(false);
    display.print(F("| "));
    if (stateEvent.trackIndex == indexWithOffset && ((stateEvent.midiValuesChanged && stateEvent.side == SIDE_LEFT) || stateEvent.midiValuesSwap)) {
      display.invertText(true);
    }
    render_filled_number(settings.midiValues[SIDE_LEFT][pageIndex][indexWithOffset][settings.variantIndexes[SIDE_LEFT][pageIndex][indexWithOffset]]);
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
    render_filled_number(settings.midiValues[SIDE_RIGHT][pageIndex][indexWithOffset][settings.variantIndexes[SIDE_RIGHT][pageIndex][indexWithOffset]]);
    display.invertText(false);
    display.print(F(" |"));
    if ((stateEvent.trackIndex == indexWithOffset || stateEvent.trackIndex < 0) && stateEvent.variantChanged && stateEvent.side == SIDE_RIGHT) {
      display.invertText(true);
    }
    display.print(settings.variantIndexes[SIDE_RIGHT][pageIndex][indexWithOffset] + 1);
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
    default:
      uint8_t count = MENU_CC;
      for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++) {
        for (uint8_t j = 0; j < ALL_TRACKS; j++) {
          if (rowIndex == count) {
             render_row_track_cc(i, j, hasActiveSubMenu);
          }
          count++;
        }
      }
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