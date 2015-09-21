#include <Wire.h>
#include <SoftPWMServo.h> 




#define SERVODEADBAND 5
#define READPWMMAXDELAYLOW 30
#define READPWMMAXDELAYHIGH 20
#define MINPWM 1200
#define MAXPWM 2000
/*
PWM output to Flight controller (PWM write)

3 - Rx Channel 1 (Left and Right)
5 - Rx Channel 2 (forward and reverse)
6 - Rx Channel 3 (climb and dive)
9 - Rx Channel 4 (rotate left and right)
*/

//PWM outut from ChipKit to Flight Controller
#define FLIGHTCONTROL_X 0
#define FLIGHTCONTROL_Y	1
#define FLIGHTCONTROL_Z	A0
#define FLIGHTCONTROL_R	3

//PWM output to Head Servo
#define HEADSERVO  A1


//PWM input from Rx to Chipkit.
#define RXCHANNEL1 8
#define RXCHANNEL2 9
#define RXCHANNEL3 10
#define RXCHANNEL4 11
#define RXCHANNEL6 13
#define RXCHANNEL5 12

//Output pins to RPi for auto and macro mode switching
#define RPIAUTOMODE   2

//Move MACROMODE to RPI Read
#define RPIMACROMODE  0
	

/***********************************************************************
	The SPEED definition is the amount to increment or decrement
	the PWM nuetral point in order to move in that direction.  
	In the case it is
	not enough to cuase movement due to wind or other issues,
	the code will increment slowly by FASTER or decrement by SLOWER
	which is a small value.  This will allow the quad to make
	small adjustments in order to compensate without being 
	over controlled. 
***********************************************************************/
#define SPEED		100	
#define STOP            1500
#define STOPHOVER       1800
#define MOVELEFT	STOP-SPEED
#define MOVERIGHT	STOP+SPEED
#define MOVEFORWARD	STOP+SPEED
#define MOVEREVERSE	STOP-SPEED
#define ROTATERIGHT	STOP-SPEED
#define ROTATELEFT	STOP+SPEED


//an integer to indicate directions


//Climbing and diving is done slower
#define MOVECLIMB		STOP+SPEED/2
#define MOVEDIVE		STOP-SPEED/2

#define SPEEDFACTOR	5
#define FASTER		SPEED/SPEEDFACTOR
#define SLOWER		(SPEED/SPEEDFACTOR)*-1


//Adjustment variables for block and controlbyte
#define  FORWARDADJUST 1
#define  REVERSEADJUST 2
#define  LEFTADJUST    3
#define  RIGHTADJUST   4
#define  CLIMBADJUST   5
#define  DIVEADJUST    6
#define  RRIGHTADJUST  7
#define  RLEFTADJUST   8

#define  ADJUSTFASTER  10
#define  ADJUSTSLOWER  20

//Global Variables

//Registers






//Speed Vars
int xSpeed = 0;
int ySpeed = 0;
int zSpeed = 0;
int rSpeed = 0;



//Servo pins for PWM output to the flight controller

bool autoMode = false;
bool autoModeInProgress = false;
bool manualModeInProgress = false;
bool forceManual = false;
bool serialOut = true;
bool macroMode = false;
bool macroModeInProgress = false;

bool forward = false;
bool reverse = false;
bool left = false;
bool right = false;
bool climb = false;
bool dive = false;
bool rLeft = false;
bool rRight = false;
bool controlByteChanged = false;
bool controlByteBad = false;
bool ledOn = false;
bool heartBeatChecked = false;

unsigned char controlByte;
unsigned char lastControlByte;

unsigned char temp;
char *r;




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



//Head control variables
long headCurrentPWM = STOP;
int headSpeedFactor = 100;
int headDirection = 1;
long headLastUpdate = millis();
long headStopPointPWM = MAXPWM;
int headStopPoint = 0;
int headDelay = 5000;
int headDivFactor = 1;

//A function that randomly moves the head around


void MoveHead()
{
  
    
	//SoftPWMServoServoWrite


        if(millis() - headLastUpdate > headDelay)      
	{
  
  		if(headDirection == 0)
			headCurrentPWM -= headSpeedFactor;
		else
			headCurrentPWM += headSpeedFactor;


		if(random(1,10000) < 30)
                {
                  headDirection++;
                  headDirection %= 2;
                  headDelay=random(1,3) * 1000;
                  headSpeedFactor = random(1,6);
                  headSpeedFactor =10;
                }
                else  
                  headDelay = 10;
                  
                if(headCurrentPWM >= MAXPWM || headCurrentPWM <= MINPWM)
		{
                    
			if(headDirection == 0)
                        {
                            headDirection = 1;
                            headCurrentPWM = MINPWM + 10;
                        }
                        else
                        {
                            headDirection = 0;
                            headCurrentPWM = MAXPWM - 10;
                        }
                        
			headSpeedFactor = random(1,3) * 10;
;
                        
                        //headCurrentPWM = headStopPointPWM;
                        
                        

			                        //Wait up to 3 seconds before moving again
                        headDelay = random(1,3);
                        headDelay *= 1000;

  
		}
                else
                {
                    

                        SoftPWMServoServoWrite(HEADSERVO,headCurrentPWM);
    		        headLastUpdate = millis();
                }
	
	}
	
}


