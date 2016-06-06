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

// constants
#define NUMPIXELS 60    // how many LEDs on the strip
#define ESP8266_LED 5   // ESP8266 on-board LED on port 5
#define COUNT_TO 20   // beam mode counter

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
bool setupFailed = false; // flag for WiFi setup validity. if failed, blink red
WiFiClient client; // create TCP connection
byte val = 0;

// LED controlling variables
unsigned int r[NUMPIXELS] = {0}; // each index holds the R, G, or B value
unsigned int g[NUMPIXELS] = {0}; // of the LED with that index
unsigned int b[NUMPIXELS] = {0};

int heads[NUMPIXELS] = {-1};   // array holding the heads and tails values
int tails[NUMPIXELS] = {-1};   // of the "beams" that go across the LED strip
int beamCnt = 0;
int beamDelay = 0;
int beamLen = -3;

// Blynk variables
int togRand, modeSel, together;
int blynk_r, blynk_g, blynk_b;

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
  WiFiMulti.addAP("Peter's iPhone", "172.20.10.2");

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
  // 0 - random
  // 1 - use zeRGBa
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

    //////////////////// mode 0 ////////////////////
    if ( modeSel == 0)
    { // bar flash mode
      if (!togRand)
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
    //////////////////// mode 1 ////////////////////
    else if ( modeSel == 1)
    { // spectrum mode (sub bass, bass, midrange, high mids, high freq)
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
    //////////////////// mode 2 ////////////////////
    else if ( modeSel == 2 )
    {
      if (val == 7 )
      { // trigger the beam
        if (beamCnt >= NUMPIXELS)
          beamCnt = 0;
          
        heads[beamCnt] = 0;
        tails[beamCnt] = beamLen;

        if (!togRand) // every beam gets a random color
        {
          r[beamCnt] = random(0x100);
          g[beamCnt] = random(0x100);
          b[beamCnt] = random(0x100);
        }

        beamCnt++;
        //Serial.println("beat det");
      }
      //if ( beamDelay % COUNT_TO == COUNT_TO && heads[0] >= 0)
      for (i = 0; i < NUMPIXELS; i++)
      {
        if ( heads[i] >= 0)
        { // update head location every COUNT_TO to control beam speed
          if (!togRand)
            strip.setPixelColor(heads[i], g[i], r[i], b[i]);
          else
            strip.setPixelColor(heads[i], 0, 0xFF, 0);    // 'On' pixel at head
          
          strip.setPixelColor(tails[i], 0);             // 'Off' pixel at tail
          
          if ( ++heads[i] >= NUMPIXELS )  // reset head
            heads[i] = -1;
    
          if ( ++tails[i] >= NUMPIXELS )  // reset tail
            tails[i] = beamLen;
    
          //Serial.println("moving head");
        }
      }
    }
  }

/*
  if (modeSel == 2)
  {
    for (i = 0; i < NUMPIXELS; i++)
    {
      if ( heads[i] >= 0)
      { // update head location every COUNT_TO to control beam speed
        strip.setPixelColor(heads[i], 0, 0xFF, 0);    // 'On' pixel at head
        strip.setPixelColor(tails[i], 0);             // 'Off' pixel at tail
        
        if ( ++heads[i] >= NUMPIXELS )  // reset head
          heads[i] = -1;
  
        if ( ++tails[i] >= NUMPIXELS )  // reset tail
          tails[i] = beamLen;
  
        //Serial.println("moving head");
      }
    }
  }*/

  // dimming in the right modes
  if ( modeSel == 0 || modeSel == 1)
  {
    for ( i = 0; i < NUMPIXELS; i++ )
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
  }
  strip.show();

  
  delay(2);
}

