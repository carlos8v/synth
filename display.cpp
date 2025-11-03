#include "display.h"

Display::Display(uint8_t screenWidth, uint8_t screenHeight, uint8_t _address,
                 bool reset) {
  Adafruit_SSD1306 _display(screenWidth, screenHeight, &Wire, reset);
  screen = _display;
  address = _address;
  width = screenWidth;
  height = screenHeight;
}

bool Display::begin(void) {
  return screen.begin(SSD1306_SWITCHCAPVCC, address);
}

void Display::initConfig(void) {
  screen.clearDisplay();
  screen.setTextSize(1);
  screen.setTextColor(SSD1306_WHITE);
  screen.cp437(true);
}

void Display::mainScreen(DisplayInfo displayInfo) {
  screen.clearDisplay();
  screen.setTextSize(1);

  screen.drawBitmap(0, 0,
                    displayInfo.outMode == OutMode::LINE_OUT
                        ? top_left_lineout_bmp
                        : top_left_speaker_bmp,
                    10, 10, 1);

  screen.setCursor(width - (TEXT_WIDTH * 3), 0);
  screen.print(displayInfo.tone);

  int x = (width / 2) - displayInfo.chord.length() * TEXT_WIDTH - 1;
  int y = (height / 2) - TEXT_HEIGHT;

  screen.setCursor(x, y);
  screen.setTextSize(2);
  screen.print(displayInfo.chord);

  screen.display();
}

char menuLabels[MAX_MENU_ITEMS][10] = {
    "ADSR",
    "Keynote",
    "Pitch",
    "Oscil.",
};

String osciLabels[4] = {"Sawn", "Sine", "Trian.", "Squar."};

void Display::menuScreen(DisplayInfo displayInfo) {
  int currentIdx = displayInfo.menuIdx;
  int previousIdx = currentIdx - 1 < 0 ? MAX_MENU_ITEMS - 1 : currentIdx - 1;
  int nextIdx = currentIdx + 1 >= MAX_MENU_ITEMS ? 0 : currentIdx + 1;

  screen.clearDisplay();

  screen.setTextSize(1);
  screen.setTextColor(SSD1306_WHITE);

  screen.drawBitmap(126, 0, scrollbar_bmp, 1, 62, 1);
  screen.fillRect(125, 64 / MAX_MENU_ITEMS * currentIdx, 3, 64 / MAX_MENU_ITEMS,
                  SSD1306_WHITE);

  screen.drawBitmap(4, 2, menuIcons[previousIdx], 16, 16, 1);
  screen.setCursor(28, 6);
  screen.print(menuLabels[previousIdx]);

  screen.drawBitmap(0, 22, cursor_outline_bmp, 128, 21, 1);
  screen.drawBitmap(4, 24, menuIcons[currentIdx], 16, 16, 1);
  screen.setCursor(28, 28);
  screen.print(menuLabels[currentIdx]);

  screen.drawBitmap(4, 46, menuIcons[nextIdx], 16, 16, 1);
  screen.setCursor(28, 50);
  screen.print(menuLabels[nextIdx]);

  // TODO: ADSR config
  if (displayInfo.menuIdx == 0) {
    screen.setCursor(89, 28);
    screen.print("Long");
    screen.drawBitmap(83, 28, arrow_left_bmp, 5, 7, 1);
    screen.drawBitmap(113, 28, arrow_right_bmp, 5, 7, 1);
  }

  // Keynote
  if (displayInfo.menuIdx == 1) {
    screen.setCursor(100, 28);
    screen.print(displayInfo.tone);
    screen.drawBitmap(94, 28, arrow_left_bmp, 5, 7, 1);
    screen.drawBitmap(113, 28, arrow_right_bmp, 5, 7, 1);
  }

  // Pitch
  if (displayInfo.menuIdx == 2) {
    screen.setCursor(100, 28);
    screen.print(String(displayInfo.pitch >= 0 ? '+' : '-') +
                 String(abs(displayInfo.pitch)));
    screen.drawBitmap(94, 28, arrow_left_bmp, 5, 7, 1);
    screen.drawBitmap(113, 28, arrow_right_bmp, 5, 7, 1);
  }

  // Oscillator
  if (displayInfo.menuIdx == 3) {
    int len = osciLabels[displayInfo.osci].length();
    int osciX = 113 - (len * TEXT_WIDTH) - (len + 1);

    screen.setCursor(osciX, 28);
    screen.print(osciLabels[displayInfo.osci]);
    screen.drawBitmap(osciX - 6, 28, arrow_left_bmp, 5, 7, 1);
    screen.drawBitmap(113, 28, arrow_right_bmp, 5, 7, 1);
  }

  screen.display();
}
