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

void Display::playMode(DisplayInfo displayInfo) {
  screen.clearDisplay();
  screen.setTextSize(1);

  screen.drawBitmap(
      0, 0,
      displayInfo.outMode == OutMode::LINE_OUT ? lineout_bmp : speaker_bmp,
      ICON_WIDTH, ICON_HEIGHT, 1);

  screen.setCursor(width - (TEXT_WIDTH * 3), 0);
  screen.print(displayInfo.tone);

  int x = (width / 2) - displayInfo.chord.length() * TEXT_WIDTH - 1;
  int y = (height / 2) - TEXT_HEIGHT;

  screen.setCursor(x, y);
  screen.setTextSize(2);
  screen.print(displayInfo.chord);

  screen.display();
}

void Display::chordMode(DisplayInfo displayInfo) {
  screen.clearDisplay();

  screen.fillRect(0, 0, width, height, SSD1306_WHITE);
  screen.fillRect(1, (height / 2) - (TEXT_HEIGHT * 2), width - 2,
                  TEXT_HEIGHT * 4, SSD1306_BLACK);

  int middleX = (width / 2);
  int middleY = (height / 2) - TEXT_HEIGHT;

  screen.setTextSize(1);
  screen.setTextColor(SSD1306_BLACK);
  screen.setCursor(middleX - (TEXT_WIDTH * 3) - 3, TEXT_HEIGHT);
  screen.print("Escala");

  screen.setTextSize(2);
  screen.setTextColor(SSD1306_WHITE);
  screen.setCursor(middleX - TEXT_WIDTH * displayInfo.tone.length(), middleY);
  screen.print(displayInfo.tone);

  int leftX = middleX - (TEXT_WIDTH * (4 + displayInfo.tone.length()));
  int rightX = middleX + (TEXT_WIDTH * (2 + displayInfo.tone.length()));

  screen.drawBitmap(leftX, middleY + 3, arrow_left_bmp, ICON_WIDTH, ICON_HEIGHT,
                    1);
  screen.drawBitmap(rightX, middleY + 3, arrow_right_bmp, ICON_WIDTH,
                    ICON_HEIGHT, 1);

  screen.display();
}
