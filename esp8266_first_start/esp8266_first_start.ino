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
unsigned int r[NUMPIXELS] = {0}; 
unsigned int g[NUMPIXELS] = {0};
unsigned int b[NUMPIXELS] = {0};
unsigned long rms, gms, bms;
int ledMode = 2;

// Blynk variables
int togRand, modeSel, together;
int blynk_r, blynk_g, blynk_b;

void setup()
{
  int i;
  
  // set up onboard LED mode
  pinMode(ESP8266_LED, OUTPUT);
  
  // set up Blynk
  Blynk.begin(auth, "PBJK", "applerules69");

  // set up WiFi connection to PC
  const uint16_t port = 5204;
  const char * host = "192.168.1.49"; // ip or dns
  
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  WiFiMulti.addAP("PBJK", "192.168.1.49");

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

  // random seed
  randomSeed(0);
}

BLYNK_WRITE(V0)
{
  togRand = param.asInt();
}

BLYNK_WRITE(V4)
{
  blynk_r = param[0].asInt();
  blynk_g = param[1].asInt();
  blynk_b = param[2].asInt();
}

BLYNK_WRITE(V5)
{
  modeSel = param.asInt();
}

BLYNK_WRITE(V6)
{
  together = param.asInt();
}

void loop()
{
  int i;
  int randG, randR, randB;

  // do nothing if wifi failed
  if (setupFailed == true)
    return;

  Blynk.run();  // Initiates Blynk
  
  if (client.available())
  { // if data is availble to read
    val = client.read();
    //Serial.println(val);

    if ( modeSel == 0)
    { // bar flash mode
      if (togRand)
      { // using random RBG value to determine colors
        randG = random(0x100);
        randR = random(0x100);
        randB = random(0x100);
      }
      else
      { // using zeRGBa on Blynk app to determine color
        randG = blynk_g;
        randR = blynk_r;
        randB = blynk_b;
      }
      
      if (val < 2)
      { // detecting < bin2 ( < 28.7Hz )
        for ( i = 0; i < 30; i++)
        {
          r[i] = randR;
          g[i] = randG;
          b[i] = randB;
        }
      }
      // bar flash mode end
    }
    else if ( modeSel == 1)
    { // spectrum mode (sub bass, bass, midrange, high mids, high freq
      if (val < 4)
      {
        if (together)
        { // each section beats together
          for (i = 0; i < 4; i++)
          {
            r[i] = 0xFF;
            g[i] = 0;
            b[i] = 0;
          }
        }
        else
        { // blink individual bins
          r[val] = 0xFF;
          g[val] = 0;
          b[val] = 0;
        }
      }
      else if (val < 11)
      {
        if (together)
        { // each section beats together
          for (i = 4; i < 11; i++)
          {
            r[i] = 0xFF;
            g[i] = 0xA5;
            b[i] = 0;
          }
        }
        else
        { // blink individual bins
          r[val] = 0xFF;
          g[val] = 0xA5;
          b[val] = 0;
        }
      }
      else if (val < 20)
      {
        if (together)
        { // each section beats together
          for (i = 11; i <20 ; i++)
          {
            r[i] = 0xFF;
            g[i] = 0xFF;
            b[i] = 0;
          }
        }
        else
        { // blink individual bins
          r[val] = 0xFF;
          g[val] = 0xFF;
          b[val] = 0;
        }
      }
      else if (val < 25)
      {
        if (together)
        { // each section beats together
          for (i = 20; i < 25; i++)
          {
            r[i] = 0;
            g[i] = 0xFF;
            b[i] = 0;
          }
        }
        else
        { // blink individual bins
          r[val] = 0;
          g[val] = 0xFF;
          b[val] = 0;
        }
      }
      else
      {
        if (together)
        { // each section beats together
          for (i = 25; i < 30; i++)
          {
            r[i] = 0;
            g[i] = 0;
            b[i] = 0xFF;
          }
        }
        else
        { // blink individual bins
          r[val] = 0;
          g[val] = 0;
          b[val] = 0xFF;
        }
      }
    }
  }

  // dimming
  for ( i = 0; i < NUMPIXELS; i++)
  {
    if (g[i] > 0)
      g[i] /= 1.05;
    
    if (r[i] > 0)
      r[i] /= 1.05;
    
    if (b[i] > 0)
      b[i] /= 1.05;
    
    
    strip.setPixelColor(i, g[i], r[i], b[i]);  
  }
  //Serial.printf("%d %d %d\r", g[0], r[0], b[0]);
  strip.show();


  /*
  if (ledMode == 1)
  {
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
  
    // write color to strip
    for (i = 15; i <45; i++) 
      strip.setPixelColor(i, g, 0, 0);
      
    for (i = 5; i < 15 ; i++)
      strip.setPixelColor(i, 0, r, 0);
    
    for (i = 45; i < 55; i++)
      strip.setPixelColor(i, 0, r, 0); 
    
    for (i = 0; i < 5 ; i++)
      strip.setPixelColor(i, 0, 0, b);
  
    for (i = 55; i < 60; i++)
      strip.setPixelColor(i, 0, 0, b);

  }

  */

  
  delay(2);
}

