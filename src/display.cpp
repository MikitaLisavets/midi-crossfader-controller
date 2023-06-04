#include <SPI.h>
#include <GyverOLED.h> // Source: https://github.com/GyverLibs/GyverOLED

#include <display.h>

GyverOLED<SSD1306_128x32, OLED_BUFFER> display;

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
    display.println(trackTitles[trackIndex]);
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
  display.println(F("Sending MIDI ..."));
  display.print(F("Page:  "));
  display.println(pageTitles[pageIndex]);
  display.print(F("Track: "));
  display.println(trackTitles[trackIndex]);

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
  display.println(F("Swap L and R values:"));
  display.print(F("Value L: "));
  display.println(settings.midiValues[SIDE_LEFT][pageIndex][trackIndex][settings.stageIndexes[SIDE_LEFT][pageIndex][trackIndex]]);
  display.print(F("Value R: "));
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

void render_row_midi_channel() {
  display.print(F("MIDI Channel: "));
  display.println(settings.midiChannel);
}

void render_row_fader_threshold() {
  display.print(F("Fader Threshold: "));
  display.println(settings.faderThreshold);
}

void render_row_track_cc(uint8_t pageIndex, uint8_t trackIndex) {
  display.print(F("Track "));
  display.print(trackTitles[trackIndex]);
  display.print(pageTitles[pageIndex]);
  display.print(F(" CC: "));
  display.println(settings.ccValues[pageIndex][trackIndex]);
}

void render_row(int8_t row_index) {
  bool is_selected = row_index == menu_selected_row;
  if (is_selected) {
    display.invertText(true);
  } else {
    display.invertText(false);
  }

  switch(row_index) {
    case MENU_LOAD: return render_row_load();
    case MENU_SAVE: return render_row_save();
    case MENU_MIDI_CHANNEL: return render_row_midi_channel();
    case MENU_FADER_THRESHOLD: return render_row_fader_threshold();
    case MENU_A1_CC: return render_row_track_cc(0, 0);
    case MENU_B1_CC: return render_row_track_cc(0, 1);
    case MENU_C1_CC: return render_row_track_cc(0, 2);
    case MENU_D1_CC: return render_row_track_cc(0, 3);
    case MENU_A2_CC: return render_row_track_cc(1, 0);
    case MENU_B2_CC: return render_row_track_cc(1, 1);
    case MENU_C2_CC: return render_row_track_cc(1, 2);
    case MENU_D2_CC: return render_row_track_cc(1, 3);
    case MENU_A3_CC: return render_row_track_cc(2, 0);
    case MENU_B3_CC: return render_row_track_cc(2, 1);
    case MENU_C3_CC: return render_row_track_cc(2, 2);
    case MENU_D3_CC: return render_row_track_cc(2, 3);
    case MENU_A4_CC: return render_row_track_cc(3, 0);
    case MENU_B4_CC: return render_row_track_cc(3, 1);
    case MENU_C4_CC: return render_row_track_cc(3, 2);
    case MENU_D4_CC: return render_row_track_cc(3, 3);
  }
}

void render_menu() {
  clear_dispay();
  display.setCursor(30, 0);
  display.println(F("=== Menu ==="));
  display.setCursor(0, 1);
  for (byte i = 0; i < SCREEN_MENU_ROWS; i++) {
    if (menu_selected_row < SCREEN_MENU_ROWS) {
      render_row(i);
    } else {
      render_row(menu_selected_row - SCREEN_MENU_ROWS + 1 + i);
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