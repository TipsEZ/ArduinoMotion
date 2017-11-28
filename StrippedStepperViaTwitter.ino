#include <ThingSpeak.h>
#include <Stepper.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <SPI.h>
/*    Christopher Cronan
Rev 0 December 4, 2015; Original coding
Rev 1 December 27,2015; Changes made for twitter account read
Rev 2 January 1, 2016;  Inital code cleanup
Rev 3 January 8, 2016;  Final code cleanup, LED blinks added for notifications,
                        blink delay increased to avoid "flashing" too quick to see
Rev 4 January 11, 2016; PHP server setup as web based, code rework for PHP Server;
                        which was unsucssful, moved on to ThingSpeak API
Rev 5 January 11, 2016  ThingSpeak account setup and configured to act as OAUTH 
                        authentication device for Twitter string read
Rev 6 January 14, 2016  Determined that ThingSpeak header file had a bug in the .cpp file
                        bug corrected and header file functional now, communication with
                        ThingSpeak now okay. Bug was with ThingSpeak return code when
                        calling the ThingSpeak.readStringField function
Rev 7 January 18, 2016  ThingSpeak information migrated in from demo/debug program.
                        Added functionality to compare old Tweet to current Tweet, thought 
                        it wasn't really needed given the nPos variable, but should help
                        with verification in case of glitch on Hazzah or ThingSpeak

-----------------------Error Code Via LED Information---------------------
      BLUE LED will mean a positive event happened, number of blink dictates the event; see below:
                    3 blinks alternating w/red will indicate that ThingSpeak server connection established 
                    5 blinks indicates WiFi connection established
      RED LED will mean a negative event happened, number of blinks dictates the event; see below:
                    3 blinks will indicate ThingSpeak server connection unsuccesful
                    10 blinks indicates server connection failed
                    20 blinks indicated WiFi is not connected, this should cycle with a short delay until
                        WiFi is properly connected, at which time 5 blue blinks occur
                    */
  // On board LED setup
  int LEDblue = 2 ;                             // GPIO 0 (onboard LED for good status)
  int LEDred = 0 ;                              // GPIO 2 (onboard LED for Errors
  
  // Blink Functions, BlinkRed for Errors; BlinkBlue for go status(s)
  void blinkRed()
  {
    pinMode (LEDred, HIGH);
    delay (250);
    pinMode (LEDred, LOW);
    delay (250);
  }

  void blinkBlue ()
  {
    pinMode (LEDblue, HIGH);
    delay (250);
    pinMode (LEDblue, LOW);
    delay (250);
  }
  
  // Network Connection Settings
  const char* ssid = "TellMyWiFiLoveHer";
  const char* password = "You're WiFi Password";

 /* // Twitter Connection Tokens & Setup ** NOW UNUSED, 
  left for figuring out direct Twitter Connect. It MUST be possible...
  const String kAccessToken = "xxxxxxxxxxxxxx";
  const String kAccessTokenSecret = "xxxxxxxxxxxxx";
  const String kAPI_Key = "xxxxxxxxxxxxx";
  const String kAPI_Secret = "xxxxxxxxxx";
  const String kConsumerAPI = "xxxxxxx";*/
  
  // ThingSpeak Twitter feed info; public channel
  unsigned long kThingSpkCH = xxxxxxxx; 
  String sTweet = "";
  String sOldTweet = "";
  int nTweetStatus = 0;

  WiFiClient client;
 
  //*28BYJ-48 Stepper Motor Data and setup
  const int kStepsPerRev = 200;     // Max steps for one revolution
  const int RPM = 30;              // Max RPM permitted  
  // GPIO Pins on Huzzah for Stepper Motor Drive output to Motor Driver Board
  int STBY = 5;                             // GPIO 5 is Driver Standby  
  // initialize Stepper lib
  Stepper myStepper(kStepsPerRev, 16, 14, 12, 13); // GPIO Pins for Stepper Motor Driver

  // intial postion of flag; 0 is down, 1 is half, 2 is full
  int nPos = 0;
  
  // Raise
  void fRaise()
  {
    for (int l_i = 0; l_i < 5; l_i++)
    {
    Serial.println ("Clockwise"); // for debugging
    myStepper.step(kStepsPerRev); // one rotation clockwise
    delay (50);
    }
  }

  // Lower
  void fLower ()
  {
    for (int l_i = 0; l_i < 5; l_i++)
    {
    Serial.println("Counter-Clockwise");
    myStepper.step(-kStepsPerRev);
    delay (50);
    }
  }
  // Function to be input to cause Hazzah to go into sleep

  // Function to be input to cause Hazzah to go into deep sleep

void setup() 
{
  // Set stepper speed for best ratio of speed v torque
  myStepper.setSpeed(RPM);

  // Initialize serial port for debugging  
  Serial.begin(115200);
  delay(100);
  
  // First connect WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(" ");
    Serial.println("WiFi Connected! :) ");
    Serial.print ("IP address: ");
    delay(1000);
    Serial.print (WiFi.localIP());
    Serial.println ("");    
  }
   // Prep Onboard LED, blink if not connected otherwise stay on if connected
   // will swap this for power saving once debugging complete
    if (WiFi.status() == WL_CONNECTED)
    { 
      for (int l_i = 0; l_i < 5; l_i++)
      {
        blinkBlue();
      }
    }
    else 
    {
      for (int l_i = 0; l_i < 20; l_i++)
      {
        blinkRed();
      }
      Serial.println ("WiFi is not connected");
     } 
     int status = WL_IDLE_STATUS;
}

void loop() 
{
 ThingSpeak.begin(client);
 // Verify ThingSpeak result code is go status
 int l_resultCode = ThingSpeak.getLastReadStatus();
    if(l_resultCode == 200)
      {
        for (int l_i=0; l_i<3; l_i++)
        {
        blinkBlue();
        blinkRed();
        }
      }
    else
    {
      {for (int l_i =0; l_i<3; l_i++)
        {
        blinkRed();
        }
         Serial.print("Error reading message.  Status was: "); 
         Serial.println(l_resultCode);
      }
    } 
  sTweet = ThingSpeak.readStringField(kThingSpkCH, 1);
  if (sTweet == sOldTweet)
    {
      nTweetStatus = 0;
      delay (10*1000); // eventually call sleep per timer
    }
    else 
    {
      nTweetStatus = 1;
      Serial.println ("The last Tweet acted upon was ");
      Serial.print (sOldTweet);
      Serial.print (" the current Tweet to act upon is ");
      Serial.println (sTweet);
      sOldTweet = sTweet;
    }
    int nStringL = sTweet.length();
    if (nTweetStatus == 1)
    {    
      // Lower flag from full staff to half staff
      if ( sTweet.substring(0, nStringL) == "Half" && nPos != 0 && nPos != 1)
      {
        fLower();
        delay (500);
        nPos = 1;
      }
      // Raise flag from bottom of pole to half staff
      if (sTweet.substring(0, nStringL) == "Half" && nPos != 2 && nPos != 1)
      {
        fRaise();
        delay (500);
        nPos = 1;
      }
      // Raise flag from half staff to full staff
      if (sTweet.substring(0, nStringL) == "Full" && nPos != 0 && nPos != 2 && nTweetStatus == 1)
      {
        fRaise();
        delay (500);
        nPos = 2;
      }
      // Raise flag from bottom of pole to full staff
      if (sTweet.substring(0, nStringL) == "Full" && nPos != 2 && nPos != 1)
      {
        fRaise();
        delay (500);
        fRaise();
        delay (500);
        nPos = 2;
      }
    delay (10);
        }
// end of void loop closing bracket       
}
