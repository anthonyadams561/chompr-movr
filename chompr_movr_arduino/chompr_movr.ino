////////////////////////////////////////////////////////////
//
// http://chompr.blogspot.com
// anthony.adams.561@gmail.com
// 
// An Arduino project for recording your sleep position
//
// microSD Breakout Board from SparkFun Electronics
// https://www.sparkfun.com/products/544
// 
// Arduino pin       microSD card pin
// ------------      -----------------
//    10                   CS
//    11                   DI
//    12                   DO
//    13                   SCK
//
// Triple Axis Accelerometer Breakout - MMA8452
// https://www.sparkfun.com/products/10955
//
// The accelerometer is connected to the Arduino I2C bus
// 
// The status LED is on Arduino pin 9
// https://www.sparkfun.com/products/544
//
////////////////////////////////////////////////////////////

#include <Wire.h>
#include <SD.h>
#include "chompr_defines.h"

#define STATUS_LED_PIN  9

bool setup_successful;

// Turn the status LED on
void turn_status_led_on()
{
  digitalWrite(STATUS_LED_PIN, HIGH);
}

// Turn the status LED off
void turn_status_led_off()
{
  digitalWrite(STATUS_LED_PIN, LOW);
}

// This function gets executed once, when the microcontroller starts up
void setup()
{
  int result, i;
  char buffer[25];
  int data[3];
  
  setup_successful = false;
  
  // Set the status LED pin to an output
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Turn the status LED off (if it isn't already)
  turn_status_led_off();
  
  // Open up the serial port
  Serial.begin(115200);
  
  Serial.println("chompr-movr is running!");
  
  // Initialize the microSD card 
  result = init_micro_sd();
  
  if (result == SUCCESS)
  {
    // Initialize the accelerometer
    result = init_accelerometer();
    
    if (result == SUCCESS)
    {
      // Read the accelerometer.
      read_accelerometer(data);
      
      // Convert the accelerometer data to an x, y, z string
      sprintf(buffer, "%d,%d,%d", data[0], data[1], data[2]);
      
      Serial.print("Current accelerometer values : ");
      Serial.println(buffer);
      
      // Blink the status LED for a while to
      // let us know that everything is ready to go
      for (i=0; i< 15; i++)
      {
        turn_status_led_on();
        delay(500);
        turn_status_led_off();
        delay(500);
      }
      
      setup_successful = true;
    }
  }
}

void loop()
{
  char buffer[25];
  int data[3];
  
  if (setup_successful)
  {
    // Start logging accelerometer values to file
    read_accelerometer(data);
    
    // Convert the accelerometer data to an x, y, z string, with a timestamp
    sprintf(buffer, "%lu,%d,%d,%d", millis(), data[0], data[1], data[2]);
    
    Serial.print("Writing data to file : ");
    Serial.println(buffer);
    sd_write_line(buffer);
    delay(5000);
  }
}
