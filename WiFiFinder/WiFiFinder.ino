// WiFiFinder finds open WiFis that don't have encryption nor a sign-in page.
//  The device will join any open netwoork it finds and additionally attempt to establish
//  an internet connection to verify that the AP doesn't reuquire signin.
//
//  Copyright (C) 2014 Nicola Cimmino
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Connections:
//
// D2  -> Display VCC
// D3  -> Display SCL 
// D4  -> Display SDA
// D5  -> Display RST
// D6  -> Display D/C
// TX  -> ESP8266 RX
// RX  -> ESP8266 TX

#include <avr/sleep.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>       // Adafruit GFX Lib (https://github.com/adafruit/Adafruit-GFX-Library)
#include <Adafruit_SSD1306.h>   // Adafruit SSD1306 Lib (https://github.com/adafruit/Adafruit_SSD1306)

// Display pins.
#define OLED_VCC      2
#define OLED_MOSI     4 
#define OLED_CLK      3
#define OLED_DC       6
#define OLED_CS       12
#define OLED_RESET    5 

// Display controller.
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

const char AT_JOIN_AP[] =  "AT+CWJAP";
const char AT_LIST_AP[] =  "AT+CWLAP";

// Found open SSIDs, liited to 4 for now.
String ssids[] = { "", "", "", "" };

////////////////////////////////////////////////////////////////////////////////
// Application setup after reset.
//
void setup()
{
  Serial.begin(115200);
}

////////////////////////////////////////////////////////////////////////////////
// Application entry point after setup() has executed.
//
void loop()
{
  
  // Power up display.
  pinMode(OLED_VCC, OUTPUT);
  digitalWrite(OLED_VCC,HIGH);

  // generate the high voltage from the 3.3v line internally.
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();   // clears the screen and buffer    
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,40);
  display.print("Scanning...");
  display.display();
  delay(1000);
  
  Serial.println(AT_LIST_AP);  
 
  byte ssidIx = 0;
  String response = "none";
  while(response != "")
  {
    response = readOneResponseLine();
    
    // Responses are similar to: +CWLAP:(4,"AP_SSID",-48)
    if(response.indexOf("+CWLAP")==0)
    {
      char type = response.charAt(9);
      String ssid = response.substring(12, response.indexOf("\"", 13));
      
      // Store only open hotspots (type 0)
      if(type=='0')
      {
        ssids[ssidIx]=ssid;
        ssidIx++;
      }
    }
  }
 
  display.clearDisplay();   // clears the screen and buffer    
  display.setTextSize(1);
  display.setCursor(0,0);
  
  // Go through each found hotspot and see if it can be joined. 
  for(int ix=0;ix<ssidIx;ix++)
  {
    Serial.print(AT_JOIN_AP);
    Serial.print("=");
    Serial.println(ssids[ix]);
    display.println(ssids[ix]);  
  }
  
  // No open hotspot found.
  if(ssidIx==0)
  {
    display.println("No Open Networks");  
  }
  display.println(ssidIx);
  display.display();
  
  delay(2000);
   
  // Change all lines connected to display to inputs, if we
  //  ground VCC then current will flow trough the clamps
  //  inside the display and take power also when the device is off.
  for(int p=2;p<=6;p++)
    pinMode(p, INPUT);
    
  // Power down the processor. To start a new scan user will reset the MCU by pressing
  //  the onboard reset button.
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu(); 
  
}

// Reads until a new line is received.
// If timmeout expires an empty string is returned.
String readOneResponseLine()
{
  String response = "";
  long startTime = millis();
  while(millis()-startTime<10000)
  {
    while (Serial.available() > 0) 
    {
       if(Serial.peek() == 13)
       {
         Serial.read();
         if(Serial.peek() == 10)
         {
           Serial.read();  
         }   
         return response;  
       }
       response += (char)Serial.read();
    }
  }
  return "";
}


