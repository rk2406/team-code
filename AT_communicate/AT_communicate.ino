#include <SoftwareSerial.h>

SoftwareSerial mySerial(5, 6);  //RX,TX
//2 3
//  Arduino pin 2 (RX) to ESP8266 TX
//  Arduino pin 3 to voltage divider then to ESP8266 RX
//  Connect GND from the Arduiono to GND on the ESP8266
//  Pull ESP8266 CH_PD HIGH

// When a command is entered in to the serial monitor on the computer 
// the Arduino will relay it to the ESP8266
//    GET /?data1=1.00&data2=8.00&submit=submit
//    htt p://saenitkrukshetra.atwebpages.com/?data1=1.00&data2=5.00&submit=submit
 //   AT+START="TCP","saenitkrukshetra.atwebpages.com",80
// AT+CWJAP="" ""
void setup() 
{
    Serial.begin(115200);     // communication with the host computer
    //while (!Serial)   { ; }
 
    // Start the software serial for communication with the ESP8266
    mySerial.begin(115200);  
 
    Serial.println("");
    Serial.println("Remember to to set Both NL & CR in the serial monitor.");
    Serial.println("Ready");
    Serial.println("");    
}
// AT+CIPSTART="TCP","api.thignspeak.com",80
 //AT+CIPSEND=42
 // GET /update?api_key=3DKZCTWR2WY59NNI&field1=100
void loop() 
{
    // listen for communication from the ESP8266 and then write it to the serial monitor
    if ( mySerial.available() > 0 )   { 
      byte b = mySerial.read();
      Serial.write( b ); 
      }
 
    // listen for user input and send it to the ESP8266
    if ( Serial.available() > 0 )       {  
      byte b=Serial.read();
      mySerial.write( b ); 
      
      }
}
