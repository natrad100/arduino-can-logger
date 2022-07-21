#include <SPI.h>
#include <SD.h>
#include <SPI.h>
#include "mcp2515_can.h"

const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

const int RECORDING_BUTTON = 3;
const int POWER_INDICATOR = 5;
const int RECORDING_INDICATOR = 6;

void create_header(File *datafile_ptr);
void handleButtonPressInterrupt();
void handleCanbusInterrupt();

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

  // CAN
  attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), handleCanbusInterrupt, FALLING); // start interrupt
  while (CAN_OK != CAN.begin(CAN_250KBPS))
  { // init can bus : baudrate = 500k
    SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
    delay(100);
  }
  SERIAL_PORT_MONITOR.println("CAN init ok!");
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
    if (dataFile && flagRecv)
    {
      flagRecv = 0;
      create_header(&dataFile);

      while (CAN_MSGAVAIL == CAN.checkReceive())
      {
        // int i = 0;
        // while (i < 100 && recording_button_state)
        {
          CAN.readMsgBuf(&len, buf);
          dataFile.print(millis());
          dataFile.print(",");
          for (int x = 0; x < len; x++)
          {
            dataFile.print(buf[x]);
          }
          dataFile.print("\n");
        }
        Serial.println(String("Closing File " + filename));
        dataFile.close();
      }
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

void handleCanbusInterrupt()
{
  flagRecv = 1;
}