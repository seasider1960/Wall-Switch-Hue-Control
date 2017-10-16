 
 /*
 To provide a hardware-based "soft switch" for the kitchen Hue lights using exisitng 3-way and 4-way switches and existing wiring
 Allows Hue lights to be always available for app and voice control while retaining convenience of physical switches
 See Philips developer info https://developers.meethue.com/documentation/getting-started
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

int groupNum = 8; // Hue Kitchen Lights Group -- see https://developers.meethue.com/documentation/groups-api
int sensorNum = 5; // Generic Hue sensor - see https://developers.meethue.com/documentation/how-use-ip-sensors
int actionNum = 1; //arbitrary value to trigger on/off rule on sensor 5 -- see https://developers.meethue.com/documentation/rules-api
int switchPin = 5; // -- control sense pin, mapped to D1 on NodeMCU
boolean state = false; // for monitoring switch state changes
boolean previousState; // ditto
int stateLock = 2500; //to avoid sending multiple commands before system can react
unsigned long stateChangeTime = 1000; // arbitrary starting number for millis() comparator
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
  }

//****************************************************************//
  
void loop() 
  {
    state = digitalRead(switchPin); // for checking for state change
    previousState == state;
    if (digitalRead(switchPin) != previousState) // state change
        {
          previousState = state;
          if (millis() - stateChangeTime > stateLock) // (multiple changes within 2.5s period ignored)
           {
             setHue(); // passes the on/off command for the Kitchen group in JSON format and makes a HTTP PUT request to the bridge
             Serial.println("Sending On/Off Command");
             stateChangeTime = millis();
             //Kitchen Group comprises:
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
        }
    }

//****************************************************************//

void setHue()
  { String command =  "{\"status\": 1}"; // passes the sensor state to toggle the Kitchen group lights
    if (client.connect(hueHubIP, hueHubPort))
     {
        client.print("PUT /api/");
        client.print(hueUsername);
        client.print("/sensors/");
        client.print(sensorNum); 
        client.println("/state HTTP/1.1");
        client.println("keep-alive");
        client.print("Host: ");
        client.println(hueHubIP);
        client.print("Content-Length: ");
        client.println(command.length());
        client.println("Content-Type: text/plain;charset=UTF-8");
        client.println();  // blank line before body
        client.println(command);
        Serial.println("lights switched");
        client.stop(); 
      }
  }


