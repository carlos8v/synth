#include "notes.h"

// E chords
// Emaj   (E3 - G#3 - B3)
Note *e_maj = makeNote("Emaj", 3, (double[]){164.81, 207.65, 246.94});
// Emaj7  (E3 - G#3 - B3 - D#4)
Note *e_maj7 = makeNote("Emaj7", 4, (double[]){164.81, 207.65, 246.94, 311.13});
// Emin   (E3 - G3 - B3)
Note *e_min = makeNote("Emin", 3, (double[]){164.81, 196.00, 246.94});
// Emin7  (E3 - G3 - B3 - D4)
Note *e_min7 = makeNote("Emin7", 4, (double[]){164.81, 196.00, 246.94, 293.66});

// F chords
// F#maj  (F#3 - A#3 - C#4)
Note *f_sharp_maj = makeNote("F#maj", 3, (double[]){184.99, 233.08, 277.18});
// F#maj7 (F#3 - A#3 - C#4 - F4)
Note *f_sharp_maj7 =
    makeNote("F#maj7", 4, (double[]){184.99, 233.08, 277.18, 349.23});
// F#min  (F#3 - A3 - C#4)
Note *f_sharp_min = makeNote("F#min", 3, (double[]){184.99, 220.00, 277.18});
// F#min7 (F#3 - A3 - C#4 - E4)
Note *f_sharp_min7 =
    makeNote("F#min7", 4, (double[]){184.99, 220.00, 277.18, 329.63});

// G chords
// G#min  (G#3 - C4 - D#4)
Note *g_sharp_maj = makeNote("G#maj", 3, (double[]){207.65, 277.18, 311.13});
// G#maj7 (G#3 - C4 - D#4 - G4)
Note *g_sharp_maj7 =
    makeNote("G#maj7", 4, (double[]){207.65, 277.18, 311.13, 392.00});
// G#min  (G#3 - B3 - D#4)
Note *g_sharp_min = makeNote("G#min", 3, (double[]){207.65, 246.94, 311.13});
// G#min7 (G#3 - B3 - D#4 - F#4)
Note *g_sharp_min7 =
    makeNote("G#min7", 4, (double[]){207.65, 246.94, 311.13, 369.99});

// A chords
// Amaj   (A3 - C#4 - E4)
Note *a_maj = makeNote("Amaj", 3, (double[]){220.00, 277.18, 329.63});
// Amaj7  (A3 - C#4 - E4 - G#4)
Note *a_maj7 = makeNote("Amaj7", 4, (double[]){220.00, 277.18, 329.63, 415.30});
// Amin   (A3 - C4 - E4)
Note *a_min = makeNote("Amin", 3, (double[]){220.00, 261.63, 329.63});
// Amin7  (A3 - C4 - E4 - G4)
Note *a_min7 = makeNote("Amin7", 4, (double[]){220.00, 261.63, 329.63, 392.00});

// B chords
// Bmaj   (B3 - D#4 - F#4)
Note *b_maj = makeNote("Bmaj", 3, (double[]){246.94, 311.13, 369.99});
// Bmaj7  (B3 - D#4 - F#4 - A#4)
Note *b_maj7 = makeNote("Bmaj7", 4, (double[]){246.94, 311.13, 369.99, 466.16});
// Bmin   (B3 - D4 - F#4)
Note *b_min = makeNote("Bmin", 3, (double[]){246.94, 293.66, 369.99});
// Bmin7  (B3 - D4 - F#4 - A4)
Note *b_min7 = makeNote("Bmin7", 4, (double[]){246.94, 293.66, 369.99, 440.00});

// C chords
// C#maj  (C#4 - F4 - G#4)
Note *c_sharp_maj = makeNote("C#maj", 3, (double[]){277.18, 349.23, 415.30});
// C#maj7 (C#4 - F4 - G#4 - C5)
Note *c_sharp_maj7 =
    makeNote("C#maj7", 4, (double[]){277.18, 349.24, 415.30, 523.25});
// C#min  (C#4 - E4 - G#4)
Note *c_sharp_min = makeNote("C#min", 3, (double[]){277.18, 329.63, 415.30});
// C#min7 (C#4 - F4 - G#4 - B4)
Note *c_sharp_min7 =
    makeNote("C#min7", 4, (double[]){277.18, 329.63, 415.30, 493.88});

// D chords
// Ddim   (D#4 - F#4 - A4)
Note *d_dim = makeNote("Ddim", 3, (double[]){311.14, 369.99, 440.00});

/*
 * Major progression:
 * I - ii - iii - IV - V - vi - d
 */

Note *e_major_scale[] = {e_maj, f_sharp_min, g_sharp_min, a_maj,
                         b_maj, c_sharp_min, d_dim};

Note *makeNote(std::string note, int keys, double frequencies[]) {
  Note *newNote = (Note *)malloc(sizeof(Note));
  newNote->note = note;
  newNote->keys = keys;

  for (int i = 0; i < keys; i++) {
    newNote->frequencies[i] = frequencies[i];
  }

  return newNote;
}

void setupNotes() {
  e_maj->major_minor = e_min;
  e_min->major_minor = e_maj;
  e_maj->major7_minor7 = e_min7;
  e_min->major7_minor7 = e_maj7;

  f_sharp_maj->major_minor = f_sharp_min;
  f_sharp_min->major_minor = f_sharp_maj;
  f_sharp_maj->major7_minor7 = f_sharp_min7;
  f_sharp_min->major7_minor7 = f_sharp_maj7;

  g_sharp_maj->major_minor = g_sharp_min;
  g_sharp_min->major_minor = g_sharp_maj;
  g_sharp_maj->major7_minor7 = g_sharp_min7;
  g_sharp_min->major7_minor7 = g_sharp_maj7;

  a_maj->major_minor = a_min;
  a_min->major_minor = a_maj;
  a_maj->major7_minor7 = a_min7;
  a_min->major7_minor7 = a_maj7;

  b_maj->major_minor = b_min;
  b_min->major_minor = b_maj;
  b_maj->major7_minor7 = b_min7;
  b_min->major7_minor7 = b_maj7;

  c_sharp_maj->major_minor = c_sharp_min;
  c_sharp_min->major_minor = c_sharp_maj;
  c_sharp_maj->major7_minor7 = c_sharp_min7;
  c_sharp_min->major7_minor7 = c_sharp_maj7;
}
