#include "chords.h"

const double A_3_BASE_FREQ = 220.00;

SemitoneListItem* firstSemitone;
SemitoneListItem* baseSemitone;

SemitoneListItem* makeSemitone(Semitone tone, std::string label) {
  SemitoneListItem* newSemitone =
      (SemitoneListItem*)malloc(sizeof(SemitoneListItem));
  strcpy(newSemitone->label, label.c_str());
  newSemitone->tone = tone;
  newSemitone->next = NULL;

  return newSemitone;
}

std::string getSemitoneLabel(Semitone semitone) {
  switch (semitone) {
    case Semitone::C:
      return "C";
    case Semitone::C_SHARP:
      return "C#";
    case Semitone::D:
      return "D";
    case Semitone::D_SHARP:
      return "D#";
    case Semitone::E:
      return "E";
    case Semitone::F:
      return "F";
    case Semitone::F_SHARP:
      return "F#";
    case Semitone::G:
      return "G";
    case Semitone::G_SHARP:
      return "G#";
    case Semitone::A:
      return "A";
    case Semitone::A_SHARP:
      return "A#";
    case Semitone::B:
      return "B";
    default:
      return "Error";
  }
}

void getSemitone(SemitoneListItem** base, Semitone tone) {
  while (1) {
    if ((*base)->tone == tone) {
      break;
    }

    *base = (*base)->next;
  }
}

void advanceSemitone(SemitoneListItem** base, int steps) {
  for (int i = 0; i < steps; i++) {
    *base = (*base)->next;
  }
}

float calcFrequency(int semitonesDist) {
  float exp = semitonesDist / 12.0;
  return A_3_BASE_FREQ * pow(2, exp);
}

Chord* makeChord(std::string chord, std::initializer_list<float> frequencies) {
  Chord* newChord = (Chord*)malloc(sizeof(Chord));

  int keys = 0;
  for (float freq : frequencies) {
    newChord->frequencies[keys] = freq;
    keys++;
  }

  newChord->keys = keys;
  memset(newChord->chord, 0, 10);
  strcpy(newChord->chord, chord.c_str());

  newChord->seven = NULL;
  newChord->major_minor = NULL;
  newChord->major7_minor7 = NULL;
  newChord->major9_minor9 = NULL;
  newChord->sus2 = NULL;
  newChord->sus4 = NULL;
  newChord->dim = NULL;
  newChord->aug = NULL;

  return newChord;
}

Chord* makeThreeKeyChord(std::string chord, Semitone destTone, int firstStep,
                         int secondStep, int pitch) {
  int pitchPrefix = pitch;

  SemitoneListItem* firstTone = firstSemitone;
  getSemitone(&firstTone, destTone);

  int firstDistance =
      firstTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float firstFreq = calcFrequency(firstDistance);

  SemitoneListItem* secondTone = firstTone;
  advanceSemitone(&secondTone, firstStep);

  pitchPrefix =
      firstTone->tone + firstStep >= 12 ? (pitchPrefix + 1) : pitchPrefix;
  int secondDistance =
      secondTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float secondFreq = calcFrequency(secondDistance);

  SemitoneListItem* thirdTone = secondTone;
  advanceSemitone(&thirdTone, secondStep);

  pitchPrefix =
      secondTone->tone + secondStep >= 12 ? (pitchPrefix + 1) : pitchPrefix;
  int thirdDistance =
      thirdTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float thridFreq = calcFrequency(thirdDistance);

  return makeChord(chord, {firstFreq, secondFreq, thridFreq});
}

Chord* makeFourKeyChord(std::string chord, Semitone destTone, int firstStep,
                        int secondStep, int thirdStep, int pitch) {
  int pitchPrefix = pitch;

  SemitoneListItem* firstTone = firstSemitone;
  getSemitone(&firstTone, destTone);

  int firstDistance =
      firstTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float firstFreq = calcFrequency(firstDistance);

  SemitoneListItem* secondTone = firstTone;
  advanceSemitone(&secondTone, firstStep);

  pitchPrefix =
      firstTone->tone + firstStep >= 12 ? (pitchPrefix + 1) : pitchPrefix;
  int secondDistance =
      secondTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float secondFreq = calcFrequency(secondDistance);

  SemitoneListItem* thirdTone = secondTone;
  advanceSemitone(&thirdTone, secondStep);

  pitchPrefix =
      secondTone->tone + secondStep >= 12 ? (pitchPrefix + 1) : pitchPrefix;
  int thirdDistance =
      thirdTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float thridFreq = calcFrequency(thirdDistance);

  SemitoneListItem* fourthTone = thirdTone;
  advanceSemitone(&fourthTone, thirdStep);

  pitchPrefix =
      thirdTone->tone + thirdStep >= 12 ? (pitchPrefix + 1) : pitchPrefix;
  int fourthDistance =
      fourthTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float fourfhFreq = calcFrequency(fourthDistance);

  return makeChord(chord, {firstFreq, secondFreq, thridFreq, fourfhFreq});
}

