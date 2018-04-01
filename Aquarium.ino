static void InitAquarium()
{
  DeviceOn(AQUARIUM_SUMP_PUMP_RELAY);
  DeviceOff(AQUARIUM_DRAIN_PUMP_RELAY);

  for(uint8_t id = 0; id < dtNBR_ALARMS; id++)
  {
    if (Alarm.isAllocated(id))
      Alarm.free(id);
  }

  // Turn lights if necessary
  time_t time_now = now();
  time_now = time_now - previousMidnight(time_now);

  DeviceOnOff(AQUARIUM_LIGHT_1_RELAY, GetLightState(AquariumLight1OnTime, AquariumLight1OffTime));
  DeviceOnOff(AQUARIUM_LIGHT_2_RELAY, GetLightState(AquariumLight2OnTime, AquariumLight2OffTime));
  DeviceOnOff(AQUARIUM_LIGHT_3_RELAY, GetLightState(AquariumLight3OnTime, AquariumLight3OffTime));

  Alarm.alarmRepeat(AquariumLight1OnTime, AquariumLight1_ON);
  Alarm.alarmRepeat(AquariumLight2OnTime, AquariumLight2_ON);
  Alarm.alarmRepeat(AquariumLight3OnTime, AquariumLight3_ON);

  Alarm.alarmRepeat(AquariumLight1OffTime, AquariumLight1_OFF);
  Alarm.alarmRepeat(AquariumLight2OffTime, AquariumLight2_OFF);
  Alarm.alarmRepeat(AquariumLight3OffTime, AquariumLight3_OFF);

  Alarm.alarmRepeat(AquariumWaterChangeStartTime, AquariumWaterChangeTimer);

  Alarm.alarmRepeat(FishFeeder1StartTimeA, FishFeeder1TimerA);
  Alarm.alarmRepeat(FishFeeder1StartTimeB, FishFeeder1TimerB);
  Alarm.alarmRepeat(FishFeeder2StartTimeA, FishFeeder2TimerA);
  Alarm.alarmRepeat(FishFeeder2StartTimeB, FishFeeder2TimerB);
}

static boolean GetLightState(time_t onTime, time_t offTime)
{
  time_t time_now = now();
  time_now = time_now - previousMidnight(time_now);

  boolean b;
  if (onTime < offTime)
  {
    b = (time_now >= onTime) && (time_now < offTime);
  }
  else
    if (onTime > offTime)
    {
      time_t t = onTime;
      onTime = offTime;
      offTime = t;
      b = !((time_now >= onTime) && (time_now < offTime));
    }
    else
      b = false;

  return b;
}


static volatile byte AquariumWaterChangingStage = 0; // 0 = nothing, 1 = draining, 2 = filling

static void ProcessAquarium()
{
  if (AquariumWaterChangingStage != 0)
    return;

  int t = AquariumTemperature(false);

  if (t < AquariumTemperatureMin)
  {
    // Turn on borh heaters
    AquariumHeater1_ON();
    AquariumHeater2_ON();
  }
  else
    if (t <= AquariumTemperatureMax)
    {
      // Turn on one heater
      // if both heaters are on, turn off heater 1, heater 2 will work
      if ((GetDeviceState(AQUARIUM_HEATER_1_RELAY) != 0) && (GetDeviceState(AQUARIUM_HEATER_2_RELAY) != 0))
        AquariumHeater1_OFF();
      else
        // if both heaters are off, turn on heater 1
        if ((GetDeviceState(AQUARIUM_HEATER_1_RELAY) == 0) && (GetDeviceState(AQUARIUM_HEATER_2_RELAY) == 0))
          AquariumHeater1_ON();
    }
    else
      if (t > AquariumTemperatureMax)
      {
        // Turn off borh heaters
        AquariumHeater1_OFF();
        AquariumHeater2_OFF();
      }
}

static void AquariumLight1_ON()
{
  DeviceOn(AQUARIUM_LIGHT_1_RELAY);
}

static void AquariumLight1_OFF()
{
  DeviceOff(AQUARIUM_LIGHT_1_RELAY);
} 

