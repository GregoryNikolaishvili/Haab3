int Settings[SETTINGS_COUNT] = 
{
  2900, 
  3000, 
  AlarmHM_Minutes(10, 15), // Aquarium Light 1 On Time
  AlarmHM_Minutes(21, 45), // Aquarium Light 1 Off Time
  AlarmHM_Minutes(10, 00), // Aquarium Light 2 On Time
  AlarmHM_Minutes(22, 00), // Aquarium Light 2 Off Time
  AlarmHM_Minutes(21, 59), // Aquarium Light 3 On Time
  AlarmHM_Minutes(23, 00), // Aquarium Light 3 Off Time

  B00000001, // Aquarium Water Change DOW - Sunday by default
  AlarmHM_Minutes(21, 8), // Aquarium Water Change Start Time 
  60 * 5, // Aquarium Water Change Drain Duration (Seconds)
  60 * 6, // Aquarium Water Change Fill Duration (Seconds)

  B01111110, // Feeder 1 A DOW - all except Sunday by default 
  AlarmHM_Minutes(11, 0), // Feeder 1 A Start Time 
  B01111110, // Feeder 1 B DOW - all except Sunday by default 
  AlarmHM_Minutes(13, 0), // Feeder 1 B Start Time 

  B01111110, // Feeder 2 DOW A- all except Sunday by default 
  AlarmHM_Minutes(16, 0), // Feeder 2 A Start Time 
  B01111110, // Feeder 2 DOW B - all except Sunday by default 
  AlarmHM_Minutes(20, 0), // Feeder 2 B Start Time 

  3400, // Max temp of board1
  3500  // Max temp of board2
};

const char* SettingNames[SETTINGS_COUNT] = 
{
  "Tank Temp. Min",
  "Tank Temp. Max",

  "Light 1 On Time",
  "Light 1 Off Time",
  "Light 2 On Time",
  "Light 2 Off Time",
  "Light 3 On Time",
  "Light 3 Off Time",

  "Water Change DOW",
  "Water Change Start Time",
  "Water Change Drain Duration (sec)",
  "Water Change Fill Duration (sec)",

  "Feed 1 DOW A",
  "Feed 1 Start Time A",
  "Feed 1 DOW B",
  "Feed 1 Start Time B",

  "Feed 2 DOW A",
  "Feed 2 Start Time A",
  "Feed 2 DOW B",
  "Feed 2 Start Time B",

  "Board1 Temp. Max",
  "Board2 Temp. Max"
};

/*
prog_char SettingName_0[] PROGMEM = "Tank Temp. Min";
 prog_char SettingName_1[] PROGMEM = "Tank Temp. Max";
 
 prog_char SettingName_2[] PROGMEM = "Light 1 On Time";
 prog_char SettingName_3[] PROGMEM = "Light 1 Off Time";
 prog_char SettingName_4[] PROGMEM = "Light 2 On Time";
 prog_char SettingName_5[] PROGMEM = "Light 2 Off Time";
 prog_char SettingName_6[] PROGMEM = "Light 3 On Time";
 prog_char SettingName_7[] PROGMEM = "Light 3 Off Time";
 
 prog_char SettingName_8[] PROGMEM = "Water Change DOW";
 prog_char SettingName_9[] PROGMEM = "Water Change Start Time";
 prog_char SettingName_10[] PROGMEM = "Water Change Duration (sec)";
 
 prog_char SettingName_11[] PROGMEM = "Feed 1 DOW A";
 prog_char SettingName_12[] PROGMEM = "Feed 1 Start Time A";
 prog_char SettingName_13[] PROGMEM = "Feed 1 DOW B";
 prog_char SettingName_14[] PROGMEM = "Feed 1 Start Time B";
 
 prog_char SettingName_15[] PROGMEM = "Feed 2 DOW A";
 prog_char SettingName_16[] PROGMEM = "Feed 2 Start Time A";
 prog_char SettingName_17[] PROGMEM = "Feed 2 DOW B";
 prog_char SettingName_18[] PROGMEM = "Feed 2 Start Time B";
 
 prog_char SettingName_19[] PROGMEM = "Board1 Temp. Max";
 prog_char SettingName_20[] PROGMEM = "Board2 Temp. Max";
 
 PGM_P SettingNames[SETTINGS_COUNT] PROGMEM = 
 {   
 SettingName_0,
 SettingName_1,
 SettingName_2,
 SettingName_3,
 SettingName_4,
 SettingName_5,
 SettingName_6,
 SettingName_7,
 SettingName_8,
 SettingName_9,
 SettingName_10,
 SettingName_11,  
 SettingName_12,  
 SettingName_13,
 SettingName_14,
 SettingName_15,  
 SettingName_16,  
 SettingName_17,
 SettingName_18, 
 SettingName_19,
 SettingName_20  
 };
 */

// Name: WebHeader, Adress: 100, Length: 58, Value: HTTP/1.1 200 OK Content-Type: text/html Connnection: close
//#define EEPROM_STR_WebHeader 100
//#define LEN_EEPROM_STR_WebHeader 65

#define StorageReadByte(__address__) (EEPROM.read(__address__))
#define StorageWriteByte(__address__, __value__) (EEPROM.write(__address__, __value__))

static void ReadSettings()
{
  if (StorageReadByte(STORAGE_VERSION_ADDRESS) != STORAGE_VERSION)
    SaveSettings();
  else
    for (byte setting_id = 0; setting_id < SETTINGS_COUNT; setting_id++)
    {
      Settings[setting_id] = (int)StorageReadInt(STORAGE_START_ADDRESS + (setting_id << 1));
    }
}

static void SaveSettings()
{
  for (byte setting_id = 0; setting_id < SETTINGS_COUNT; setting_id++)
  {
    StorageWriteInt(STORAGE_START_ADDRESS + (setting_id << 1), (unsigned int)Settings[setting_id]);
  }
  StorageWriteByte(STORAGE_VERSION_ADDRESS, STORAGE_VERSION);
}

static unsigned int StorageReadInt(int address)
{
  return ((unsigned int)EEPROM.read(address) << 8) + EEPROM.read(address+1);
}

static void StorageWriteInt(int address, unsigned int value)
{
  EEPROM.write(address, (value >> 8) & 0xFF);
  EEPROM.write(address + 1, value & 0xFF);
}

/*
static void StorageWriteString(int address, char* value)
 {
 int len = strlen(value) + 1;
 for (int i = address; i < address + len; i++)
 EEPROM.write(i, value[i]);
 }
 */

/*
static void StorageSendToStream(Stream *stream, int address, int len)
 {
 for (int i = address; i < address + len; i++)
 stream->print((char)EEPROM.read(i));
 }
 */


