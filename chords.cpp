#include "chords.h"

const double A_3_BASE_FREQ = 220.00;

SemitoneListItem *firstSemitone;
SemitoneListItem *baseSemitone;

/*
 * Major progression:
 * I - ii - iii - IV - V - vi - d
 */
Chord *major_scale[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

SemitoneListItem *makeSemitone(Semitone tone, std::string label) {
  SemitoneListItem *newSemitone =
      (SemitoneListItem *)malloc(sizeof(SemitoneListItem));
  strcpy(newSemitone->label, label.c_str());
  newSemitone->tone = tone;
  newSemitone->next = NULL;

  return newSemitone;
}

void getSemitone(SemitoneListItem **target, Semitone tone) {
  while (1) {
    if ((*target)->tone == tone) {
      break;
    }

    (*target) = (*target)->next;
  }
}

void advanceSemitone(SemitoneListItem **target, int steps) {
  for (int i = 0; i < steps; i++) {
    (*target) = (*target)->next;
  }
}

double calcFrequency(double semitonesDist) {
  double exp = semitonesDist / 12.0;
  return A_3_BASE_FREQ * pow(2, exp);
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
  newChord->sus2 = NULL;
  newChord->sus4 = NULL;

  return newChord;
}

Chord *makeThreeKeyChord(std::string chord, Semitone destTone, int firstStep,
                         int secondStep, int higherOctave) {
  int octavePrefix = higherOctave;

  SemitoneListItem *firstTone = firstSemitone;
  getSemitone(&firstTone, destTone);

  double firstDistance =
      firstTone->tone + (octavePrefix * 12.0) - baseSemitone->tone;
  double firstFreq = calcFrequency(firstDistance);

  SemitoneListItem *secondTone = firstTone;
  advanceSemitone(&secondTone, firstStep);

  octavePrefix =
      firstTone->tone + firstStep >= 12 ? (octavePrefix + 1) : octavePrefix;
  double secondDistance =
      secondTone->tone + (octavePrefix * 12.0) - baseSemitone->tone;
  double secondFreq = calcFrequency(secondDistance);

  SemitoneListItem *thirdTone = secondTone;
  advanceSemitone(&thirdTone, secondStep);

  octavePrefix =
      secondTone->tone + secondStep >= 12 ? (octavePrefix + 1) : octavePrefix;
  double thirdDistance =
      thirdTone->tone + (octavePrefix * 12.0) - baseSemitone->tone;
  double thridFreq = calcFrequency(thirdDistance);

  return makeChord(chord, {firstFreq, secondFreq, thridFreq});
}

Chord *makeFourKeyChord(std::string chord, Semitone destTone, int firstStep,
                        int secondStep, int thirdStep, int higherOctave) {
  int octavePrefix = higherOctave;

  SemitoneListItem *firstTone = firstSemitone;
  getSemitone(&firstTone, destTone);

  int firstDistance =
      firstTone->tone + (octavePrefix * 12.0) - baseSemitone->tone;
  double firstFreq = calcFrequency(firstDistance);

  SemitoneListItem *secondTone = firstTone;
  advanceSemitone(&secondTone, firstStep);

  octavePrefix =
      firstTone->tone + firstStep >= 12 ? (octavePrefix + 1) : octavePrefix;
  int secondDistance =
      secondTone->tone + (octavePrefix * 12.0) - baseSemitone->tone;
  double secondFreq = calcFrequency(secondDistance);

  SemitoneListItem *thirdTone = secondTone;
  advanceSemitone(&thirdTone, secondStep);

  octavePrefix =
      secondTone->tone + secondStep >= 12 ? (octavePrefix + 1) : octavePrefix;
  int thirdDistance =
      thirdTone->tone + (octavePrefix * 12.0) - baseSemitone->tone;
  double thridFreq = calcFrequency(thirdDistance);

  SemitoneListItem *fourthTone = thirdTone;
  advanceSemitone(&fourthTone, thirdStep);

  octavePrefix =
      thirdTone->tone + thirdStep >= 12 ? (octavePrefix + 1) : octavePrefix;
  int fourthDistance =
      fourthTone->tone + (octavePrefix * 12.0) - baseSemitone->tone;
  double fourfhFreq = calcFrequency(fourthDistance);

  return makeChord(chord, {firstFreq, secondFreq, thridFreq, fourfhFreq});
}

Chord *makeMajorChord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 4, 3, higherOctave);
}

