#ifndef MAIN_H
#define MAIN_H

#include "AudioTools.h"
#include "AudioTools/AudioLibs/MaximilianDSP.h"
#include "axis.h"
#include "chords.h"
#include "config.h"
#include "display.h"

#define MAX_OSCI 4

typedef enum {
  SAWN_OSCI = 0,
  SINE_OSCI = 1,
  TRIANGLE_OSCI = 2,
  SQUARE_OSCI = 3,
} OSCI;

#define MAX_ADSR 4

typedef enum {
  SHORT = 0,
  SWELL = 1,
  LONG = 2,
  SUSTAIN = 3,
} ADSR_OPTION;

// Options for Attack, Decay, Sustain, Release
int ADSROptions[4][4] = {
    {1, 100, 500, 200},    // SHORT
    {800, 200, 500, 300},  // SWELL
    {100, 200, 500, 800},  // LONG
    {0, 0, 0, 0},          // SUSTAIN - ignore ADSR
};

Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Display display(&screen);

DisplayInfo displayInfo;
SemaphoreHandle_t mutex_display;

I2SStream out;
Maximilian maximilian(out);

maxiOsc osc[5];
maxiClock myClock;
maxiFilter hipass, lowpass;
maxiEnv envelope;

SynthMode currentMode = SynthMode::PLAY_MODE;

int menuIdx = 0;
int adsrOption = ADSR_OPTION::LONG;
int pitch = 0;
int osci = OSCI::SAWN_OSCI;
int filterCutoff = 200;

Axis axis(MOD_MAX_X, MOD_MAX_Y);
int modReleased = 1;

int keyNotes[] = {0, 0, 0, 0, 0, 0, 0};
int lastChordIdx = 0;
int currentNote = 0;
int keyReleased = 1;

Semitone baseKey = Semitone::C;
Chord* scale[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
Chord* chordToPlay = NULL;

#endif
