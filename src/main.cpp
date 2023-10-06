#include <main.h>
#include <display.h>
#include <midi.h>
#include <utils.h>

// === Initialize variables ===

uint8_t const TRACK_PINS[NUMBER_OF_TRACKS_ON_SCREEN] = {5, 7, 9, A1};
uint8_t const PAGE_PINS[NUMBER_OF_PAGES] = {6, 8, A2, A0};

bool isMenuActive = false;
bool isSubMenuActive = false;

bool shouldScreenUpdate = false;
bool wasAction = false;

uint8_t currentPage = 0;
int8_t selectedMenuRow = 0;
int8_t trackOffset = 0;

int8_t pressedTrackButtonIndex;
int8_t pressedPageButtonIndex;
side_t pressedSideButtonIndex;

Settings settings;

StateEvent stateEvent;

uint8_t midiValues[NUMBER_OF_PAGES][NUMBER_OF_ALL_TRACKS] = {
  {0}
};


// ============================

// === Handlers ===

void reset_settings_to_default() {
  settings = {
    .midiChannel = 0,
    .faderThreshold = 10,
    .scrollFastSpeed = 10,
    .variantIndexes = {{{0}}, {{0}}},
    .midiValues = {{{{0}}}, {{{0}}}},
    .ccValues = {{0}},
    .autoLoadSettings = false,
  };

  for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++) {
      for (uint8_t j = 0; j < NUMBER_OF_ALL_TRACKS; j++) {
          for (uint8_t k = 0; k < NUMBER_OF_VARIANTS; k++) {
              settings.midiValues[1][i][j][k] = 127;
          }
      }
  }

  uint8_t count = 0;
  for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++) {
    for (uint8_t j = 0; j < NUMBER_OF_ALL_TRACKS; j++) {
      settings.ccValues[i][j] = count;
      count++;
    }
  }
}

void reset_state_to_default() {
  stateEvent = {
    .pageChanged = false,
    .variantChanged = false,
    .midiValuesChanged = false,
    .midiValuesSwap = false,
    .trackIndex = -1,
    .side = SIDE_NONE
  };
}

void handle_page_press(uint8_t newPageIndex) {
  /*
    only PAGE button pressed:
      - change page index
  */

  currentPage = newPageIndex;

  stateEvent.pageChanged = true;
}

void handle_variant_change(int8_t newVariantIndex, int8_t trackIndex, side_t side) {
  int8_t indexWithOffset = (trackOffset + trackIndex) % NUMBER_OF_ALL_TRACKS;

  if (is_button_pressed(trackIndex)) {
    /*
      PAGE button and LEFT/RIGHT button and TRACK button are pressed:
        - change left/right variant for track on current page
    */
    settings.variantIndexes[side][currentPage][indexWithOffset] = newVariantIndex;
    stateEvent.trackIndex = indexWithOffset;
  } else {
    /*
      PAGE button and LEFT/RIGHT button pressed:
        - change left/right variant for all tracks on current page
    */
    for (int8_t tIndex = 0; tIndex < NUMBER_OF_ALL_TRACKS; tIndex++) {
      settings.variantIndexes[side][currentPage][tIndex] = newVariantIndex;
    }
    stateEvent.trackIndex = -1;
  }

  stateEvent.variantChanged = true;
  stateEvent.side = side;
}

void handle_track_press(byte trackIndex) {
  /*
    TRACK button pressed:
      - send midi event to learn with current value
  */

  int8_t indexWithOffset = (trackOffset + trackIndex) % NUMBER_OF_ALL_TRACKS;


  control_change(settings.midiChannel, settings.ccValues[currentPage][indexWithOffset], midiValues[currentPage][indexWithOffset]);
  send_midi();

  stateEvent.trackIndex = indexWithOffset;
}