static void AquariumLight2_ON()
{
  DeviceOn(AQUARIUM_LIGHT_2_RELAY);
}

static void AquariumLight2_OFF()
{
  DeviceOff(AQUARIUM_LIGHT_2_RELAY);
} 

static void AquariumLight3_ON()
{
  DeviceOn(AQUARIUM_LIGHT_3_RELAY);
}

static void AquariumLight3_OFF()
{
  DeviceOff(AQUARIUM_LIGHT_3_RELAY);
} 

static void AquariumHeater1_ON()
{
  DeviceOn(AQUARIUM_HEATER_1_RELAY);
} 

static void AquariumHeater1_OFF()
{
  DeviceOff(AQUARIUM_HEATER_1_RELAY);
} 

static void AquariumHeater2_ON()
{
  DeviceOn(AQUARIUM_HEATER_2_RELAY); 
} 

static void AquariumHeater2_OFF()
{
  DeviceOff(AQUARIUM_HEATER_2_RELAY); 
} 

static void AquariumWaterChangeTimer()
{
  if (bitRead(AquariumWaterChangeDOW, weekday() - 1) != 0) // (Sunday is day 1)  
    AquariumStartWaterChange();
}


static void AquariumStartWaterChange() // start draining
{
  if (AquariumWaterChangingStage == 0)
  {
    Log("Start Water Change");

    AquariumWaterChangingStage = 1; // draining
    Alarm.timerOnce(AquariumWaterChangeDrainDuration, AquariumStopDraining);

    AquariumHeater1_OFF();
    AquariumHeater2_OFF();

    DeviceOff(AQUARIUM_SUMP_PUMP_RELAY);
    DeviceOn(AQUARIUM_DRAIN_PUMP_RELAY);
  }
  else // safety
  {
    AquariumWaterChangingStage = 0; 
    DeviceOff(AQUARIUM_DRAIN_PUMP_RELAY);
    DeviceOn(AQUARIUM_SUMP_PUMP_RELAY);
  }
}

static void AquariumStopDraining()
{
  if (AquariumWaterChangingStage == 1)
  {
    Log("Stop Water Change Drain");

    AquariumWaterChangingStage = 2; // filling
    Alarm.timerOnce(AquariumWaterChangeFillDuration, AquariumStopFilling);

    AquariumHeater1_ON();
    AquariumHeater2_ON();
  }
  else
    AquariumWaterChangingStage = 0; 

  DeviceOff(AQUARIUM_DRAIN_PUMP_RELAY);
  DeviceOn(AQUARIUM_SUMP_PUMP_RELAY);
}

static void AquariumStopFilling()
{
  if (AquariumWaterChangingStage == 2)
  {
    Log("Stop Water Change Fill");
  }

  AquariumWaterChangingStage = 0; 
  DeviceOff(AQUARIUM_DRAIN_PUMP_RELAY);
  DeviceOn(AQUARIUM_SUMP_PUMP_RELAY);
}

static void AquariumFeeder1()
{
  Log("Feeder 1");

  DeviceOn(DEVICE_FEEDER_1);
  delay(500);
  DeviceOff(DEVICE_FEEDER_1);
}


static void AquariumFeeder2()
{
  Log("Feeder 2");

  DeviceOn(DEVICE_FEEDER_2);
  delay(500);
  DeviceOff(DEVICE_FEEDER_2);
}

static void FishFeeder1TimerA()
{
  if (bitRead(FishFeeder1DOWA, weekday() - 1) != 0) // (Sunday is day 1)  
    AquariumFeeder1();
}

static void FishFeeder1TimerB()
{
  if (bitRead(FishFeeder1DOWB, weekday() - 1) != 0) // (Sunday is day 1)  
    AquariumFeeder1();
}

static void FishFeeder2TimerA()
{
  if (bitRead(FishFeeder2DOWA, weekday() - 1) != 0) // (Sunday is day 1)  
    AquariumFeeder2();
}

static void FishFeeder2TimerB()
{
  if (bitRead(FishFeeder2DOWB, weekday() - 1) != 0) // (Sunday is day 1)  
    AquariumFeeder2();
}











