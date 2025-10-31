#ifndef MAIN_H
#define MAIN_H

#include "AudioTools.h"
#include "AudioTools/AudioLibs/MaximilianDSP.h"
#include "axis.h"
#include "chords.h"
#include "config.h"
#include "display.h"

Display display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS, OLED_RESET);
DisplayInfo displayInfo;
SemaphoreHandle_t mutex_display;

I2SStream out;
Maximilian maximilian(out);

typedef enum {
  Base = 0,
  MajorMinor = 1,
  Major7Minor7 = 2,
  Sus2 = 3,
  Sus4 = 4,
} KeyModifier;

maxiOsc osc[5];
maxiClock myClock;
maxiFilter lowpass;
maxiEnv envelope;

SynthMode currentMode = SynthMode::PlayMode;

Axis axis(MOD_MAX_X, MOD_MAX_Y);
int modReleased = 1;

int keyNotes[] = {0, 0, 0, 0, 0, 0, 0};
int lastChordIdx = 0;
int currentNote = 0;
int keyReleased = 1;

Semitone currentTone = Semitone::C;
Chord* currentScale[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
Chord* chordToPlay = NULL;

#endif
