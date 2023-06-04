#include <SPI.h>
#include <GyverOLED.h> // Source: https://github.com/GyverLibs/GyverOLED
#include <display.h>

GyverOLED<SSD1306_128x32, OLED_BUFFER> display;

char trackTitles[4] = { 'A', 'B', 'C', 'D' };
char pageTitles[4] = { '1', '2', '3', '4' };
char stageTitles[4] = { '1', '2', '3', '4' };

void clear_dispay() {
  display.clear();
  display.setScale(1);
  display.invertText(false);
  display.setCursor(0, 0);
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

  display.setScale(2);
  display.setCursor(25, 1);
  display.print(F("X"));
  refresh_dispay();
  delay(30);
  display.print(F("-"));
  refresh_dispay();
  delay(30);
  display.print(F("F"));
  refresh_dispay();
  delay(30);
  display.print(F("a"));
  refresh_dispay();
  delay(30);
  display.print(F("d"));
  refresh_dispay();
  delay(30);
  display.print(F("e"));
  refresh_dispay();
  delay(30);
  display.print(F("r"));
  refresh_dispay();
  delay(300);
}


void render_page_press() {
  clear_dispay();
  display.setScale(3);
  display.print(F("Page:"));
  display.println(pageTitles[pageIndex]);
  refresh_dispay();
}

void render_main() {
  clear_dispay();

  for (int i = 0; i < NUMBER_OF_TRACKS; i++) {
    display.print(trackTitles[i]);
    display.print(pageTitles[pageIndex]);
    display.print(F(": "));
    display.print(stageTitles[settings.stageIndexes[SIDE_LEFT][pageIndex][i]]);
    display.print(F("| "));
    render_filled_number(settings.midiValues[SIDE_LEFT][pageIndex][i][settings.stageIndexes[SIDE_LEFT][pageIndex][i]]);
    display.print(F("<"));
    render_filled_number(midiValues[pageIndex][i]);
    display.print(F(">"));
    render_filled_number(settings.midiValues[SIDE_RIGHT][pageIndex][i][settings.stageIndexes[SIDE_RIGHT][pageIndex][i]]);
    display.print(F(" |"));
    display.print(stageTitles[settings.stageIndexes[SIDE_RIGHT][pageIndex][i]]);
    display.print(F("|"));
    display.println();
  }

  refresh_dispay();
}

void render_stage_change(int8_t trackIndex, side_t side) {
  clear_dispay();
  display.setScale(2);
  if (trackIndex >= 0) {
    if (side == SIDE_LEFT) {
      display.print(F("Stage L: "));
    } else {
      display.print(F("Stage R: "));
    }
    display.println(stageTitles[settings.stageIndexes[side][pageIndex][trackIndex]]);
    display.print(F("Track: "));
    display.print(trackTitles[trackIndex]);
    display.println(pageTitles[pageIndex]);
  } else {
    if (side == SIDE_LEFT) {
      display.print(F("Stage L: "));
    } else {
      display.print(F("Stage R: "));
    }
    display.println(stageTitles[settings.stageIndexes[side][pageIndex][0]]);
    display.println(F("All tracks"));
  }

  refresh_dispay();
}

void render_track_press(int8_t trackIndex) {
  clear_dispay();
  display.setScale(2);
  display.println(F("Sending ..."));
  display.print(F("Track: "));
  display.print(trackTitles[trackIndex]);
  display.println(pageTitles[pageIndex]);
  refresh_dispay();
}

void render_midi_value_change(int8_t trackIndex, side_t side) {
  clear_dispay();
  display.setScale(2);

  if (side == SIDE_LEFT) {
    display.println(F("Value L:"));
  } else {
    display.println(F("Value R:"));
  }

  display.println(settings.midiValues[side][pageIndex][trackIndex][settings.stageIndexes[side][pageIndex][trackIndex]]);

  refresh_dispay();
}

void render_midi_values_swap(int8_t trackIndex) {
  clear_dispay();
  display.setScale(2);
  display.println(F("Swap values"));
  display.print(settings.midiValues[SIDE_LEFT][pageIndex][trackIndex][settings.stageIndexes[SIDE_LEFT][pageIndex][trackIndex]]);
  display.print(F(" <> "));
  display.println(settings.midiValues[SIDE_RIGHT][pageIndex][trackIndex][settings.stageIndexes[SIDE_RIGHT][pageIndex][trackIndex]]);
  display.print(F("Track: "));
  display.println(trackTitles[trackIndex]);
  refresh_dispay();
}

// === Menu ===

void render_row_load() {
  display.println(F("Load Settings"));
}

void render_row_save() {
  display.println(F("Save Settings"));
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
  display.print(F("Track "));
  display.print(trackTitles[trackIndex]);
  display.print(pageTitles[pageIndex]);
  display.print(F(" CC: "));
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
    case MENU_MIDI_CHANNEL: return render_row_midi_channel(hasActiveSubMenu);
    case MENU_FADER_THRESHOLD: return render_row_fader_threshold(hasActiveSubMenu);
    case MENU_AUTO_LOAD_SETTINGS: return render_row_auto_load_settings(hasActiveSubMenu);
    case MENU_A1_CC: return render_row_track_cc(0, 0, hasActiveSubMenu);
    case MENU_B1_CC: return render_row_track_cc(0, 1, hasActiveSubMenu);
    case MENU_C1_CC: return render_row_track_cc(0, 2, hasActiveSubMenu);
    case MENU_D1_CC: return render_row_track_cc(0, 3, hasActiveSubMenu);
    case MENU_A2_CC: return render_row_track_cc(1, 0, hasActiveSubMenu);
    case MENU_B2_CC: return render_row_track_cc(1, 1, hasActiveSubMenu);
    case MENU_C2_CC: return render_row_track_cc(1, 2, hasActiveSubMenu);
    case MENU_D2_CC: return render_row_track_cc(1, 3, hasActiveSubMenu);
    case MENU_A3_CC: return render_row_track_cc(2, 0, hasActiveSubMenu);
    case MENU_B3_CC: return render_row_track_cc(2, 1, hasActiveSubMenu);
    case MENU_C3_CC: return render_row_track_cc(2, 2, hasActiveSubMenu);
    case MENU_D3_CC: return render_row_track_cc(2, 3, hasActiveSubMenu);
    case MENU_A4_CC: return render_row_track_cc(3, 0, hasActiveSubMenu);
    case MENU_B4_CC: return render_row_track_cc(3, 1, hasActiveSubMenu);
    case MENU_C4_CC: return render_row_track_cc(3, 2, hasActiveSubMenu);
    case MENU_D4_CC: return render_row_track_cc(3, 3, hasActiveSubMenu);
  }
}

void render_menu() {
  clear_dispay();
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
  display.setScale(2);
  display.println(F("Loading..."));
  refresh_dispay();
}

void render_saving() {
  clear_dispay();
  display.setScale(2);
  display.println(F("Saving..."));
  refresh_dispay();
}

void init_display() {
  display.init();
  render_init_screen();
  clear_dispay();
}