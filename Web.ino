// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte LocalMac[] = { 
  0x00, 0x1D, 0x60, 0xE0, 0x2A, 0x31 };
IPAddress LocalIp(192, 168, 2, 9);

// Initialize the Ethernet server library with the IP address and port you want to use (port 80 is default for HTTP):
WebServer webserver("", 80);

void httpNotFound(WebServer &server)
{
  P(failMsg) =
    "HTTP/1.0 404 Bad Request" CRLF
    WEBDUINO_SERVER_HEADER
    "Content-Type: text/html" CRLF
    CRLF
    "<h2>File Not Found !</h2>";
  server.printP(failMsg);
}

void defaultCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  /* for a GET or HEAD, send the standard "it's all OK headers" */
  server.httpSuccess();

  /* we don't output the body for a HEAD request */
  if (type == WebServer::GET)
  {
    /* store the HTML in program memory using the P macro */
    server.print("<html><head><title>HAAB</title>""<body>""<h2>Haab Info</h2>");
    server.print("</h1><form action='/relay' method='POST'><table width=\"100%\" border=1>");

    server.print("<tr><td>Curent time</td><td>"); 
    WebPrintDateTime(server, now()); 
    server.print("</td></tr>");

    server.print("<tr><td>Up time</td><td>");
    server.print(sys.uptime());
    server.print("</td></tr>");

    server.print("<tr><td>Time status</td><td>");
    switch (timeStatus())
    {
    case 0: 
      server.print("Time not set"); 
      break;
    case 1: 
      server.print("Time needs sync"); 
      break;
    default: 
      server.print("OK"); 
      break;
    }
    server.print("</td></tr>");

    server.print("<tr><td>Free memory</td><td>");
    server.print(sys.ramFree());
    server.print(" / ");
    server.print(sys.ramSize());
    server.print(" Bytes");
    server.print("</td></tr>");

    server.print("<tr><td>Board 1 temperature</td><td>");
    server.print(GetBoard1Temperature() / 100.0);
    server.print(" C");
    server.print("</td></tr>");

    server.print("<tr><td>Board 2 temperature</td><td>");
    server.print(GetBoard2Temperature() / 100.0);
    server.print(" C");
    server.print("</td></tr>");

    server.print("<tr><td>Error code</td><td>");
    server.print(ErrorCode);
    server.print("</td></tr>");

    server.print("<tr><td>last IR value</td><td>");
    server.print(LastReceivedIRValue, HEX);
    server.print("</td></tr>");

    server.print("<tr><td>Aquarium temperature</td><td>");
    server.print(AquariumTemperature(false) / 100.0);
    server.print(" C");
    server.print("</td></tr>");

    server.print("<tr><td>WC stage</td><td>");
    server.print(AquariumWaterChangingStage);
    server.print("</td></tr>");

    for (int device = 0; device < ALL_DEVICE_COUNT; device++)
    {    
      if ((device != UNKNOWN_1_RELAY) && (device != UNKNOWN_2_RELAY) && (device != DEVICE_FEEDER_1) && (device != DEVICE_FEEDER_2) )
      {
        server.print("<tr><td>");
        server.print(DeviceNames[device]);
        server.print("</td><td><button style=\"width: 200px\" name='");
        server.print(device + 1);
        server.print("' value='-1'>");
        if (GetDeviceState(device))
          server.print("ON");
        else
          server.print("OFF");
        server.print("</button></td></tr>");
      }
    }

    server.print("</form></body></html>");

    /*   
     WriteRow(server, GetDeviceState(device), -301 - device, device == ALL_DEVICE_COUNT - 1);
     server.printP(message);*/
  }
}

void timeCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {
      /* readPOSTparam returns false when there are no more parameters to read from the input. We pass in buffers for it to store the name and value strings along with the length of those buffers. */
      repeat = server.readPOSTparam(name, 16, value, 16);

      if (strcmp(name, "TIME") == 0)
      {
        time_t tm = strtoul(value, NULL, 10);
        RTC.set(tm);
        setTime(tm);
        Log("New date and time set");
        InitAquarium();
      }
    } 
    while (repeat);

    // after procesing the POST data, tell the web browser to reload the page using a GET method. 
    server.httpSeeOther("");
    return;
  }

  /* for a GET or HEAD, send the standard "it's all OK headers" */
  server.httpSuccess();
}

void relayCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {
      /* readPOSTparam returns false when there are no more parameters to read from the input. We pass in buffers for it to store the name and value strings along with the length of those buffers. */
      repeat = server.readPOSTparam(name, 16, value, 16);

      int id = strtol(name, NULL, 10); 
      int onOffValue = strtol(value, NULL, 10);
      if ((id > 0) && (id <= ALL_DEVICE_COUNT))
      {
        if (onOffValue == -1)
          DeviceSwitch(id - 1);      
        else
          DeviceOnOff(id - 1, onOffValue != 0);
      }
    }
    while (repeat);

    // after procesing the POST data, tell the web browser to reload the page using a GET method. 
    server.httpSeeOther("");
    return;
  }

  /* for a GET or HEAD, send the standard "it's all OK headers" */
  server.httpSuccess();
}

void settingsCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {
      /* readPOSTparam returns false when there are no more parameters to read from the input. We pass in buffers for it to store the name and value strings along with the length of those buffers. */
      repeat = server.readPOSTparam(name, 16, value, 16);

      int id = strtol(name, NULL, 10); 
      int set_value = strtol(value, NULL, 10);
      if ((id > 0) && (id <= SETTINGS_COUNT))
        Settings[id - 1] = set_value;
      SaveSettings();
      InitAquarium();
    }
    while (repeat);

    // after procesing the POST data, tell the web browser to reload the page using a GET method. 
    server.httpSeeOther("");
    return;
  }

  /* for a GET or HEAD, send the standard "it's all OK headers" */
  server.httpSuccess();
}

void displayCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {
      /* readPOSTparam returns false when there are no more parameters to read from the input. We pass in buffers for it to store the name and value strings along with the length of those buffers. */
      repeat = server.readPOSTparam(name, 16, value, 16);

      if (strcmp(name, "RESET") == 0)
      {
        InitLcd();
      }
    } 
    while (repeat);

    // after procesing the POST data, tell the web browser to reload the page using a GET method. 
    server.httpSeeOther("");
    return;
  }

  /* for a GET or HEAD, send the standard "it's all OK headers" */
  server.httpSuccess();
}

void jsonCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    server.httpFail();
    return;
  }

  //server.httpSuccess(false, "application/json");
  server.httpSuccess("application/json");

  if (type == WebServer::HEAD)
    return;

  server.print("{");

  int value;
  for (byte setting_id = 0; setting_id < SETTINGS_COUNT; setting_id++)
  {
    value = Settings[setting_id];
    WriteRow(server, value, setting_id + 1, false);
  }

  WriteRow(server, now(), TIME_SETTING_ID, false); 
  WriteRow(server, now() - boardStartTime , -110, false);
  WriteRow(server, timeStatus(), -102, false);
  //WriteRow(server, "Battery", analogRead(VBAT_RECV_PIN) * REFERENCE_VOLTAGE_MULT, -104);
  WriteRow(server, sys.ramFree(), -106, false);
  WriteRow(server, GetBoard1Temperature(), -108, false);
  WriteRow(server, GetBoard2Temperature(), -109, false);
  WriteRow(server, ErrorCode, -112, false);
  WriteRow(server, LastReceivedIRValue, -119, false);

  WriteRow(server, AquariumTemperature(false), -201, false);
  WriteRow(server, AquariumWaterChangingStage, -203, false);
  //WriteRow(server, "PH Level", analogRead(PH_METER_RECV_PIN) * REFERENCE_VOLTAGE_MULT, -205);

  for (int device = 0; device < ALL_DEVICE_COUNT; device++)
    WriteRow(server, GetDeviceState(device), -301 - device, device == ALL_DEVICE_COUNT - 1);

  server.print("}");
}

