#include "chompr_defines.h"

// The SD card chip select pin
#define SD_CHIP_SELECT_PIN  10

// The name of the file conatining the last used file number
const char NUMBER_FILE_NAME[] = "numbers.txt";

// File reference
File log_file;

// Read a line from the microSD card
void sd_read_line(File f, char* buffer)
{
  int ptr = 0;
  char c;
  
  while (f.available())
  {
    c = f.read();
    if (c == '\n')
    {
      buffer[ptr] = '\0';
      return;
    }
    else
    {
      buffer[ptr] = c;
      ptr++;
    }
  }
}

// Write a line to the microSD card
void sd_write_line(char *buffer)
{
  log_file.println(buffer);
  log_file.flush();
}

// Configure pins, initialize the microSD card library and open up a log file
int init_micro_sd()
{
  char buffer[25];
  int result, file_num;
  File number_file;
  
  
  // Pin 10 (slave select) and the chip select pin must be outputs
  pinMode(10, OUTPUT); 
  pinMode(SD_CHIP_SELECT_PIN, OUTPUT);
  
  // Initialize the library
  if (SD.begin(SD_CHIP_SELECT_PIN)) 
  {
    Serial.println("Successfully initialized microSD card");
    result = SUCCESS;
    
    // Check if the numbers file exists
    // This file contains the last used file number
    if (SD.exists((char*) NUMBER_FILE_NAME)) 
    {
      Serial.println("Number file exists\nReading next available number");
      number_file = SD.open((char*) NUMBER_FILE_NAME, FILE_WRITE);
      
      // Move to the start of the file
      number_file.seek(0);
      sd_read_line(number_file, buffer);
      
      // Convert the string to an integer and then increment
      // This is the next available file number
      file_num = atoi(buffer) + 1;
      
      // Move to the start of the file
      number_file.seek(0);
      
      // Write the new file number to the microSD card
      number_file.print(String(file_num));
      number_file.close();
    }
    else
    {
      Serial.println("Number file does not exist\nCreating new file");
      file_num = 0;
      number_file = SD.open((char*) NUMBER_FILE_NAME, FILE_WRITE);
      number_file.write("0");
      number_file.close();
    }
    
    Serial.print("New file number : ");
    Serial.println(file_num, DEC);
    
    // Open the log file
    sprintf(buffer, "log_%d.txt", file_num);
    Serial.print("Opening ");
    Serial.println(buffer);
    log_file = SD.open(buffer, FILE_WRITE);
    
    result = SUCCESS;
  }
  else
  {
    Serial.println("Failed to initialize microSD card");
    result = FAILURE;
  }
  
  return result;
}
