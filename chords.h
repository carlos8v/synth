#ifndef CHORDS
#define CHORDS

#include <cmath>
#include <cstring>
#include <initializer_list>
#include <string>

enum Semitone {
  C,
  C_SHARP,
  D,
  D_SHARP,
  E,
  F,
  F_SHARP,
  G,
  G_SHARP,
  A,
  A_SHARP,
  B,
};

enum ChordType {
  Major,
  Minor,
  Dim,
};

typedef struct SemitoneListItem {
  char label[3];
  Semitone tone;
  SemitoneListItem *next;
} SemitoneListItem;

typedef struct Chord {
  char chord[10];
  int keys;
  double frequencies[4];

  Chord *major_minor;
  Chord *major7_minor7;
  Chord *sus2;
  Chord *sus4;
} Chord;

void setupChords(Semitone initialTone);

extern Chord *major_scale[7];

#endif
