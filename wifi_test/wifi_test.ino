#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

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

uint8_t id;

const char* ssid = "Mohale Digital Media (PTY) LTD";
const char* password =  "Chris5995==";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.8.119/accessControl/php/processing.php";


// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(9600); 
  pinMode(26,OUTPUT); //Wifi LED
  digitalWrite(26,LOW);
  
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");

while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
    // initialize the LCD
  lcd.begin();

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
     
     // Turn on the blacklight and print a message.
    lcd.setCursor(0, 1);
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Found");
    lcd.setCursor(0, 1);
    lcd.print("fingerprint!");
  
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    
    // Turn on the blacklight and print a message.
    lcd.setCursor(0, 1);
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("not found!");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));

  lcd.clear();
  lcd.backlight();
  lcd.print("Reading sensor!");
  
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      digitalWrite(26,HIGH);
      delay(1000);
      digitalWrite(26,LOW); 
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }

  
Serial.println("Ready to enroll a fingerprint!");
 
  lcd.clear();
  lcd.backlight();
  lcd.print("Ready to enroll!");
  delay(2000);
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");

  pinMode(12,OUTPUT);
  pinMode(27,OUTPUT);
  digitalWrite(12,LOW);
  digitalWrite(27,LOW);

  
  lcd.clear();
  lcd.backlight();
  lcd.print("Enter ID!");
  id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (!  getFingerprintEnroll());
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
   
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("finger!");
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
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
       lcd.clear();
       lcd.backlight();
        lcd.setCursor(0, 0);
       lcd.print("No");
       lcd.setCursor(0, 1);
       lcd.print("fingerprint!");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
        lcd.clear();
        lcd.backlight();
        lcd.setCursor(0, 0);
        lcd.print("No");
        lcd.setCursor(0, 1);
        lcd.print("fingerprint");
      return p;
    default:
      Serial.println("Unknown error");
        lcd.clear();
        lcd.backlight();
        lcd.print("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  lcd.clear();
  lcd.backlight();
  lcd.print("Remove finger");
  
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Place the");
  lcd.setCursor(0, 1);
  lcd.print("same fnger");
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
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
      lcd.clear();
      lcd.backlight();
      lcd.print("No fingerprint");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      lcd.clear();
      lcd.backlight();
       lcd.setCursor(0, 0);
      lcd.print("No");
      lcd.setCursor(0, 1);
      lcd.print("fingerprint");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
      lcd.clear();
      lcd.backlight();
      lcd.print("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
      lcd.clear();
      lcd.backlight();
      lcd.setCursor(0, 0);
      lcd.print("Communication");
      lcd.setCursor(0, 1);
      lcd.print("error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
      Serial.println("Fingerprints did not match");
      digitalWrite(27,HIGH);
      delay(10000);
      digitalWrite(27,LOW);
      lcd.clear();
      lcd.backlight();
      lcd.setCursor(0, 0);
      lcd.print("Print didn't");
      lcd.setCursor(0, 1);
      lcd.print("match!");
    return p;
  } else {
    Serial.println("Unknown error");
      lcd.clear();
      lcd.backlight();
      lcd.print("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
      Serial.println("Stored!");
      lcd.clear();
      lcd.backlight();
      lcd.print("Stored!");
      digitalWrite(12,HIGH);
      delay(10000);
      digitalWrite(12,LOW);
      
      HTTPClient http;

      String serverPath = serverName + "?printId=" + String(id);
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      Serial.print(httpResponseCode);
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();

      
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
      lcd.clear();
      lcd.backlight();
      lcd.print("Error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
       lcd.clear();
      lcd.backlight();
      lcd.setCursor(0, 1);
      lcd.print("Could not");
      lcd.setCursor(0, 1);
      lcd.print("store in location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
      lcd.clear();
      lcd.backlight();
      lcd.print("Error");
    return p;
  } else {
    Serial.println("Unknown error");
      lcd.clear();
      lcd.backlight();
      lcd.print("Unknown error");
    return p;
  }

  return true;
}
