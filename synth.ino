#include "AudioTools.h"
#include "AudioTools/AudioLibs/MaximilianDSP.h"
#include "config.h"
#include "notes.h"

I2SStream out;
Maximilian maximilian(out);

int keyNotes[] = { 0, 0, 0, 0, 0, 0, 0 };
int lastNoteIdx = -1;
int keyReleased = 0;
int currentNote = 0;

// Current note index for sequencing
int currentCount;

maxiClock myClock;
maxiFilter filter;

maxiOsc osc[4];
maxiEnv envelope;

// For DEBUG
Note **currentScale = e_major_scale;

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

  // Initialize notes references
  setupNotes();
}

void playArpeggio(float *output, Note *note, int currentNote) {
  double single = osc[currentNote].sawn(note->frequencies[currentNote]);
  double filtered = filter.lores(single, 1000, 0.8);  // Low-pass filter
  output[0] = output[1] = filtered;
}

void playNote(float *output, Note *note) {
  double chord;

  for (int i = 0; i < note->keys; i++) {
    chord += osc[i].sawn(note->frequencies[i]);
  }
  chord /= note->keys;

  double filtered = filter.lores(chord, 1000, 0.8);
  output[0] = output[1] = chord;
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
    // First note to play or switch note
    if (lastNoteIdx < 0 || lastNoteIdx != pressedIdx) {
      keyReleased = 0;
      lastNoteIdx = pressedIdx;

      // Unplay note
    } else if (lastNoteIdx == pressedIdx) {
      lastNoteIdx = -1;
      keyReleased = 0;
    }
  }

  if (pressedIdx < 0) {
    keyReleased = 1;
  }

  // Sustain key
  if (lastNoteIdx >= 0) {
    playNote(output, currentScale[lastNoteIdx]);
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

void loop() {
  maximilian.copy();  // Call the audio processing callback

  checkKeyPress();
}
