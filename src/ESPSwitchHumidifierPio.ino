
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "DHT.h"

#define DHTPIN D3
#define DHTTYPE DHT22
#define firebaseURl "fir-esp-f02d6.firebaseio.com"
#define authCode "mNBkNkrdBrB5d49yGCHPP7c0wngwQiXdw928jrzL"

//#define wifiName "Amarianovy"
//#define wifiPass "pitsuha06"
#define wifiName "ONE E1003"
#define wifiPass "8b385ea9f92d"

const uint16_t irLed = D2; // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(irLed);      // Set the GPIO to be used to sending the message.

uint16_t switchOn[71] = {9250, 4532, 594, 530, 618, 552, 594, 552, 594, 528, 618, 552, 594, 528, 618, 552, 596, 528, 618, 1666, 596, 1642, 620, 1644, 616, 1642, 618, 1644, 618, 1644, 616, 1668, 594, 1642, 618, 552, 594, 528, 618, 552, 594, 552, 598, 548, 598, 524, 618, 530, 616, 530, 618, 1644, 616, 1666, 594, 1666, 594, 1666, 594, 1666, 594, 1646, 614, 1644, 616, 1666, 596, 40192, 9228, 2250, 594};
uint16_t heatOnOFf[71] = {9286, 4508, 620, 528, 596, 552, 618, 530, 618, 528, 618, 528, 672, 476, 594, 554, 618, 530, 618, 1642, 618, 1644, 620, 1646, 666, 1592, 622, 1640, 620, 1642, 620, 1642, 620, 1642, 594, 556, 594, 1664, 618, 530, 618, 528, 618, 530, 594, 552, 594, 558, 614, 530, 592, 1668, 618, 528, 618, 1644, 668, 1592, 612, 1650, 594, 1668, 592, 1668, 618, 1644, 598, 40186, 9230, 2230, 698}; // NEC FF40BF
uint16_t nightMode[71] = {9220, 4510, 666, 498, 644, 500, 646, 500, 588, 554, 648, 496, 648, 496, 648, 498, 646, 498, 648, 1612, 652, 1586, 674, 1588, 672, 1588, 674, 1588, 676, 1586, 672, 1590, 672, 1590, 698, 470, 652, 1588, 674, 476, 672, 496, 654, 1588, 698, 448, 674, 474, 678, 474, 698, 1566, 672, 494, 658, 1606, 652, 1590, 672, 500, 648, 1588, 674, 1610, 652, 1588, 674, 40124, 9258, 2248, 596}; // NEC FF48B7

char command;
boolean isOn = false;
float humidity;
float temperature;
float heatingIndex;
unsigned long currentTime;
int interval = 60000;
DHT dht(DHTPIN, DHTTYPE);
String chipId = "esp";

void setup()
{
  irsend.begin();
  Serial.begin(9600);
  dht.begin();
  setupWifi();
  setupFirebase();
}

void sendRawValue(uint16_t rawValue[], String actionName)
{
  Serial.println(actionName);
  irsend.sendRaw(rawValue, 67, 38); // Send a raw data capture at 38kHz.
  delay(2000);
}
void setupFirebase()
{
  Firebase.begin(firebaseURl, authCode);
}

void setupWifi()
{
  WiFi.begin(wifiName, wifiPass);
  Serial.println("Hey i 'm connecting...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(".");
    delay(500);
  }
  Serial.println();
  Serial.println("I 'm connected and my IP address: ");
  Serial.println(WiFi.localIP());
}
void readHumidity()
{
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  heatingIndex = dht.computeHeatIndex(temperature, humidity, false);

  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Heat index: ");
  Serial.print(heatingIndex);
  Serial.println(" *C ");
}

void waitOneMinute()
{
  delay(2000);
}

void switchOnHumidifier()
{
  if (!isOn)
  {
    sendRawValue(switchOn, "SwitchOn");
    sendRawValue(heatOnOFf, "Heating");
    sendRawValue(nightMode, "Night Mode");
    isOn = true;
  }
  else
  {
    sendRawValue(switchOn, "SwitchOFF");
    isOn = false;
  }
}

void sendDataToFirebase()
{
  if (millis() - currentTime > interval)
  {
    currentTime = millis();
    sendToFirebase("temperature", temperature);
    sendToFirebase("humidity", humidity);
    sendToFirebase("heatingIndex", heatingIndex);
  }
}
void sendToFirebase(String attributeName, float value)
{
  String path = chipId + "/humidityHome/";
  Firebase.setFloat(path + attributeName, value);
  
  delay(1000);
}
void loop()
{
  waitOneMinute();
  readHumidity();
  sendDataToFirebase();
  if (!isnan(humidity))
  {
    if (humidity < 42 && !isOn)
    {
      switchOnHumidifier();
      isOn = true;
    }
    else if (humidity > 52 && isOn)
    {
      sendRawValue(switchOn, "SwitchOFF");
      isOn = false;
    }
  }
}
