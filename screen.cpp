#include "screen.h"


int g = 0;


OledScreen::OledScreen()
{
        display.init(OLED_I2C_RESET,3);
        display.begin();

        display.clearDisplay();
        display.setCursor(0,0);
 	display.setTextColor(WHITE);
        display.setTextSize(1);
}

void OledScreen::WriteText(string a)
{
	display.setCursor(0,0);
	display.clearDisplay();
	display.print((char*)a.c_str());
	display.display();

	
}



/*
int main()
{
	OledScreen os;
	int i;
	string b(".");

	os.WriteText(b);
	while(1)
	{
		os.WriteText(b);
		i++;

	}
	return 0;
}

*/
/*
int main()
{

	display.init(OLED_I2C_RESET,3);
	display.begin();
	  // init done
 display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(100,0);
  display.clearDisplay();   // clears the screen and buffer


  // draw a single pixel
//  display.drawPixel(10, 10, WHITE);
 // display.display();
char p[30];
while(1)
{
display.clearDisplay();
display.setCursor(0,20);
 
	cin >> p;
      display.print(p);
	display.print("\n");
	 display.display();


}
  
*/



