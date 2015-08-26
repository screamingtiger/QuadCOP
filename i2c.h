#include <wiringPi.h>
#include <wiringPiI2C.h>


//Block Definitions
#define BLOCKSTART      204
#define BLOCKSTOP       190
#define BLOCKRESET      195

//I2C Addresses
#define I2C_CONTROLSWITCH_ID 4
#define I2C_SENSORARRAY_ID 5
#define I2C_MEMS_ID 0x1e
#define I2C_SDA 8
#define I2C_SCL 9
#define I2C_HEARTBEAT_VALUE 16571657

//Registers
#define I2C_CONTROL_REGISTER 60
#define I2C_HEARTBEAT_REGISTER 22
#define MOVEX 30
#define MOVEY 31
#define MOVEZ 32
#define MOVER 33


int SendBlock(int address,int *data,int count);

int SendControlByte(int cb);
