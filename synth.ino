#include "synth.h"

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
  Serial.begin(115200);

  Wire.setPins(DISPLAY_SDA, DISPLAY_SCL);
  Wire.begin();

  if (!display.begin(SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }

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

  envelope.setAttack(ADSROptions[adsrOption][0]);
  envelope.setDecay(ADSROptions[adsrOption][1]);
  envelope.setSustain(ADSROptions[adsrOption][2]);
  envelope.setRelease(ADSROptions[adsrOption][3]);

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
  populateScale(scale, baseKey, pitch);
  chordToPlay = scale[0];

  // Initial display config
  displayInfo.mode = currentMode;
  displayInfo.baseKey = String(getSemitoneLabel(baseKey).c_str());
  displayInfo.chord = String(chordToPlay->chord);

  displayInfo.menuIdx = 0;
  displayInfo.adsr = adsrOption;
  displayInfo.pitch = pitch;
  displayInfo.filterCutoff = filterCutoff;

  display.initConfig(displayInfo);
}

void playArpeggio(float* output, Chord* chord) {
  double adsr = envelope.adsr(1., !keyReleased);

  double single = osc[currentNote].sawn(chord->frequencies[currentNote]);
  double filtered = lowpass.lores(single, adsr * 1500, 0.8);  // Low-pass filter

  output[0] = output[1] = filtered * adsr;
}

void playChord(float* output, Chord* chord) {
  double out = 0;
  double adsr;

  if (adsrOption == ADSR_OPTION::SUSTAIN) {
    adsr = 1.0;
  } else if (currentMode == SynthMode::PLAY_MODE) {
    adsr = envelope.adsr(1.0, !keyReleased);
  } else {
    adsr = 0.0;
  }

  double baseFreq = chord->frequencies[0] / 2;
  out += base.sawn(baseFreq);

  for (int i = 0; i < chord->keys; i++) {
    if (i == 0) {
      out += osc[i].sawn(chord->frequencies[i] + lfo.sinebuf(0.2));
    } else {
      out += osc[i].sawn(chord->frequencies[i]);
    }
  }

  out /= chord->keys + 1;

  out = lowpass.lores(
      out, adsr * (chord->frequencies[chord->keys - 1] + filterCutoff), 1.0);
  out = hipass.hires(out, adsr * (baseFreq - filterCutoff), 1.0);

  output[0] = output[1] = out * adsr;
}

void play(float* output) {
  // TODO: handle clock
  // myClock.ticker();  // Advance clock

  // if (myClock.tick) {
  //   // For arpeggios
  //   currentNote = (currentNote + 1) % 3;  // Cycle through notes 0–3
  // }

  if (lastChordIdx >= 0) {
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
    if (keyReleased || (!keyReleased && lastChordIdx == idx)) {
      pressedIdx = idx;
      break;
    }
  }

  if (keyReleased && pressedIdx >= 0) {
    keyReleased = 0;
    lastChordIdx = pressedIdx;
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
      chordToPlay = scale[lastChordIdx]->major_minor;
      break;
    case AxisPosition::AXIS_UP_RIGHT:
      chordToPlay = scale[lastChordIdx]->seven;
      break;
    case AxisPosition::AXIS_RIGHT:
      chordToPlay = scale[lastChordIdx]->major7_minor7;
      break;
    case AxisPosition::AXIS_DOWN_RIGHT:
      chordToPlay = scale[lastChordIdx]->major9_minor9;
      break;
    case AxisPosition::AXIS_DOWN:
      chordToPlay = scale[lastChordIdx]->sus4;
      break;
    case AxisPosition::AXIS_DOWN_LEFT:
      chordToPlay = scale[lastChordIdx]->sus2;
      break;
    case AxisPosition::AXIS_LEFT:
      chordToPlay = scale[lastChordIdx]->dim;
      break;
    case AxisPosition::AXIS_UP_LEFT:
      chordToPlay = scale[lastChordIdx]->aug;
      break;
    default:
      chordToPlay = scale[lastChordIdx];
      break;
  }

  if (chordToPlay == NULL) {
    chordToPlay = scale[lastChordIdx];
  }
}

