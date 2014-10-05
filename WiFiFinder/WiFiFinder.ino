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

char rxBuffer[255];

////////////////////////////////////////////////////////////////////////////////
// Application setup after reset.
//
void setup()
{
   // Power up display.
  pinMode(OLED_VCC, OUTPUT);
  digitalWrite(OLED_VCC,HIGH);

  // generate the high voltage from the 3.3v line internally.
  display.begin(SSD1306_SWITCHCAPVCC);
 
  Serial.begin(115200);
  
  display.clearDisplay();   // clears the screen and buffer    
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
 
  Serial.println("AT+RST"); // restet and test if module is redy
  delay(1000); 
  if(!Serial.find("ready")) {
    display.println("ESP8266 doesn't respond.");
    display.display();
    
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
  else
  {
    Serial.println("AT+CWMODE=3");
    delay(1000);
    display.println("Scanning....");
    display.display();
    delay(5000);
  }    
}

char ssid[32];

////////////////////////////////////////////////////////////////////////////////
// Application entry point after setup() has executed.
//
void loop()
{
   
  
  Serial.readBytes(rxBuffer, 253);
  for(int ix=0;ix<254;ix++)
  {
    rxBuffer[ix]=0;  
  }
  Serial.setTimeout(15000);
  Serial.println("AT+CWLAP"); 
  //readResponse();
  
  Serial.readBytes(rxBuffer, 254);
  
  display.clearDisplay();   // clears the screen and buffer    
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print(rxBuffer);
  display.display();
  delay(2000);
  return;
  
  char* responseStart=strstr(rxBuffer,"+CWLAP");
  if(responseStart==0)
  {
    return;  
  }
  
  bool atLeastOneOpenFound = false;
  while(true)
  {
    responseStart=strstr(responseStart+1,"+CWLAP");
    if(responseStart!=0)
    {
      char type = *(responseStart+8);
      *strstr(responseStart+11, "\"")=0;
      strcpy(ssid, responseStart+11);
      //if(type=='0')
      {
        /*Serial.print("AT+CWJAP=\"");
        Serial.print(ssid);
        Serial.println("\",\"\"");*/
        display.print(type);
        display.print("-");
        display.println(ssid);
        display.display();
        //delay(2000);
        atLeastOneOpenFound=true;
      }
      while(*responseStart!=0)
      {
        responseStart++;
        if(responseStart-rxBuffer>254)
        {
          break;
        } 
      }
    }
    else
    {
      break;  
    }
  }
  display.display();
  
  /*
  if(!atLeastOneOpenFound)
  {
    display.println("No open networks found!"); 
    display.display();
  }
  
  delay(3000);
  */
}

// Reads until the end of response.
// If timmeout expires an empty string is returned.
byte readResponse()
{
  
  //long startTime = millis();
  byte ix=0;
  //rxBuffer[0]=0;
  //while(millis()-startTime<5000)
  {
    while(strstr(rxBuffer,"OK")==0)
    {
      if(Serial.available()) 
      {
        rxBuffer[ix]=(char)Serial.read();
        ix++;
        if(ix>253) break;
      }
    }
  }
  rxBuffer[ix]=0;
  return ix;
}


