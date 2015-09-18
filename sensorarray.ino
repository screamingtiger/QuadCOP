/********************************************************
	SensorArray
	Written By Joey Thompson 2015
	Sponsored by Element 14 "Sci Fi Your Pi" contest"

	Compiles under Raspberry Pi (TM) for Arduino Nano or Mini (TM).

*********************************************************/


#include <wire.h>

/********************************************
Sensors Attached:
Note, all pings are tiggered the same time and only 1 is read at a time.
Ping Triggers are on pin 1
Ping1 Echo 2
Ping2 Echo 3		
Ping3 Echo 6	
Ping4 Echo 7
Flame	A0
LED	A1
SDA	4
SCL	5

*********************************************/

//Define Pin Configuration
#define PING1TRIGGER 1	
#define PING1ECHO	2	
#define PING2TRIGGER	1
#define PING2ECHO	3
#define PING3TRIGGER	1
#define PING3ECHO	6
#define PING4TRIGGER	1
#define PING4ECHO	7
#define FLAMESENSOR	A0
#define LED		A1
#define SDA		4
#define SCL		5

#define MAXQ 		100

//I2C ID
#define I2C_SENSORARRAY_ID 5


//Global Queues
//holds the # of inches to detected target
char pingSensor1Q[];
char pingSensor2Q[];
char pingSensor3Q[];
char pingSensor4Q[];
char flameSensorQ[];


//Pointers for sending values
int pingSensor1QReadIndex;
int pingSensor2QReadIndex;
int pingSensor3QReadIndex;
int pingSensor4QReadIndex;
int flameSensorQReadIndex;

//pointers for reading values
int pingSensor1QWriteIndex;
int pingSensor2QWriteIndex;
int pingSensor3QWriteIndex;
int pingSensor4QWriteIndex;
int flameSensorQWriteIndex;



//NEW block method of sending data
#define STARTBLOCK	204
#define STOPBLOCK	190
#define RESETBLOCK	195

bool blockStarted = false;

int block[10];
int blockCounter = 0;
bool blockCompleted = false;
bool blockBlock = false;
int blockSkipped = 0 ;


//I2C block algorithm
//define registers for commands
#define REGPING1	1
#define REGPING2	2
#define REGPING3	3
#define REGPING4	4
#define REGFIRE	5
#define REGLED	6
#define CLEARQ	7



//Recieved and processes I2C bytes as they come in
//Set block variables as required top indicate block status
void I2CReceiveEventBlock(int numBytes)
{
        //Every 2 bytes are our data pairs, writes come in groups of 3

        unsigned char cb, cbc, reg;
	cb = Wire.receive();
	if(!blockBlock)
	{
		
                //Block Start
                if(cb == 204)
		{
			blockStarted  = true;
			blockCounter = 0;
			return;
		}

                //Block End
		if(cb == 190)
		{
			if(blockStarted)
			{
				blockStarted = false;
				blockCompleted = true;
				blockBlock = true;
				return;
			}
		}
                //Block Reset
                if(cb == 195)
                {
                   blockStarted = false;
                   blockCompleted = false;
                   blockCounter = 0;
                }
	
                //Data
		if(blockStarted)
		{
			block[blockCounter++] = cb;
			blockCounter %= 10;
		}
	}
        else
          blockSkipped++;

}




//Given a Q #, sends data from that Q and increments read counter
void SendQ(int i)
{
	if(i == 1)
	{
		write.write(pingSensor1Q[pingSensor1QReadIndex]);
		//zero out value.
		pingSensor1Q[pingSensor1QReadIndex++] = 0;
		pingSensor1QReadIndex %= MAXQ;
		
	}

	if(i == 2)
	{
		write.write(pingSensor2Q[pingSensor2QReadIndex]);
		//zero out value.
		pingSensor2Q[pingSensor2QReadIndex++] = 0;
		pingSensor2QReadIndex %= MAXQ;
		
	}


	if(i == 3)
	{
		write.write(pingSensor3Q[pingSensor3QReadIndex]);
		//zero out value.
		pingSensor3Q[pingSensor3QReadIndex++] = 0;
		pingSensor3QReadIndex %= MAXQ;
		
	}


	if(i == 4)
	{
		write.write(pingSensor4Q[pingSensor4QReadIndex]);
		//zero out value.
		pingSensor4Q[pingSensor4QReadIndex++] = 0;
		pingSensor4QReadIndex %= MAXQ;
		
	}


	if(i == 5)
	{
		write.write(fireSensorQ[fireSensorQReadIndex]);
		//zero out value.
		fireSensorQ[fireSensorQReadIndex++] = 0;
		fireSensorQReadIndex %= MAXQ;
		
	}

	
}


