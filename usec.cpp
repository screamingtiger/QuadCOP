#include <iostream>
#include <sys/time.h>
using namespace std;





long currentMS;
long previousMS = 0;
long currentS;
long previousS = 0;
double lastLapsed = 0;



timeval currentTime;

inline void GetTime()
{
        gettimeofday(&currentTime,NULL);
        previousMS = currentMS;
	previousS = currentS;
        currentMS = currentTime.tv_usec ;
	currentS = currentTime.tv_sec;
        lastLapsed = (currentS - previousS); 
	lastLapsed += (double)(currentMS - previousMS) / 1000000;
	
	
}

int main(void)
{


/*
 gettimeofday(&currentTime,NULL);
cout <<  currentTime.tv_sec << endl;
cout <<  currentTime.tv_usec << endl;
cout << (long)( currentTime.tv_sec*1000000 + currentTime.tv_usec);
*/	

double ts1,ts2;
gettimeofday(&currentTime,NULL);
ts1 = currentTime.tv_sec  + (double)currentTime.tv_usec/1000000;


while(1)
	{
		usleep(500000);
		gettimeofday(&currentTime,NULL);
		ts2 = currentTime.tv_sec  + (double)currentTime.tv_usec/1000000;	
		cout << ts2 - ts1 << endl;
		ts1 = ts2;

	}
	return 0;
}

