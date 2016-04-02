//////////////////////////////////////////////////////////////////////////////
//
// This sketch is based on the MMA8452Q breakout board 
// sample code from Nathan Seidle of SparkFun Electronics
//
//////////////////////////////////////////////////////////////////////////////

#include <Wire.h>
#include "chompr_defines.h"

// The SparkFun breakout board defaults to 1, set to 0 if SA0 jumper on the bottom of the board is set
#define MMA8452_ADDRESS 0x1D // 0x1D if SA0 is high, 0x1C if low

// Define a few of the registers that we will be accessing on the MMA8452
#define OUT_X_MSB 0x01
#define XYZ_DATA_CFG 0x0E
#define WHO_AM_I 0x0D
#define CTRL_REG1 0x2A

// Sets full-scale range to +/-2, 4, or 8g. Used to calc real g values.
#define GSCALE 2 

void readAccelData(int *destination)
{
  byte rawData[6]; // x/y/z accel register data stored here

  readRegisters(OUT_X_MSB, 6, rawData); // Read the six raw data registers into data array

  // Loop to calculate 12-bit ADC and g value for each axis
  for(int i = 0; i < 3 ; i++)
  {
    int gCount = (rawData[i*2] << 8) | rawData[(i*2)+1]; //Combine the two 8 bit registers into one 12-bit number
    gCount >>= 4; //The registers are left align, here we right align the 12-bit integer

    // If the number is negative, we have to make it so manually (no 12-bit data type)
    if (rawData[i*2] > 0x7F)
    {
      gCount = ~gCount + 1;
      gCount *= -1; // Transform into negative 2's complement #
    }

    destination[i] = gCount; //Record this gCount into the 3 int array
  }
}

// Initialize the MMA8452 registers
// See the many application notes for more info on setting all of these registers:
// http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=MMA8452Q
int initMMA8452()
{
  byte c = readRegister(WHO_AM_I); // Read WHO_AM_I register
  if (c == 0x2A) // WHO_AM_I should always be 0x2A
  {
    MMA8452Standby(); // Must be in standby to change registers

    // Set up the full scale range to 2, 4, or 8g.
    byte fsr = GSCALE;
    if(fsr > 8) fsr = 8; // Easy error check
    fsr >>= 2; // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
    writeRegister(XYZ_DATA_CFG, fsr);
  
    // The default data rate is 800Hz and we don't modify it in this example code
    // Set to active to start reading
    MMA8452Active();
    return SUCCESS;
  }
  else
  {
    return FAILURE;
  }
}

// Sets the MMA8452 to standby mode. It must be in standby to change most register settings
void MMA8452Standby()
{
  byte c = readRegister(CTRL_REG1);
  // Clear the active bit to go into standby
  writeRegister(CTRL_REG1, c & ~(0x01)); 
}

// Sets the MMA8452 to active mode. Needs to be in this mode to output data
void MMA8452Active()
{
  byte c = readRegister(CTRL_REG1);
  // Set the active bit to begin detection
  writeRegister(CTRL_REG1, c | 0x01); 
}

// Read bytesToRead sequentially, starting at addressToRead into the dest byte array
void readRegisters(byte addressToRead, int bytesToRead, byte * dest)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  // End the transmission but keep the connection active
  Wire.endTransmission(false); 
  
  // Ask for bytes, once done, bus is released by default
  Wire.requestFrom(MMA8452_ADDRESS, bytesToRead); 
  
  // Keep reading until we get the # of bytes we expect
  while(Wire.available() < bytesToRead); 

  for(int x = 0 ; x < bytesToRead ; x++)
    dest[x] = Wire.read();
}

// Read a single byte from addressToRead and return it as a byte
byte readRegister(byte addressToRead)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  // End the transmission but keep the connection active
  Wire.endTransmission(false); 

  //Ask for 1 byte, once done, bus is released by default
  Wire.requestFrom(MMA8452_ADDRESS, 1); 
  
  // Wait for the data to come back
  while(!Wire.available()) ; 
  return Wire.read(); 
}

// Writes a single byte (dataToWrite) into addressToWrite
void writeRegister(byte addressToWrite, byte dataToWrite)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToWrite);
  Wire.write(dataToWrite);
  Wire.endTransmission(); 
}

int read_accelerometer(int *data)
{  
  // Raw accelerometer data (x, y, z) is returned as 12-bit signed integers
  readAccelData(data);
  
  return SUCCESS;
}

int init_accelerometer()
{
  int result;
  
  // Start up the I2C library
  Wire.begin();  
  
  // Initialize the accelerometer chip
  result = initMMA8452(); 
  if (result == SUCCESS)
  {
    Serial.println("Accelerometer successfully initialized");
  }
  else
  {
    Serial.println("Failed to initialize accelerometer");
  }
  
  return result;
}
