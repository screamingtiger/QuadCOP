#include <string>
#include <iostream>

#include "ArduiPi_SSD1306.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"



using namespace std;


class OledScreen
{
public:
	OledScreen();
	void WriteText(string a);

	int currentX;
	int currentY;
	

	Adafruit_SSD1306 display;
};
