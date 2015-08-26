/************************************************
GPS Wrapper - Microstack GPS

Joey Thompson 4/21/2015
multi-threaded GPS program

***********************************************/
#include <string>
#include <sys/time.h>
using namespace std;
#include <pthread.h>
#include <unistd.h>                     //Used for UART
#include <fcntl.h>                      //Used for UART
#include <termios.h>            //Used for UART
#include <stdlib.h>
#include "TinyGPS++.h"
#include <math.h>

#define METERSTOINCHES 		39.3701
#define MINMETERS 		0.0836897
#define MININCHES 		MINMETERS*METERSTOINCHES
#define GPSINCHESDEADBAND 	MININCHES*3
#define GPSMETERSDEADBAND	.0836897
#define GPSHEADINGDEADBAND	.5

//Macro Waypoint Structure
struct WayPoint
{
        double lng;
        double lat;
        double alt;
        double heading;
};



class GPS
{

	public:
		GPS();
		~GPS();
		 static void * GPSMainThread(void *);
	
		GPS *thisGPS;	
		bool GetGPS();
		string GetGPStxt();
		bool CheckGPS();
		bool CalculateVars();
		double GetTimeStamp();
		double GetLapsedTime(double ts);
		int Initialize();
		int Start();
		bool Rx();
		void SetupUART();
		TinyGPSPlus tinyGPS;	
		double GetLat();
		double GetLong();
		double GetAlt();
		double GetAge();
		double GetHeading();
		double CalibrateAltitude();

		double DistanceBetween(WayPoint *wp1,WayPoint *wp2);
		double DistanceBetween(double lat1,double lng1,double lat2,double lng2);
		double DistanceBetween(WayPoint *wp,double lat,double lng);

		bool WayPointReached(WayPoint *wp);
		//bool HeadingReached(double heading);
		//bool HeadingReached(WayPoint *wp);


		
		
		
		string currentGPStxt;
		string previousGPStxt;
	
		double currentLat;
		double currentLong;
		double previousLat;
		double previousLong;
		double currentHeading;
		double previousHeading;
		double currentHeadingGPS;
                double previousHeadingGPS;
		double altitudeOffset;

		double currentAlt;
		double previousAlt;
		double velocityX;
		double velocityY;
		double velocityZ;
		double velocityR;
                bool bufferBlocked;
		bool shutDown;
                int bufferCount;
                int bufferIndex;
		pthread_t gpsThread;
		unsigned char rx_buffer[2001];
		double age;


		bool isLocked;
		bool readBlock;

		double traveledX;
		double traveledY;
		double traveledZ;

		timeval currentTime;
		double lastGPSCheck;
		double lapsedGPS;
		double prevLastGPSCheck;
		int uart0_filestream;
	
};



