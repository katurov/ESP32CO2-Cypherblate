#include <Arduino.h>

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"

#include <Wire.h>
#include <SensirionCore.h>
#include <SensirionI2CScd4x.h>

#include <WiFi.h>
#include <esp_now.h>


#include "MadimiOne_Regular30pt7b.h"
#include "MadimiOne_Regular15pt7b.h"

#define DEG2RAD 0.0174532925

#define TFT_DC 26
#define TFT_CS 27
#define TFT_RST 25
// #define TFT_BL  27
// If display breakout has a backlight control pin, that can be defined here
// as TFT_BL. On some breakouts it's not needed, backlight is always on.

// Display constructor for primary hardware SPI connection -- the specific
// pins used for writing to the display are unique to each board and are not
// negotiable. "Soft" SPI (using any pins) is an option but performance is
// reduced; it's rarely used, see header file for syntax if needed.
Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

SensirionI2CScd4x scd4x;


// Colors for Cypherblate
unsigned int colours[7] = {
  GC9A01A_DARKGREEN,
  GC9A01A_GREEN,
  GC9A01A_GREENYELLOW,
  GC9A01A_YELLOW,
  GC9A01A_ORANGE,
  GC9A01A_RED,
  GC9A01A_MAROON
};

// Variables to know the size of the textbox
int16_t xb = 0;
int16_t yb = 0;
uint16_t w = 1;
uint16_t h = 1;

// A center for CO2 label
uint16_t x = 120;
uint16_t y = 150;

// A buf for CO2 label, for temperature and humidity
String buf;
char tbuf[4];

#define WIFI_CHANNEL 1
#define MY_NAME "DEV NODE"

#define MY_NAME "ORIGNODE"
#define MY_PASSWORD "THISISTH"

uint8_t partnerAddress[6] = { 0xC8, 0x2B, 0x96, 0x2F, 0x69, 0xE8 };  // Test ENV
// uint8_t partnerAddress[6] = { 0xEC, 0xFA, 0xBC, 0x4C, 0x47, 0xD3 }; // Prod ENV
esp_now_peer_info_t peerInfo;

void transmissionComplete(const uint8_t *receiver_mac, esp_now_send_status_t transmissionStatus) {
  Serial.println(transmissionStatus == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void dataReceived(const uint8_t *senderMac, const uint8_t *data, int dataLength) {}

struct __attribute__((packed)) dataPacket {
  char rule;
  float voltage;
  float temp1;
  float temp2;
  float temp3;
  float prssr;
  float humid;
};

dataPacket packet;


// #########################################################################
// Draw an arc with a defined thickness
// #########################################################################
// https://forum.arduino.cc/t/adafruit_gfx-fillarc/397741/6
//
// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 3 degree segments to draw (120 => 360 degree arc)
// rx = x axis radius
// yx = y axis radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same an arc of a circle is drawn

void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour) {

  byte seg = 3;  // Segments are 3 degrees wide = 120 segments for 360 degrees
  byte inc = 3;  // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * DEG2RAD);
    float sy = sin((i - 90) * DEG2RAD);
    uint16_t x0 = sx * (rx - w) + x;
    uint16_t y0 = sy * (ry - w) + y;
    uint16_t x1 = sx * rx + x;
    uint16_t y1 = sy * ry + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * DEG2RAD);
    float sy2 = sin((i + seg - 90) * DEG2RAD);
    int x2 = sx2 * (rx - w) + x;
    int y2 = sy2 * (ry - w) + y;
    int x3 = sx2 * rx + x;
    int y3 = sy2 * ry + y;

    tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
  }
}

void drawCypherblate() {

  int start_deg = 258;

  for (int i = 0; i < 7; i++) {
    fillArc(120, 120, start_deg + (i * 30), 9, 120, 120, 25, colours[i]);
  }

  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(1);
  tft.setCursor(15, 155);
  tft.println("400");
  tft.setCursor(220, 155);
  tft.println("5K");
}

