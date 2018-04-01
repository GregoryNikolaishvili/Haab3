boolean SDCardPresent = false;

#define FAN1_TEMPERATURE_DELTA 50
#define FAN2_TEMPERATURE_DELTA 100

static void ProcessBoard()
{
  int T;

  T = Board1TemperatureMax + (GetDeviceState(DEVICE_BOARD_1_FAN) ? -FAN1_TEMPERATURE_DELTA : FAN1_TEMPERATURE_DELTA);
  DeviceOnOff(DEVICE_BOARD_1_FAN, GetBoard1Temperature() > T);

  T = Board2TemperatureMax + (GetDeviceState(DEVICE_BOARD_2_FAN) ? -FAN2_TEMPERATURE_DELTA : FAN2_TEMPERATURE_DELTA);
  DeviceOnOff(DEVICE_BOARD_2_FAN, GetBoard2Temperature() > T);

  ShowLcdInfo();
}

static void InitSDCard()
{
  // see if the card is present and can be initialized:
  if (SD.begin(SD_CARD_PIN)) 
  {
    SDCardPresent = true;
#ifdef DEBUGING
    Serial.println("SD Card OK");
#endif
  }
  else
  {
    Log("SD Card failed, or not present");
  }
}

static void Log(const char* value)
{
#ifdef DEBUGING
  Serial.println(value);
#endif

  if (!SDCardPresent)
    return;

  File dataFile = SD.open(sys.getLogFileName("d"), FILE_WRITE);
  if (dataFile) 
  {
    PrintDateTime(&dataFile, now());
    dataFile.println(value);
    dataFile.close();
  }  
}

static void Log(const char* value, const char* value2)
{
#ifdef DEBUGING
  Serial.print(value);
  Serial.println(value2);
#endif

  if (!SDCardPresent)
    return;

  File dataFile = SD.open(sys.getLogFileName("d"), FILE_WRITE);
  if (dataFile) 
  {
    PrintDateTime(&dataFile, now());
    dataFile.print(value);
    dataFile.println(value2);
    dataFile.close();
  }  
}

static void Log(const char* value, const int value2)
{
#ifdef DEBUGING
  Serial.print(value);
  Serial.println(value2);
#endif

  if (!SDCardPresent)
    return;

  File dataFile = SD.open(sys.getLogFileName("d"), FILE_WRITE);
  if (dataFile) 
  {
    PrintDateTime(&dataFile, now());
    dataFile.print(value);
    dataFile.println(value2);
    dataFile.close();
  }  
}

static void LogAquariumState()
{
  if (!SDCardPresent)
    return;

  File dataFile = SD.open(sys.getLogFileName("a"), FILE_WRITE);
  if (dataFile) 
  {
    dataFile.print("{\"TIME\":");
    dataFile.print(now());
    dataFile.print(",\"E\":");
    dataFile.print(ErrorCode);

    dataFile.print(",\"T\":");
    dataFile.print(AquariumTemperature(false));

    dataFile.print(",\"L1\":");
    dataFile.print(GetDeviceState(AQUARIUM_LIGHT_1_RELAY));
    dataFile.print(",\"L2\":");
    dataFile.print(GetDeviceState(AQUARIUM_LIGHT_2_RELAY));
    dataFile.print(",\"L3\":");
    dataFile.print(GetDeviceState(AQUARIUM_LIGHT_3_RELAY));
    dataFile.print(",\"H1\":");
    dataFile.print(GetDeviceState(AQUARIUM_HEATER_1_RELAY));
    dataFile.print(",\"H2\":");
    dataFile.print(GetDeviceState(AQUARIUM_HEATER_2_RELAY));
    dataFile.println("}");

    dataFile.close();
  }  
}

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_En_pin, LCD_Rw_pin, LCD_Rs_pin, 4, 5, 6, 7);

// Creat a set of new characters
const uint8_t charBitmap[4][8] = {
  {
    0x00, 0x06, 0x1F, 0x16, 0x10, 0x10, 0x10, 0x10                           }
  , // Sump
  {
    0x01, 0x01, 0x01, 0x01, 0x0D, 0x1F, 0x0C, 0x00                           }
  , // Drain
  {
    0x00, 0x15, 0x0E, 0x11, 0x15, 0x11, 0x0E, 0x15                           }
  , // Fan
  {
    0x00, 0x1F, 0x04, 0x1F, 0x04, 0x1F, 0x04, 0x1F                           } // Heater
};


