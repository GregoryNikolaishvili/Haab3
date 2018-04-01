#define ALL_DEVICE_COUNT 20

unsigned int RelayDriverStateDefault    = 0b0010000000000000; // Right bit is index 0, left bit is index 15
const unsigned int RelayDriverInversion = 0b0000000001000000; // Right bit is index 0, left bit is index 15
//const unsigned int RelayDriverLog       = 0b1111100000000001; // Right bit is index 0, left bit is index 15
const unsigned int RelayDriverLog       = 0b1111111111111111; // Right bit is index 0, left bit is index 15

unsigned int ArduinoPinStateDefault     = 0b0000000000000000; // Right bit is index 0, left bit is index 15
const unsigned int ArduinoPinInversion  = 0b0000000000000000; // Right bit is index 0, left bit is index 15
//const unsigned int ArduinoPinLog        = 0b0000000000000000; // Right bit is index 0, left bit is index 15
const unsigned int ArduinoPinLog        = 0b1111111111111111; // Right bit is index 0, left bit is index 15

unsigned int RelayDriverState = 0;
unsigned int ArduinoPinState = 0;

const unsigned int ArduinoPinDevices [ALL_DEVICE_COUNT - 16] = 
{
  BOARD_1_FAN_PIN,
  BOARD_2_FAN_PIN,

  FEEDER_1_PIN,
  FEEDER_2_PIN
};

const char * DeviceNames[ALL_DEVICE_COUNT] = 
{
  "Bell",
  "HT Light 2",
  "HT Light 1",
  "Transf. 110",

  "Buttkicker",
  "UNKNOWN 1", // Feeder 1
  "UNKNOWN 2", // Feeder 2, reversed
  "TV Backlight",

  "Tank Light 3",
  "Tank Light 1",
  "Tank Light 2",
  "Drain Pump",

  "Heater 2",
  "Sump Pump",
  "Reserve",
  "Heater 1",

  "Fan 1",
  "Fan 2",

  "Feeder 1",
  "Feeder 2"
};


/*
prog_char DeviceName_0[] PROGMEM = "Bell"; 
 prog_char DeviceName_1[] PROGMEM = "HT Light 2";
 prog_char DeviceName_2[] PROGMEM = "HT Light 1";
 prog_char DeviceName_3[] PROGMEM = "Transf. 110";
 
 prog_char DeviceName_4[] PROGMEM = "Buttkicker";
 prog_char DeviceName_5[] PROGMEM = "Feeder 1"; // Feeder 1
 prog_char DeviceName_6[] PROGMEM = "Feeder 2"; // Feeder 2, reversed
 prog_char DeviceName_7[] PROGMEM = "TV Backlight";
 
 
 prog_char DeviceName_8[] PROGMEM  = "Tank Light 3";
 prog_char DeviceName_9[] PROGMEM  = "Tank Light 1";
 prog_char DeviceName_10[] PROGMEM = "Tank Light 2";
 prog_char DeviceName_11[] PROGMEM = "Drain Pump";
 
 prog_char DeviceName_12[] PROGMEM = "Heater 2";
 prog_char DeviceName_13[] PROGMEM = "Sump Pump";
 prog_char DeviceName_14[] PROGMEM = "Reserve";
 prog_char DeviceName_15[] PROGMEM = "Heater 1";
 
 prog_char DeviceName_16[] PROGMEM = "Fan 1";
 prog_char DeviceName_17[] PROGMEM = "Fan 2";
 
 prog_char DeviceName_18[] PROGMEM = "Feeder 1";
 prog_char DeviceName_19[] PROGMEM = "Feeder 2";
 
 PGM_P DeviceNames[ALL_DEVICE_COUNT] PROGMEM = 
 {   
 DeviceName_0,
 DeviceName_1,
 DeviceName_2,
 DeviceName_3,
 DeviceName_4,
 DeviceName_5,
 DeviceName_6,
 DeviceName_7,
 
 DeviceName_8,
 DeviceName_9,
 DeviceName_10,
 DeviceName_11,
 DeviceName_12,
 DeviceName_13,
 DeviceName_14,
 DeviceName_15,
 
 DeviceName_16,
 DeviceName_17,
 DeviceName_18,
 DeviceName_19
 };
 */

