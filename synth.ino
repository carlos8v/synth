#include "synth.h"

void displayScreen(void* parameter) {
  for (;;) {
    // Acquire mutex before accessing the shared buffer
    if (xSemaphoreTake(mutex_display, portMAX_DELAY) == pdTRUE) {
      switch (displayInfo.mode) {
        case SynthMode::PLAY_MODE:
          display.playMode(displayInfo);
          break;
        case SynthMode::CHORD_MODE:
          display.chordMode(displayInfo);
          break;
        default:
          break;
      }

      xSemaphoreGive(mutex_display);  // Release mutex
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(115200);

  Wire.setPins(DISPLAY_SDA, DISPLAY_SCL);
  Wire.begin();

  if (!display.begin()) {
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

  myClock.setTicksPerBeat(4);
  myClock.setTempo(100);

  envelope.setAttack(100);
  envelope.setDecay(200);
  envelope.setSustain(500);
  envelope.setRelease(800);

  auto cfg = out.defaultConfig(TX_MODE);
  cfg.is_master = true;       // ESP32 generates the clock
  cfg.pin_bck = I2S_BCK_PIN;  // Bit Clock (BCK)
  cfg.pin_ws = I2S_WS_PIN;    // Word Select / LRCK
  cfg.pin_data = I2S_DATA_PIN;
  cfg.sample_rate = 32000;
  cfg.buffer_size = 512;

  out.begin(cfg);
  maximilian.begin(cfg);

  // Initialize chords references
  setupChords();
  populateScale(currentScale, currentTone);
  chordToPlay = currentScale[0];

  displayInfo.tone = String(getSemitoneLabel(currentTone).c_str());
  displayInfo.chord = String(chordToPlay->chord);

  display.initConfig();
}

void playArpeggio(float* output, Chord* chord) {
  double adsr = envelope.adsr(1., !keyReleased);

  double single = osc[currentNote].sawn(chord->frequencies[currentNote]);
  double filtered = lowpass.lores(single, adsr * 1500, 0.8);  // Low-pass filter

  output[0] = output[1] = filtered * adsr;
}

void playChord(float* output, Chord* chord) {
  double out = 0;

  double adsr = envelope.adsr(1., !keyReleased);

  for (int i = 0; i < chord->keys; i++) {
    out += osc[i].sawn(chord->frequencies[i]);
  }

  out /= chord->keys;
  out = lowpass.lores(out, adsr * 1500, 0.8);

  output[0] = output[1] = out * adsr;
}

void play(float* output) {
  myClock.ticker();  // Advance clock

  if (myClock.tick) {
    // For arpeggios
    currentNote = (currentNote + 1) % 3;  // Cycle through notes 0–3
  }

  if (currentMode != SynthMode::PLAY_MODE) return;

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
      chordToPlay = currentScale[lastChordIdx]->major_minor;
      break;
    case AxisPosition::AXIS_UP_RIGHT:
      chordToPlay = currentScale[lastChordIdx]->seven;
      break;
    case AxisPosition::AXIS_RIGHT:
      chordToPlay = currentScale[lastChordIdx]->major7_minor7;
      break;
    case AxisPosition::AXIS_DOWN_RIGHT:
      chordToPlay = currentScale[lastChordIdx]->major9_minor9;
      break;
    case AxisPosition::AXIS_DOWN:
      chordToPlay = currentScale[lastChordIdx]->sus4;
      break;
    case AxisPosition::AXIS_DOWN_LEFT:
      chordToPlay = currentScale[lastChordIdx]->sus2;
      break;
    case AxisPosition::AXIS_LEFT:
      chordToPlay = currentScale[lastChordIdx]->dim;
      break;
    case AxisPosition::AXIS_UP_LEFT:
      chordToPlay = currentScale[lastChordIdx]->aug;
      break;
    default:
      chordToPlay = currentScale[lastChordIdx];
      break;
  }

  if (chordToPlay == NULL) {
    chordToPlay = currentScale[lastChordIdx];
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
    currentMode = SynthMode::CHORD_MODE;
    modReleased = 0;
    delay(200);
  }
}

void chordMode() {
  int x = analogRead(MOD_PIN_X);
  int y = analogRead(MOD_PIN_Y);

  AxisPosition axisPosition = axis.getPosition(x, y);

  switch (axisPosition) {
    case AxisPosition::AXIS_CENTER:
      // Reset key press on axis centered
      if (!keyReleased) {
        keyReleased = 1;
        delay(150);
      }
      break;

    case AxisPosition::AXIS_LEFT:
      if (keyReleased) {
        keyReleased = 0;
        currentTone = getPreviousSemitone(currentTone);
        delay(150);
      }
      break;

    case AxisPosition::AXIS_RIGHT:
      if (keyReleased) {
        keyReleased = 0;
        currentTone = getNextSemitone(currentTone);
        delay(150);
      }
      break;

    default:
      break;
  }

  int modPressed = digitalRead(MOD_KEY_PIN) == LOW;
  if (!modPressed && !modReleased) {
    modReleased = 1;
  }

  if (modPressed && modReleased) {
    modReleased = 0;
    populateScale(currentScale, currentTone);
    delay(200);

    currentMode = SynthMode::PLAY_MODE;
    chordToPlay = currentScale[0];
  }
}

void loop() {
  // Update the shared buffer safely
  if (xSemaphoreTake(mutex_display, portMAX_DELAY) == pdTRUE) {
    displayInfo.mode = currentMode;
    displayInfo.outMode = digitalRead(OUT_MODE_PIN) == LOW ? OutMode::LINE_OUT
                                                           : OutMode::SPEAKERS;
    displayInfo.tone = String(getSemitoneLabel(currentTone).c_str());
    displayInfo.chord = String(chordToPlay->chord);

    xSemaphoreGive(mutex_display);
  }

  switch (currentMode) {
    case SynthMode::PLAY_MODE:
      playMode();
      break;
    case SynthMode::CHORD_MODE:
      chordMode();
    default:
      break;
  }
}
