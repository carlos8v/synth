#ifndef MAIN_H
#define MAIN_H

#include "AudioTools.h"
#include "AudioTools/AudioLibs/MaximilianDSP.h"

#include "axis.h"
#include "chords.h"
#include "config.h"

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

#endif
