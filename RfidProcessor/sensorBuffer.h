#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>
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
