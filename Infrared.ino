
// IR Receiver
decode_results irResults;
IRrecv InfraredReceiver(INFRARED_RECV_PIN); // 1 bytes

static void InitIR()
{
  // Start IR Receiver
  InfraredReceiver.enableIRIn(); // Start the receiver
}

unsigned long LastReceivedIRValue = 0;
unsigned long prevIRValue = 0;
unsigned long previousMillisIR = 0;
unsigned long previousMillisIRProcessed = 0;

static void ProcessIR()
{
  if (InfraredReceiver.decode(&irResults)) 
  {
    unsigned long irvalue = irResults.value;
    if (irvalue == 0)
    {
      InfraredReceiver.resume(); // Receive the next value
      return;
    }

    //InfraredDump(&irResults);
    unsigned long currentMillis = millis();

    Log("Received IR: ", irvalue);

#ifdef DEBUGING
    Serial.print("Received IR: ");
    Serial.print(irvalue, HEX);
#endif

    if ((currentMillis - previousMillisIRProcessed) > 1000) // do not process code withing 0.5 sec
    {
      if ((currentMillis < previousMillisIR) || ((currentMillis - previousMillisIR) > 500)) // if more than 0.5 sec passed after previous event
        prevIRValue = 0;

      if ((irvalue != 0) && (irvalue == prevIRValue))
      {
        prevIRValue = 0;
        previousMillisIRProcessed = currentMillis; 

        if (LcdPresent)
        {
          ClearLcdRow(1);
          lcd.print("IR: 0x");
          lcd.print(irvalue, HEX);
        }
#ifdef DEBUGING
        Serial.println('*');
#endif      
        LastReceivedIRValue = irvalue;
        ProcessIRCode(irvalue);
      }
      else
      {
        prevIRValue = irvalue;
#ifdef DEBUGING
        Serial.println();
#endif      
      }
    }
#ifdef DEBUGING
    else
      Serial.println();
#endif      

    previousMillisIR = currentMillis;
    InfraredReceiver.resume(); // Receive the next value
  }
}


static boolean ProcessIRCode(unsigned long value)
{
  switch(value) {
    
  case 0x9EB92: 
    DeviceOn(HT_LIGHT_1_RELAY);
    break;
  case 0x5EB92: 
    DeviceOff(HT_LIGHT_1_RELAY);
    break;
  case 0x3EB92:
    DeviceOn(HT_LIGHT_2_RELAY);
    break;
  case 0xDEB92:
    DeviceOff(HT_LIGHT_2_RELAY);
    break;
  case 0x5F0C:
    DeviceOn(TV_BACKLIGHT_RELAY);
    break;
  case 0x520C:
    DeviceOff(TV_BACKLIGHT_RELAY);
    break;
  case 0x410D:
    DeviceOn(BELL_RELAY);
    break;
  case 0x210D:
    DeviceOff(BELL_RELAY);
    break;
    
  case 0x540C:
    DeviceOn(TRANSFORMER_110_RELAY);
    break;
  case 0xA8B92:
    DeviceOff(TRANSFORMER_110_RELAY);
    break;
  case 0x220C:
    DeviceOn(BUTTKICKER_RELAY);
    break;
  case 0x3C0C:
    DeviceOff(BUTTKICKER_RELAY);
    break;

  case 0x11:
    DeviceOn(AQUARIUM_LIGHT_1_RELAY);
    break;
  case 0x811:
    DeviceOff(AQUARIUM_LIGHT_1_RELAY);
    break;
  case 0x411:
    DeviceOn(AQUARIUM_LIGHT_2_RELAY);
    break;
  case 0xC11:
    DeviceOff(AQUARIUM_LIGHT_2_RELAY);
    break;
  case 0x211:
    DeviceOn(AQUARIUM_LIGHT_3_RELAY);
    break;
  case 0xA11:
    DeviceOff(AQUARIUM_LIGHT_3_RELAY);
    break;
    
  case 0x611:
    DeviceOn(AQUARIUM_SUMP_PUMP_RELAY);
    break;
  case 0xE11:
    DeviceOff(AQUARIUM_SUMP_PUMP_RELAY);
    break;
  case 0x111:
    DeviceOn(AQUARIUM_DRAIN_PUMP_RELAY);
    break;
  case 0x51:
    DeviceOff(AQUARIUM_DRAIN_PUMP_RELAY);
    break;
    
  case 0x90b92:  // Sony 0
    AquariumStartWaterChange();
    break;
    
  case 0x4CB92: // Play
    StartHomeCinema();
    break;

  case 0x1CB92: // Stop
    StopHomeCinema();
    break;
    
/*  case 0x20B92:
    DeviceSwitch(AQUARIUM_HEATER_1_RELAY);
    break;
  case 0xA0B92:
    DeviceSwitch(AQUARIUM_HEATER_2_RELAY);
    break;

  case 0x9EB92: // Up Arrow
    AquariumFeeder1();
    break;
  case 0x5EB92: // Down Arrow
    AquariumFeeder2();
    break;
  case 0x4CB92: // Play
    AquariumStartWaterChange();   
    break; */

  default: 
    return false;
  }
  return true;
}