void handle_midi_value_change(int8_t trackIndex, side_t side) {
  /*
    TRACK button and LEFT/RIGHT button pressed:
      listen to POT and change left/right midi value
  */
  int8_t speed = is_encoder_turned_fast() ? settings.scrollFastSpeed : 1;
  int8_t indexWithOffset = (trackOffset + trackIndex) % NUMBER_OF_ALL_TRACKS;
  int16_t potValue;

  if (is_encoder_turned_right()) {
    wasAction = true;
    potValue = safe_midi_value(settings.midiValues[side][currentPage][indexWithOffset][settings.variantIndexes[side][currentPage][indexWithOffset]] + speed);
    settings.midiValues[side][currentPage][indexWithOffset][settings.variantIndexes[side][currentPage][indexWithOffset]] = potValue;
  }
  if (is_encoder_turned_left()) {
    wasAction = true;
    potValue = safe_midi_value(settings.midiValues[side][currentPage][indexWithOffset][settings.variantIndexes[side][currentPage][indexWithOffset]] - speed);
    settings.midiValues[side][currentPage][indexWithOffset][settings.variantIndexes[side][currentPage][indexWithOffset]] = potValue;
  }

  stateEvent.midiValuesChanged = true;
  stateEvent.trackIndex = indexWithOffset;
  stateEvent.side = side;
}

void handle_midi_values_swap(byte trackIndex) {
  /*
    TRACK button and LEFT or RIGHT button pressed and POT button are pressed:
      - swap left and right midi values
  */
  int8_t indexWithOffset = (trackOffset + trackIndex) % NUMBER_OF_ALL_TRACKS;
  int8_t temp = settings.midiValues[SIDE_LEFT][currentPage][indexWithOffset][settings.variantIndexes[SIDE_LEFT][currentPage][indexWithOffset]];
  settings.midiValues[SIDE_LEFT][currentPage][indexWithOffset][settings.variantIndexes[SIDE_LEFT][currentPage][indexWithOffset]] = settings.midiValues[SIDE_RIGHT][currentPage][indexWithOffset][settings.variantIndexes[SIDE_RIGHT][currentPage][indexWithOffset]];
  settings.midiValues[SIDE_RIGHT][currentPage][indexWithOffset][settings.variantIndexes[SIDE_RIGHT][currentPage][indexWithOffset]] = temp;

  stateEvent.midiValuesSwap = true;
  stateEvent.trackIndex = indexWithOffset;
}

// ================

void setup() {
  // Turn off board leds
  pinMode(LED_BUILTIN_TX, INPUT);
  pinMode(LED_BUILTIN_RX, INPUT);

  // Initialize board pins
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);

  for (int8_t i = 0; i < NUMBER_OF_TRACKS_ON_SCREEN; i++) {
    pinMode(TRACK_PINS[i], INPUT_PULLUP);
  }

  for (int8_t i = 0; i < NUMBER_OF_PAGES; i++) {
    pinMode(PAGE_PINS[i], INPUT_PULLUP);
  }

  init_display();
  init_encoder();

  load_settings(settings);
  if (!settings.autoLoadSettings) {
    reset_settings_to_default();
  }
  reset_state_to_default();

  render_main(stateEvent);

  #ifndef PERFOMANCE_CHECK
    Serial.begin(9600);
  #endif
}