Chord *makeMajor7Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeFourKeyChord(chord, destTone, 4, 3, 4, higherOctave);
}

Chord *makeMinorChord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 3, 4, higherOctave);
}

Chord *makeMinor7Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeFourKeyChord(chord, destTone, 3, 4, 3, higherOctave);
}

Chord *makeSus2Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 2, 5, higherOctave);
}

Chord *makeSus4Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 5, 2, higherOctave);
}

Chord *makeDimChord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 3, 3, higherOctave);
}

void freeChord(Chord *chord) {
  if (chord->major_minor) {
    free(chord->major_minor);
  }

  if (chord->major7_minor7) {
    free(chord->major7_minor7);
  }

  free(chord);
}

// Only major scale
void populateScale(Semitone semitone) {
  SemitoneListItem *base = firstSemitone;
  getSemitone(&base, semitone);

  ChordType chordTypes[] = {
      ChordType::Major, ChordType::Minor, ChordType::Minor, ChordType::Major,
      ChordType::Major, ChordType::Minor, ChordType::Dim};
  int steps[] = {2, 2, 1, 2, 2, 2, 2};
  int octave = 0;

  for (int i = 0; i < 7; i++) {
    if (major_scale[i] != NULL) {
      freeChord(major_scale[i]);
    }

    std::string baseLabel = base->label;
    std::string majorMinorLabel = base->label;
    std::string major7Minor7Label = base->label;
    std::string dimLabel = base->label;

    std::string sus2Label = base->label;
    sus2Label += "sus2";
    std::string sus4Label = base->label;
    sus4Label += "sus4";

    switch (chordTypes[i]) {
      case ChordType::Major:
        baseLabel += "maj";
        majorMinorLabel += "min";
        major7Minor7Label += "min7";

        major_scale[i] = makeMajorChord(baseLabel, base->tone, octave);
        major_scale[i]->major_minor =
            makeMinorChord(majorMinorLabel, base->tone, octave);
        major_scale[i]->major7_minor7 =
            makeMinor7Chord(major7Minor7Label, base->tone, octave);
        major_scale[i]->sus2 = makeSus2Chord(sus2Label, base->tone, octave);
        major_scale[i]->sus4 = makeSus4Chord(sus4Label, base->tone, octave);
        break;
      case ChordType::Minor:
        baseLabel += "min";
        majorMinorLabel += "maj";
        major7Minor7Label += "maj7";

        major_scale[i] = makeMinorChord(baseLabel, base->tone, octave);
        major_scale[i]->major_minor =
            makeMajorChord(majorMinorLabel, base->tone, octave);
        major_scale[i]->major7_minor7 =
            makeMajor7Chord(major7Minor7Label, base->tone, octave);
        major_scale[i]->sus2 = makeSus2Chord(sus2Label, base->tone, octave);
        major_scale[i]->sus4 = makeSus4Chord(sus4Label, base->tone, octave);
        break;
      case ChordType::Dim:
        dimLabel += "dim";
        major_scale[i] = makeDimChord(dimLabel, base->tone, octave);
        break;
      default:
        major_scale[i] = NULL;
        break;
    }

    if (base->tone + steps[i] >= 12) {
      octave++;
    }

    if (i < 6) {
      advanceSemitone(&base, steps[i]);
    }
  }
}

void setupChords(Semitone initalTone) {
  std::string labels[] = {"C",  "C#", "D",  "D#", "E",  "F",
                          "F#", "G",  "G#", "A",  "A#", "B"};
  Semitone base_semitones[] = {
      Semitone::C,       Semitone::C_SHARP, Semitone::D,
      Semitone::D_SHARP, Semitone::E,       Semitone::F,
      Semitone::F_SHARP, Semitone::G,       Semitone::G_SHARP,
      Semitone::A,       Semitone::A_SHARP, Semitone::B,
  };

  SemitoneListItem *last, *current;
  for (int i = 0; i < 12; i++) {
    current = makeSemitone(base_semitones[i], labels[i]);

    // Chain semitones
    if (i == 0) {
      firstSemitone = current;
    } else {
      last->next = current;
    }

    // Set base semitone
    if (current->tone == Semitone::A) {
      baseSemitone = current;
    }

    last = current;
  }

  // Cycle back to the first semitone
  last->next = firstSemitone;

  // Initialize chords
  populateScale(initalTone);
}