void setup() {
  Serial.begin(115200);

#if defined(TFT_BL)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif  // end TFT_BL

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP is FAILED");
  } else {
    Serial.println("ESP is OK");
  }
  esp_now_register_send_cb(transmissionComplete);  // this function will get called once all data is sent
  esp_now_register_recv_cb(dataReceived);          // this function will get called whenever we receive data

  // Register peer
  memcpy(peerInfo.peer_addr, partnerAddress, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("ESP is FAILED to ADD PEER");
  } else {
    Serial.println("ESP ADD PEER OK");
  }

  packet.rule = 't';



  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // stop potentially previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error) {
    Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error) {
    Serial.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.print("Error trying to execute startPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  Serial.println("Waiting for first measurement... (5 sec)");
  delay(5000);


  // Make the Display ready for our command
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(GC9A01A_BLACK);
  tft.setTextWrap(false);

  // Draw color Arcs
  drawCypherblate();
}

void loop() {
  long g, c;

  uint16_t error;
  char errorMessage[256];

  delay(100);

  // Read Measurement
  uint16_t co2 = 0;
  float temperature = 0.0f;
  float humidity = 0.0f;
  bool isDataReady = false;
  error = scd4x.getDataReadyFlag(isDataReady);
  if (error) {
    Serial.print("Error trying to execute getDataReadyFlag(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
    return;
  }
  if (!isDataReady) {
    return;
  }
  error = scd4x.readMeasurement(co2, temperature, humidity);
  if (error) {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else if (co2 == 0) {
    Serial.println("Invalid sample detected, skipping.");
  } else {
    Serial.print("Co2:");
    Serial.print(co2);
    Serial.print("\t");
    Serial.print("Temperature:");
    Serial.print(temperature);
    Serial.print("\t");
    Serial.print("Humidity:");
    Serial.println(humidity);
  }

  packet.temp1 = temperature;
  packet.temp2 = 0.0;
  packet.temp3 = float(co2);
  packet.prssr = 0.0;
  packet.humid = humidity;
  packet.voltage = 0.0;


  buf = String(co2);

  if (co2 < 1000) {
    g = map(co2, 350, 1000, 258, 258 + 2 * 30);
    if (co2 < 600) c = 0;
    else c = 1;
  } else {
    if (co2 < 2000) {
      g = map(co2, 1000, 2000, 258 + 2 * 30, 258 + 4 * 30);
      if (co2 < 1500) c = 2;
      else c = 3;
    } else {
      g = map(co2, 2000, 5000, 258 + 4 * 30, 258 + 7 * 30);
      c = map(co2, 2000, 4000, 4, 6);
    }
  }

  tft.fillCircle(120, 120, 95, GC9A01A_BLACK);
  tft.fillRect(0, 178, 240, 240, GC9A01A_BLACK);

  fillArc(120, 120, g, 4, 93, 93, 25, colours[int(c)]);

  tft.setFont(&MadimiOne_Regular30pt7b);
  tft.setTextSize(1);
  tft.setTextColor(GC9A01A_WHITE);
  tft.getTextBounds(buf, x, y, &xb, &yb, &w, &h);  //calc width of new string
  tft.setCursor(x - w / 2, y);
  tft.println(buf);


  tft.setFont(&MadimiOne_Regular15pt7b);
  tft.setTextSize(1);

  sprintf(tbuf, "%.1f", temperature);

  tft.getTextBounds(tbuf, x, y, &xb, &yb, &w, &h);  //calc width of new string
  tft.setCursor(100 - w, 200);
  tft.println(tbuf);

  tft.setTextColor(0xad9d);

  sprintf(tbuf, "%.1f", humidity);
  tft.setCursor(140, 200);
  tft.println(tbuf);

  esp_err_t result = esp_now_send(partnerAddress, (uint8_t *)&packet, sizeof(packet));

  delay(5000);
}
