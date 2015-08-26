#include "heading.h"
#include <iostream>
using namespace std;

Heading::Heading(int address)
{
	this->address = address;
}


int Heading::Initialize()
{
	cout << "HEADING INIT here" << endl;
	int t;	
	memsBoard = wiringPiI2CSetup(address);
	t = wiringPiI2CWriteReg8(memsBoard,0x02,0x00);
	if(memsBoard < 0)
		return memsBoard;

	if(t < 0)
		return t;

	t = wiringPiI2CWriteReg8(memsBoard,0x01,0x20);
	if(t < 0)
		return t;

	return 0;
	
}


float CorrectHeading(float heading)
{
	float _0to90 = -100;
	float _91to180 = -136;
	float _181to270 = -100;
	float _270to360 = 20;

	if(heading >=100 && heading <= 100+90)
		return	heading + _0to90;

        if(heading >= 91 && heading <= 180)
                return  heading + _91to180;
        if(heading >=181 && heading <= 271)
                return  heading + _181to270;
        if(heading >=271 && heading <= 360)
                return  heading + _270to360;

	return heading;



}

float Heading::GetHeading()
{
	x  = wiringPiI2CReadReg8(memsBoard,0x03) << 8;
        x |=   wiringPiI2CReadReg8(memsBoard,0x04);

 	z = wiringPiI2CReadReg8(memsBoard,0x05) << 8;
        z |=   wiringPiI2CReadReg8(memsBoard,0x06);

	y =  wiringPiI2CReadReg8(memsBoard,0x07) << 8;
        y |=   wiringPiI2CReadReg8(memsBoard,0x08);

	fx = x;
	fy = y;
	fz = z;

	 float heading = atan2(fy,fx);

        float declinationAngle = 0.22;
  	heading += declinationAngle;
  	if(heading < 0)
    	heading += 2*PI;
	if(heading > 2*PI)
    	heading -= 2*PI;

	previousHeading = currentHeading;
  	currentHeading = heading * 180/M_PI;
	//currentHeading = CorrectHeading(currentHeading);

	return currentHeading;




}



bool Heading::HeadingReached(double heading)
{
        double b = GetHeading();
        if(fabs(b - heading) <= HEADINGDEADBAND)
                return true;
        else
                return false;
}




