/*
 created 24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 modified 18 Sep 2014
 by Bobby Chan @ SparkFun Electronics Inc.

 Modified by Toni Klopfenstein @ SparkFun Electronics
  September 2015
  https://github.com/sparkfun/CAN-Bus_Shield

 SD Card Datalogger

 This example is based off an example code from Arduino's site
 http://arduino.cc/en/Tutorial/Datalogger and it shows how to
 log data from three analog sensors with a timestamp based on when
 the Arduino began running the current program to an SD card using
 the SD library https://github.com/greiman/SdFat by William
 Greiman. This example code also includes an output to the
 Serial Monitor for debugging.

 The circuit:
 * analog sensors on analog pins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

 This example code is in the public domain.
 */

#include <SPI.h>
#include <SD.h>

const int RECORDING_BUTTON = 3;
const int POWER_INDICATOR = 5;
const int RECORDING_INDICATOR = 6;

void create_header(File *datafile_ptr);
void handleButtonPressInterrupt();

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.

// Chip Select pin is tied to pin 9 on the SparkFun CAN-Bus Shield
const int chipSelect = 4;
volatile bool recording_button_state = LOW; // low is stopped
void setup()
{
  // Setup pins

  pinMode(RECORDING_BUTTON, INPUT_PULLUP); // Interupt pins 2 and 3 only on uno
  pinMode(POWER_INDICATOR, OUTPUT);
  pinMode(RECORDING_INDICATOR, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(RECORDING_BUTTON), handleButtonPressInterrupt, FALLING);

  digitalWrite(POWER_INDICATOR, LOW);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(chipSelect, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  Serial.println("Ready to record");
}

void loop()
{
  int ix = 0;
  // While we want to record
  while (recording_button_state)
  {
    digitalWrite(RECORDING_INDICATOR, LOW);
    String name = String(ix);
    String filename = String(name + ".csv");
    Serial.println(String("Opening file " + filename));

    File dataFile = SD.open(filename.c_str(), FILE_WRITE);
    if (dataFile)
    {
      create_header(&dataFile);

      // while(dataFile){
      int i = 0;
      while (i < 100 && recording_button_state)
      {
        dataFile.print(millis());
        dataFile.print(", test\n");
        i++;
      }
      Serial.println(String("Closing File " + filename));
      dataFile.close();
      ix++;
    }
    // if the file isn't open, pop up an error:
    else
    {
      Serial.println(String("error opening " + filename));
    }
  }
}

void create_header(File *datafile_ptr)
{
  String header_cols[2] = {"timestamp", "data"};
  for (int i = 0; i < 2; i++)
  {
    datafile_ptr->print(header_cols[i]);
    if (i < 2 - 1)
    {
      datafile_ptr->print(",");
    }
    datafile_ptr->print("\n");
  }
}

void handleButtonPressInterrupt()
{
  bool held = true;
  for (int i = 0; i < 100; i++)
  {
    delay(20);
    if (digitalRead(RECORDING_BUTTON))
    {
      held = false;
      break;
    }
  }
  if (held)
  {
    recording_button_state = !recording_button_state;
    Serial.println("Record Stop/Start Triggered");
  }
}