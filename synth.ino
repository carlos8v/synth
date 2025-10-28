#include "synth.h"

maxiOsc osc[4];
maxiClock myClock;
maxiFilter lowpass;
maxiEnv envelope;

SynthMode currentMode = SynthMode::PlayMode;

Axis axis(MOD_MAX_X, MOD_MAX_Y);
int modReleased = 1;

int keyNotes[] = {0, 0, 0, 0, 0, 0, 0};
int lastChordIdx = -1;
int currentNote = 0;
int keyReleased = 1;

Semitone currentTone = Semitone::C;
Chord* currentScale[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
Chord* chordToPlay = NULL;

void setup() {
  Serial.begin(115200);

  pinMode(KEY_1_PIN, INPUT_PULLUP);
  pinMode(KEY_2_PIN, INPUT_PULLUP);
  pinMode(KEY_3_PIN, INPUT_PULLUP);
  pinMode(KEY_4_PIN, INPUT_PULLUP);
  pinMode(KEY_5_PIN, INPUT_PULLUP);
  pinMode(KEY_6_PIN, INPUT_PULLUP);
  pinMode(KEY_7_PIN, INPUT_PULLUP);

  pinMode(MOD_KEY_PIN, INPUT_PULLUP);

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
}

void playArpeggio(float* output, Chord* chord) {
  double adsr = envelope.adsr(1., !keyReleased);

  double single = osc[currentNote].sawn(chord->frequencies[currentNote]);
  double filtered = lowpass.lores(single, adsr * 1000, 0.8);  // Low-pass filter

  output[0] = output[1] = filtered * adsr;
}

void playChord(float* output, Chord* chord) {
  double out = 0;

  double adsr = envelope.adsr(1., !keyReleased);

  for (int i = 0; i < chord->keys; i++) {
    out += osc[i].sawn(chord->frequencies[i]);
  }

  out /= chord->keys;
  out = lowpass.lores(out, adsr * 1000, 0.8);

  output[0] = output[1] = out * adsr;
}

void play(float* output) {
  myClock.ticker();  // Advance clock

  if (myClock.tick) {
    // For arpeggios
    currentNote = (currentNote + 1) % 3;  // Cycle through notes 0–3
  }

  if (currentMode != SynthMode::PlayMode) return;

  if (lastChordIdx >= 0) {
    // Sustain chord
    playChord(output, chordToPlay);
  }
}

void checkKeyPress() {
  keyNotes[0] = !digitalRead(KEY_1_PIN);
  keyNotes[1] = !digitalRead(KEY_2_PIN);
  keyNotes[2] = !digitalRead(KEY_3_PIN);
  keyNotes[3] = !digitalRead(KEY_4_PIN);
  keyNotes[4] = !digitalRead(KEY_5_PIN);
  keyNotes[5] = !digitalRead(KEY_6_PIN);
  keyNotes[6] = !digitalRead(KEY_7_PIN);

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

  // For DEBUG
  if (lastChordIdx >= 0 && !keyReleased) {
    Serial.println("Playing: " + String(chordToPlay->chord));
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
    case AxisPosition::AXIS_RIGHT:
      chordToPlay = currentScale[lastChordIdx]->sus4;
      break;
    case AxisPosition::AXIS_DOWN:
      chordToPlay = currentScale[lastChordIdx]->major7_minor7;
      break;
    case AxisPosition::AXIS_LEFT:
      chordToPlay = currentScale[lastChordIdx]->sus2;
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

  int modPressed = !digitalRead(MOD_KEY_PIN);
  if (!modPressed && !modReleased) {
    modReleased = 1;
  }

  if (modPressed && modReleased) {
    lastChordIdx = -1;
    currentMode = SynthMode::ChordMode;
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

  int modPressed = !digitalRead(MOD_KEY_PIN);
  if (!modPressed && !modReleased) {
    modReleased = 1;
  }

  if (modPressed && modReleased) {
    modReleased = 0;
    populateScale(currentScale, currentTone);
    delay(200);

    currentMode = SynthMode::PlayMode;
  }
}

void loop() {
  switch (currentMode) {
    case SynthMode::PlayMode:
      playMode();
      break;
    case SynthMode::ChordMode:
      chordMode();
    default:
      break;
  }
}
