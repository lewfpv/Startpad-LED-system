// ESP32 MAC: EC:DA:3B:BF:E6:44
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>
#include "message.h"

//VERSION 1.1
String fwversion = "1.1";

// Globális változók a fogadó oldalon
int led1_r = 255;
int led1_g = 0;
int led2_r = 255;
int led2_g = 0;
int led3_r = 255;
int led3_g = 0;
int led4_r = 255;
int led4_g = 0;

int led_brightness = 200;  // 0–255
bool leds_off = false;
bool ledStateChanged = true;

// LED pinek és darabszám
#define NUM_LEDS 37  // minden LED csík 1 pixel
#define LED1_PIN 10
#define LED2_PIN 2
#define LED3_PIN 3
#define LED4_PIN 1

// LED csíkok objektumai
Adafruit_NeoPixel led1(NUM_LEDS, LED1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel led2(NUM_LEDS, LED2_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel led3(NUM_LEDS, LED3_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel led4(NUM_LEDS, LED4_PIN, NEO_GRB + NEO_KHZ800);

// Üzenet struktúra
typedef struct struct_message {
  char id[32];
  int value;
} struct_message;

struct_message incomingData;

// Segédfüggvény LED szín állításra
void setColor(Adafruit_NeoPixel &strip, uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}



//SendNOW(cimzett mac cim, üzenet);
void SendNOW(const uint8_t *mac, const Message &msg) {
  esp_err_t result = esp_now_send(mac, (uint8_t *)&msg, sizeof(msg));
  (result == ESP_OK) ? Serial.println("✅ Message sent successfully") : Serial.printf("❌ Send error (%d)\n", result);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Send status: OK, " + String(macStr) : "Send status: FAIL, " + String(macStr));
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  if (len != sizeof(Message)) {
    Serial.printf("⚠️ Invalid message size: %d\n", len);
    return;
  }

  Message msg;
  memcpy(&msg, incomingData, sizeof(msg));
  Serial.printf("📩 Received msg: from=%s, type=%d, index=%d, value=%d\n", macStr, msg.type, msg.index, msg.value);

  switch (msg.type) {
    case 9: //led
        switch (msg.index) {
          case 0:
              led1_r = 255;
              led1_g = 0;
              led2_r = 255;
              led2_g = 0;
              led3_r = 255;
              led3_g = 0;
              led4_r = 255;
              led4_g = 0;
              ledStateChanged = true;  // jelezzük, hogy frissíteni kell
              leds_off = false; // turn ON led
          break;
        }
    break;
    case 5: //led
        switch (msg.index) {
          case 0:
              led_brightness = msg.value;
              Serial.printf("💡 Brightness set to: %d\n", led_brightness);
              ledStateChanged = true;  // jelezzük, hogy frissíteni kell
          break;
          case 1:
              switch (msg.value){
                case 0:
                leds_off = true; // turn ON led
                ledStateChanged = true;  // jelezzük, hogy frissíteni kell
                break;
                case 1:
                leds_off = false; // turn ON led
                ledStateChanged = true;  // jelezzük, hogy frissíteni kell
                break;
              }
          break;
        }
    break;
    case 1: // 1. tipusú verseny üzenet
      switch (msg.index) {
        case 1:  //R1 versenyző
          switch (msg.value) {
            case 0:
                  led1_r = 0;
                  led1_g = 255;
                  ledStateChanged = true;  // jelezzük, hogy frissíteni kell
            break;
          }
        break;
        case 2:
          switch (msg.value) {
            case 0:
                  led2_r = 0;
                  led2_g = 255;
                  ledStateChanged = true;  // jelezzük, hogy frissíteni kell
            break;
          }
        break;
        case 3:
          switch (msg.value) {
            case 0:
                  led3_r = 0;
                  led3_g = 255;
                  ledStateChanged = true;  // jelezzük, hogy frissíteni kell
            break;
          }
        break;
        case 4:
          switch (msg.value) {
            case 0:
                  led4_r = 0;
                  led4_g = 255;
                  ledStateChanged = true;  // jelezzük, hogy frissíteni kell
            break;
          }
        break;
      }
    break;
    case 2: // Fényerő
      //analogWrite(LED_BUILTIN, msg.value);
    break;
    default:
      Serial.println("❓ Unknown message type");
  }
}

void setup() {
  delay(2000);  // VÁRJ 1 másodpercet, hogy a Serial monitor kapcsolódni tudjon
  Serial.begin(115200);

  // LED-ek inicializálása és alapból piros
  led1.begin(); setColor(led1, led1_r, 0, 0);
  led2.begin(); setColor(led2, led2_r, 0, 0);
  led3.begin(); setColor(led3, led3_r, 0, 0);
  led4.begin(); setColor(led4, led4_r, 0, 0);

  Serial.print("ESP32 MAC: ");
  Serial.println(WiFi.macAddress());

  // ESP-NOW inicializálás
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW Ready, waiting for messages...");
}

void loop() {
    if (ledStateChanged) {
        if (leds_off) {
            setColor(led1, 0, 0, 0);
            setColor(led2, 0, 0, 0);
            setColor(led3, 0, 0, 0);
            setColor(led4, 0, 0, 0);
        } else {
            int rr1 = (led1_r * led_brightness) / 255;
            int gg1 = (led1_g * led_brightness) / 255;
            int rr2 = (led2_r * led_brightness) / 255;
            int gg2 = (led2_g * led_brightness) / 255;
            int rr3 = (led3_r * led_brightness) / 255;
            int gg3 = (led3_g * led_brightness) / 255;
            int rr4 = (led4_r * led_brightness) / 255;
            int gg4 = (led4_g * led_brightness) / 255;
            setColor(led1, rr1, gg1, 0);
            setColor(led2, rr2, gg2, 0);
            setColor(led3, rr3, gg3, 0);
            setColor(led4, rr4, gg4, 0);
        }
        ledStateChanged = false; // állapot vissza nullára
    }
}