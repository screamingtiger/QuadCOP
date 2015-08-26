#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <math.h>
#define SENSORS_GAUSS_TO_MICROTESLA       (100)
#define     PI 3.1415926535897932384626433832795
#define HEADINGADDRESS          0x1e
#define HEADINGDEADBAND		2



static float _hmc5883_Gauss_LSB_XY = 1100.0F;
static float _hmc5883_Gauss_LSB_Z  = 980.0F;



class Heading
{
	public:
	int memsBoard;
	int address;
	float fx,fy,fz;
	short int x,y,z;
	float currentHeading,previousHeading;

	Heading(int address);
	int Initialize();
	float GetHeading();
	bool HeadingReached(double);
	
};
