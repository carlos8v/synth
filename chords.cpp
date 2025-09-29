#include "chords.h"

const double A_4_BASE_FREQ = 440.00;

SemitoneListItem *firstSemitone;
SemitoneListItem *baseSemitone;

/*
 * Major progression:
 * I - ii - iii - IV - V - vi - d
 */

Chord *c_major_scale[7];
Chord *d_major_scale[7];
Chord *e_major_scale[7];
Chord *f_major_scale[7];
Chord *g_major_scale[7];
Chord *a_major_scale[7];
Chord *b_major_scale[7];

SemitoneListItem *makeSemitone(Semitone tone) {
  SemitoneListItem *newSemitone =
      (SemitoneListItem *)malloc(sizeof(SemitoneListItem));
  newSemitone->tone = tone;
  newSemitone->next = NULL;

  return newSemitone;
}

SemitoneListItem *getSemitone(Semitone tone) {
  SemitoneListItem *current = firstSemitone;
  while (1) {
    if (current->tone == tone) {
      break;
    }

    current = current->next;
  }

  return current;
}

SemitoneListItem *advanceSemitone(SemitoneListItem *base, int steps) {
  for (int i = 0; i < steps; i++) {
    base = base->next;
  }

  return base;
}

double calcFrequency(double freq, int semitones) {
  double exp = (double)semitones / 12.0;
  return freq * pow(2, exp);
}

Chord *makeChord(std::string chord, std::initializer_list<double> frequencies) {
  Chord *newChord = (Chord *)malloc(sizeof(Chord));

  int keys = 0;
  for (double freq : frequencies) {
    newChord->frequencies[keys] = freq;
    keys++;
  }

  newChord->keys = keys;
  strcpy(newChord->chord, chord.c_str());

  newChord->major_minor = NULL;
  newChord->major7_minor7 = NULL;

  return newChord;
}

Chord *makeThreeKeyChord(std::string chord, Semitone destTone, int firstStep,
                         int secondStep, int higherOctave) {
  int octavePrefix = higherOctave ? 12 : 0;
  SemitoneListItem *firstTone = getSemitone(destTone);
  int firstDistance = (firstTone->tone + octavePrefix) - baseSemitone->tone;
  double firstFreq = calcFrequency(A_4_BASE_FREQ, firstDistance);

  octavePrefix = firstTone->tone + firstStep >= 12 ? 12 : 0;
  SemitoneListItem *secondTone = advanceSemitone(firstTone, firstStep);
  int secondDistance = (secondTone->tone + octavePrefix) - firstTone->tone;
  double secondFreq = calcFrequency(firstFreq, secondDistance);

  octavePrefix = secondTone->tone + secondStep >= 12 ? 12 : 0;
  SemitoneListItem *thirdTone = advanceSemitone(secondTone, secondStep);
  int thridDistance = (thirdTone->tone + octavePrefix) - secondTone->tone;
  double thridFreq = calcFrequency(secondFreq, thridDistance);

  return makeChord(chord, {firstFreq, secondFreq, thridFreq});
}

Chord *makeMajorChord(std::string chord, Semitone destTone, int octaveHigher) {
  return makeThreeKeyChord(chord, destTone, 4, 3, octaveHigher);
}

Chord *makeMinorChord(std::string chord, Semitone destTone, int octaveHigher) {
  return makeThreeKeyChord(chord, destTone, 3, 4, octaveHigher);
}

Chord *makeDimChord(std::string chord, Semitone destTone, int octaveHigher) {
  return makeThreeKeyChord(chord, destTone, 3, 3, octaveHigher);
}

void setupChords() {
  SemitoneListItem *last, *current;
  Semitone base_semitones[] = {
      Semitone::C_4,       Semitone::C_SHARP_4, Semitone::D_4,
      Semitone::D_SHARP_4, Semitone::E_4,       Semitone::F_4,
      Semitone::F_SHARP_4, Semitone::G_4,       Semitone::G_SHARP_4,
      Semitone::A_4,       Semitone::A_SHARP_4, Semitone::B_4,
  };

  for (int i = 0; i < 12; i++) {
    current = makeSemitone(base_semitones[i]);

    // Chain semitones
    if (i == 0) {
      firstSemitone = current;
    } else {
      last->next = current;
    }

    // Set base semitone
    if (current->tone == Semitone::A_4) {
      baseSemitone = current;
    }

    last = current;
  }

  // Cycle back to the first semitone
  last->next = firstSemitone;

  // Initialize scales
  e_major_scale[0] = makeMajorChord("Emaj", Semitone::E_4, 0);
  e_major_scale[1] = makeMinorChord("F#min", Semitone::F_SHARP_4, 0);
  e_major_scale[2] = makeMinorChord("G#min", Semitone::G_SHARP_4, 0);
  e_major_scale[3] = makeMajorChord("Amaj", Semitone::A_4, 0);
  e_major_scale[4] = makeMajorChord("Bmaj", Semitone::B_4, 0);
  e_major_scale[5] = makeMinorChord("C#min", Semitone::C_SHARP_4, 1);
  e_major_scale[6] = makeDimChord("D#dim", Semitone::D_SHARP_4, 1);

  // Free unused pointers
  free(last);
  free(firstSemitone);
  free(baseSemitone);
}