static void InitRelays()
{
  //  pinMode (RELAY_DRIVER_NCS, OUTPUT);

  RelayDriverState = (PortExpanderReadState(PORT_EXPANDER2_ADDRESS) << 8) | PortExpanderReadState(PORT_EXPANDER1_ADDRESS);

  for (int device = 0; device < 16; device++)
  {
    delay(200);
    DeviceOnOff(device, bitRead(RelayDriverStateDefault, device));
  }
  // For safety
  delay(200);
  //  PortExpanderWrite(PORT_EXPANDER1_ADDRESS, lowByte(RelayDriverState ^ RelayDriverInversion));
  //  PortExpanderWrite(PORT_EXPANDER2_ADDRESS, highByte(RelayDriverState ^ RelayDriverInversion));

  RelayDriverState = RelayDriverStateDefault;
  ArduinoPinState = ArduinoPinStateDefault;
  for (int device = 0; device < ALL_DEVICE_COUNT - 16; device++)
  {
    pinMode(ArduinoPinDevices[device], OUTPUT);
    digitalWrite(ArduinoPinDevices[device], bitRead(ArduinoPinState ^ ArduinoPinInversion, device));
  }
}

static void PortExpanderWrite(int address, int data)
{
  Wire.beginTransmission(address);
  Wire.write(~data);
  Wire.endTransmission();
}

static byte PortExpanderReadState(int address)
{
  Wire.requestFrom(address, 1);

  byte B;
  if (Wire.available()) 
    B = Wire.read();
  else B = 0xFF;
  return ~B;
}  


static boolean GetDeviceState(byte device)
{
  if (device < 16)
    return bitRead(RelayDriverState, device) != 0;
  else
    return bitRead(ArduinoPinState, device - 16) != 0;
}

////////////////////////////////////////////////////////////////////

static void DeviceLog(byte device, boolean onOff)
{
  //  char buffer[20];
  //  strcpy_P(buffer, (char*)pgm_read_word(&(DeviceNames[device]))); // Necessary casts and dereferencing, just copy. 

  boolean do_log;
  if (device < 16)
    do_log = bitRead(RelayDriverLog, device);
  else
    do_log = bitRead(ArduinoPinLog, device - 16);
  if (do_log)
  {
    Log((char*)DeviceNames[device], (char*)(onOff ? ": ON" : ": OFF"));
  }

  if (LcdPresent)
  {
    ShowLcdInfo();
    ClearLcdRow(1);
    lcd.print(DeviceNames[device]);
    lcd.print(GetDeviceState(device) ? ": ON" : ": OFF");
  }

#ifdef DEBUGING
  if (!do_log)
  {
    Serial.print(DeviceNames[device]);
    Serial.println(GetDeviceState(device) ? ": ON" : ": OFF");
  }
#endif
}

//// Devices
static void DeviceOnOff(byte device, boolean onOff)
{
  if (GetDeviceState(device) == onOff)
    return;

  if (device < 16)
  {
    bitWrite(RelayDriverState, device, onOff);
    if (device >= 8)
      PortExpanderWrite(PORT_EXPANDER2_ADDRESS, highByte(RelayDriverState ^ ArduinoPinInversion));
    else
      PortExpanderWrite(PORT_EXPANDER1_ADDRESS, lowByte(RelayDriverState ^ ArduinoPinInversion));
  }
  else
  {
    device = device - 16;
    bitWrite(ArduinoPinState, device, onOff);
    digitalWrite(ArduinoPinDevices[device], bitRead(ArduinoPinState ^ ArduinoPinInversion, device));
    device = device + 16;
  }

  DeviceLog(device, onOff);
}

static void DeviceOn(byte device)
{
  DeviceOnOff(device, true);
}

static void DeviceOff(byte device)
{
  DeviceOnOff(device, false);
}

static void DeviceSwitch(byte device)
{
  DeviceOnOff(device, !GetDeviceState(device));
}












