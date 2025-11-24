#ifndef MAIN_H
#define MAIN_H

#include "AudioTools.h"
#include "AudioTools/AudioLibs/MaximilianDSP.h"
#include "axis.h"
#include "chords.h"
#include "config.h"
#include "audio/click.h"
#include "display.h"

#define MAX_ADSR 4

typedef enum {
  SHORT = 0,
  SWELL = 1,
  LONG = 2,
  SUSTAIN = 3,
} ADSR_OPTION;

// Options for Attack, Decay, Sustain, Release
int ADSROptions[4][4] = {
    {100, 100, 500, 400},   // SHORT
    {1000, 100, 300, 600},  // SWELL
    {100, 300, 500, 1000},  // LONG
    {0, 0, 0, 0},           // SUSTAIN - ignore ADSR
};

Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Display display(&screen);

DisplayInfo displayInfo;
SemaphoreHandle_t mutex_display;

I2SStream out;
Maximilian maximilian(out);

// TODO: handle clock
// maxiClock myClock;
maxiOsc osc[5], oscPitch[2], lfo;
maxiFilter hipass, lowpass;
maxiEnv envelope;
maxiSample clickEffect;

SynthMode currentMode = SynthMode::PLAY_MODE;

int soundEffect = 0;
int menuIdx = 0;
int adsrOption = ADSR_OPTION::LONG;
int pitch = 0;
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
