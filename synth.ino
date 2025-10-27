#include "AudioTools.h"
#include "AudioTools/AudioLibs/MaximilianDSP.h"
#include "axis.h"
#include "chords.h"
#include "config.h"

I2SStream out;
Maximilian maximilian(out);

int keyNotes[] = {0, 0, 0, 0, 0, 0, 0};

int lastChordIdx = -1;
int currentNote = 0;
int keyReleased = 1;

enum KeyModifier {
  Base = 0,
  MajorMinor = 1,
  Major7Minor7 = 2,
  Sus2 = 3,
  Sus4 = 4,
};
int keyModifier = KeyModifier::Base;
Axis axis(MOD_MAX_X, MOD_MAX_Y);

maxiClock myClock;
maxiFilter lowpass;

maxiOsc osc[4];
maxiEnv envelope;

// For DEBUG
Semitone initialTone = Semitone::C;
Chord** currentScale = major_scale;
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

  myClock.setTicksPerBeat(4);
  myClock.setTempo(100);

  envelope.setAttack(1);
  envelope.setDecay(1);
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
  setupChords(initialTone);
}

void playArpeggio(float* output, Chord* chord, int currentNote) {
  double single = osc[currentNote].sawn(chord->frequencies[currentNote]);
  double filtered = lowpass.lores(single, 1000, 0.8);  // Low-pass filter
  output[0] = output[1] = filtered;
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

  // Sustain chord
  if (lastChordIdx >= 0) {
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

  if (lastChordIdx >= 0 && !keyReleased) {
    Serial.println("Playing: " + String(chordToPlay->chord));
  }
}

void checkModifier() {
  AxisPosition axisPosition = axis.getPosition(analogRead(MOD_PIN_X), analogRead(MOD_PIN_Y));

  switch (axisPosition) {
    case AxisPosition::AXIS_UP:
      keyModifier = KeyModifier::MajorMinor;
      break;
    case AxisPosition::AXIS_RIGHT:
      keyModifier = KeyModifier::Sus4;
      break;
    case AxisPosition::AXIS_DOWN:
      keyModifier = KeyModifier::Major7Minor7;
      break;
    case AxisPosition::AXIS_LEFT:
      keyModifier = KeyModifier::Sus2;
      break;
    default:
      keyModifier = KeyModifier::Base;
      break;
  }
}

void updateChordToPlay() {
  switch (keyModifier) {
    case KeyModifier::Base:
      chordToPlay = currentScale[lastChordIdx];
      break;
    case KeyModifier::MajorMinor:
      chordToPlay = currentScale[lastChordIdx]->major_minor;
      break;
    case KeyModifier::Major7Minor7:
      chordToPlay = currentScale[lastChordIdx]->major7_minor7;
      break;
    case KeyModifier::Sus2:
      chordToPlay = currentScale[lastChordIdx]->sus2;
      break;
    case KeyModifier::Sus4:
      chordToPlay = currentScale[lastChordIdx]->sus4;
      break;
    default:
      chordToPlay = currentScale[lastChordIdx];
      break;
  }

  if (chordToPlay == NULL) {
    chordToPlay = currentScale[lastChordIdx];
  }
}

void loop() {
  maximilian.copy();  // Call the audio processing callback

  checkModifier();
  checkKeyPress();
  updateChordToPlay();
}
