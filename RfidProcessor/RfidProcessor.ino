#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>
// #include "sensorBuffer.h"

#define DEBUG 1 //remove or comment this line when you are not debugging

// CONSTANTS
// The number of RFID readers
const byte numReaders = 2;
// Each reader has a unique Slave Select pin
const byte ssPins[] = {A2, A3};
// They'll share the same reset pin
const byte resetPin = 8;

const int flopSensor = 0;
const int turnSensor = 4;
const int riverSensor = 5;

// GLOBALS
// Initialise an array of MFRC522 instances representing each reader
MFRC522 mfrc522[numReaders];
// The tag IDs currently detected by each reader
String currentIDs[numReaders];  

/* Set the block to which has the data we want to read */
/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  

/* Create another array to read data from Block */
byte readBlockData[18];
/* Length of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;

MFRC522::MIFARE_Key key;

String flop[3] = {"","",""};

/* If the card being read matches this, we process it.
 * This should handle any "accidental" readings that may occur.
 */
class sensorBuffer
{
  int sensorNum;
  String card;

public:
  sensorBuffer()
  {
    sensorNum = -1;
    card = "";
  }

  sensorBuffer(int s, String c)
  {
    sensorNum = s;
    card = c;
  }

  bool isEqual(int s, String c)
  {
    return sensorNum == s && card == c;
  }

  void printData()
  {
    Serial.println("Sensor Buffer's Data:");
    Serial.print("Sensor Number: ");
    Serial.println(sensorNum);
    /*
    Serial.print("Card: ");
    Serial.println(card);
    Serial.println("");
    */
  }
};

sensorBuffer bufferTuple;
/**
   Helper function to return a string ID from byte array
*/
String dump_byte_array(byte *buffer, byte bufferSize) {
  String read_rfid = "";
  for (byte i = 0; i < bufferSize; i++) {
    read_rfid = read_rfid + String(buffer[i], HEX);
  }
  return read_rfid;
}

boolean ReadDataFromBlock(MFRC522 mfrc522, int blockNum, byte (&readBlockData)[18]) 
{
  MFRC522::StatusCode status;
  // Authenticating the desired data block for Read access using Key A 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
#ifdef DEBUG
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
#endif
     return false;
  }

  // Reading data from the Block 
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK)
  {
#ifdef DEBUG
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    return false;
  }
  else
  {
#ifdef DEBUG
    Serial.println("Block was read successfully");  
#endif
    return true;
  }
  
}

/* This function will handle the entirety of processing each sensor */
void ProcessSensor(MFRC522 mfrc522, int sensorNum)
{
  // String to hold the ID detected by each sensor
  String readRFID = "";

  // If the sensor detects a tag and is able to read it
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Extract the ID from the tag
    readRFID = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  }
  /* Note that we can't else { return; } here to save processing on the loop  
   * because that would ignore a card being picked up off a sensor
   */

  // If the current reading is different from the last known reading
  if (readRFID != currentIDs[sensorNum]) {
    // Update the stored value for this sensor
    currentIDs[sensorNum] = readRFID;
  }
  else
    return; //If this sensor hasn't changed, we continue to the next sensor

  //if the function returns false, we continue to other sensors
  if(!ReadDataFromBlock(mfrc522, blockNum, readBlockData)) 
  {
    return;
    // Not sure how to handle a card that a sensor read "on accident."
  }
  //readBlockData now has the card name in it

  if(bufferTuple.isEqual(sensorNum, readBlockData))
  {
    // print card to Serial according to seat num, flop, turn, or river
    if(sensorNum == flopSensor) // flop
    {
#ifdef DEBUG
      Serial.println("Card belongs in the flop");
#endif
      int i = 0;
      while(flop[i] != "" && flop[i] != readBlockData) // find out if the flop has empty spaces
      {
        i++;
        if(i > 2) // we reset the flop
        {
          flop[0] = flop[1] = flop[2] = "";
          i = 0;
        }
      }
      flop[i] = readBlockData;
      if( i == 2 ) // the flop needs to be printed
      {
        Serial.print("Flop: ");
        for(int a = 0; a < 3; a++)
        {
          Serial.print('[');
          Serial.print(flop[a]);
          Serial.print(']');
        }
        Serial.println(""); // new line
      }
    }
    else if(sensorNum == turnSensor) // turn
    {
#ifdef DEBUG
      Serial.println("Card belongs in the turn slot");
#endif
      // print readBlockData as turn
      Serial.print("Turn: ");
      Serial.print('[');
      for (uint8_t i = 0; i < bufferLen - 2; i++)
      {
        Serial.write(readBlockData[i]); //Note Serial.write instead of Serial.print
      }
      Serial.println("]");
    }
    else if(sensorNum == riverSensor) // river
    {
#ifdef DEBUG
      Serial.println("Card belongs in the river slot");
#endif
      // print readBlockData as river
      Serial.print("River: ");
      Serial.print('[');
      for (uint8_t i = 0; i < bufferLen - 2; i++)
      {
        Serial.write(readBlockData[i]); //Note Serial.write instead of Serial.print
      }
      Serial.println("]");
    }
    else // hold cards
    {
#ifdef DEBUG
      Serial.println("Card belongs as a hold card");
#endif
      // print readBlockData as hold cards for a seat
      Serial.print("Seat ");
      Serial.print(sensorNum);
      Serial.print(" [");
      for (uint8_t i = 0; i < bufferLen - 2; i++)
      {
        Serial.write(readBlockData[i]); //Note Serial.write instead of Serial.print
      }
      Serial.println("]");
    }
  }
  else
  {
#ifdef DEBUG
    Serial.println("Card does not match bufferTuple");
#endif
    bufferTuple = sensorBuffer(sensorNum, readBlockData);
  }
#ifdef DEBUG
  bufferTuple.printData();
#endif
}


void setup() {
#ifdef DEBUG
  // Initialise serial communications channel with the PC
  Serial.begin(9600);
  Serial.println(F("Serial communication started"));
#endif

  // Initialise the SPI bus
  SPI.begin();

  for (uint8_t i = 0; i < numReaders; i++) { // Initialise the reader // Note that SPI pins on the reader must always be connected to certain // Arduino pins (on an Uno, MOSI=> pin11, MISO=> pin12, SCK=>pin13)
    // The Slave Select (SS) pin and reset pin can be assigned to any pin
    mfrc522[i].PCD_Init(ssPins[i], resetPin);

    // Set the gain to max - not sure this makes any difference...
    mfrc522[i].PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_max);

#ifdef DEBUG
    // Dump some debug information to the serial monitor
    Serial.print(F("Reader #"));
    Serial.print(i + 1);
    Serial.print(F(" initialised on pin "));
    Serial.print(String(ssPins[i]));
    Serial.print(F(". Antenna strength: "));
    Serial.print(mfrc522[i].PCD_GetAntennaGain());
    Serial.print(F(". Version : "));
    mfrc522[i].PCD_DumpVersionToSerial();
#endif

    // Slight delay before activating next reader
    delay(100);
  }

#ifdef DEBUG
  Serial.println(F("--- END SETUP ---"));
#endif
}

/**
   Main loop
*/
void loop() {
  /* Prepare the key for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  // Loop through each reader
  for (uint8_t i = 0; i < numReaders; i++) {
    // Initialise the sensor
    mfrc522[i].PCD_Init();

    ProcessSensor(mfrc522[i], i);

    // Halt PICC
    mfrc522[i].PICC_HaltA();
    // Stop encryption on PCD
    mfrc522[i].PCD_StopCrypto1();
  }
}
