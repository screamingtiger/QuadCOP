#include "gps.h"
#include <iostream>
using namespace std;
GPS::GPS()
{

}

GPS::~GPS()
{
	bufferBlocked = false;
	readBlock = false;
	shutDown = false;
        bufferCount = 0;
        bufferIndex = 0;
	altitudeOffset = 0;
        thisGPS = this;
	age = 0;

}


string GPS::GetGPStxt()
{
	string r;

	return r;
}

double GPS::GetLat()
{
        double l;
        readBlock = true;
	l = currentLat;
        readBlock = false;
        return l;
}



double GPS::GetLong()
{
        double l;
        readBlock = true;
        l = currentLong;
        readBlock = false;
        return l;
}

double GPS::GetAlt()
{
        double l;
        readBlock = true;
        l = currentAlt - altitudeOffset;
        readBlock = false;
        return l;
}


double GPS::GetHeading()
{
        double l;
        readBlock = true;
        l = currentHeading;
        readBlock = false;
        return l;
}


double GPS::GetAge()
{
	age = GetLapsedTime(lastGPSCheck);
	return age;
}	

bool GPS::GetGPS()
{
	while(readBlock);
        previousLat = currentLat;
        previousLong = currentLong;
	previousAlt = currentAlt;
	previousHeadingGPS = currentHeadingGPS;

        currentLat = tinyGPS.location.lat();
        currentLong = tinyGPS.location.lng();
	currentAlt = tinyGPS.altitude.feet();
	currentHeadingGPS = tinyGPS.course.deg();
	prevLastGPSCheck = lastGPSCheck;
	lastGPSCheck = GetTimeStamp();

        return true;
}



double GPS::DistanceBetween(WayPoint *wp1,WayPoint *wp2)
{
	return METERSTOINCHES*tinyGPS.distanceBetween(wp1->lng,wp2->lat,wp2->lng,wp2->lat);
}


double GPS::DistanceBetween(double lat1,double lng1,double lat2,double lng2)
{

        return METERSTOINCHES*tinyGPS.distanceBetween(lat1,lng1,lat2,lng2);
}


double GPS::DistanceBetween(WayPoint *wp,double lat,double lng)
{

        return METERSTOINCHES*tinyGPS.distanceBetween(wp->lat,wp->lng,lat,lng);
}




bool GPS::WayPointReached(WayPoint *wp)
{
	double distance = DistanceBetween(wp,GetLat(),GetLong());
	if(distance <= GPSINCHESDEADBAND)
		return true;
	else
		return false; 
}

/*
bool GPS::HeadingReached(double heading)
{
	double b = GetHeading();
	if(fabs(b - heading) <= GPSHEADINGDEADBAND)
		return true;
	else
		return false;
}



bool GPS::HeadingReached(WayPoint *wp)
{
	return HeadingReached(wp->heading);

}
*/

bool GPS::CheckGPS()
{
	isLocked = true;
	return isLocked;

}



double GPS::CalibrateAltitude()
{
	altitudeOffset = GetAlt();
}


bool GPS::CalculateVars()
{
	double gpsLapsed = prevLastGPSCheck - lastGPSCheck;
	double deltaX;
	double deltaY;
	double deltaZ;


	deltaX = abs(currentLat - previousLat);
	deltaY = abs(currentLong = previousLong);

	if(currentLat >= previousLat)	
		traveledX = deltaX;
	else
		traveledX = deltaX*-1;
	
	if(currentLong >= previousLong)
		traveledY = deltaY;
	else
		traveledY = deltaY*-1;

	


	
	velocityX = traveledX / lapsedGPS; 
	velocityY = traveledY / lapsedGPS;
	velocityZ = -.56;
	return true;

}



double GPS::GetTimeStamp()
{
        double ts;
        gettimeofday(&currentTime,NULL);
        ts =  currentTime.tv_sec;
        ts +=  (double)currentTime.tv_usec / 1000000;
        return ts;

}

double GPS::GetLapsedTime(double timeStamp)
{
        double ts;
        gettimeofday(&currentTime,NULL);
        ts =  currentTime.tv_sec;
        ts +=  (double)currentTime.tv_usec / 1000000;
        return ts - timeStamp;



}


void * GPS::GPSMainThread(void *arg)
{
	GPS *gps = (GPS*)arg;
	
	if(gps == NULL)
		cerr << "UNABLE TO ATTACH GPS OBJECT" << endl;
	gps->SetupUART();
	cout << "here" << endl;
	while(!gps->shutDown)
	{
		
		if(gps->Rx())
		{
			for(int i=0;i<gps->bufferCount;i++)
			{
				if(gps->tinyGPS.encode(gps->rx_buffer[i]))
				{
					gps->GetGPS();
				}
				
			}
			gps->bufferBlocked = false;
		}	

	}


	return NULL;
}

int GPS::Initialize()
{
        return 0;
}

int GPS::Start()
{
	pthread_create(&(gpsThread),NULL,this->GPSMainThread,this);
        return 0;

}


void GPS::SetupUART()
{
//-------------------------
//----- SETUP USART 0 -----
//-------------------------
//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively

//OPEN THE UART
//The flags (defined in fcntl.h):
//      Access modes (use 1 of these):
//              O_RDONLY - Open for reading only.
//              O_RDWR - Open for reading and writing.
//              O_WRONLY - Open for writing only.
//
//      O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
        //if there is no input immediately available (instead of blocking). Likewise, write requests can also return
        //immediately with a failure status if the output can't be written immediately.
        //
        //O_NOCTTY - When set and path identifies a terminal device, open() shall not
        //cause the terminal device to become the controlling terminal for the process.


        uart0_filestream = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);          //Open in non blocking read/write mode
        if (uart0_filestream == -1)
        {
                //ERROR - CAN'T OPEN SERIAL PORT
                //printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
        }

        //CONFIGURE THE UART
        //The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
        //      Baud rate:- B1200, B2400, B4800, B9600,
        //B19200, B38400, B57600,
        ///B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
        //      CSIZE:- CS5, CS6, CS7, CS8
        //      CLOCAL - Ignore modem status lines
        //      CREAD - Enable receiver
        //      IGNPAR = Ignore characters with parity errors
        //      ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
        //      PARENB - Parity enable
        //      PARODD - Odd parity (else even)

        struct termios options;
        tcgetattr(uart0_filestream, &options);
        options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;         //<Set baud rate
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(uart0_filestream, TCIFLUSH);
        tcsetattr(uart0_filestream, TCSANOW, &options);
}



bool GPS::Rx()
{
	if(bufferBlocked)
		return false;

        //----- CHECK FOR ANY RX BYTES -----
        if (uart0_filestream != -1)
        {
                // Read up to 2000 characters from the port if they are there
                bufferCount = read(uart0_filestream, (void*)rx_buffer, 2000);          //Filestream, buffer to store in, number of bytes to read (max)
                if (bufferCount < 0)
                {
                        //An error occured (will occur if there are no bytes)
			return false;
                }
                else if (bufferCount == 0)
                {
                        //No data waiting
			return false;
                }
                else
                {
                        rx_buffer[bufferCount] = '\0';
			bufferBlocked = true;
			return true;
                }
        }


}
