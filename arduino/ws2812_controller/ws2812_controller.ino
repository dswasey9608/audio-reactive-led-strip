#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NeoPixelBus.h>

// Set to the number of LEDs in your LED strip
#define NUM_LEDS 150
// Maximum number of packets to hold in the buffer. Don't change this.
#define BUFFER_LEN 1024
// Toggles FPS output (1 = print FPS over serial, 0 = disable output)
#define PRINT_FPS 0

// Define states for mode toggles
#define AUDIO_MODE 0
#define ALT_LED_MODE 1
#define EE_MODE 2

//NeoPixelBus settings
const uint8_t PixelPin = 3;  // make sure to set this to the correct pin, ignored for Esp8266(set to 3 by default for DMA)

// Wifi and socket settings
const char* ssid     = "minas_tirith";
const char* password = "th3wh1tetreesu23!";
unsigned int localPort = 7777;
char packetBuffer[BUFFER_LEN];

// LED strip
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> ledstrip(NUM_LEDS, PixelPin);

WiFiUDP port;

// Network information
// IP must match the IP in config.py
IPAddress ip(10,0,0,50);
// Set gateway to your router's gateway
IPAddress gateway(10,0,0,1);
IPAddress subnet(255, 255, 255, 0);

// Mode selections
uint8_t MODE;

// Set up additional mode LEDs
const int led_pin = 5;
const int button_pin = 4;
int led_state = LOW;
unsigned long button_time = 0;
unsigned long prev_btime = 0;
unsigned long release_time = 0;

void ICACHE_RAM_ATTR isr(){
  button_time = millis();
  bool released = false;
  unsigned long release_time = 0;
  if (button_time - prev_btime > 250){
    while (!released && millis()-button_time < 2100){
      if (digitalRead(button_pin) == HIGH){
        released = true;
        release_time = millis();
      }
    }

    
    if (millis() - button_time >= 2000){
      MODE = EE_MODE;
      Serial.println("EE MODE");
      if (release_time == 0){
        unsigned long delay = millis();
        unsigned long rtime = delay;
        unsigned long prtime = delay;
        bool first_release_flag = false;
        released = false;
        while(!released){
          if (first_release_flag){
            if (digitalRead(button_pin)){
              rtime = millis();
              if (rtime-prtime > 250){
                released = true;
              }
            }
          } else if (digitalRead(button_pin)){
            first_release_flag = true;
            prtime = millis();
          }
        }
      }
    } else {
      MODE = (MODE + 1) % 2;
      if (MODE == AUDIO_MODE){
        digitalWrite(led_pin, LOW);
      }
      prev_btime = button_time;
    }
  }
}


void setup() {
    MODE = AUDIO_MODE;
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);
    pinMode(button_pin, INPUT_PULLUP);
    attachInterrupt(button_pin, isr, FALLING);
    Serial.begin(9600);
    WiFi.config(ip, gateway, subnet);
    WiFi.begin(ssid, password);
    Serial.println("");
    // Connect to wifi and print the IP address over serial
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    port.begin(localPort);
    ledstrip.Begin();//Begin output
    ledstrip.Show();//Clear the strip for use
}

uint8_t N = 0;
#if PRINT_FPS
    uint16_t fpsCounter = 0;
    uint32_t secondTimer = 0;
#endif

void loop() {
  if (MODE == AUDIO_MODE){
    // Read data over socket
    int packetSize = port.parsePacket();
    // If packets have been received, interpret the command
    if (packetSize) {
        int len = port.read(packetBuffer, BUFFER_LEN);
        for(int i = 0; i < len; i+=4) {
            packetBuffer[len] = 0;
            N = packetBuffer[i];
            RgbColor pixel((uint8_t)packetBuffer[i+1], (uint8_t)packetBuffer[i+2], (uint8_t)packetBuffer[i+3]);
              
            ledstrip.SetPixelColor(N, pixel);
        } 
        ledstrip.Show();
        #if PRINT_FPS
            fpsCounter++;
            Serial.print("/");//Monitors connection(shows jumps/jitters in packets)
        #endif
    }
    #if PRINT_FPS
        if (millis() - secondTimer >= 1000U) {
            secondTimer = millis();
            Serial.printf("FPS: %d\n", fpsCounter);
            fpsCounter = 0;
        }   
    #endif
  } else if (MODE == ALT_LED_MODE){
    digitalWrite(led_pin, !digitalRead(led_pin));
    Serial.println("LED toggled");
    delay(500);
  } else if (MODE == EE_MODE){
    digitalWrite(led_pin, !digitalRead(led_pin));
    Serial.println("Let's get crazy!");
    delay(100);
  }
}