//Clears all the Qs and sets values to 0
//This way if data is sent that doesnt exist yet the RPFS just gets a 0
void ClearQs()
{
	pingSensor1QReadIndex = 0;
	pingSensor2QReadIndex = 0;
	pingSensor3QReadIndex = 0;
	pingSensor4QReadIndex = 0;
	flameSensorQReadIndex = 0;
	
	pingSensor1QWriteIndex = 0;
	pingSensor2QWriteIndex = 0;
	pingSensor3QWriteIndex = 0;
	pingSensor4QWriteIndex = 0;
	flameSensorQWriteIndex = 0;


	//zero out all the values;
	for(int i=0;i< MAXQ)
	{
		pingSensor1Q[i] = 0;	
		pingSensor2Q[i] = 0;
		pingSensor3Q[i] = 0;
		pingSensor4Q[i] = 0;
		flameSensorQ[i] = 0;
	}
	
}


//Toggles the Luxeon LED that is connected to a mechanical relay
void ToggleLED(int i)
{
	if(i == 1)
		digitalWrite(LED,1);
	else
		digitalWrite(LED,0);

}

//After a block is completed, the block is parsed and the command execute
void ProcessBlock()
{
  	int reg = block[0];
	if(reg == REGPING1)
		SendQ[1];
	if(reg == REGPING2)
		SendQ[2];
	if(reg == REGPING3)
		SendQ[3];
	if(reg == REGPING4)
		SendQ[4];
	if(reg == REGFIRE)
		SendQ[5];
	if(reg == REGLED)
		ToggleLED(block[1]);
	if(reg == CLEARQ)
		ClearQs();

	//Allow a new block to come in
	blockBlock = false;

 
  
}




//Taken from Arduino.cc
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
int microsecondsToInches(long microseconds)
{

  double d = microseconds / 74 / 2;
  if(d >0 && d < 1)
		return 1;
else
	(int)return d;
}


void setup()
{

	//initialize pins, Qs and Pointers
	pinMode(PING1TRIGGER,OUTPUT);
	pinMode(PING1ECHO,INPUT);
	pinMode(PING2ECHO,INPUT);
	pinMode(PING3ECHO,INPUT);
	pinMode(PING4ECHO,INPUT);
	pinMode(FLAMESENSOR,INPUT);
	


	pingSensor1Q = new char[MAXQ];
	pingSensor2Q = new char[MAXQ];
	pingSensor3Q = new char[MAXQ];
	pingSensor4Q = new char[MAXQ];
	flameSensorQ = new char[MAXQ];

	ClearQs();

	//setup I2C
	Wire.begin(I2C_SENSORARRAY_ID);
	Wire.onReceive(I2CReceiveEventBlock);



	
}



int FirePingSensor(int pin)
{
	long duration;
	digitalWrite(PING1TRIGGER,LOW);
  	delayMicroseconds(2);
	digitalWrite(PING1TRIGGER,HIGH);
  	delayMicroseconds(10);
	digitalWrite(PING1TRIGGER,LOW);
	duration = pulseIn(pin,HIGH);
	
	return (int)microsecondsToInches(duration);
	


}

void loop()
{
	int result;

	//Probe Sensors every .5 seconds
	delay(500);
	//Fire each ping and store results if needed
	result = FirePingSensor(PING1ECHO);
	if(result > 0)
	{
		pingSensor1Q[pingSensor1QWriteIndex++] = result;
		pingSensor1QWriteIndex %= MAXQ;
	}
	
	
	result = FirePingSensor(PING2ECHO);
	if(result > 0)
	{
		pingSensor2Q[pingSensor2QWriteIndex++] = result;
		pingSensor2QWriteIndex %= MAXQ;
	}
	
	result = FirePingSensor(PING3ECHO);
	if(result > 0)
	{
		pingSensor3Q[pingSensor3QWriteIndex++] = result;
		pingSensor3QWriteIndex %= MAXQ;
	}
	
	result = FirePingSensor(PING4ECHO);
	if(result > 0)
	{
		pingSensor4Q[pingSensor4QWriteIndex++] = result;
		pingSensor4QWriteIndex %= MAXQ;
	}

	//Check the Fire Sensor
	result = analogrRead(FLAMESENSOR);
	if(results > 0)
	{
		flameSensorQ[flameSensorQWriteIndex++] = result;
		flameSensorQWriteIndex %= MAXQ;
	}

	//Check to see if we have a command incoming

	if(blockBlock &&   blockCompleted)
       {
                  ProcessBlock();
	}

}
	
	

	



	
	

}
