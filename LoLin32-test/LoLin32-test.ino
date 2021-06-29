#include "SSD1306.h"

SSD1306 display(0x3c, 5, 4);

void setup() {
  display.init();
  display.setFont(ArialMT_Plain_24);
  display.setColor(WHITE);
 // display.setTestAlignment(TEST_ALIGN_CENTER);
  display.drawString(20, 20, "Ping");

  display.display();

}

void loop() {
  // put your main code here, to run repeatedly:

}
