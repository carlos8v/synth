#include "AudioTools.h"
#include "AudioTools/AudioLibs/MaximilianDSP.h"
#include "chords.h"
#include "config.h"

I2SStream out;
Maximilian maximilian(out);

int keyNotes[] = {0, 0, 0, 0, 0, 0, 0};

int lastChordIdx = -1;
int currentNote = 0;
int keyReleased = 0;

enum KeyModifier {
  Base = 0,
  MajorMinor = 1,
  Major7Minor7 = 2,
};
int lastKeyModifier = KeyModifier::Base;
int keyModifier = KeyModifier::Base;

// Current note index for sequencing
int currentCount;

maxiClock myClock;
maxiFilter filter;

maxiOsc osc[4];
maxiEnv envelope;

// For DEBUG
Chord **currentScale = e_major_scale;

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

  envelope.setAttack(0);
  envelope.setDecay(1);
  envelope.setSustain(1);
  envelope.setRelease(1000);

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
}

void playArpeggio(float *output, Chord *chord, int currentNote) {
  double single = osc[currentNote].sawn(chord->frequencies[currentNote]);
  double filtered = filter.lores(single, 1000, 0.8);  // Low-pass filter
  output[0] = output[1] = filtered;
}

void playChord(float *output, Chord *chord) {
  double out = 0;

  for (int i = 0; i < chord->keys; i++) {
    out += osc[i].sawn(chord->frequencies[i]);
  }

  out /= chord->keys;

  double filtered = filter.lores(out, 1000, 0.8);
  output[0] = output[1] = filtered;
}

void play(float *output) {
  myClock.ticker();  // Advance clock

  if (myClock.tick) {
    // For arpeggios
    currentNote = (currentNote + 1) % 3;  // Cycle through notes 0–3
  }

  int pressedIdx = -1;
  for (int idx = 0; idx < 7; idx++) {
    if (keyNotes[idx]) {
      pressedIdx = idx;
      break;
    }
  }

  // Can process key press
  if (keyReleased && pressedIdx >= 0) {
    // First chord to play, key or modifier change
    if (lastChordIdx < 0 || lastChordIdx != pressedIdx ||
        lastKeyModifier != keyModifier) {
      keyReleased = 0;
      lastChordIdx = pressedIdx;
      lastKeyModifier = keyModifier;

      // Unplay chord
    } else if (lastChordIdx == pressedIdx) {
      lastChordIdx = -1;
      keyReleased = 0;
      lastKeyModifier = KeyModifier::Base;
    }
  }

  if (pressedIdx < 0) {
    keyReleased = 1;
  }

  // Sustain chord
  if (lastChordIdx >= 0) {
    Chord *chordToPlay;

    switch (lastKeyModifier) {
      case KeyModifier::Base:
        chordToPlay = currentScale[lastChordIdx];
        break;
      case KeyModifier::MajorMinor:
        chordToPlay = currentScale[lastChordIdx]->major_minor != NULL
                          ? currentScale[lastChordIdx]->major_minor
                          : currentScale[lastChordIdx];
        break;
      case KeyModifier::Major7Minor7:
        chordToPlay = currentScale[lastChordIdx]->major7_minor7 != NULL
                          ? currentScale[lastChordIdx]->major7_minor7
                          : currentScale[lastChordIdx];
        break;
      default:
        chordToPlay = currentScale[lastChordIdx];
        break;
    }

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
}

void checkModifier() {
  int yValue = analogRead(MOD_PIN_Y);
  int yPercent = map(yValue, 0, 4095, 0, 100);

  // Neutral position
  if (yPercent >= 40 && yPercent <= 60) {
    keyModifier = KeyModifier::Base;
    // Upward position
  } else if (yPercent >= 60) {
    keyModifier = KeyModifier::MajorMinor;
    // keyModifier position
  } else if (yPercent <= 40) {
    keyModifier = KeyModifier::Major7Minor7;
  }
}

void loop() {
  maximilian.copy();  // Call the audio processing callback

  checkModifier();
  checkKeyPress();
}
