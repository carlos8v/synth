#include "synth.h"

/**
 * CRC8 simple calculation
 * Based on https://github.com/PaulStoffregen/OneWire/blob/master/OneWire.cpp
 */
uint8_t crc8(const uint8_t* addr, uint8_t len) {
  uint8_t crc = 0;

  while (len--) {
    uint8_t inbyte = *addr++;
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}

void saveSettings() {
  uint8_t buffer[sizeof(SynthSettings) + 1];  // Use the last byte for CRC

  memcpy(buffer, &settings, sizeof(settings));
  buffer[sizeof(settings)] = crc8(buffer, sizeof(settings));

  for (int i = 0; i < sizeof(buffer); i++) {
    EEPROM.write(i, buffer[i]);
  }

  EEPROM.commit();
}

void loadSettings() {
  uint8_t buffer[sizeof(settings) + 1];  // Use the last byte for CRC

  for (int i = 0; i < sizeof(buffer); i++) {
    buffer[i] = uint8_t(EEPROM.read(i));
  }

  // Check CRC
  if (crc8(buffer, sizeof(settings)) == buffer[sizeof(settings)]) {
    memcpy(&settings, buffer, sizeof(settings));
    return;
  }

  Serial.println("Bad CRC, loading default settings");
  // Initialize with default values in case of settings not written

  settings.mode = SynthMode::PLAY_MODE;
  settings.baseKey = Semitone::C;
  settings.lastChord = 0;

  settings.menuIdx = 0;
  settings.adsr = ADSR_OPTION::LONG;
  settings.pitch = 0;
  settings.scale = 0;
  settings.filterCutoff = 200;
  saveSettings();
}

void displayScreen(void* parameter) {
  for (;;) {
    // Acquire mutex before accessing the shared buffer
    if (xSemaphoreTake(mutex_display, portMAX_DELAY) == pdTRUE) {
      switch (displayInfo.mode) {
        case SynthMode::PLAY_MODE:
          display.mainScreen(displayInfo);
          break;
        case SynthMode::MENU_MODE:
          display.menuScreen(displayInfo);
          break;
        default:
          break;
      }

      xSemaphoreGive(mutex_display);  // Release mutex
    }
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void setup() {
  EEPROM.begin(64);
  Serial.begin(115200);

  Wire.setPins(DISPLAY_SDA, DISPLAY_SCL);
  Wire.begin();

  if (!display.begin(SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }

  loadSettings();

  // Create the mutex for shared buffer access
  mutex_display = xSemaphoreCreateMutex();

  // Create a FreeRTOS task to run on Core 0
  xTaskCreatePinnedToCore(displayScreen, "Display screen", 10000, NULL, 1, NULL,
                          0);

  pinMode(KEY_1_PIN, INPUT_PULLUP);
  pinMode(KEY_2_PIN, INPUT_PULLUP);
  pinMode(KEY_3_PIN, INPUT_PULLUP);
  pinMode(KEY_4_PIN, INPUT_PULLUP);
  pinMode(KEY_5_PIN, INPUT_PULLUP);
  pinMode(KEY_6_PIN, INPUT_PULLUP);
  pinMode(KEY_7_PIN, INPUT_PULLUP);

  pinMode(MOD_KEY_PIN, INPUT_PULLUP);
  pinMode(OUT_MODE_PIN, INPUT_PULLUP);

  // TODO: handle clock
  // myClock.setTicksPerBeat(4);
  // myClock.setTempo(120);

  envelope.setAttack(ADSROptions[settings.adsr][0]);
  envelope.setDecay(ADSROptions[settings.adsr][1]);
  envelope.setSustain(ADSROptions[settings.adsr][2]);
  envelope.setRelease(ADSROptions[settings.adsr][3]);

  auto cfg = out.defaultConfig(TX_MODE);
  cfg.is_master = true;       // ESP32 generates the clock
  cfg.pin_bck = I2S_BCK_PIN;  // Bit Clock (BCK)
  cfg.pin_ws = I2S_WS_PIN;    // Word Select / LRCK
  cfg.pin_data = I2S_DATA_PIN;
  cfg.sample_rate = 32000;
  cfg.buffer_size = 512;

  maxiSettings::setup(32000, 2, 512);

  out.begin(cfg);
  maximilian.begin(cfg);

  // Initialize chords references
  setupChords();
  populateScale(scale, settings.baseKey, settings.pitch);
  chordToPlay = scale[settings.lastChord];

  // Initial display config
  displayInfo.mode = settings.mode;
  displayInfo.baseKey = String(getSemitoneLabel(settings.baseKey).c_str());
  displayInfo.chord = String(chordToPlay->chord);

  displayInfo.menuIdx = settings.menuIdx;
  displayInfo.adsr = settings.adsr;
  displayInfo.scale = settings.scale;
  displayInfo.pitch = settings.pitch;
  displayInfo.filterCutoff = settings.filterCutoff;

  display.initConfig(displayInfo);

  clickEffect.setSample(click_vector);
}

void playArpeggio(float* output, Chord* chord) {
  float adsr = envelope.adsr(1., !keyReleased);

  float single = osc[currentNote].sawn(chord->frequencies[currentNote]);
  float filtered = lowpass.lores(single, adsr * 1500, 0.8);  // Low-pass filter

  output[0] = output[1] = filtered * adsr;
}

void playChord(float* output, Chord* chord) {
  float out = 0.0, adsr;

  if (settings.adsr == ADSR_OPTION::SUSTAIN) {
    adsr = 1.0;
  } else if (settings.mode == SynthMode::PLAY_MODE) {
    adsr = envelope.adsr(1.0, !keyReleased);
  } else {
    adsr = 0.0;
  }

  float bottomFreq = chord->frequencies[0];
  float topFreq = chord->frequencies[chord->keys - 1];

  out += osc[0].sawn(chord->frequencies[0] + lfo.sinebuf(0.2));
  for (int i = 1; i < chord->keys; i++) {
    out += osc[i].sawn(chord->frequencies[i]);
  }

  out += oscPitch[0].square(chord->frequencies[0] / 2.0f) * 0.2f;
  out += oscPitch[1].saw(chord->frequencies[0] * 2.0f) * 0.2f;

  out /= chord->keys;

  out = lowpass.lores(out, adsr * (topFreq + settings.filterCutoff), 1.0);
  out = hipass.hires(out, adsr * (bottomFreq - settings.filterCutoff), 1.0);

  output[0] += out * adsr;
  output[1] = output[0];
}

void play(float* output) {
  // TODO: handle clock
  // myClock.ticker();  // Advance clock

  // if (myClock.tick) {
  //   // For arpeggios
  //   currentNote = (currentNote + 1) % 3;  // Cycle through notes 0–3
  // }

  // Trigger sound effects
  if (soundEffect) {
    soundEffect = 0;
    clickEffect.trigger();
  }

  output[0] = output[1] = clickEffect.playOnce();

  if (settings.lastChord >= 0) {
    // Sustain chord
    playChord(output, chordToPlay);
  }
}

void checkKeyPress() {
  keyNotes[0] = digitalRead(KEY_1_PIN) == LOW;
  keyNotes[1] = digitalRead(KEY_2_PIN) == LOW;
  keyNotes[2] = digitalRead(KEY_3_PIN) == LOW;
  keyNotes[3] = digitalRead(KEY_4_PIN) == LOW;
  keyNotes[4] = digitalRead(KEY_5_PIN) == LOW;
  keyNotes[5] = digitalRead(KEY_6_PIN) == LOW;
  keyNotes[6] = digitalRead(KEY_7_PIN) == LOW;

  int pressedIdx = -1;
  for (int idx = 0; idx < 7; idx++) {
    if (!keyNotes[idx]) continue;

    // First pressed key *or
    // Maintaining the same pressed key
    if (keyReleased || (!keyReleased && settings.lastChord == idx)) {
      pressedIdx = idx;
      break;
    }
  }

  if (keyReleased && pressedIdx >= 0) {
    keyReleased = 0;
    settings.lastChord = pressedIdx;
  }

  if (pressedIdx < 0) {
    keyReleased = 1;
  }
}

void checkKeyModifier() {
  int x = analogRead(MOD_PIN_X);
  int y = analogRead(MOD_PIN_Y);
  AxisPosition axisPosition = axis.getPosition(x, y);

  switch (axisPosition) {
    case AxisPosition::AXIS_UP:
      chordToPlay = scale[settings.lastChord]->major_minor;
      break;
    case AxisPosition::AXIS_UP_RIGHT:
      chordToPlay = scale[settings.lastChord]->seven;
      break;
    case AxisPosition::AXIS_RIGHT:
      chordToPlay = scale[settings.lastChord]->major7_minor7;
      break;
    case AxisPosition::AXIS_DOWN_RIGHT:
      chordToPlay = scale[settings.lastChord]->major9_minor9;
      break;
    case AxisPosition::AXIS_DOWN:
      chordToPlay = scale[settings.lastChord]->sus4;
      break;
    case AxisPosition::AXIS_DOWN_LEFT:
      chordToPlay = scale[settings.lastChord]->sus2;
      break;
    case AxisPosition::AXIS_LEFT:
      chordToPlay = scale[settings.lastChord]->dim;
      break;
    case AxisPosition::AXIS_UP_LEFT:
      chordToPlay = scale[settings.lastChord]->aug;
      break;
    default:
      chordToPlay = scale[settings.lastChord];
      break;
  }

  if (chordToPlay == NULL) {
    chordToPlay = scale[settings.lastChord];
  }
}

void playMode() {
  checkKeyPress();
  checkKeyModifier();

  int modPressed = digitalRead(MOD_KEY_PIN) == LOW;
  if (!modPressed && !modReleased) {
    modReleased = 1;
  }

  if (modPressed && modReleased) {
    settings.mode = SynthMode::MENU_MODE;
    modReleased = 0;
    delay(200);
  }
}

void menuMode() {
  int x = analogRead(MOD_PIN_X);
  int y = analogRead(MOD_PIN_Y);

  AxisPosition axisPosition = axis.getPosition(x, y);

  // Reset key press on axis centered
  if (!keyReleased && axisPosition == AxisPosition::AXIS_CENTER) {
    keyReleased = 1;
    delay(150);

    // Handle menu change
  } else if (keyReleased && (axisPosition == AxisPosition::AXIS_UP ||
                             axisPosition == AxisPosition::AXIS_DOWN)) {
    keyReleased = 0;

    if (axisPosition == AxisPosition::AXIS_UP) {
      settings.menuIdx = settings.menuIdx - 1 >= 0 ? (settings.menuIdx - 1)
                                                   : MAX_MENU_ITEMS - 1;
    } else if (axisPosition == AxisPosition::AXIS_DOWN) {
      settings.menuIdx = (settings.menuIdx + 1) % MAX_MENU_ITEMS;
    }

    soundEffect = 1;  // Trigger sound effect

    // Handle menu option change
  } else if (axisPosition == AxisPosition::AXIS_LEFT ||
             axisPosition == AxisPosition::AXIS_RIGHT) {
    // Keynote
    if (keyReleased && settings.menuIdx == MenuOption::MENU_KEYNOTE) {
      keyReleased = 0;
      if (axisPosition == AxisPosition::AXIS_LEFT) {
        settings.baseKey = getPreviousSemitone(settings.baseKey);
      } else if (axisPosition == AxisPosition::AXIS_RIGHT) {
        settings.baseKey = getNextSemitone(settings.baseKey);
      }

      delay(150);
    }

    // Pitch
    if (keyReleased && settings.menuIdx == MenuOption::MENU_PITCH) {
      keyReleased = 0;
      if (axisPosition == AxisPosition::AXIS_LEFT) {
        settings.pitch = max(settings.pitch - 1, -2);
      } else if (axisPosition == AxisPosition::AXIS_RIGHT) {
        settings.pitch = min(settings.pitch + 1, 2);
      }

      delay(150);
    }

    // Scale
    if (keyReleased && settings.menuIdx == MenuOption::MENU_SCALE) {
      keyReleased = 0;
      if (axisPosition == AxisPosition::AXIS_LEFT) {
        settings.scale -= 1;
        if (settings.scale < 0) {
          settings.scale = MAX_SCALE - 1;
        }
      } else if (axisPosition == AxisPosition::AXIS_RIGHT) {
        settings.scale += 1;
        if (settings.scale >= MAX_SCALE) {
          settings.scale = 0;
        }
      }

      delay(150);
    }

    // ADSR
    if (keyReleased && settings.menuIdx == MenuOption::MENU_ADSR) {
      keyReleased = 0;
      if (axisPosition == AxisPosition::AXIS_LEFT) {
        settings.adsr -= 1;
        if (settings.adsr < 0) {
          settings.adsr = ADSR_OPTION::SUSTAIN;
        }

      } else if (axisPosition == AxisPosition::AXIS_RIGHT) {
        settings.adsr += 1;
        if (settings.adsr >= MAX_ADSR) {
          settings.adsr = ADSR_OPTION::SHORT;
        }
      }

      envelope.setAttack(ADSROptions[settings.adsr][0]);
      envelope.setDecay(ADSROptions[settings.adsr][1]);
      envelope.setSustain(ADSROptions[settings.adsr][2]);
      envelope.setRelease(ADSROptions[settings.adsr][3]);

      delay(150);
    }

    // Filter
    if (settings.menuIdx == MenuOption::MENU_FILTER) {
      if (axisPosition == AxisPosition::AXIS_LEFT) {
        settings.filterCutoff = max(settings.filterCutoff - 10, 0);
      } else if (axisPosition == AxisPosition::AXIS_RIGHT) {
        settings.filterCutoff = min(settings.filterCutoff + 10, 500);
      }

      delay(50);
    }
  }

  // Change back to play mode
  int modPressed = digitalRead(MOD_KEY_PIN) == LOW;
  if (!modPressed && !modReleased) {
    modReleased = 1;
  }

  if (modPressed && modReleased) {
    modReleased = 0;
    populateScale(scale, settings.baseKey, settings.scale, settings.pitch);
    delay(200);

    settings.mode = SynthMode::PLAY_MODE;
    chordToPlay = scale[settings.lastChord];
    saveSettings();
  }
}

void loop() {
  maximilian.copy();  // Call the audio processing callback

  switch (settings.mode) {
    case SynthMode::PLAY_MODE:
      playMode();
      break;
    case SynthMode::MENU_MODE:
      menuMode();
    default:
      break;
  }

  // Update the shared buffer safely
  if (xSemaphoreTake(mutex_display, portMAX_DELAY) == pdTRUE) {
    displayInfo.mode = settings.mode;
    displayInfo.outMode = digitalRead(OUT_MODE_PIN) == LOW ? OutMode::SPEAKERS
                                                           : OutMode::LINE_OUT;
    displayInfo.baseKey = String(getSemitoneLabel(settings.baseKey).c_str());
    displayInfo.chord = String(chordToPlay->chord);

    displayInfo.menuIdx = settings.menuIdx;
    displayInfo.adsr = settings.adsr;
    displayInfo.pitch = settings.pitch;
    displayInfo.scale = settings.scale;
    displayInfo.filterCutoff = settings.filterCutoff;

    xSemaphoreGive(mutex_display);
  }
}
