#include <Adafruit_Fingerprint.h>

#include <Adafruit_Fingerprint.h>

#include <Adafruit_Fingerprint.h>

/***************************************************
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_Fingerprint.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"


// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
//SoftwareSerial mySerial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial2

#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); 

String serverName = "http://192.168.8.119/accessControl/php/data.txt";
String serverNamePost = "http://192.168.8.119/accessControl/php/processing.php";
String serverNamePostReport = "http://192.168.8.119/accessControl/php/processing.php";
String serverNamePostTime = "http://192.168.8.119/accessControl/php/processing.php";
String ClosedOffice = "http://192.168.8.119/accessControl/php/processing.php";

//Wifi code
const char* ssid = "Mohale Digital Media (PTY) LTD";
const char* password =  "Chris5995==";


// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;
 
void setup()
{

  lcd.begin();
  
  Serial.begin(9600);


  +
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
 
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
 
  //End wifi connection
  
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  pinMode(12,OUTPUT); //Blue LED
  pinMode(26,OUTPUT); //Wifi LED
  pinMode(27,OUTPUT); //Red LED
  pinMode(33,OUTPUT); //Relay
  digitalWrite(27,LOW);
  digitalWrite(26,LOW);
  digitalWrite(12,LOW);
  digitalWrite(33,HIGH);


  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

void loop()                     // run over and over again
{
 
  getFingerprintID();
  delay(50);            //don't ned to run this at full speed.
  Serial.println(digitalRead(33));

  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(26,HIGH);
    delay(1000);
    digitalWrite(26,LOW);   
  }
  
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
      Serial.println("Authenticating..");
      lcd.clear();
      lcd.backlight();
      lcd.print("Authenticating..");
    
    /////////------------------------------------------------------------------------------------
      HTTPClient httpA;

      String serverPathTime = serverNamePostTime + "?findTime=now";
      httpA.begin(serverPathTime.c_str());
      
      // Send HTTP GET request
      int httpResponseCodeTime = httpA.GET();
      String getTimeRes = httpA.getString();
      if (httpResponseCodeTime>0){
          Serial.println(httpA.getString());
          if(getTimeRes == "Morning" || getTimeRes == "Day"){
              //////////////////
              Serial.println(httpA.getString());
               delay(500);
               httpA.end();
               HTTPClient http;
          
                String serverPath = serverNamePost + "?sendPrintId=" + String(finger.fingerID);
                
                // Your Domain name with URL path or IP address with path
                http.begin(serverPath.c_str());
                
                // Send HTTP GET request
                int httpResponseCode = http.GET();
                Serial.println(httpResponseCode);
                if (httpResponseCode>0) {
            
                    //Read file
                    http.begin(serverName);
                  
                    // Send HTTP GET request
                    int httpCode = http.GET();
                    
                    if (httpCode>0) {
                          Serial.print("HTTP Response code: ");
                          Serial.println(httpResponseCode);
                          String dataRespondFile = http.getString();
                          Serial.println(dataRespondFile);
                          
                          if(dataRespondFile == "Registerd"){
          
                             String serverPathReport = serverNamePostReport + "?printIdReport=" + String(finger.fingerID) + "&accessStatus=Approved";
                
                             // Your Domain name with URL path or IP address with path
                             http.begin(serverPathReport.c_str());
          
                            // Send HTTP GET request
                            int httpResponseCodeReport = http.GET();
          
                            if (httpResponseCodeReport>0){
                                Serial.println("send to db");
                            }
                            http.end();
          
                            Serial.println(finger.fingerID);
                            Serial.println("Found a print match!");
                            lcd.clear();
                            lcd.backlight();
                            lcd.print("Access granted");
                            digitalWrite(12,HIGH);
                            digitalWrite(33,LOW);
                            delay(10000);
                            digitalWrite(12,LOW);
                            digitalWrite(33,HIGH);
                            lcd.clear();
                            lcd.backlight();
                            lcd.print("Gate closed");
                            delay(1000);
                            digitalWrite(27,LOW);
                            digitalWrite(33,HIGH);
                            lcd.clear();
                            lcd.backlight();
                            lcd.print("System Online!");
                            
                        }else if(dataRespondFile == "Blocked"){
          
                             String serverPathReport = serverNamePostReport + "?printIdReport=" + String(finger.fingerID) + "&accessStatus=Denied";
                
                             // Your Domain name with URL path or IP address with path
                             http.begin(serverPathReport.c_str());
          
                            // Send HTTP GET request
                            int httpResponseCodeReport = http.GET();
          
                            if (httpResponseCodeReport>0){
                                Serial.println("send to db");
                            }
                            http.end();
                          
                            lcd.clear();
                            lcd.backlight();
                            lcd.print("User blocked");
                            digitalWrite(27,HIGH);
                            digitalWrite(33,HIGH);
                            Serial.println("User blocked");
                            delay(1000);
                            digitalWrite(27,LOW);
                            digitalWrite(33,HIGH);
                            lcd.clear();
                            lcd.backlight();
                            lcd.print("System Online!");
                       }else{
                            Serial.println("User Does not exist___g");
                            digitalWrite(27,HIGH);
                            lcd.clear();
                            lcd.backlight();
                            lcd.print("User Does not exist");
                            delay(1000);
                            digitalWrite(27,LOW);
                            lcd.clear();
                            lcd.backlight();
                            lcd.print("System Online!");
                       }
                      
                    }
                    else {
                      Serial.print("Error code: ");
                      Serial.println(httpResponseCode);
                    }
                    // Free resources
                    http.end();   
                }
                else {


                }
                // Free resources
                http.end();
         
              /////////////////
            
          }else if(getTimeRes == "Closed"){
          
              HTTPClient httpCLose;

              String sOfficeClose = ClosedOffice + "?office=closed&userIdPrintxx=" + String(finger.fingerID);
              httpCLose.begin(sOfficeClose.c_str());
              
              // Send HTTP GET request
              int httpResponseOfficeClose = httpCLose.GET();
              String getCloseRes = httpCLose.getString();
       
              if (httpResponseOfficeClose>0){
                
                 lcd.clear();
                 lcd.backlight();
                 lcd.print("Office closed!");
                 digitalWrite(27,HIGH);
                 digitalWrite(33,HIGH);
                 delay(1000);
                 lcd.clear();
                 lcd.backlight();  
                 digitalWrite(27,LOW);        
                 lcd.clear();
                 lcd.backlight();
                 lcd.print("System Online!");
                  
              }else{
                Serial.println("Faild request");
              }

              httpCLose.end();
          }
      }else{
           lcd.clear();
           lcd.backlight();
           lcd.print("Server error!");
           digitalWrite(27,HIGH);
           delay(1000);
           digitalWrite(27,LOW);
   
      }
      httpA.end();

    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    lcd.clear();
    lcd.backlight();
    lcd.print("Access denied");
    digitalWrite(27,HIGH);
    digitalWrite(33,HIGH);
    Serial.println("Did not find a match");
    delay(500);
    lcd.clear();
    lcd.backlight();
    digitalWrite(27,LOW); 
    digitalWrite(33,HIGH);
    lcd.clear();
    lcd.backlight();
    lcd.print("System Online!");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); 
  Serial.println(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