void loop_menu() {
  if (is_encoder_turned_right()) {
    wasAction = true;
    if (isSubMenuActive) {
      if (selectedMenuRow == MENU_MIDI_CHANNEL) {
        settings.midiChannel = safe_midi_value(settings.midiChannel + 1);
      }
      if (selectedMenuRow == MENU_FADER_THRESHOLD) {
        settings.faderThreshold = safe_midi_value(settings.faderThreshold + 1);
      }
      if (selectedMenuRow == MENU_AUTO_LOAD_SETTINGS) {
        settings.autoLoadSettings = settings.autoLoadSettings ? false : true;
      }
      if (selectedMenuRow == MENU_SCROLL_FAST_SPEED) {
        settings.scrollFastSpeed = safe_midi_value(settings.scrollFastSpeed + 1);
      }


      uint8_t count = MENU_CC;
      for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++) {
        for (uint8_t j = 0; j < NUMBER_OF_ALL_TRACKS; j++) {
          if (selectedMenuRow == count) {
            increase_cc_value(i, j);
          }
          count++;
        }
      }
    } else {
      selectedMenuRow++;
      if (selectedMenuRow >= MAX_MENU_ROWS) {
        selectedMenuRow = 0;
      }
    }
  }

  if (is_encoder_turned_left()) {
    wasAction = true;
    if (isSubMenuActive) {
      if (selectedMenuRow == MENU_MIDI_CHANNEL) {
        settings.midiChannel = safe_midi_value(settings.midiChannel - 1);
      }
      if (selectedMenuRow == MENU_FADER_THRESHOLD) {
        settings.faderThreshold = safe_midi_value(settings.faderThreshold - 1);
      }
      if (selectedMenuRow == MENU_AUTO_LOAD_SETTINGS) {
        settings.autoLoadSettings = settings.autoLoadSettings ? false : true;
      }
      if (selectedMenuRow == MENU_SCROLL_FAST_SPEED) {
        settings.scrollFastSpeed = safe_midi_value(settings.scrollFastSpeed - 1);
      }

      uint8_t count = MENU_CC;
      for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++) {
        for (uint8_t j = 0; j < NUMBER_OF_ALL_TRACKS; j++) {
          if (selectedMenuRow == count) {
            decrease_cc_value(i, j);
          }
          count++;
        }
      }
    } else {
      selectedMenuRow--;
      if (selectedMenuRow < 0) {
        selectedMenuRow = MAX_MENU_ROWS - 1;
      }
    }
  }

  if (is_encoder_clicked() || is_right_button_pressed()) {
    wasAction = true;
    if (selectedMenuRow == MENU_LOAD) {
      render_loading();
      load_settings(settings);
    } else if (selectedMenuRow == MENU_SAVE) {
      render_saving();
      save_settings(settings);
    } else if (selectedMenuRow == MENU_RESET) {
      render_resetting();
      reset_settings_to_default();
    } else if (isSubMenuActive) {
      isSubMenuActive = false;
      render_menu();
    } else {
      isSubMenuActive = true;
      render_menu();
    }
    delay(CLICK_TIMEOUT);
  }

  if (is_left_button_pressed()) {
    if (isSubMenuActive) {
      isSubMenuActive = false;
      render_menu();
      delay(CLICK_TIMEOUT);
    } else {
      isMenuActive = false;
      selectedMenuRow = 0;
      shouldScreenUpdate = true;
    }
    return;
  }


  if (is_button_pressed(pressedPageButtonIndex) || is_button_pressed(pressedTrackButtonIndex)) {
    shouldScreenUpdate = true;
    isSubMenuActive = false;
    isMenuActive = false;
    selectedMenuRow = 0;
    return;
  }
}

