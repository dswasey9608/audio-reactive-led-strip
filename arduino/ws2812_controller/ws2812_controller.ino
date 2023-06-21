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
#define RGB_SEL_MODE 1
#define RED 0x01
#define GREEN 0x02
#define BLUE 0x04

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

// Mode selections and button presses
uint8_t MODE;
const uint8_t NUM_MODES = 2;
bool switch_mode_flag = false;
bool short_press = false;
unsigned long init_time = 0;


// Set up additional buttons and LEDs
const uint8_t rled_pin = 5;   // D1
const uint8_t gled_pin = 4;   // D2
const uint8_t bled_pin = 2;   // D4

const uint8_t r_btn_pin = 14; // D5
const uint8_t m_btn_pin = 12; // D6
const uint8_t l_btn_pin = 13; // D7

// RGB_SEL_MODE variables
uint8_t CUR_COLOR = RED;
bool ADD_COLOR = false;
bool SUB_COLOR = false;
uint16_t RVAL = 0;
uint16_t GVAL = 0;
uint16_t BVAL = 0;
uint8_t COLOR_INCREMENT = 25;


/**
 * @brief Debounce a button press
 * 
 * @param pin_num: Number of pin to check
 * @param level:   Level to debounce
 */
inline void debounce(const uint8_t &pin_num, const bool level){
  bool first_switched = false;
  unsigned long init_stime = 0;
  bool cur_level = level;
  while (true){
    cur_level = digitalRead(pin_num);
    if (first_switched && cur_level == level){
      if (millis()-init_stime > 75){
        return;
      }
    } else if (cur_level == level){
      first_switched = true;
      init_stime = millis();
    }
  }
}

/**
 * @brief Displays the current mode number in binary on individual R, G, B LEDs
 */
inline void display_mode(){
  // Do one-based modes so mode zero has an indicator
  uint8_t mode_p1 = MODE + 1;

  // Use bits of mode_p1 to indicate mode on LED pins
  digitalWrite(bled_pin, mode_p1 & 0x01);
  digitalWrite(gled_pin, (mode_p1 & 0x02) >> 1);
  digitalWrite(rled_pin, (mode_p1 & 0x04) >> 2);
}

/**
 * @brief Turn off all RGB-select LEDs
 */
inline void clear_mode_leds(){
  digitalWrite(rled_pin, LOW);
  digitalWrite(gled_pin, LOW);
  digitalWrite(bled_pin, LOW);
}

/**
 * @brief Display currently selected color on R, G, or B LED.
 */
inline void write_rgb_sel(){
  digitalWrite(rled_pin, CUR_COLOR & 0x01);
  digitalWrite(gled_pin, (CUR_COLOR & 0x02) >> 1);
  digitalWrite(bled_pin, (CUR_COLOR  & 0x04) >> 2);
}

/**
 * @brief ISR for changing LED modes
 * 
 * @details Checks for a double button press before changing modes
 */
void ICACHE_RAM_ATTR change_mode_isr(){
  // Debounce the button on press
  debounce(m_btn_pin, LOW);

  // If the first button was registered and another comes within 0.5 seconds...
  if (switch_mode_flag && millis() - init_time < 500){
    switch_mode_flag = false;

    // Double press registered, flash to indicate mode change
    for (int i = 0; i < 2; i++){
        digitalWrite(rled_pin, HIGH);
        digitalWrite(gled_pin, HIGH);
        digitalWrite(bled_pin, HIGH);
        delay(100);
        clear_mode_leds();
        delay(100);
      }
      MODE = (MODE + 1) % NUM_MODES;
      CUR_COLOR = RED;

      // Display mode and clear LEDs
      display_mode();
      delay(500);
      clear_mode_leds();
  } else if (switch_mode_flag){
    // Clear switch flag since timing wasn't met
    switch_mode_flag = false;
    short_press = true;
  } else {
    // Set switch flag
    switch_mode_flag = true;
    short_press = true;
    init_time = millis();
  }

  // Debounce the release
  debounce(m_btn_pin, HIGH);
}

/**
 * @brief Set a flag for adding color to currently selected color
 */
void ICACHE_RAM_ATTR add_isr(){
  debounce(r_btn_pin, LOW);
  ADD_COLOR = true;
  debounce(r_btn_pin, HIGH);
}

/**
 * @brief Set a flag for subtracting color to currently selected color
 */
void ICACHE_RAM_ATTR sub_isr(){
  debounce(l_btn_pin, LOW);
  SUB_COLOR = true;
  debounce(l_btn_pin, HIGH);
}

/**
 * @brief Set up the pins and some initial values, connect to WiFi
 */
void setup() {
    MODE = AUDIO_MODE;

    // Set LED pins
    pinMode(rled_pin, OUTPUT);
    pinMode(gled_pin, OUTPUT);
    pinMode(bled_pin, OUTPUT);
    clear_mode_leds();

    // Set button pins
    pinMode(m_btn_pin, INPUT_PULLUP);
    pinMode(l_btn_pin, INPUT_PULLUP);
    pinMode(r_btn_pin, INPUT_PULLUP);

    // Set interrupts
    attachInterrupt(m_btn_pin, change_mode_isr, FALLING);
    attachInterrupt(r_btn_pin, add_isr, FALLING);
    attachInterrupt(l_btn_pin, sub_isr, FALLING);

    // Configure WiFi
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
    clear_mode_leds();
    if (short_press){
      short_press = false;
    }
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
  } else if (MODE == RGB_SEL_MODE){
    write_rgb_sel();

    // Change active color if a button press was registered
    if (short_press){
      short_press = false;
      CUR_COLOR = ((CUR_COLOR << 1) | (CUR_COLOR >> 2)) & (0x7);
      write_rgb_sel();
    }

    // Check for color updates
    if (ADD_COLOR){
      ADD_COLOR = false;
      RVAL += (CUR_COLOR & 0x01) * COLOR_INCREMENT;
      GVAL += ((CUR_COLOR & 0x02) >> 1) * COLOR_INCREMENT;
      BVAL += ((CUR_COLOR & 0x04) >> 1) * COLOR_INCREMENT;

      // Check for overflow
      if (RVAL > 255){
        RVAL = 255;
      }
      if (BVAL > 255){
        BVAL = 255;
      }
      if (GVAL > 255){
        GVAL = 255;
      }
    }
    
    if (SUB_COLOR){
      SUB_COLOR = false;
      RVAL -= (CUR_COLOR & 0x01) * COLOR_INCREMENT;
      GVAL -= ((CUR_COLOR & 0x02) >> 1) * COLOR_INCREMENT;
      BVAL -= ((CUR_COLOR & 0x04) >> 1) * COLOR_INCREMENT;  

      // Check for underflow
      if (RVAL > 255){
        RVAL = 0;
      }
      if (BVAL > 255){
        BVAL = 0;
      }
      if (GVAL > 255){
        GVAL = 0;
      }
    }

    // Set current RGB values for LED strip
    for (int i = 0; i < NUM_LEDS; i++){
      ledstrip.SetPixelColor(i, RgbColor((uint8_t)RVAL, (uint8_t)GVAL, (uint8_t)BVAL));
    }

    // Update strip
    ledstrip.Show();
  }
}