Chord* makeFiveKeyChord(std::string chord, Semitone destTone, int firstStep,
                        int secondStep, int thirdStep, int fourthStep,
                        int pitch) {
  int pitchPrefix = pitch;

  SemitoneListItem* firstTone = firstSemitone;
  getSemitone(&firstTone, destTone);

  int firstDistance =
      firstTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float firstFreq = calcFrequency(firstDistance);

  SemitoneListItem* secondTone = firstTone;
  advanceSemitone(&secondTone, firstStep);

  pitchPrefix =
      firstTone->tone + firstStep >= 12 ? (pitchPrefix + 1) : pitchPrefix;
  int secondDistance =
      secondTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float secondFreq = calcFrequency(secondDistance);

  SemitoneListItem* thirdTone = secondTone;
  advanceSemitone(&thirdTone, secondStep);

  pitchPrefix =
      secondTone->tone + secondStep >= 12 ? (pitchPrefix + 1) : pitchPrefix;
  int thirdDistance =
      thirdTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float thridFreq = calcFrequency(thirdDistance);

  SemitoneListItem* fourthTone = thirdTone;
  advanceSemitone(&fourthTone, thirdStep);

  pitchPrefix =
      thirdTone->tone + thirdStep >= 12 ? (pitchPrefix + 1) : pitchPrefix;
  int fourthDistance =
      fourthTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float fourfhFreq = calcFrequency(fourthDistance);

  SemitoneListItem* fifthTone = fourthTone;
  advanceSemitone(&fifthTone, fourthStep);

  pitchPrefix =
      fourthTone->tone + fourthStep >= 12 ? (pitchPrefix + 1) : pitchPrefix;
  int fifthDistance =
      fifthTone->tone + (pitchPrefix * 12.0) - baseSemitone->tone;
  float fifthFreq = calcFrequency(fifthDistance);

  return makeChord(chord,
                   {firstFreq, secondFreq, thridFreq, fourfhFreq, fifthFreq});
}

Chord* makeMajorChord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 4, 3, higherOctave);
}

Chord* make7Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeFourKeyChord(chord, destTone, 4, 3, 3, higherOctave);
}

Chord* makeMajor7Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeFourKeyChord(chord, destTone, 4, 3, 4, higherOctave);
}

Chord* makeMajor9Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeFiveKeyChord(chord, destTone, 4, 3, 4, 3, higherOctave);
}

Chord* makeMinorChord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 3, 4, higherOctave);
}

Chord* makeMinor7Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeFourKeyChord(chord, destTone, 3, 4, 3, higherOctave);
}

Chord* makeMinor9Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeFiveKeyChord(chord, destTone, 3, 4, 3, 4, higherOctave);
}

Chord* makeSus2Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 2, 5, higherOctave);
}

Chord* makeSus4Chord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 5, 2, higherOctave);
}

Chord* makeDimChord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 3, 3, higherOctave);
}

Chord* makeAugChord(std::string chord, Semitone destTone, int higherOctave) {
  return makeThreeKeyChord(chord, destTone, 4, 4, higherOctave);
}

void freeChord(Chord* chord) {
  if (chord->seven) free(chord->seven);
  if (chord->major_minor) free(chord->major_minor);
  if (chord->major7_minor7) free(chord->major7_minor7);
  if (chord->major9_minor9) free(chord->major9_minor9);
  if (chord->sus2) free(chord->sus2);
  if (chord->sus4) free(chord->sus4);
  if (chord->dim) free(chord->dim);
  if (chord->aug) free(chord->aug);

  free(chord);
}

