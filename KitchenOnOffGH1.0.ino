 
 /*
 To provide a hardware-based "soft switch" for the kitchen Hue lights using exisitng 3-way and 4-way switches and existing wiring
 Allows Hue lights to be always available for app and voice control while retaining convenience of physical switches
 Adapted from code by James Bruce (MakeUseOf.com) https://gist.github.com/jamesabruce/8bc9faf3c06e30c3f6fc which in turn was
 adapted from code by Gilson Oguime. https://github.com/oguime/Hue_W5100_HT6P20B/blob/master/Hue_W5100_HT6P20B.ino
 */

#include <ESP8266WiFi.h> //Using NodeMCU with ESP8266
#include <SPI.h>

 
// Hue constants
 
const char hueHubIP[] = "<IP Address>";  // Hue hub IP
const char hueUsername[] = "<Hue Developer Name>";  // Hue developer name
const int hueHubPort = 80;

// Local network details

const char ssid[] = "<SSID>";  //  network SSID (name)
const char pass[] = "<Password>";       // network password

// Other

int lightNum = 1; // Kitchen downlight 8 - arbitary light from kitchen group to check on/off status of all group lights
int switchPin = 5; // -- control sense pin, mapped to D1 on NodeMCU
boolean onOffCommandSent = false; //avoids multiple commands - one command per state change of physical switches
boolean state = false; // for monitoring switch state changes
boolean previousState; // ditto
boolean onOffState = false; //to store actual on/off state of lights as reported by Hue bridge
int stateLock = 2500; //to avoid sending multiple commands before system can react
int getHueTime = 2000; //check on/off status every 2s
unsigned long stateChangeTime; 
unsigned long buffer=0;  //buffer for received data storage
unsigned long addr;

WiFiClient client;
 
//****************************************************************//

void setup()
  {
    Serial.begin(115200);
    pinMode(switchPin, INPUT_PULLUP); // pin will be pulled low by grounding load terminal on switch
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
  
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(2000);
      Serial.print("...");
    }

    Serial.print("IP number assigned by DHCP is ");
    Serial.println(WiFi.localIP());
    stateChangeTime = millis();

    getHue(1); // initial request for on/off status of lights
    
  }

//****************************************************************//
  
void loop() 
  {
     if(millis() % getHueTime == 0) //checks light state every 5s
      {
        getHue(1);
      }

      state = digitalRead(switchPin); // for checking for state change
      previousState == state;
      
     if (digitalRead(switchPin) != previousState) // state change (multiple changes within 2.5s period ignored)
        {
          previousState = state;
          stateChangeTime = millis();
          if (millis() - stateChangeTime > stateLock) // (multiple changes within 2.5s period ignored)
            {
              stateChangeTime = millis();
            }
            if (onOffState == false) // if lights are off, send "on" command
             {
               //Serial.println("Sending On Command");
               String command =  "{\"on\": true}";  //see Philips developer info https://developers.meethue.com/documentation/getting-started
               setHue(8,command); // 8 is system id for Kitchen Group (downlights and lightstrips) which comprise:
               
               //Downlight 1 (3) (number in () is system id)
               //Downlight 2 (5)
               //Downlight 3 (6)
               //Downlight 4 (8)
               //Downlight 5 (7)
               //Downlight 6 (11)
               //Downlight 7 (9)
               //Downlight 8 (1)
               //Lightstrips 1 (2)
               //Lightstrips 2 (4)
               //Lightstrips 3 (10)
               //Serial.println("lights on");
            }
          else if (onOffState == true) // if lights are on, send "off" command
           {
              String command = "{\"on\": false}";
              setHue(8,command);
              onOffCommandSent = true;
              //Serial.println("lights off");
            }
        } 
    }

//****************************************************************//

void setHue(int groupNum,String command) // setHue() passes the on/off commands in JSON format and makes a HTTP PUT request to the bridge
  {
    if (client.connect(hueHubIP, hueHubPort))
     {
        client.print("PUT /api/");
        client.print(hueUsername);
        client.print("/groups/");
        client.print(groupNum); 
        client.println("/action HTTP/1.1");
        client.println("keep-alive");
        client.print("Host: ");
        client.println(hueHubIP);
        client.print("Content-Length: ");
        client.println(command.length());
        client.println("Content-Type: text/plain;charset=UTF-8");
        client.println();  // blank line before body
        client.println(command);
        //Serial.println("lights switched");
        client.stop(); 
      }
  }

void getHue(int lightNum) // getHue asks bridge for on/off status so appropriate command is sent on state change
  {
    if (client.connect(hueHubIP, hueHubPort))
    {
      client.print("GET /api/");
      client.print(hueUsername);
      client.print("/lights/");
      client.print(lightNum);  
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(hueHubIP);
      client.println("Content-type: application/json");
      client.println("keep-alive");
      client.println();
      //Serial.println("Running getHue");
      while (client.connected())
      {
        if (client.available())
        {
          client.findUntil("\"on\":", "\0");
          onOffState = (client.readStringUntil(',') == "true");  // if light is on, set variable to true
          //Serial.print("Are lights on?"  );
          //Serial.println(onOffState);
          break;
        }
      }
      client.stop();
    }
  }