bool ControlByteCheck(unsigned char cb,unsigned char cbc)
{
	bool valid = true;
	////////Serial1.print("CB: ");
	////////Serial1.println(cb);
	////////Serial1.print("CBC: ");
	////////Serial1.println(cbc);

	if(cb % 17 != cbc)
		valid =  false;

	//Check for opposing motions, which may mean there is an issue
	//forward and reverse requested
	if(cb % 2 && (cb>>1) % 2)
		valid = false;
	//left and right requested
	if( (cb>>2) % 2 && (cb>>3) % 2)
		valid = false;	
	//rotate left and rotate right requested
	if((cb>>4) % 2 && (cb>>5) % 2)
		valid = false;
	//climb and dive requested
	if((cb>>6) %2 && (cb>>7) %2)
		valid = false;

	return valid;	
	
	
	
}

inline int ParseControlByte()
{
	int cb = controlByte;
	rRight = false;
	rLeft = false;
	forward = false;
	reverse = false;
	left = false;
	right = false;
	climb = false;
	dive = false;


		if(cb & 1)
                {
			rRight = true;
                        rSpeed = ROTATERIGHT;
                }
		cb >>= 1;
                if(cb & 1)
                {
                        rLeft = true;
                        rSpeed = ROTATELEFT;
                }
                cb >>= 1;
                if(cb & 1)
                {
                        dive = true;
                        zSpeed = MOVEDIVE;
                }
                cb >>= 1;
                if(cb & 1)
                {
                        climb = true;
                        zSpeed = MOVECLIMB;
                }
                cb >>= 1;
                if(cb & 1)
                {
                        right = true;
                        xSpeed = MOVERIGHT;
                }
                cb >>= 1;
                if(cb & 1)
                {
                        left = true;
                        xSpeed = MOVELEFT;
                }
                cb >>= 1;
                if(cb & 1)
                {
                        reverse = true;
                        ySpeed = MOVEREVERSE;
                }
                cb >>= 1;
                if(cb & 1)
                {
                        forward = true;
                        ySpeed = MOVEFORWARD;
                }
                
                //Set default speeds here
                if(!forward && !reverse)
                    ySpeed = STOP;
                if(!left && !right)
                    xSpeed = STOP;
                if(!climb && !dive)
                    zSpeed = STOPHOVER;


}

void PrintDirections()
{
	////Serial1.println("------------------------------");

	//if(forward)
		////Serial1.println("FORWARD");
	//if(reverse)
		//////Serial1.println("REVERSE");
	//if(left)
		//////Serial1.println("LEFT");
	//if(right)
		//////Serial1.println("RIGHT");
	//if(climb)
		//////Serial1.println("CLIMB");
	//if(dive)
		//////Serial1.println("DIVE");
	//if(rLeft)
		//////Serial1.println("RLEFT");
	//if(rRight)
		//////Serial1.println("RRIGHT");
        //////Serial1.println("------------------------------");




}




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



void ProcessBlock()
{
  int reg = block[0];
  if(reg == 22)
  {
    //heartbeat check
    heartBeatChecked = true;

  }
  else if(reg == 60)
  {

    if(ControlByteCheck(block[1],block[2]))
    {
      lastControlByte = controlByte;
      controlByte = block[1];
      controlByteChanged = true;
      controlByteBad = false;

    }
    else if(reg == 90)
    {
        //Speed Change
        //block[1] is the direction and block[2] is to indicate speedup or slowdown
        int direction = block[1];
        int adjust = 0;
        
        if(block[2] == ADJUSTFASTER)
          adjust = FASTER;
        if(block[2] == ADJUSTSLOWER)
          adjust = SLOWER;
        
        
        if(direction == FORWARDADJUST || direction == REVERSEADJUST)
          ySpeed += adjust;

        if(direction == LEFTADJUST || direction == RIGHTADJUST)
          xSpeed += adjust;
          
        if(direction == RRIGHTADJUST || direction == RLEFTADJUST)
          rSpeed += adjust;
          
        if(direction == CLIMBADJUST || direction == DIVEADJUST)
            zSpeed += adjust;
            
        controlByteChanged = true;
        controlByteBad = false;
        
          
          
          
    }
      controlByteBad = true;
  }
  else
    controlByteBad = true; 

  blockBlock = false;

  
}