// Only major scale
void populateScale(Chord** scale, Semitone semitone, int pitch) {
  SemitoneListItem* base = firstSemitone;
  getSemitone(&base, semitone);

  ChordType chordTypes[] = {
      ChordType::MAJOR, ChordType::MINOR, ChordType::MINOR, ChordType::MAJOR,
      ChordType::MAJOR, ChordType::MINOR, ChordType::DIM};
  int steps[] = {2, 2, 1, 2, 2, 2, 2};

  for (int i = 0; i < 7; i++) {
    if (scale[i] != NULL) {
      Chord* oldChord = scale[i];
      scale[i] = NULL;
      freeChord(oldChord);
    }

    std::string baseLabel = base->label;
    std::string sevenLabel = base->label;
    std::string majorMinorLabel = base->label;
    std::string major7Minor7Label = base->label;
    std::string major9Minor9Label = base->label;

    std::string sus2Label = base->label;
    std::string sus4Label = base->label;
    std::string dimLabel = base->label;
    std::string augLabel = base->label;

    sevenLabel += "7";
    sus2Label += "sus2";
    sus4Label += "sus4";
    dimLabel += "dim";
    augLabel += "aug";

    switch (chordTypes[i]) {
      case ChordType::MAJOR:
        baseLabel += "maj";
        majorMinorLabel += "min";
        major7Minor7Label += "maj7";
        major9Minor9Label += "maj9";

        scale[i] = makeMajorChord(baseLabel, base->tone, pitch);
        scale[i]->seven = make7Chord(sevenLabel, base->tone, pitch);
        scale[i]->major_minor =
            makeMinorChord(majorMinorLabel, base->tone, pitch);
        scale[i]->major7_minor7 =
            makeMajor7Chord(major7Minor7Label, base->tone, pitch);
        scale[i]->major9_minor9 =
            makeMajor9Chord(major9Minor9Label, base->tone, pitch);
        scale[i]->sus2 = makeSus2Chord(sus2Label, base->tone, pitch);
        scale[i]->sus4 = makeSus4Chord(sus4Label, base->tone, pitch);
        scale[i]->dim = makeDimChord(dimLabel, base->tone, pitch);
        scale[i]->aug = makeAugChord(augLabel, base->tone, pitch);
        break;
      case ChordType::MINOR:
        baseLabel += "min";
        majorMinorLabel += "maj";
        major7Minor7Label += "min7";
        major9Minor9Label += "min9";

        scale[i] = makeMinorChord(baseLabel, base->tone, pitch);
        scale[i]->seven = make7Chord(sevenLabel, base->tone, pitch);
        scale[i]->major_minor =
            makeMajorChord(majorMinorLabel, base->tone, pitch);
        scale[i]->major7_minor7 =
            makeMinor7Chord(major7Minor7Label, base->tone, pitch);
        scale[i]->major9_minor9 =
            makeMinor9Chord(major9Minor9Label, base->tone, pitch);
        scale[i]->sus2 = makeSus2Chord(sus2Label, base->tone, pitch);
        scale[i]->sus4 = makeSus4Chord(sus4Label, base->tone, pitch);
        scale[i]->dim = makeDimChord(dimLabel, base->tone, pitch);
        scale[i]->aug = makeAugChord(augLabel, base->tone, pitch);
        break;
      case ChordType::DIM:
        scale[i] = makeDimChord(dimLabel, base->tone, pitch);
        break;
      default:
        scale[i] = NULL;
        break;
    }

    if (base->tone + steps[i] >= 12) {
      pitch++;
    }

    if (i < 6) {
      advanceSemitone(&base, steps[i]);
    }
  }
}

void setupChords() {
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

    if (i == 0) {
      firstSemitone = current;
    } else {
      // Chain semitones
      last->next = current;
      current->previous = last;
    }

    // Set base semitone
    if (current->tone == Semitone::A) {
      baseSemitone = current;
    }

    last = current;
  }

  // Cycle back to the first semitone
  last->next = firstSemitone;
  firstSemitone->previous = last;
}

Semitone getPreviousSemitone(Semitone semitone) {
  SemitoneListItem* listTone = firstSemitone;
  getSemitone(&listTone, semitone);

  return listTone->previous->tone;
}

Semitone getNextSemitone(Semitone semitone) {
  SemitoneListItem* listTone = firstSemitone;
  getSemitone(&listTone, semitone);

  return listTone->next->tone;
}
