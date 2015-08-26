#include "i2c.h"



int fd;



int SendBlock(int address,int *data,int count)
{
        bool error = false;


        //Send Block Reset
        if(wiringPiI2CWrite(fd,BLOCKRESET) == -1)
                error = true;

        //Send Block Start
        if(wiringPiI2CWrite(fd,BLOCKSTART) == -1)
                error = true;

        //Send the address
        if(wiringPiI2CWrite(fd,address) == -1)
                error = true;





        for(int i=0;i<count && !error;i++)
                if(wiringPiI2CWrite(fd,data[i]) == -1)
                        error = true;

        if(wiringPiI2CWrite(fd,BLOCKSTOP) == -1)
                error = true;


        if(error)
                return -1;
        else
                return 0;


}

int SendControlByte(int cb)
{

	return SendBlock(I2C_CONTROLSWITCH_ID,&cb,1);
}




