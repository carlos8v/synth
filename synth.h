#ifndef MAIN_H
#define MAIN_H

#include "AudioTools.h"
#include "AudioTools/AudioLibs/MaximilianDSP.h"

#include "axis.h"
#include "chords.h"
#include "config.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

I2SStream out;
Maximilian maximilian(out);

typedef enum {
  Base = 0,
  MajorMinor = 1,
  Major7Minor7 = 2,
  Sus2 = 3,
  Sus4 = 4,
} KeyModifier;

typedef enum {
  PlayMode = 0,
  ChordMode = 1,
} SynthMode;

volatile char display_buffer[SCREEN_WIDTH];
SemaphoreHandle_t mutex_display;

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

#endif
