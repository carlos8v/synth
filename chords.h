#ifndef CHORDS
#define CHORDS

#include <cmath>
#include <cstring>
#include <initializer_list>
#include <string>

enum Semitone {
  C_4,
  C_SHARP_4,
  D_4,
  D_SHARP_4,
  E_4,
  F_4,
  F_SHARP_4,
  G_4,
  G_SHARP_4,
  A_4,
  A_SHARP_4,
  B_4,
};

typedef struct SemitoneListItem {
  Semitone tone;
  SemitoneListItem *next;
} SemitoneListItem;

typedef struct Chord {
  char chord[10];
  int keys;
  double frequencies[4];

  Chord *major_minor;
  Chord *major7_minor7;
} Chord;

void setupChords();

extern Chord *c_major_scale[7];
extern Chord *d_major_scale[7];
extern Chord *e_major_scale[7];
extern Chord *f_major_scale[7];
extern Chord *g_major_scale[7];
extern Chord *a_major_scale[7];
extern Chord *b_major_scale[7];

#endif