void setup()
{
	r = new char[20];
	
	//Random numbers for Head movement.
	randomSeed(analogRead(A1));

	//if(serialOut)
		////Serial1.begin(9600);
	////Serial1.println("RESET");
	//Setup RPi input pins

	//Setup PWM pins going to flight controller


	//Setup PWM input pins from RX
	pinMode(RXCHANNEL1,INPUT);
	pinMode(RXCHANNEL2,INPUT);
 	pinMode(RXCHANNEL3,INPUT);
 	pinMode(RXCHANNEL4,INPUT);
 	pinMode(RXCHANNEL6,INPUT);
	pinMode(RXCHANNEL5,INPUT);

	pinMode(RPIAUTOMODE,OUTPUT);
	pinMode(RPIMACROMODE,OUTPUT);

	digitalWrite(RPIAUTOMODE,LOW);
	digitalWrite(RPIMACROMODE,LOW);

	Wire.begin(40);
        Wire.onReceive(I2CReceiveEventBlock);

	//Center all the servos
	SoftPWMServoServoWrite(HEADSERVO,STOP);
        SoftPWMServoServoWrite(FLIGHTCONTROL_X, STOP);
	SoftPWMServoServoWrite(FLIGHTCONTROL_Y, STOP);
        SoftPWMServoServoWrite(FLIGHTCONTROL_Z, MINPWM);
        SoftPWMServoServoWrite(FLIGHTCONTROL_R, STOP);
	
}



inline int ReadPWM2(int pin)
{
	unsigned long m1,m2;
	int d = 0;
	unsigned long functionStart = millis();
	//for(int i=0;i<2 && millis()-functionStart <= READPWMMAXDELAY;i++)
	//{	
	//wait for the pin to go low
	while(digitalRead(pin) == HIGH && millis()-functionStart <= READPWMMAXDELAYLOW);
	while(digitalRead(pin) == LOW && millis()-functionStart <= READPWMMAXDELAYLOW);
	m1 = micros();
	while(digitalRead(pin) == HIGH && millis()-functionStart <= READPWMMAXDELAYLOW);
	m2 = micros();
	d = (m2-m1);
	//}
	
	//d /= 2;


	////////Serial1.print("pin: ");
	////////Serial1.println(pin);

	//if(d < MINPWM || d > MAXPWM)
	//f	d = 0;

	if(millis() - functionStart <= READPWMMAXDELAYLOW)
	{
		////////Serial1.println(d);
		return d;
	}
	else
	{
		////////Serial1.println(0);
		return 0;
	}
}



char * CToS(unsigned char c)
{
  
  char temp[20]; 
  char t;
   int i = 0;
   
  while(c > 0)
 {
     t = c % 10;
     c = c  / 10;
     t = t + '0';
     temp[i] = t;
     i++;
 } 
 int q = 0;
 for(int j=i-1;j>=0;j--)
   r[q++] = temp[j];
 r[i] = 0;
return r;  
}

//More Global Vars for loop
int channel1;
int prevChannel1 = 0;
int channel2;
int prevChannel2 = 0;
int channel3;
int prevChannel3 = 0;
int channel4;
int prevChannel4 = 0;
int channel6;
int channel5;


unsigned int channel1Errors = 0;
unsigned int channel2Errors = 0;
unsigned int channel3Errors = 0;
unsigned int channel4Errors = 0;
unsigned int channel5Errors = 0;
unsigned int channel6Errors = 0;

bool servo1Removed = false;

char controlChar = 0;





