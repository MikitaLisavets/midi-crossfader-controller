#define OLED_SPI_SPEED 4000000ul
#include <GyverOLED.h> // Source: https://github.com/GyverLibs/GyverOLED
#include <display.h>

GyverOLED<SSD1306_128x64, OLED_BUFFER> display;
#define SCREEN_MENU_ROWS 7

const char PROGMEM trackTitles[NUMBER_OF_ALL_TRACKS] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P' };

void clear_display() {
  display.clear();
}

void reset_display() {
  display.home();
  display.setScale(1);
  display.invertText(false);
  display.autoPrintln(false);
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
  display.update();

  display.print(F("-"));
  display.update();

  display.print(F("F"));
  display.update();

  display.print(F("a"));
  display.update();

  display.print(F("d"));
  display.update();

  display.print(F("e"));
  display.update();

  display.print(F("r"));
  display.update();
  delay(CLICK_TIMEOUT);
}

void render_main(
  StateEvent &stateEvent
) {
  reset_display();

  for (int8_t i = 0; i < NUMBER_OF_TRACKS_ON_SCREEN; i++) {
    int8_t indexWithOffset = (trackOffset + i) % NUMBER_OF_ALL_TRACKS;

    if (stateEvent.trackIndex == indexWithOffset && !stateEvent.variantChanged && !stateEvent.midiValuesChanged && !stateEvent.midiValuesSwap) {
      display.invertText(true);
    }
    display.print((char)pgm_read_byte(&trackTitles[indexWithOffset]));
    if (stateEvent.pageChanged) {
      display.invertText(true);
    }
    display.print(currentPage + 1);
    display.invertText(false);
    display.print(F(": "));
    if ((stateEvent.trackIndex == indexWithOffset || stateEvent.trackIndex < 0) && stateEvent.variantChanged && stateEvent.side == SIDE_LEFT) {
      display.invertText(true);
    }
    display.print(settings.variantIndexes[SIDE_LEFT][currentPage][indexWithOffset] + 1);
    display.invertText(false);
    display.print(F("| "));
    if (stateEvent.trackIndex == indexWithOffset && ((stateEvent.midiValuesChanged && stateEvent.side == SIDE_LEFT) || stateEvent.midiValuesSwap)) {
      display.invertText(true);
    }
    render_filled_number(settings.midiValues[SIDE_LEFT][currentPage][indexWithOffset][settings.variantIndexes[SIDE_LEFT][currentPage][indexWithOffset]]);
    display.invertText(false);
    display.print(F("<"));
    if (stateEvent.trackIndex == indexWithOffset) {
      render_filled_number(midiValues[currentPage][indexWithOffset]);
    } else {
      display.print(F("-|-"));
    }
    display.print(F(">"));
    if (stateEvent.trackIndex == indexWithOffset && ((stateEvent.midiValuesChanged && stateEvent.side == SIDE_RIGHT) ||stateEvent.midiValuesSwap)) {
      display.invertText(true);
    }
    render_filled_number(settings.midiValues[SIDE_RIGHT][currentPage][indexWithOffset][settings.variantIndexes[SIDE_RIGHT][currentPage][indexWithOffset]]);
    display.invertText(false);
    display.print(F(" |"));
    if ((stateEvent.trackIndex == indexWithOffset || stateEvent.trackIndex < 0) && stateEvent.variantChanged && stateEvent.side == SIDE_RIGHT) {
      display.invertText(true);
    }
    display.print(settings.variantIndexes[SIDE_RIGHT][currentPage][indexWithOffset] + 1);
    display.invertText(false);
    display.println(F(" "));
    if (i < NUMBER_OF_TRACKS_ON_SCREEN - 1) {
      display.println(F("----------------------"));
    }
  }

  display.print(F("_________"));
  for (int8_t j = 0; j < NUMBER_OF_ALL_TRACKS / NUMBER_OF_TRACKS_ON_SCREEN; j++) {
    if (j ==  trackOffset / NUMBER_OF_TRACKS_ON_SCREEN) {
      display.print(F("*"));
    } else {
      display.print(F("."));
    }
  }
  display.println(F("_________"));

  display.update();
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

void render_row_track_cc(uint8_t currentPage, uint8_t trackIndex, bool hasActiveSubMenu) {
  display.print(F("Control Change "));
  display.print((char)pgm_read_byte(&trackTitles[trackIndex]));
  display.print(currentPage + 1);
  display.print(F(": "));
  if (hasActiveSubMenu) {
    display.invertText(true);
  }
  display.print(settings.ccValues[currentPage][trackIndex]);
}

void render_row(int8_t rowIndex) {
  bool isSelected = rowIndex == selectedMenuRow;
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
        for (uint8_t j = 0; j < NUMBER_OF_ALL_TRACKS; j++) {
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
    if (selectedMenuRow < SCREEN_MENU_ROWS) {
      render_row(i);
    } else {
      render_row(selectedMenuRow - SCREEN_MENU_ROWS + 1 + i);
    }
    display.println(F("                    "));
  }
  display.update();
}

void render_loading() {
  clear_display();
  reset_display();
  display.setScale(2);
  display.setCursor(0, 3);
  display.println(F("Loading..."));
  display.update();
}

void render_saving() {
  clear_display();
  reset_display();
  display.setScale(2);
  display.setCursor(0, 3);
  display.println(F("Saving..."));
  display.update();
}

void render_resetting() {
  clear_display();
  reset_display();
  display.setScale(2);
  display.setCursor(0, 3);
  display.println(F("Reset..."));
  display.update();
}

void init_display() {
  display.init();
  render_init_screen();
  clear_display();
  reset_display();
}