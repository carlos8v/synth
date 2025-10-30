#ifndef CHORDS
#define CHORDS

#include <cmath>
#include <cstring>
#include <initializer_list>
#include <string>

typedef enum {
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
} Semitone;

typedef enum {
  Major,
  Minor,
  Dim,
} ChordType;

typedef struct SemitoneListItem {
  char label[3];
  Semitone tone;
  SemitoneListItem* previous;
  SemitoneListItem* next;
} SemitoneListItem;

typedef struct Chord {
  char chord[10];
  int keys;
  double frequencies[4];

  Chord* major_minor;
  Chord* major7_minor7;
  Chord* sus2;
  Chord* sus4;
} Chord;

void setupChords();
void populateScale(Chord** scale, Semitone semitone);

std::string getSemitoneLabel(Semitone semitone);
Semitone getPreviousSemitone(Semitone semitone);
Semitone getNextSemitone(Semitone semitone);

#endif