void loop()
{
      ////Serial1.println("HERE");
	unsigned int a = 0;
        int channel6LastChecked = 0;
        
                             
        
	
	while(1)
	{
          ///////Serial1.println("Loop");
          //delay(100);
         // autoMode = false;
      	//Move the Head.
	MoveHead(); 


	if(!autoMode || forceManual)
	{

		if(!manualModeInProgress)
		{
			//////Serial1.println("Entering Manual Mode");
			manualModeInProgress = true;
			autoModeInProgress = false;
		}
/*
                if(heartBeatChecked)
                {
                    //////Serial1.println("Heartbeat Checked");
                    heartBeatChecked = false;
                }
                if(controlByteChanged)
                {
                  //////Serial1.print("Bytes REceived: ");
                  //////Serial1.println(CToS(temp));
                  controlByteChanged = false;
                }
                */
		//Main manual mode logic
		if(channel1Errors < 50)
			channel1 = ReadPWM2(RXCHANNEL1);
		if(channel1 == 0)
			;//channel1Errorsx++;
		else
			channel1Errors = 0;
			

	
		if(channel1Errors < 50 && channel1 != 0 && abs(prevChannel1 - channel1) > SERVODEADBAND)
		{
			//Serial1.print(prevChannel1);	
			//Serial1.print("---");
			//Serial1.println(channel1);
			//Serial1.println(abs(prevChannel1 - channel1));


			prevChannel1 = channel1;
			//velocityX.writeMicroseconds(channel1);
                        SoftPWMServoServoWrite(FLIGHTCONTROL_X, channel1);
                        servo1Removed = false;
		}
                else
                {
                  if(channel1Errors > 50)
                  {
                    if(!servo1Removed)
                    {
                        //Serial1.println("SERVO 1 REMOVED"); 
                        servo1Removed = true;
                    } 
                    else
                    {
                      channel1Errors++;
                      channel1Errors %= 5000;  //After 500 cycles see if the servo comes back up.
                    }
                  }
                }
		if(channel2Errors < 50)	
			channel2 = ReadPWM2(RXCHANNEL2);
		if(channel2 == 0) 
			;//channel2Errors++;
		else
			channel2Errors = 0;
	
		if(channel2 != 0 && abs(prevChannel2 - channel2) > SERVODEADBAND)
		{
			SoftPWMServoServoWrite(FLIGHTCONTROL_Y, channel2);
			prevChannel2 = channel2;

		}
		if(channel3Errors < 50)	
			channel3 = ReadPWM2(RXCHANNEL3);
		if(channel3 == 0)
			;//channel3Errors++;
		else
			channel3Errors = 0;

		if(channel3 != 0 && abs(prevChannel3 - channel3) > SERVODEADBAND)
		{
			SoftPWMServoServoWrite(FLIGHTCONTROL_Z, channel3);
			prevChannel3 = channel3;
		
		}
		if(channel4Errors < 50)	
			channel4 = ReadPWM2(RXCHANNEL4);	
		if(channel4 == 0)
			;//channel4Errors++;
		else
			channel4Errors = 0;

		if(channel4 != 0 && abs(prevChannel4 - channel4) > SERVODEADBAND)
		{
			SoftPWMServoServoWrite(FLIGHTCONTROL_R, channel4);
			prevChannel4 = channel4;

		}


		channel6 = ReadPWM2(RXCHANNEL6);
		
		if(channel6 == 0 || channel6 >= STOP)
		{
			autoMode = true;
			digitalWrite(RPIAUTOMODE,HIGH);
		}
		else
		{
			if(channel5Errors < 50)
				channel5 = ReadPWM2(RXCHANNEL5);
			if(channel5 == 0)
			channel5Errors++;
			else
				channel5Errors = 0;
			if(channel5 > STOP)
			{
				macroMode = true;
				if(!macroModeInProgress)
				{
					digitalWrite(RPIMACROMODE,HIGH);
					//////Serial1.println("Entering Macro Record Mode.");
					macroModeInProgress = true;
				}
			}
			else
			{
				if(macroModeInProgress)
				{
					macroModeInProgress = false;
					macroMode = false;
					//////Serial1.println("Leaving Macro Record Mode.");
					digitalWrite(RPIMACROMODE,LOW);	
				}
			}
		}
	}
	else
	{
		//Main automode logic here
		if(!autoModeInProgress)
		{
			//////Serial1.println("Entering AUTOMODE");
			autoModeInProgress = true;
			manualModeInProgress =false;
			macroMode = false;
			digitalWrite(RPIMACROMODE,LOW);
			digitalWrite(RPIAUTOMODE,HIGH);
		}
		//automode get directions from Rpi

                //Do Testing
                
                            

                if(blockBlock &&   blockCompleted)
                {
                  ////////Serial1.println("BLOCK");
                  ProcessBlock();
                  //if(controlByteBad)
                    //////Serial1.println("BAD");
                  //else
                    //////Serial1.println("GOOD");
                  ////////Serial1.println(CToS(block[0]));
                  ////////Serial1.println(CToS(block[1]));
                  ////////Serial1.println(CToS(block[2]));
                  ////////Serial1.print("Skipped: ");
                  //////Serial1.println(blockSkipped);
                       
                }
		
		if(controlByteChanged)
		{
			controlByteChanged = false;
			ParseControlByte();
			//PrintDirections();



                            SoftPWMServoServoWrite(FLIGHTCONTROL_X, xSpeed);
                            SoftPWMServoServoWrite(FLIGHTCONTROL_Y, ySpeed);
                            SoftPWMServoServoWrite(FLIGHTCONTROL_Z, zSpeed);
                            SoftPWMServoServoWrite(FLIGHTCONTROL_R, rSpeed);


		}

                if(millis() - channel6LastChecked > 2000)
                {
        		channel6 = ReadPWM2(RXCHANNEL6);

	        	if(channel6 != 0 &&  channel6 < STOP)
		        {
		            ////////Serial1.println(channel6);	
                	    autoMode = false;
			    digitalWrite(RPIAUTOMODE,LOW);
		        }
                        channel6LastChecked =millis();
                }




	

	}
	}

}
