#define DEBUGING
//#define SUPER_DEBUG

// Read info in constants.h
#include "constants.h"
#include "System.h"

#include <Wire.h> // 207 bytes
#include <OneWire.h>
#include <Time.h> // 68 bytes
#include <TimeAlarms.h> // 0 bytes
#include <DS1307RTC.h> // 0 bytes // a basic DS1307 library that returns time as a time_t
#include <SPI.h> // 0 bytes
#include <Ethernet.h> // 668 bytes
#include <WebServer.h>
#include <EEPROM.h>
#include <Bounce.h> // 0 bytes
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>

/////////// WARNING ///////////////
// TimeAlarms only supported 6 alarms, so TimeAlarms.h modified with this: #define dtNBR_ALARMS 16  // max is 255
///////////////////////////////////

uint8_t ErrorCode = 0;

// Blinking LED
byte blinkingLedState = HIGH;

boolean LcdPresent = false;

unsigned long previousMillis = 0xFFFFFFFF;        // will store last time LED was updated
unsigned long secondTicks = 0;
time_t boardStartTime;

// Bouncing
Bounce bouncer_HT_Light1 = Bounce(BUTTON_HT_LIGHT_1, 30);  // 20 bytes
Bounce bouncer_HT_Light2 = Bounce(BUTTON_HT_LIGHT_2, 30);  // 20 bytes 

System sys;

void setup()
{
  delay(100);

  if (dtNBR_ALARMS != 16)
    ErrorCode = ErrorCode  | ERR_SYSTEM;

  digitalWrite(FEEDER_2_PIN, LOW);
  pinMode(FEEDER_1_PIN, OUTPUT); // Needs LOW at start, old EHEIM

  digitalWrite(FEEDER_2_PIN, HIGH);
  pinMode(FEEDER_2_PIN, OUTPUT); // Needs HIGH at start, new EHEIM

#ifdef DEBUGING
  Serial.begin(9600);
  Serial.println();
  Serial.println("Initializing..");
#endif

  pinMode(BLINKING_LED, OUTPUT);
  pinMode(SD_CARD_PIN, OUTPUT);
  pinMode(ETHERNET_PIN, OUTPUT);

  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(53, OUTPUT);

  digitalWrite(BLINKING_LED, HIGH);    
  digitalWrite(SD_CARD_PIN, HIGH); // Disable SD card
  digitalWrite(ETHERNET_PIN, HIGH); // Disable Ethernet card

  analogReference(EXTERNAL);

  pinMode(VBAT_RECV_PIN, INPUT);      
  pinMode(BOARD2_TEMP_RECV_PIN, INPUT);      

  pinMode(BUTTON_HT_LIGHT_1, INPUT);
  pinMode(BUTTON_HT_LIGHT_2, INPUT);

  Wire.begin();

  InitLcd();

  InitRelays();

  ReadSettings();

  // Sync time with RTC
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  unsigned long startMillis = millis();
  do 
  {
    delay(100);
    if (timeStatus() == timeSet)
      break;
  } 
  while(millis() - startMillis < 1000);
  if (timeStatus() != timeSet)
    ErrorCode = ErrorCode | ERR_TIME_NOT_SET;

  InitSDCard();
  
  Log("Start");

  InitAquariumTempSensor();

  InitAquarium();

  InitIR();

  InitWWW();

#ifdef DEBUGING
  Serial.print("Free memory: ");
  Serial.println(sys.ramFree());
#endif
  delay(1000);

  LcdPresent = true;
  ShowLcdInfo();

  boardStartTime = now();
  Alarm.timerRepeat(1, OncePerSecond);
}

void  loop()
{
  unsigned long currentMillis = millis();

  if ((currentMillis < previousMillis) || (currentMillis - previousMillis >= 500))
  {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;

    OncePerHalfSecond();
  }

  //  HT LIGHTS 
  if (bouncer_HT_Light1.update() && (bouncer_HT_Light1.read() == HIGH))
    DeviceSwitch(HT_LIGHT_1_RELAY);

  if (bouncer_HT_Light2.update() && (bouncer_HT_Light2.read() == HIGH))
    DeviceSwitch(HT_LIGHT_2_RELAY);
  // END OF HT LIGHTS 

  ProcessIR();

  ProcessWWW(); 
}

static void OncePerHalfSecond(void)
{
  // Blinking
  blinkingLedState = !blinkingLedState;
  digitalWrite(BLINKING_LED, blinkingLedState);    

  Alarm.delay(0); // service the alarm timers once per blink
}

static void OncePerSecond(void)
{
  if ((secondTicks + 2) % PROCESS_INTERVAL == 0) // 2 seconds before processing aquarium
    StartAquariumTempMeasurement();

  if (secondTicks % PROCESS_INTERVAL == 0)
  {
    ProcessAquarium();
    ProcessBoard();
  }

  if (secondTicks % 60 == 0)
    OncePerMinute();

  if (secondTicks % 3600 == 0)
    OncePerHour();

  secondTicks++;
}


static void OncePerMinute(void)
{
  LogAquariumState();
}

static void OncePerHour(void)
{
}



