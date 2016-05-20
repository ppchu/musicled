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
  WiFiMulti.addAP("PBJK", "applerules69");

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

  for (i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, 0xFFFFFF);
  }
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
    Serial.println(val);
    strip.setBrightness(val);
    strip.show();
  }
  
  delay(9);
}

