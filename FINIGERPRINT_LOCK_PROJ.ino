#include <Key.h>
#include <Keypad.h>
#include <Servo.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
/*char hexaKeys[ROWS][COLS] = {
  {1, 2, 3, 4},
  {5, 6, 7, 8},
  {9, 10, 11, 12},
  {13, 14, 15, 16}
  };

*/
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '8', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

/*
  char hexaKeys[ROWS][COLS] = {
  {1, 2, 3, 4 },
  {5, 6, 7, 8 },
  {9, 10, 11, 12 },
  {13, 14, 15, 16 },
  {18, 19, 20, 21}
  };
  byte rowPins[ROWS] = {10, 9, 8, 7, 6}; //connect to the row pinouts of the keypad
  byte colPins[COLS] = {2, 3, 4, 5}; //connect to the column pinouts of the keypad

*/
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define SM 13
#define BUZ 10

SoftwareSerial mySerial(11, 12);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Servo myservo;  // create servo object to control a servo

uint8_t id;
int pos = 0;    // variable to store the servo position
//int T_Cnt = 0;
int timer = 0;
bool state = 0;
int FP_ID = 0;


void setup(){
  Serial.begin(9600);
  delay(15);
  pinMode(BUZ, OUTPUT);
  digitalWrite(BUZ, HIGH);   // turn the LED on (HIGH is the voltage level)

  myservo.attach(13);  // attaches the servo on pin 9 to the servo object
  myservo.write(0);              // tell servo to go to position in variable 'pos'
  while (!Serial);  
  delay(100);
  
  // set the data rate for the sensor serial port
  finger.begin(9600);
  
  lcd.clear();
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Biometric Safe!!");
  Serial.println("Finger Print based Safe Lock System!!!");
  delay(2000);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    lcd.clear();
    lcd.home();
    lcd.print("FPSensor Conkted");
    delay(2000);
  } 
  else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.clear();
    lcd.home();
    lcd.print("FPSensor Issue!!!");
    delay(2000);
    while (1) {
      delay(1);
    }
  }

  HomeScreen();
  delay(1000);
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop(){
  char customKey = customKeypad.getKey();
  
  if (customKey){
    switch (customKey){
    case 'A':
      Serial.println(customKey + "Pressed");
      lcd.clear();
      lcd.home();
      lcd.print("Place Finger!!!");

      //sei();
      timer = 0;
      while(!FP_ID){
//        Serial.print("Loop");
        FP_ID = getFingerprintIDez();
        delay(50);            //don't ned to run this at full speed.
        Serial.println("Finger Print ID: #" + String(FP_ID));
      }      
      FP_ID = 0;
//      Serial.print("Loop Out");
      HomeScreen();
      delay(1000);
      break;
      
    case 'B':
      Serial.println(customKey);
      Serial.println("Ready to enroll a fingerprint!");
      Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
      lcd.clear();
      lcd.home();
      lcd.print("Use Serial Comn.");
      lcd.setCursor(0,1);
      lcd.print("& Follow Command");
      id = readnumber();
      if (id == 0) {// ID #0 not allowed, try again!
         return;
      }
      Serial.print("Enrolling ID #");
      Serial.println(id);

      while (!getFingerprintEnroll() );
      HomeScreen();
      break;
      
    case 'D':
      lcd.clear();
      lcd.home();
      lcd.print("Use Serial Comn.");
      lcd.setCursor(0,1);
      lcd.print("& Follow Command");
      delay(2000);

      Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
      uint8_t id = readnumber();
      if (id == 0) {// ID #0 not allowed, try again!
         return;
      }
      Serial.print("Deleting ID #");
      Serial.println(id);
      
      deleteFingerprint(id);
      HomeScreen();
      delay(1000);
      break;
    }
  }
}

uint8_t getFingerprintEnroll() {
    
    int p = -1;
    Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
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
        return p;
        case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
        default:
        Serial.println("Unknown error");
        return p;
    }
    Serial.println("Remove finger");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER) {
        p = finger.getImage();
    }
    Serial.print("ID "); Serial.println(id);
    p = -1;
    Serial.println("Place same finger again");
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
        return p;
        case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
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
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
    } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match, may be address already taken!!!!");
    return p;
    } else {
    Serial.println("Unknown error");
    return p;
    }
    Serial.print("ID "); Serial.println(id);
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
    } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
    } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
    } else {
    Serial.println("Unknown error");
    return p;
    }
}


// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    Serial.println("Waiting For Finger Scanning!!!");
    return 0;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Finger Print not Available!!!");
    Invalid_Image();
    return -1;
  }

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  {
    Serial.println("Image not Found in Search!!!");
    Invalid_Image();
    return -1;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  lcd.clear();
  lcd.home();
  lcd.print("Matching ID #" + String(finger.fingerID));
  lcd.setCursor(0, 1);
  lcd.print("Opening Safe!!!");
  delay(1000);

  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
  delay(3000);
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
  return finger.fingerID;
}

void Invalid_Image(void){
    lcd.clear();
    lcd.home();
    lcd.print("Invalid Finger!!!");
    
    for(int i = 0; i<5; i++){
        digitalWrite(BUZ, LOW);   // turn the BUZ on (HIGH is the voltage level)
        delay(200);                       // wait for a second
        digitalWrite(BUZ, HIGH);    // turn the BUZ off by making the voltage LOW
        delay(200);                       // wait for a second
    }
    HomeScreen();
}

uint8_t deleteFingerprint(uint8_t id) {
    uint8_t p = -1;
    
    p = finger.deleteModel(id);
    if (p == FINGERPRINT_OK) {
        Serial.println("Deleted!");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FINGERPRINT_BADLOCATION) {
        Serial.println("Could not delete in that location");
        return p;
    } else if (p == FINGERPRINT_FLASHERR) {
        Serial.println("Error writing to flash");
        return p;
    } else {
        Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
        return p;
    }   
}

void HomeScreen(void){
    lcd.clear();
    lcd.home();
    lcd.print("A-Scan B-Enroll");
    lcd.setCursor(0, 1);
    lcd.print("D - Delete ID");
}
