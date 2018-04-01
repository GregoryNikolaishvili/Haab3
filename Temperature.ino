//// Temperature Sensor

// DS18B20
OneWire  wire1(DS18B20_PIN);
byte DS18B20_Address[8];

static void InitAquariumTempSensor()
{
  if (!wire1.search(DS18B20_Address)) 
  {
    Log("DS18B20 not found");
    wire1.reset_search();
    ErrorCode = ErrorCode | ERR_AQUARIUM_TEMP;
    delay(250);
    return;
  }

  /*
#ifdef DEBUGING
   Serial.print("ROM =");
   for(int i = 0; i < 8; i++) 
   {
   Serial.write(' ');
   Serial.print(DS18B20_Address[i], HEX);
   }
   #endif
   */

  if (OneWire::crc8(DS18B20_Address, 7) != DS18B20_Address[7]) 
  {
    ErrorCode = ErrorCode | 0x08;
    Log("DS18B20 CRC is not valid!");
    return;
  }

  ErrorCode = ErrorCode & (~ERR_AQUARIUM_TEMP);

  StartAquariumTempMeasurement();
}

static void StartAquariumTempMeasurement()
{
  if ((ErrorCode & ERR_AQUARIUM_TEMP) != 0)
    InitAquariumTempSensor();

  if ((ErrorCode & ERR_AQUARIUM_TEMP) != 0)
    return;

  wire1.reset();
  wire1.select(DS18B20_Address);
  wire1.write(0x44, 1);         // start conversion, with parasite power on at the end
}

// Returns temperature multiplied by 100
static int AquariumTemperature(boolean requestMeasure)
{
  if ((ErrorCode & ERR_AQUARIUM_TEMP) != 0)
    return 9999;

  if (requestMeasure)
  {
    StartAquariumTempMeasurement();
    delay(1000);     // maybe 750ms is enough, maybe not
  }

  // we might do a wire1.depower() here, but the reset will take care of it.
  byte present = wire1.reset();

  if (present == 0)
  {
    ErrorCode = ErrorCode | ERR_AQUARIUM_TEMP;
    return 9999;
  }

  wire1.select(DS18B20_Address);    
  wire1.write(0xBE);  // Read Scratchpad

#ifdef SUPER_DEBUG
  Serial.print("Temp Data = ");
#endif

  byte data[12];
  for (int i = 0; i < 9; i++) // we need 9 bytes
  {           
    data[i] = wire1.read();
#ifdef SUPER_DEBUG
    Serial.print(data[i], HEX);
    Serial.print(" ");
#endif
  }

#ifdef SUPER_DEBUG
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();
#endif

  // convert the data to actual temperature
  unsigned int raw = (data[1] << 8) | data[0];
  byte cfg = (data[4] & 0x60);

#ifdef SUPER_DEBUG
  Serial.print("Temp raw: ");
  Serial.println(raw);  
#endif

  if (cfg == 0x00) 
    raw = raw << 3;  // 9 bit resolution, 93.75 ms
  else 
    if (cfg == 0x20) 
    raw = raw << 2; // 10 bit res, 187.5 ms
  else 
    if (cfg == 0x40) 
    raw = raw << 1; // 11 bit res, 375 ms
  // default is 12 bit resolution, 750 ms conversion time

  byte sign = data[1] & 0x80;  // test most sig bit
  if (sign) // negative
    raw = (raw ^ 0xFF) + 1; // 2's complement the answer

  int T = raw * 25 / 4; // 100 / 16
  if (sign) // negative
    T = -T;
  return T;

  // 0.37 degrees more than should be
}


/*
#define DS1631_I2C_ADDR 0x4F
 
 // Returns temperature multiplied by 100
 static int AquariumTemperature()
 {
 // READ T°
 Wire.beginTransmission(DS1631_I2C_ADDR);
 Wire.write((int)(0xAA)); // @AA : Temperature
 Wire.endTransmission();
 Wire.requestFrom(DS1631_I2C_ADDR, 2); // READ 2 bytes
 if (Wire.available()) // 1st byte
 {
 byte Th = Wire.read(); // receive a byte
 Wire.available(); // 2nd byte
 byte Tl = Wire.read(); // receive a byte
 
 // T° processing
 byte sign = Th & 0x80;
 int T = ((Th << 4) + (Tl >> 4)) * 100 / 16;
 if (sign != 0)
 T =  -T;
 return T;
 }
 else
 return 9999;
 }
 
 
 static void InitTempSensor()
 {
 // Stop conversion to be able to modify "Access Config" Register
 Wire.beginTransmission(DS1631_I2C_ADDR);
 Wire.write((int)(0x22)); // Stop conversion
 Wire.endTransmission();  
 
 // Read "Access Config" regsiter
 //  Wire.beginTransmission(DS1631_I2C_ADDR);
 //  Wire.write((int)(0xAC)); // @AC : Access Config
 //  Wire.endTransmission();
 //  Wire.requestFrom(DS1631_I2C_ADDR,1); //Read 1 byte
 //  Wire.available();
 //  int AC = Wire.read(); // receive a byte
 
 // WRITE into "Access Config" Register
 Wire.beginTransmission(DS1631_I2C_ADDR);
 Wire.write(0xAC); // @AC : Acces Config
 Wire.write(0x0C); // Continuous conversion & 12 bits resolution
 Wire.endTransmission();
 
 // READ "Access Config" register
 //  Wire.beginTransmission(DS1631_ADDR);
 //  Wire.write((int)(0xAC)); // @AC : Acces Config
 //  Wire.endTransmission();
 //  Wire.requestFrom(DS1631_ADDR,1);
 //  Wire.available();
 //  AC = Wire.read();
 
 // START conversion to get T°
 Wire.beginTransmission(DS1631_I2C_ADDR);
 Wire.write((int)(0x51)); // Start Conversion
 Wire.endTransmission();
 
 if (AquariumTemperature() == 9999)
 ErrorCode = ErrorCode | ERR_AQUARIUM_TEMP;
 }
 */


#define DS3231_I2C_ADDRESS 0x68


// Returns RTC temperature multiplied by 100
static int GetBoard1Temperature(void)
{
  //temp registers (11h-12h) get updated automatically every 64s
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 2);

  if (Wire.available()) // 1st byte
  {
    byte Th = Wire.read(); // receive a byte
    Wire.available(); // 2nd byte
    byte Tl = Wire.read(); // receive a byte

    byte sign = Th & 0x80;
    int raw = ((Th << 8) | Tl) >> 6;
    if (sign) // negative
      raw = (raw ^ 0xFF) + 1; // 2's complement the answer
    int T = (((Th << 8) | Tl) >> 6) * 25; // 100 / 4
    if (sign)
      T = -T;
    return T;
  }
  else
    return 9999;
}

// Returns board 2 temperature multiplied by 100
static int GetBoard2Temperature(void)
{
  unsigned long value = analogRead(BOARD2_TEMP_RECV_PIN);
  delay(1);
  value = 0;
  for (int i = 0; i < 10; i++)
  {
    value = value + analogRead(BOARD2_TEMP_RECV_PIN);
    delay(1);
  }
 
  return (5000ul * value / 1023ul);
}

