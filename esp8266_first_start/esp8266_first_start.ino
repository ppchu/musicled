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
#define MINIBAR_LEN_TREB 7 // length of small sections that light up in mode 3
#define MINIBAR_LEN_BASS 30
#define MINIBAR_LEN_MIDL 15 

#define DATAPIN   2 // GPIO2 - MOSI
#define CLOCKPIN  4 // GPIO4 - CLK

#define IP_ADDR "172.20.10.6"
#define SSID_NAME "Peter's iPhone"
#define SSID_PW "peteriscool"

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
bool setupFailed = false; // flag for WiFi setup. if failed, blink onboard LED
WiFiClient client; // create TCP connection
byte val = 0;

// LED controlling variables
unsigned int r[NUMPIXELS] = {0}; // each index holds the R, G, or B value
unsigned int g[NUMPIXELS] = {0}; // of the LED with that index
unsigned int b[NUMPIXELS] = {0};

int heads[NUMPIXELS] = {-1};   // array holding the heads and tails values
int tails[NUMPIXELS] = {-1};   // of the "beams" that go across the LED strip
int beamCnt = 0;
int beamDelay = 0;      // counter that increments and every...
int beamPropDelay = 4;  // ...beamPropDelay it updates the beam
int beamLen = -7;       // length of beam in LEDs

// Blynk variables
int customize, modeSel;         // flags set by Blynk app
int blynk_r, blynk_g, blynk_b;  // r,g,b values set by Blynk app

void setup()
{
  // set up onboard LED mode
  pinMode(ESP8266_LED, OUTPUT);
  
  // set up Blynk
  Blynk.begin(auth, SSID_NAME, SSID_PW);

  // set up WiFi connection to PC
  const uint16_t port = 5204;
  const char * host = IP_ADDR; // ip or dns
  
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  WiFiMulti.addAP(SSID_NAME, IP_ADDR);

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
      Serial.println("connection failed, restart system");
      setupFailed = true;;
  }
 
  strip.begin();
  strip.show();

  // random seed
  randomSeed(0);
}

// Blynk functions for virtual pins
BLYNK_WRITE(V0)
{
  modeSel = 0;
}
BLYNK_WRITE(V1)
{
  modeSel = 1;
}
BLYNK_WRITE(V2)
{
  modeSel = 2;
}
BLYNK_WRITE(V3)
{
  modeSel = 3;
}
BLYNK_WRITE(V4)
{
  customize = param.asInt();
}
BLYNK_WRITE(V5)
{
  blynk_r = param.asInt();
}
BLYNK_WRITE(V6)
{
  blynk_g = param.asInt();
}
BLYNK_WRITE(V7)
{
  blynk_b = param.asInt();
}
BLYNK_WRITE(V8)
{
  beamPropDelay = param.asInt();
}
BLYNK_READ(V9)
{
  Blynk.virtualWrite(9, modeSel);
}

