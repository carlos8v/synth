#ifndef NOTES
#define NOTES

#include <string>
#include <cstring>
#include <initializer_list>

typedef struct Note {
  char note[10];
  int keys;
  double frequencies[4];

  Note *major_minor;
  Note *major7_minor7;
} Note;

void setupNotes();
Note * makeNote(std::string note, std::initializer_list<double> frequencies);

extern Note *e_major_scale[];

#endif