/*
     Use the following to test 
 curl "192.168.2.9/fs"
 curl "192.168.2.9/fs/datafile.txt"
 curl -X DELETE "192.168.2.9/fs/datafile.txt"
 */
static void fsAccessCmd(WebServer &server, WebServer::ConnectionType type, char **url_path, char *url_tail, bool tail_complete)
{
  if(!tail_complete) 
    server.httpServerError();

  boolean isLs = strncmp(url_path[0], "ls", 3) == 0;

  //Only serve files under the "/fs" and "/ls" path
  if( (!isLs) && (strncmp(url_path[0], "fs", 3) != 0))
  {
    httpNotFound(server);
    return;
  }

  if ((url_path[1] == 0) || (url_path[1][0] == 0))
  {
    // do an ls
    if (isLs)
    {
      server.httpSuccess("application/json");
      server.print("{");
    }
    else
    {
      server.httpSuccess("text/plain");
      server.println("Filename\t\tSize");
      server.println();
    }

    File dir = SD.open("/", FILE_READ);
    dir.rewindDirectory();

    boolean isFirst = true;

    while(true) 
    {
      File entry = dir.openNextFile();
      if (!entry) 
        break;

      if (isLs)
      {
        if (!isFirst)
          server.print(",");
        isFirst = false;

        server.print("\"");
        server.print(entry.name());
        server.print("\":");
        server.print(entry.size(), DEC);
      }
      else
      {
        server.print(entry.name());
        server.print("\t\t");
        server.println(entry.size(), DEC);
      }

      entry.close();
    }
    if (isLs)
    {
      server.print("}");
    }
    dir.close();
  }
  else{
    // access a file
    File dataFile = SD.open(url_path[1], FILE_READ);
    if (dataFile) 
    {
      if(type == WebServer::GET)
      {
        server.httpSuccess("text/plain");
        while (dataFile.available())
          webserver.write(dataFile.read());
        dataFile.close();
      }
      else 
        if(type == WebServer::DELETE)
      {
        server.httpSuccess();
        SD.remove(url_path[1]);
      }
    }
    else
      httpNotFound(server);
  }
}


static void InitWWW()
{
  // start the Ethernet connection and the server:
  Ethernet.begin(LocalMac, LocalIp);

  /* setup our default command that will be run when the user accesses the root page on the server */
  webserver.setDefaultCommand(&defaultCmd);
  webserver.addCommand("json", &jsonCmd);
  webserver.addCommand("relay", &relayCmd);
  webserver.addCommand("time", &timeCmd);
  webserver.addCommand("settings", &settingsCmd);
  webserver.addCommand("display", &displayCmd);
  webserver.setUrlPathCommand(&fsAccessCmd);

  webserver.begin();

  if (Ethernet.localIP() == INADDR_NONE)
    ErrorCode = ErrorCode | ERR_INTERNET;

#ifdef DEBUGING
  Serial.print("Server is at: ");
  Serial.println(Ethernet.localIP());
#endif
}

static void ProcessWWW()
{
  webserver.processConnection();
}

static void WriteRow(WebServer &server, long value, int id, boolean is_last) 
{
  server.print("\"");
  server.print(id);
  server.print("\":");
  server.print(value);
  if (!is_last)
    server.print(",");
}

static void WebPrintDateTime(WebServer &server, time_t value)
{
  server.print(day(value));
  server.print('/');
  server.print(month(value));
  server.print('/');
  server.print(year(value) - 2000);
  server.print(' ');

  server.print(hour(value));
  server.print(':');
  server.print(minute(value));
  server.print(':');
  server.print(second(value));
  server.print(' ');
}







































