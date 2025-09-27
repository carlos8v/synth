#ifndef NOTES
#define NOTES

#include <string>
#include <stdio.h>
#include <stdlib.h>

typedef struct Note {
  std::string note;
  int keys;
  double frequencies[4];

  Note *major_minor;
  Note *major7_minor7;
} Note;

void setupNotes();
Note * makeNote(std::string note, int keys, double frequencies[]);

extern Note *e_major_scale[];

#endif