void playMode() {
  maximilian.copy();  // Call the audio processing callback

  checkKeyPress();
  checkKeyModifier();

  int modPressed = digitalRead(MOD_KEY_PIN) == LOW;
  if (!modPressed && !modReleased) {
    modReleased = 1;
  }

  if (modPressed && modReleased) {
    lastChordIdx = 0;
    currentMode = SynthMode::MENU_MODE;
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
      menuIdx = menuIdx - 1 >= 0 ? (menuIdx - 1) : MAX_MENU_ITEMS - 1;
    } else if (axisPosition == AxisPosition::AXIS_DOWN) {
      menuIdx = (menuIdx + 1) % MAX_MENU_ITEMS;
    }

    delay(150);

    // Handle menu option change
  } else if (axisPosition == AxisPosition::AXIS_LEFT ||
             axisPosition == AxisPosition::AXIS_RIGHT) {
    // Keynote
    if (keyReleased && menuIdx == 0) {
      keyReleased = 0;
      if (axisPosition == AxisPosition::AXIS_LEFT) {
        baseKey = getPreviousSemitone(baseKey);
      } else if (axisPosition == AxisPosition::AXIS_RIGHT) {
        baseKey = getNextSemitone(baseKey);
      }

      delay(150);
    }

    // Pitch
    if (keyReleased && menuIdx == 1) {
      keyReleased = 0;
      if (axisPosition == AxisPosition::AXIS_LEFT) {
        pitch = max(pitch - 1, -2);
      } else if (axisPosition == AxisPosition::AXIS_RIGHT) {
        pitch = min(pitch + 1, 2);
      }

      delay(150);
    }

    // ADSR
    if (keyReleased && menuIdx == 2) {
      keyReleased = 0;
      if (axisPosition == AxisPosition::AXIS_LEFT) {
        adsrOption -= 1;
        if (adsrOption < 0) {
          adsrOption = ADSR_OPTION::SUSTAIN;
        }

      } else if (axisPosition == AxisPosition::AXIS_RIGHT) {
        adsrOption += 1;
        if (adsrOption >= MAX_ADSR) {
          adsrOption = ADSR_OPTION::SHORT;
        }
      }

      envelope.setAttack(ADSROptions[adsrOption][0]);
      envelope.setDecay(ADSROptions[adsrOption][1]);
      envelope.setSustain(ADSROptions[adsrOption][2]);
      envelope.setRelease(ADSROptions[adsrOption][3]);

      delay(150);
    }

    // Filter
    if (menuIdx == 3) {
      if (axisPosition == AxisPosition::AXIS_LEFT) {
        filterCutoff = max(filterCutoff - 10, 0);
      } else if (axisPosition == AxisPosition::AXIS_RIGHT) {
        filterCutoff = min(filterCutoff + 10, 500);
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
    populateScale(scale, baseKey, pitch);
    delay(200);

    currentMode = SynthMode::PLAY_MODE;
    chordToPlay = scale[0];
  }
}

void loop() {
  switch (currentMode) {
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
    displayInfo.mode = currentMode;
    displayInfo.outMode = digitalRead(OUT_MODE_PIN) == LOW ? OutMode::LINE_OUT
                                                           : OutMode::SPEAKERS;
    displayInfo.baseKey = String(getSemitoneLabel(baseKey).c_str());
    displayInfo.chord = String(chordToPlay->chord);

    displayInfo.menuIdx = menuIdx;
    displayInfo.adsr = adsrOption;
    displayInfo.pitch = pitch;
    displayInfo.filterCutoff = filterCutoff;

    xSemaphoreGive(mutex_display);
  }
}
