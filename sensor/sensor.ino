#include <SoftwareSerial.h>


#include <TinyGPS.h>

#include <SPI.h>
#include <SD.h>
#include "HX711.h"

#define calibration_factor -2070.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define DOUT  12
#define CLK  2

TinyGPS gps;

//  3 ibterrupt 
SoftwareSerial espSerial =  SoftwareSerial(5,6);
SoftwareSerial l =  SoftwareSerial(7,8);      // arduino RX pin=2  arduino TX pin=3    connect the arduino RX pin to esp8266 module TX pin   -  connect the arduino TX pin to esp8266 module RX pin
HX711 scale(DOUT, CLK);

const int chipSelect = 4;
int pot_F_init,pot_B_init;
float Dist_F,Dist_B;

int half_revolutions=0;
int a;
int rpm = 0;
unsigned long lastmillis = 0;




String apiKey = "teamAccelerons";     // replace with your channel's thingspeak WRITE API key
String ssid="BDDN";    // Wifi network SSID
String password ="Nischay1997";  // Wifi network password

boolean DEBUG=true;

float lat,lang;



void showResponse(int waitTime){
    long t=millis();
    char c;
    while (t+waitTime>millis()){
      if (espSerial.available()){
        c=espSerial.read();
        if (DEBUG) Serial.print(c);
      }
    }
}
boolean thingSpeakWrite(float value1,float value2,float Dist_B,float Dist_F,float x){
  String cmd = "AT+CIPSTART=\"TCP\",\"";                  // TCP connection
  cmd += "dweet.io";                               // api.thingspeak.com
  cmd += "\",80";
  //String cmd ="https://dweet.io/dweet/for/my-thing-name?hello=world;
  espSerial.println(cmd);
  if (DEBUG) Serial.println(cmd);
  if(espSerial.find("Error")){
    if (DEBUG) Serial.println("AT+CIPSTART error");
    return false;
  }
  String getStr = "GET /dweet/for/";   // prepare GET string
  getStr += apiKey;
  getStr +="?a=";
  getStr += String(value1,6);
  getStr +="&b=";
  getStr += String(value2,6);
  getStr +="&c=";
  getStr += String(Dist_B,6);
  getStr +="&d=";
  getStr += String(Dist_F,6);
  getStr +="&e=";
  getStr += String(x,6);
  // getStr +="&field3=";
  // getStr += String(value3);
  // ...
  getStr += "\r\n\r\n";
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  espSerial.println(cmd);
  if (DEBUG)  Serial.println(cmd);
  delay(10);
  if(espSerial.find(">")){
    espSerial.print(getStr);
    if (DEBUG)  Serial.print(getStr);
  }
  else{
    espSerial.println("AT+CIPCLOSE");
    // alert user
    if (DEBUG)   Serial.println("AT+CIPCLOSE");
    return false;
  }
  return true;
}

void rpm_fan(){
  half_revolutions++;
}


void setup() {                
  DEBUG=true;
  Serial.begin(115200); 
  espSerial.begin(115200); 
  espSerial.println("AT+RST"); 
  showResponse(1000);
  espSerial.println("AT+CWMODE=1");  
  showResponse(1000);
  espSerial.println("AT+CWJAP=\""+ssid+"\",\""+password+"\"");  
  showResponse(5000);
   if (DEBUG)  Serial.println("Setup completed");

  

  espSerial.end();

  pot_F_init=analogRead(A4);
  pot_B_init=analogRead(A5);

   scale.set_scale(calibration_factor);  

 attachInterrupt(0, rpm_fan, FALLING); //pin 2 is interrupt 0 and pin 3 is interrupt 1


}
void loop() {
   l.begin(9600);
 
            bool newData = false;
            unsigned long chars;
            unsigned short sentences, failed;
          
            // For one second we parse GPS data and report some key values
            for (unsigned long start = millis(); millis() - start < 1000;)
            {
              while (l.available())
              {
                char c = l.read();
                // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
                if (gps.encode(c)) // Did a new valid sentence come in?
                  newData = true;
              }
            }
          
            if (newData)
            {
              float flat, flon;
              unsigned long age;
              gps.f_get_position(&flat, &flon, &age);
              Serial.print("LAT=");
              Serial.println( flat, 6);
              Serial.print(" LON=");
              Serial.println(flon, 6);
            lat=flat;
            lang=flon;
            }
  l.end();
          
  espSerial.begin(115200); 
   
        Serial.println(lat,6);
        Serial.println(lang,6);

 Dist_F=(analogRead(A4)-pot_F_init)/7.33;
 Dist_B=(analogRead(A5)-pot_B_init)/7.33;
  File dataFile = SD.open("Loadprod.csv", FILE_WRITE);
  if (dataFile) {
    int data=9.8*scale.get_units();
    dataFile.print(data);
    dataFile.print(",");
    dataFile.print(Dist_F);
    dataFile.print(",");
    dataFile.println(Dist_B);
    dataFile.close();
  }

     
 if (millis() - lastmillis == 1000){ //Update every one second, this will be equal to reading frecuency (Hz).
 detachInterrupt(0);//Disable interrupt when calculating
 a = half_revolutions/2;
 rpm = a * 60; // Convert frecuency to RPM, note: this works for one interruption per full rotation. For two interrups per full rotation use half_revolutions * 30.

   
   Serial.print("Reading: ");
Serial.print(9.8*scale.get_units(),1);
float x=9.8*scale.get_units();
  Serial.print(" Newtons"); 
  Serial.println();
Serial.print(" pot back="); 
  Serial.println(Dist_B);
Serial.print(" pot front="); 
  Serial.println(Dist_F);


Serial.print("\t RPM =\t"); //print the word "RPM" and tab.
 Serial.print(rpm); // print the rpm value.
 Serial.print("\t Hz=\t"); //print the word "Hz".
 Serial.println(a); //print revolutions per second or Hz. And print new line

 
 
 half_revolutions = 0; // Restart the RPM counter
 lastmillis = millis(); // Update lasmillis
 attachInterrupt(0, rpm_fan, FALLING); //enable interrupt


        if(thingSpeakWrite(lat,lang,Dist_B,Dist_F,x)){
          
          Serial.println("datawritten");
        }
        else{
          Serial.println("sorry ");
        }


  espSerial.end();

}
