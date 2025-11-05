#include "display.h"

Display::Display(Adafruit_SSD1306* _screen) {
  shouldDraw = true;
  screen = _screen;
  width = _screen->width();
  height = _screen->height();
}

bool Display::begin(uint8_t address) {
  return screen->begin(SSD1306_SWITCHCAPVCC, address);
}

void Display::initConfig(DisplayInfo displayInfo) {
  screen->setTextSize(1);
  screen->setTextColor(SSD1306_WHITE);
  screen->cp437(true);

  // Draw first screen
  shouldDraw = true;
  mainScreen(displayInfo);
}

void Display::mainScreen(DisplayInfo displayInfo) {
  // First draw or redraw whole screen on mode change
  if (shouldDraw || displayInfo.mode != lastInfo.mode) {
    shouldDraw = false;
    lastInfo.mode = displayInfo.mode;
    lastInfo.outMode = displayInfo.outMode;
    lastInfo.chord = displayInfo.chord;

    screen->clearDisplay();

    // Base key
    screen->setTextSize(1);
    screen->setCursor(width - (TEXT_WIDTH * 3), 0);
    screen->print(displayInfo.baseKey);

    // Out mode icon
    screen->drawBitmap(0, 0,
                       displayInfo.outMode == OutMode::LINE_OUT
                           ? top_left_lineout_bmp
                           : top_left_speaker_bmp,
                       10, 10, 1);

    // Chord
    int len = displayInfo.chord.length();
    int x = (width / 2) - len * TEXT_WIDTH - (len + 1);
    int y = (height / 2) - TEXT_HEIGHT;

    screen->setCursor(x, y);
    screen->setTextSize(2);
    screen->print(displayInfo.chord);

    screen->display();
    return;
  }

  // Update only out mode icon
  if (displayInfo.outMode != lastInfo.outMode) {
    shouldDraw = 1;
    lastInfo.outMode = displayInfo.outMode;

    screen->fillRect(0, 0, 10, 10, SSD1306_BLACK);  // Clear last icon
    screen->drawBitmap(0, 0,
                       displayInfo.outMode == OutMode::LINE_OUT
                           ? top_left_lineout_bmp
                           : top_left_speaker_bmp,
                       10, 10, 1);
  }

  // Update only chord
  if (!displayInfo.chord.equalsIgnoreCase(lastInfo.chord)) {
    shouldDraw = 1;
    lastInfo.chord = displayInfo.chord;

    int len = displayInfo.chord.length();
    int x = (width / 2) - len * TEXT_WIDTH - (len + 1);
    int y = (height / 2) - TEXT_HEIGHT;

    // Clear last chord
    screen->fillRect(0, y, width, TEXT_HEIGHT * 2, SSD1306_BLACK);

    screen->setCursor(x, y);
    screen->setTextSize(2);
    screen->print(displayInfo.chord);
  }

  if (shouldDraw) {
    shouldDraw = false;
    screen->display();
  }
}

char menuLabels[MAX_MENU_ITEMS][10] = {
    "Keynote",
    "Pitch",
    "ADSR",
    "Filter",
};

String ADSRLabels[4] = {"Short", "Swell", "Long", "Sustain"};

void Display::menuScreen(DisplayInfo displayInfo) {
  // Update last info
  lastInfo.mode = displayInfo.mode;

  int currentIdx = displayInfo.menuIdx;
  int previousIdx = currentIdx - 1 < 0 ? MAX_MENU_ITEMS - 1 : currentIdx - 1;
  int nextIdx = currentIdx + 1 >= MAX_MENU_ITEMS ? 0 : currentIdx + 1;

  screen->clearDisplay();

  screen->setTextSize(1);
  screen->setTextColor(SSD1306_WHITE);

  screen->drawBitmap(126, 0, scrollbar_bmp, 1, 62, 1);
  screen->fillRect(125, 64 / MAX_MENU_ITEMS * currentIdx, 3,
                   64 / MAX_MENU_ITEMS, SSD1306_WHITE);

  screen->drawBitmap(4, 2, menuIcons[previousIdx], 16, 16, 1);
  screen->setCursor(28, 6);
  screen->print(menuLabels[previousIdx]);

  screen->drawBitmap(0, 22, cursor_outline_bmp, 128, 21, 1);
  screen->drawBitmap(4, 24, menuIcons[currentIdx], 16, 16, 1);
  screen->setCursor(28, 28);
  screen->print(menuLabels[currentIdx]);

  screen->drawBitmap(4, 46, menuIcons[nextIdx], 16, 16, 1);
  screen->setCursor(28, 50);
  screen->print(menuLabels[nextIdx]);

  // Keynote
  if (displayInfo.menuIdx == 0) {
    screen->setCursor(100, 28);
    screen->print(displayInfo.baseKey);
    screen->drawBitmap(94, 28, arrow_left_bmp, 5, 7, 1);
    screen->drawBitmap(113, 28, arrow_right_bmp, 5, 7, 1);

    // Pitch
  } else if (displayInfo.menuIdx == 1) {
    screen->setCursor(100, 28);
    screen->print(String(displayInfo.pitch >= 0 ? '+' : '-') +
                  String(abs(displayInfo.pitch)));
    screen->drawBitmap(94, 28, arrow_left_bmp, 5, 7, 1);
    screen->drawBitmap(113, 28, arrow_right_bmp, 5, 7, 1);

    // ADSR
  } else if (displayInfo.menuIdx == 2) {
    int len = ADSRLabels[displayInfo.adsr].length();
    int adsrX = 113 - (len * TEXT_WIDTH) - (len + 1);

    screen->setCursor(adsrX, 28);
    screen->print(ADSRLabels[displayInfo.adsr]);
    screen->drawBitmap(adsrX - 6, 28, arrow_left_bmp, 5, 7, 1);
    screen->drawBitmap(113, 28, arrow_right_bmp, 5, 7, 1);

    // Filter
  } else if (displayInfo.menuIdx == 3) {
    screen->drawRect(90, 28, 20, 7, SSD1306_WHITE);
    screen->fillRect(91, 29, map(displayInfo.filterCutoff, 0, 500, 0, 19), 6,
                     SSD1306_WHITE);
    screen->drawBitmap(83, 28, arrow_left_bmp, 5, 7, 1);
    screen->drawBitmap(113, 28, arrow_right_bmp, 5, 7, 1);
  }

  screen->display();
}
