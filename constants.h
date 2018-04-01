/////////////////////////////////
// Arduino pinout

//  D0 - RX
//  D1 - TX
//  +D2 - IR Receiver 
//  +D3 - Board 1 Fan control (On arduino shield)
//  D4 - SD SPI CS
//  +D5 - HT Light Knob 1
//  +D6 - Extra 1 (2 Pin connector on board 1)                                                                      !!!!!!!!!!  FREE !!!!!!!!!! PWM
//  +D7 - HT Light Knob 2

//  +D8 - Aquarium Temperature Sensor
//  +D9 - LED (On arduino shield)
//  D10 - Ethernet SPI CS
//  D11 - Not connected (but should be SPI MOSI)
//  D12 - Not connected (but should be SPI MISO)
//  D13 - SPI SCK

//  A0 - SD Write Protect
//  A1 - SD Detect
//  +A2 - VBAT (NOW FREE) maybe humidity sensor?                                                                    !!!!!!!!!!  FREE !!!!!!!!!!
//  +A3 - BOARD2 TEMPERATURE (LM35)
//  -A20 +A4 - I2C  SDA (data)
//  -A21 +A5 - I2C  SCL (clock)
//


/////////////////////////////////
// RTC Board pinout (looking at DS3231 chip, battery is on back size)
// 1. SDA  - To Arduino A4
// 2. SCL  - To Arduino A5
// 3. VBAT (NOW DISCONNECTED)
// 4. GND
// 5. VCC  - 5v
// 6. 32KHz
// 7. INT/SQW
// 8. RST

/////////////////////////////////

#include <arduino.h>

// Error Code Bits
#define ERR_TIME_NOT_SET 0x01
#define ERR_AQUARIUM_TEMP 0x02
#define ERR_INTERNET 0x04
#define ERR_SYSTEM 0x80;

// For TimeAlarms Class
#define USE_SPECIALIST_METHODS

//// CONSTANTS /////

#define PROCESS_INTERVAL 15 // how frequentrly data will be checked and processed


#define REFERENCE_VOLTAGE 5.00
//#define REFERENCE_VOLTAGE_MULT (long)REFERENCE_VOLTAGE / 1024

#define BLINKING_LED 9
//#define RELAY_DRIVER_NCS 9

#define INFRARED_RECV_PIN 2 // D2
#define BOARD2_TEMP_RECV_PIN A3 
#define VBAT_RECV_PIN A2 

#define SD_CARD_PIN 4
#define ETHERNET_PIN 10

#define DS18B20_PIN 8 // 1Wire

#define BUTTON_HT_LIGHT_1 5
#define BUTTON_HT_LIGHT_2 7

#define BOARD_1_FAN_PIN 3
#define BOARD_2_FAN_PIN 43

#define FEEDER_1_PIN 40  // Old Eheim feeder, needs HIGH to turn on
#define FEEDER_2_PIN 41  // New Eheim feeder, needs LOW to turn on


// RELAYS

// PCF8574 ID - 1 0 0 A2, A1, A0: 0, 0, 0
#define PORT_EXPANDER1_ADDRESS  0x20
// PCF8574 ID - 1 0 0 A2, A1, A0: 1, 0, 0
#define PORT_EXPANDER2_ADDRESS 0x24


// 1st board
#define BELL_RELAY 0
#define HT_LIGHT_2_RELAY 1
#define HT_LIGHT_1_RELAY 2
#define TRANSFORMER_110_RELAY 3
#define BUTTKICKER_RELAY 4
#define UNKNOWN_1_RELAY 5
#define UNKNOWN_2_RELAY 6
#define TV_BACKLIGHT_RELAY 7

// 2nd board
#define AQUARIUM_LIGHT_3_RELAY 8
#define AQUARIUM_LIGHT_1_RELAY 9
#define AQUARIUM_LIGHT_2_RELAY 10
#define AQUARIUM_DRAIN_PUMP_RELAY 11
#define AQUARIUM_HEATER_2_RELAY 12
#define AQUARIUM_SUMP_PUMP_RELAY 13
#define AQUARIUM_RESERVE_RELAY 14
#define AQUARIUM_HEATER_1_RELAY 15

// Arduino connected devices

#define DEVICE_BOARD_1_FAN 16
#define DEVICE_BOARD_2_FAN 17
#define DEVICE_FEEDER_1 18
#define DEVICE_FEEDER_2 19


// LCD
#define LCD_I2C_ADDR    0x27  // Define I2C Address where the PCF8574A is
#define LCD_BACKLIGHT_PIN     3
#define LCD_En_pin  2
#define LCD_Rw_pin  1
#define LCD_Rs_pin  0


///////////////////

#define STORAGE_VERSION 2
#define STORAGE_VERSION_ADDRESS 0
#define STORAGE_START_ADDRESS 1

#define SETTINGS_COUNT 22
#define TIME_SETTING_ID -100

#define AlarmHM_Minutes(_hr_, _min_) (_hr_ * 60 + _min_)

#define AquariumTemperatureMin ((int)Settings[0])
#define AquariumTemperatureMax ((int)Settings[1])

#define AquariumLight1OnTime ((time_t)Settings[2] * SECS_PER_MIN)
#define AquariumLight1OffTime ((time_t)Settings[3] * SECS_PER_MIN)

#define AquariumLight2OnTime ((time_t)Settings[4] * SECS_PER_MIN)
#define AquariumLight2OffTime ((time_t)Settings[5] * SECS_PER_MIN)

#define AquariumLight3OnTime ((time_t)Settings[6] * SECS_PER_MIN)
#define AquariumLight3OffTime ((time_t)Settings[7] * SECS_PER_MIN)

#define AquariumWaterChangeDOW ((timeDayOfWeek_t)Settings[8])

#define AquariumWaterChangeStartTime ((time_t)Settings[9] * SECS_PER_MIN)
#define AquariumWaterChangeDrainDuration ((int)Settings[10])
#define AquariumWaterChangeFillDuration ((int)Settings[11])

#define FishFeeder1DOWA ((timeDayOfWeek_t)Settings[12])
#define FishFeeder1StartTimeA ((time_t)Settings[13] * SECS_PER_MIN)
#define FishFeeder1DOWB ((timeDayOfWeek_t)Settings[14])
#define FishFeeder1StartTimeB ((time_t)Settings[15] * SECS_PER_MIN)

#define FishFeeder2DOWA ((timeDayOfWeek_t)Settings[16])
#define FishFeeder2StartTimeA ((time_t)Settings[17] * SECS_PER_MIN)
#define FishFeeder2DOWB ((timeDayOfWeek_t)Settings[18])
#define FishFeeder2StartTimeB ((time_t)Settings[19] * SECS_PER_MIN)

#define Board1TemperatureMax ((int)Settings[20])
#define Board2TemperatureMax ((int)Settings[21])

