/**************************************************************
 * Blynk is a platform with iOS and Android apps to control
 * Arduino, Raspberry Pi and the likes over the Internet.
 * You can easily build graphic interfaces for all your
 * projects by simply dragging and dropping widgets.
 *
 *   Downloads, docs, tutorials: http://www.blynk.cc
 *   Blynk community:            http://community.blynk.cc
 *   Social networks:            http://www.fb.com/blynkapp
 *                               http://twitter.com/blynk_app
 *
 * Blynk library is licensed under MIT license
 * This example code is in public domain.
 *
 **************************************************************
 * This example runs directly on ESP8266 chip.
 *
 * You need to install this for ESP8266 development:
 *   https://github.com/esp8266/Arduino
 *
 * Please be sure to select the right ESP8266 module
 * in the Tools -> Board menu!
 *
 * Change WiFi ssid, pass, and Blynk auth token to run :)
 *
 **************************************************************/

#include <Adafruit_DotStar.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFiMulti.h>

#define NUMPIXELS 60
#define ESP8266_LED 5

#define DATAPIN   2 // GPIO2 - MOSI
#define CLOCKPIN  4 // GPIO4 - CLK

// WiFi
ESP8266WiFiMulti WiFiMulti;

// Use above defined pins for DATA and CLK
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN);

// Use hardware SPI (DATA-13, SCLK-SCL)
//Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS);

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "62999818c539415093b705f3be062d70";

// global variables
bool setupFailed = false;
// Use WiFiClient class to create TCP connections
WiFiClient client;
byte val = 0;
int pixelColor = 0;
int r = 0, g = 0, b = 0;
unsigned long rms, gms, bms;

void setup()
{
  int i;
  
  // set up onboard LED mode
  pinMode(ESP8266_LED, OUTPUT);
  
  // set up Blynk
  Blynk.begin(auth, "Peter's iPhone", "peteriscool");

  // set up WiFi connection to PC
  const uint16_t port = 5204;
  const char * host = "172.20.10.2"; // ip or dns
  
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  WiFiMulti.addAP("Peter's iPhone", "peteriscool");

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);

  Serial.print("connecting to ");
  Serial.println(host);

  if (!client.connect(host, port)) {
      Serial.println("connection failed");
      Serial.println("wait 5 sec...");
      delay(5000);
      setupFailed = true;;
  }
 
  strip.begin();
  strip.show();
}

int toggle, toggle2 = 0;

BLYNK_WRITE(V0)
{
  toggle = param.asInt();
}

void loop()
{
  int i;

  // do nothing if wifi failed
  if (setupFailed == true)
    return;

  Blynk.run();  // Initiates Blynk

  if (client.available())
  { // if data is availble to read
    val = client.read();
  }
  Serial.println(val);

  // bass
  if (val == '1')
    g = 0xFF;

  // middle
  if (val == '2')
  {
    r = 0xFF;
  }

  // treble
  if (val == '3')
  {
    b = 0xFF;
  }

  // dimming
  if (g > 0)
    g -= 5;
  
  if (r > 0)
    r -= 5;
  
  if (b > 0)
    b -= 5;

  // write color to strip
  for (i = 15; i <45; i++) 
  {
    strip.setPixelColor(i, g, 0, 0);
  }
  
  for (i = 5; i < 15 ; i++)
  {
    strip.setPixelColor(i, 0, r, 0);
  }
  for (i = 45; i < 55; i++)
  {
    strip.setPixelColor(i, 0, r, 0);
  }    
  
  for (i = 0; i < 5 ; i++)
  {
    strip.setPixelColor(i, 0, 0, b);
  }
  for (i = 55; i < 60; i++)
  {
    strip.setPixelColor(i, 0, 0, b);
  }
  strip.show();
  
  delay(15);
}