static void InitLcd()
{
  delay(100);
  lcd.begin (16, 2);
  for (int i = 0; i < 4; i++)
  {
    lcd.createChar (i, (uint8_t *)charBitmap[i] );
  }

  lcd.setBacklightPin(LCD_BACKLIGHT_PIN, POSITIVE);
  lcd.clear (); // go home
  lcd.print("Starting...");  
  lcd.setBacklight(HIGH); // Backlight on
}


static void ClearLcdRow(int row)
{
  lcd.setCursor (0, row);
  for (int i = 0; i < 16; i++)
    lcd.print(' ');
  lcd.setCursor (0, row);
}


static void ShowLcdInfo()
{
  if (!LcdPresent)
    return;

  static boolean showingBoard1 = false;

  lcd.clear();

  if (ErrorCode != 0)
  {
    lcd.print("Error: ");
    lcd.print(ErrorCode, HEX);
  }
  else
  {
    time_t value = now();
    int v = hour(value);
    if (v < 10)
      lcd.print('0');
    lcd.print(v);
    lcd.print(':');
    v = minute(value);
    if (v < 10)
      lcd.print('0');
    lcd.print(v);
  }

  if (GetDeviceState(AQUARIUM_SUMP_PUMP_RELAY))
  {
    lcd.setCursor (10, 0);
    lcd.print((char)0x00);
  }
  if (GetDeviceState(AQUARIUM_DRAIN_PUMP_RELAY))
  {
    lcd.setCursor (11, 0);
    lcd.print((char)0x01);
  }
  if (GetDeviceState(AQUARIUM_HEATER_1_RELAY))
  {
    lcd.setCursor (12, 0);
    lcd.print((char)0x03);
  }
  if (GetDeviceState(AQUARIUM_HEATER_2_RELAY))
  {
    lcd.setCursor (13, 0);
    lcd.print((char)0x03);
  }

  if (GetDeviceState(DEVICE_BOARD_1_FAN))
  {
    lcd.setCursor (14, 0);
    lcd.print((char)0x02);
  }
  if (GetDeviceState(DEVICE_BOARD_2_FAN))
  {
    lcd.setCursor (15, 0);
    lcd.print((char)0x02);
  }

  int T = AquariumTemperature(false);
  lcd.setCursor (0, 1);
  lcd.print("A=");
  lcd.print(T / 100);
  lcd.print('.');
  lcd.print((T % 100) / 10);
  lcd.write(0xDF);

  lcd.setCursor (8, 1);
  if (showingBoard1)
  {
    T = GetBoard1Temperature();
    lcd.print("B1=");
  }
  else
  {
    T = GetBoard2Temperature();
    lcd.print("B2=");
  }
  showingBoard1 = !showingBoard1;
  lcd.print(T / 100);
  lcd.print('.');
  lcd.print((T % 100) / 10);
  lcd.write(0xDF);
}


static void PrintDateTime(Stream* stream, time_t value)
{
  stream -> print(day(value));
  stream -> print('/');
  stream -> print(month(value));
  stream -> print('/');
  stream -> print(year(value) - 2000);
  stream -> print(' ');

  stream -> print(hour(value));
  stream -> print(':');
  stream -> print(minute(value));
  stream -> print(':');
  stream -> print(second(value));
  stream -> print(' ');
}

static volatile int HomeCinemaStage = 0; // 0 = nothing, 1 = 110v ON, 2 = Backlight ON, 3 = Light 1 OFF;     -3 = Light 2 ON, -2 = Light 1 ON, -1 = Backlight OFF, = 0 = 110v OFF

static void ProcessHomeCinema()
{
  HomeCinemaStage++;

  switch (HomeCinemaStage){
  case 1:  
    DeviceOn(TRANSFORMER_110_RELAY); 
    break;
  case 2: 
    DeviceOn(TV_BACKLIGHT_RELAY); 
    break;
  case 3: 
    DeviceOff(HT_LIGHT_1_RELAY); 
    HomeCinemaStage = 0;
    break;
    break;


  case -3: 
    DeviceOn(HT_LIGHT_2_RELAY); 
    break;
  case -2: 
    DeviceOn(HT_LIGHT_1_RELAY); 
    break;
  case -1: 
    DeviceOff(TV_BACKLIGHT_RELAY); 
    break;
  case 0:
    DeviceOff(TRANSFORMER_110_RELAY); 
    break;

  default:  
    HomeCinemaStage = 0;
  }
  if (HomeCinemaStage != 0)
    Alarm.timerOnce(3, ProcessHomeCinema);
}

static void StartHomeCinema()
{
  HomeCinemaStage = 0; // 1
  ProcessHomeCinema();
}

static void StopHomeCinema()
{
  HomeCinemaStage = -4;// -3
  ProcessHomeCinema();
}