void loop_main() {
  if (is_left_button_pressed() && is_right_button_pressed()) {
    if (is_button_pressed(pressedTrackButtonIndex)) {
      handle_midi_values_swap(pressedTrackButtonIndex);
    }
  } else if (is_left_button_pressed()) {
    if (is_button_pressed(pressedTrackButtonIndex) && is_button_pressed(pressedPageButtonIndex)) {
      handle_variant_change(pressedPageButtonIndex, pressedTrackButtonIndex, SIDE_LEFT);
    } else if (is_button_pressed(pressedPageButtonIndex)) {
      handle_variant_change(pressedPageButtonIndex, -1, SIDE_LEFT);
    } else if (is_button_pressed(pressedTrackButtonIndex)) {
      handle_midi_value_change(pressedTrackButtonIndex, SIDE_LEFT);
    }
  } else if (is_right_button_pressed()) {
    if (is_button_pressed(pressedTrackButtonIndex) && is_button_pressed(pressedPageButtonIndex)) {
      handle_variant_change(pressedPageButtonIndex, pressedTrackButtonIndex, SIDE_RIGHT);
    } else if (is_button_pressed(pressedPageButtonIndex)) {
      handle_variant_change(pressedPageButtonIndex, -1, SIDE_RIGHT);
    } else if (is_button_pressed(pressedTrackButtonIndex)) {
      handle_midi_value_change(pressedTrackButtonIndex, SIDE_RIGHT);
    }
  } else if (is_button_pressed(pressedTrackButtonIndex)) {
    handle_track_press(pressedTrackButtonIndex);
  } else if (is_button_pressed(pressedPageButtonIndex)) {
    handle_page_press(pressedPageButtonIndex);
  } else {
    if (is_encoder_turned_left()) {
      wasAction = true;
      if (trackOffset - 4 < 0) {
        trackOffset = NUMBER_OF_ALL_TRACKS - 4;
      } else {
        trackOffset = trackOffset - 4;
      }
    }
    if (is_encoder_turned_right()) {
      wasAction = true;
      if (trackOffset + 4 >= NUMBER_OF_ALL_TRACKS) {
        trackOffset = 0;
      } else {
        trackOffset = trackOffset + 4;
      }
    }
  }

  int16_t faderValue = analogRead(FADER_PIN);

  if (faderValue > 1023 - settings.faderThreshold) {
    faderValue = 1023 - settings.faderThreshold;
  } else if (faderValue < 0 + settings.faderThreshold) {
    faderValue = 0 + settings.faderThreshold;
  }

  uint8_t previousMidiValues[NUMBER_OF_PAGES][NUMBER_OF_ALL_TRACKS];

  for (int8_t trackIndex = 0; trackIndex < NUMBER_OF_ALL_TRACKS; trackIndex++) {
    previousMidiValues[currentPage][trackIndex] = midiValues[currentPage][trackIndex];

    if (settings.midiValues[SIDE_RIGHT][currentPage][trackIndex][settings.variantIndexes[SIDE_RIGHT][currentPage][trackIndex]] < settings.midiValues[SIDE_LEFT][currentPage][trackIndex][settings.variantIndexes[SIDE_LEFT][currentPage][trackIndex]]) {
      midiValues[currentPage][trackIndex] = map(faderValue, 1023 - settings.faderThreshold, 0 + settings.faderThreshold, settings.midiValues[SIDE_RIGHT][currentPage][trackIndex][settings.variantIndexes[SIDE_RIGHT][currentPage][trackIndex]], settings.midiValues[SIDE_LEFT][currentPage][trackIndex][settings.variantIndexes[SIDE_LEFT][currentPage][trackIndex]]);
    } else {
      midiValues[currentPage][trackIndex] = map(faderValue, 0 + settings.faderThreshold, 1023 - settings.faderThreshold, settings.midiValues[SIDE_LEFT][currentPage][trackIndex][settings.variantIndexes[SIDE_LEFT][currentPage][trackIndex]], settings.midiValues[SIDE_RIGHT][currentPage][trackIndex][settings.variantIndexes[SIDE_RIGHT][currentPage][trackIndex]]);
    }

    if (previousMidiValues[currentPage][trackIndex] != midiValues[currentPage][trackIndex]) {
      control_change(settings.midiChannel, settings.ccValues[currentPage][trackIndex], midiValues[currentPage][trackIndex]);
      previousMidiValues[currentPage][trackIndex] = midiValues[currentPage][trackIndex];
      send_midi();
    }
  }
}


#ifndef PERFOMANCE_CHECK
  long perfomanceTimer;
#endif

void loop() {
  #ifndef PERFOMANCE_CHECK
    perfomanceTimer = micros();
  #endif

  int8_t previousPressedTrackButtonIndex = pressedTrackButtonIndex;
  int8_t previousPressedPageButtonIndex = pressedPageButtonIndex;
  side_t previousPressedSideButtonIndex = pressedSideButtonIndex;

  reset_state_to_default();
  encoder_tick();

  pressedTrackButtonIndex = get_pressed_track_button();
  pressedPageButtonIndex = get_pressed_page_button();
  pressedSideButtonIndex = is_left_button_pressed() ? SIDE_LEFT : is_right_button_pressed() ? SIDE_RIGHT : SIDE_NONE;

  wasAction = previousPressedTrackButtonIndex != pressedTrackButtonIndex || previousPressedPageButtonIndex != pressedPageButtonIndex || previousPressedSideButtonIndex != pressedSideButtonIndex;

  if (isMenuActive) {
    loop_menu();
  } else {
    if (is_encoder_clicked()) {
      isMenuActive = true;
      shouldScreenUpdate = true;
    } else {
      loop_main();
    }
  }

  if (shouldScreenUpdate) {
    if (isMenuActive) {
      render_menu();
    } else {
      render_main(stateEvent);
    }
  }

  shouldScreenUpdate = wasAction;

  #ifndef PERFOMANCE_CHECK
    Serial.println(micros() - perfomanceTimer);
  #endif
}