void loop()
{
  int i;
  int randG, randR, randB;

  // blink onboard LED if wifi failed
  if (setupFailed == true)
  {
    digitalWrite(ESP8266_LED, HIGH);
    delay(1000);
    digitalWrite(ESP8266_LED, LOW);
    delay(1000); 
    return;
  }
  
  Blynk.run();  // Initiates Blynk
  
  if (client.available())
  { // if data is availble to read
    val = client.read();

    //////////////////// mode 0 ////////////////////
    if ( modeSel == 0)  // bar flash mode
    {
      if (!customize)   // custom colors?
      { 
        randG = random(0x100);
        randR = random(0x100);
        randB = random(0x100);
      }
      else  // using zeRGBa on Blynk app to determine color
      { 
        randG = blynk_g;
        randR = blynk_r;
        randB = blynk_b;
      }
      
      if (val == 10) // detecting == bin2 (71 < val < 86 Hz )
      { 
        for ( i = 0; i < NUMPIXELS; i++)
        {
          r[i] = randR;
          g[i] = randG;
          b[i] = randB;
        }
      }
    }
    //////////////////// mode 1 ////////////////////
    else if ( modeSel == 1 )
    { // spectrum mode (sub bass, bass, midrange, high mids, high freq)
      if (customize)
      {
        r[val] = blynk_r; 
        g[val] = blynk_g;
        b[val] = blynk_b;
      }
      else
      {
        if (val < 5)
        { // red
          r[val] = 0xFF;
          g[val] = 0;
          b[val] = 0;
        }
        else if (val < 10)
        { // orange
          r[val] = 0xFF;
          g[val] = 0xA5;
          b[val] = 0;
        }
        else if (val < 15)
        { // yellow
          r[val] = 0xFF;
          g[val] = 0xFF;
          b[val] = 0;
        }
        else if (val < 20)
        { // chartreuse
          r[val] = 0x7F;
          g[val] = 0xFF;
          b[val] = 0;
        }
        else if (val < 25)
        { // green
          r[val] = 0;
          g[val] = 0x80;
          b[val] = 0;
        }
        else if (val < 30)
        { // spring
          r[val] = 0;
          g[val] = 0xE6;
          b[val] = 0x73;
        }
        else if (val < 35)
        { // cyan
          r[val] = 0;
          g[val] = 0xFF;
          b[val] = 0xFF;
        }
        else if (val < 40)
        { // azure
          r[val] = 0xF0;
          g[val] = 0xFF;
          b[val] = 0xFF;
        }
        else if (val < 45)
        { // blue
          r[val] = 0;
          g[val] = 0;
          b[val] = 0xFF;
        }
        else if (val < 50)
        { // violet
          r[val] = 0xEE;
          g[val] = 0x82;
          b[val] = 0xEE;
        }
        else if (val < 55)
        { // magenta
          r[val] = 0xFF;
          g[val] = 0x00;
          b[val] = 0xFF;
        }
        else
        { // rose
          r[val] = 0xFF;
          g[val] = 0;
          b[val] = 0xFF;
        }
      }

    }
    //////////////////// mode 2 ////////////////////
    else if ( modeSel == 2 )
    {
      if (val == 17 )
      { // trigger the beam
        if (beamCnt >= NUMPIXELS)
          beamCnt = 0;
          
        heads[beamCnt] = 0;
        tails[beamCnt] = beamLen;

        if (!customize) // every beam gets a random color
        {
          r[beamCnt] = random(0x100);
          g[beamCnt] = random(0x100);
          b[beamCnt] = random(0x100);
        }
        else
        { // using zeRGBa on Blynk app to determine color
          g[beamCnt] = blynk_g;
          r[beamCnt] = blynk_r;
          b[beamCnt] = blynk_b;
        }
        beamCnt++;
      }
    }
    //////////////////// mode 3 ////////////////////
    else if ( modeSel == 3 )
    { // random small bars (bass, mid, treble)

      // random colors if custom button not pressed on Blynk app
      if (!customize)
      {
        randG = random(0x100);
        randR = random(0x100);
        randB = random(0x100);  
      }

      // determine location of bass bar
      i = random(60 - MINIBAR_LEN_BASS);
      if (val == 10)
      {
        if (customize)
        {
          randG = blynk_g;
          randR = blynk_r;
          randB = blynk_b;
        }
        for (int j = i ; j < i + MINIBAR_LEN_BASS; j++)
        {
          r[j] = randR;
          g[j] = randG;
          b[j] = randB;
        }
      }
      // determine location of mid bar
      i = random(60 - MINIBAR_LEN_MIDL);
      if (val == 17)
      {
        if (customize)
        {
          randG = blynk_r;
          randR = blynk_b;
          randB = blynk_g;
        }
        for (int j = i ; j < i + MINIBAR_LEN_MIDL; j++)
        {
          r[j] = randR;
          g[j] = randG;
          b[j] = randB;
        }
      }
      // determine location of treble bar
      i = random(60 - MINIBAR_LEN_TREB);
      if (val == 54)
      {
        if (customize)
        {
          randG = blynk_b;
          randR = blynk_g;
          randB = blynk_r;
        }
        for (int j = i ; j < i + MINIBAR_LEN_TREB; j++)
        {
          r[j] = randR;
          g[j] = randG;
          b[j] = randB;
        }
      }
    }
    /* // future work - average spectrum energy
    else if ( modeSel == 4 )
    { // spectrum avg
      for (i = 0; i < NUMPIXELS; i++)
      {
        r[i] = val;
        g[i] = val;
        b[i] = val;
        strip.setPixelColor(i, g[i], r[i], b[i]); 
      }
    }
    */
  } // end of client available loop

  // mode 2 - beam propagation
  // update head location every beamPropDelay to control beam propagation speed
  if (beamDelay++ % beamPropDelay == 0)
  {
    for (i = 0; i < NUMPIXELS; i++)
    {
      strip.setPixelColor(heads[i], g[i], r[i], b[i]);    // 'On' pixel at head
      strip.setPixelColor(tails[i], 0);                   // 'Off' pixel at tail
      
      if (heads[i] >= 0)
      { 
        if ( ++heads[i] >= NUMPIXELS)  // reset head
          heads[i] = -1;
      }
      if (tails[i] >= beamLen)
      {
        if ( ++tails[i] >= NUMPIXELS )  // reset tail
          tails[i] = beamLen - 1;  
      }
    }
  }

  // dimming, all modes except beam (mode 2)
  if ( modeSel == 0 || modeSel == 1 || modeSel == 3 )
  {
    for ( i = 0; i < NUMPIXELS; i++ )
    {
      if (g[i] > 0)
        g[i] /= 1.04;
      
      if (r[i] > 0)
        r[i] /= 1.04;
      
      if (b[i] > 0)
        b[i] /= 1.04;
      
      strip.setPixelColor(i, g[i], r[i], b[i]);  
    }
  }
  strip.show();

  delay(5); // give uController time to maintain behind the scenes tasks
}
