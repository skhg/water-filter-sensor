/*
 * CALIBRATION VALUES
 * 
 * All of these must be reset after disassembling and reassembling the base.
 */

/*
 * Turn on debug output to allow for calibration. Turn off to stop the TX LED flashing
 * 
 * Values format:
 * MINIMUM_WATER_LEVEL, LOW_SAFE_LEVEL, HIGH_SAFE_LEVEL, MAXIMUM_WATER_LEVEL, CURRENT_WEIGHT, CURRENT_WEIGHT_SMOOTHED_AVERAGE
 */
const boolean DEBUG_ON = true;

/*
 * The maximum weight of the system. Above this level, water will overflow.
 * 
 * (Residual in top tank + full lower tank)
 */
const int MAXIMUM_WATER_LEVEL = 9682;

/*
 * The weight of the system when tap no longer flows out.
 * 
 * (Residual in top tank + residual in lower tank)
 */
const int MINIMUM_WATER_LEVEL = 9409;








/*
 * USER PREFERENCES
 * 
 * Settings which can be adjusted depending on the desired display
 */

/*
 * The fractional fill level (0.0 to 1.0) below which we should alert to refill (Blue flashing)
 */
const double LOW_SAFE_WATER_FRACTION = 0.3;

/* 
 * The highest fractional fill level (0.0 to 1.0) we aim to reach when we refill
 */
const double HIGH_SAFE_WATER_FRACTION = 0.8;

/*
 * How many values to use for smoothing out the data and forming a moving average
 */
const int MOVING_AVERAGE_BUCKET_SIZE = 20;

/**
 * IP Address and port to send data to
 */
#define IP_1 192
#define IP_2 168
#define IP_3 178
#define IP_4 29
#define IP_PORT 8081

/**
 * No need to make further changes below here
 */



const int LOAD_SENSOR_READ_INTERVAL = 100;
const int LOAD_SENSOR_TRANSMIT_INTERVAL = 1000;




/*
 * LED total brightness factor
 * 
 * If your LED is too bright, reduce this value (0.0 to 1.0)
 */
const double LED_MASTER_SCALE = 1.0;

/*
 * LED relative brightness factors
 * 
 * Each of the RGB LED's components may appear to be slightly brighter than the others.
 * Set the dimmest LED to 1.0 and adjust the others accordingly (0.0 to 1.0)
 * 
 */
const double LED_BLUE_SCALE = 1;
const double LED_RED_SCALE = 1;
const double LED_GREEN_SCALE = 1;



/**
 * System configuration values.
 * 
 * These are unlikely to be needed to be changed after development is complete
 */

/**
 * Short wait on startup to try to prevent the BMP180 sensor for returning bad data
 */
#define STARTUP_DELAY_MS 5000

/**
 * Delay between taking readings
 */
#define LOOP_DELAY_MS 1

/**
 * Delay before reboot
 */
#define REBOOT_DELAY_MS 1000

#define LED_MAX_PULSE_VALUE 1023

/*
  NodeMCU v3 Arduino/Physical pinout map
  
  Arduino | NodeMCU | Available for use
  0       | D3      | HX711_SCK
  1       | TX      | X
  2       | D4      | 
  3       | RX      | X
  4       | D2      | HX711_DT
  5       | D1      | 
  6       |         | X
  7       |         | X
  8       |         | X
  9       | S2      | X
  10      | S3      | X
  11      |         | X
  12      | D6      | LED_GREEN
  13      | D7      | 
  14      | D5      | LED_BLUE
  15      | D8      | LED_RED
  16      | D0      | 
 */
#define BLUE_LED 14
#define GREEN_LED 12
#define RED_LED 15
#define LOAD_SENSOR_DATA 4
#define LOAD_SENSOR_CLOCK 0

#include <movingAvg.h>
#include<ESP8266WiFi.h> 
#include<home_wifi.h>
#include <Q2HX711.h>

/**
 * Application starts here
 */
WiFiClient client;
IPAddress server(IP_1,IP_2,IP_3,IP_4);
Q2HX711 load_sensor(LOAD_SENSOR_DATA, LOAD_SENSOR_CLOCK);
movingAvg smooth_weight(MOVING_AVERAGE_BUCKET_SIZE);

int low_safe_level;
int high_safe_level;
int sensor_cycle = 0; // Current state of the cycle for sensor reading
int transmit_cycle = 0;

//state used only for controlling light pulse cycle
int pulse_brightness = 0;    // how bright the LED is during current cycle
int fadeAmount = 1;    // how many points to fade the LED by

boolean isFirstReading = true;

void setup() {
  int range = MAXIMUM_WATER_LEVEL - MINIMUM_WATER_LEVEL;
  low_safe_level = (range * LOW_SAFE_WATER_FRACTION) + MINIMUM_WATER_LEVEL;
  high_safe_level = (range * HIGH_SAFE_WATER_FRACTION) + MINIMUM_WATER_LEVEL;
  
  // LED pins are all outputs
  pinMode(BLUE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Serial.begin(9600);

  smooth_weight.begin();

  smooth_weight.reading(MINIMUM_WATER_LEVEL);

  delay(STARTUP_DELAY_MS);
}

int readWeight() {

  if(sensor_cycle == LOAD_SENSOR_READ_INTERVAL){
    if(sensor_cycle >= LOAD_SENSOR_READ_INTERVAL){
      sensor_cycle = 0;
    }

    int curr_weight = (load_sensor.read() / 1000);
    smooth_weight.reading(curr_weight);

    if(DEBUG_ON){
      Serial.print(MINIMUM_WATER_LEVEL);
      Serial.print(",");
      Serial.print(low_safe_level);
      Serial.print(",");
      Serial.print(high_safe_level);
      Serial.print(",");
      Serial.print(MAXIMUM_WATER_LEVEL);
      Serial.print(",");
      Serial.print(curr_weight);
      Serial.print(",");
      Serial.println(smooth_weight.getAvg());
    }
    
  }

  sensor_cycle++;
    
  return smooth_weight.getAvg();

}

void transmitReading(int value) {

  if(transmit_cycle == LOAD_SENSOR_TRANSMIT_INTERVAL){
    if(transmit_cycle >= LOAD_SENSOR_TRANSMIT_INTERVAL){
      transmit_cycle = 0;
    }

    String json = "{\"load\":"+String(value)+"}";

    if (client.connect(server, IP_PORT)) {      
      // Make a HTTP request:
      client.println("POST /data/live HTTP/1.0");
      client.println("Content-Type: application/json");
      client.print("Content-Length: ");
      client.println(json.length());
      client.println();
      client.println(json);
      client.println();
      client.stop();
    }else{
      Serial.println("ERROR: Failed to connect");
      reboot();
    }
    
  }

  transmit_cycle++; 
}

void bluePulse() {
  rgbPulse(false, false, true);
}

void solidGreen() {
  rgbSolid(false, true, false);
}

void solidBlue() {
  rgbSolid(false, false, true);
}

void redPulse() {
  rgbPulse(true, false, false);
}

void setColours(boolean red, boolean green, boolean blue, int brightness) {
  if(blue) {
    analogWrite(BLUE_LED, brightness * LED_MASTER_SCALE * LED_BLUE_SCALE);
  }else{
    analogWrite(BLUE_LED, 0);
  }
  if(red) {
    analogWrite(RED_LED, brightness * LED_MASTER_SCALE * LED_RED_SCALE);
  }else{
    analogWrite(RED_LED, 0);
  }
  if(green) {
    analogWrite(GREEN_LED, brightness * LED_MASTER_SCALE * LED_GREEN_SCALE);
  }else{
    analogWrite(GREEN_LED, 0);
  }
}

void rgbSolid(boolean red, boolean green, boolean blue) {
  setColours(red, green, blue, LED_MAX_PULSE_VALUE);
}

void rgbPulse(boolean red, boolean green, boolean blue) {

  setColours(red, green, blue, pulse_brightness);
  
  // change the brightness for next time through the loop:
  pulse_brightness = pulse_brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (pulse_brightness <= 0 || pulse_brightness >= LED_MAX_PULSE_VALUE) {
    fadeAmount = -fadeAmount;
  }
}

void loop() {
  connectToWifi();

  int current_weight = readWeight();


  if(current_weight <= low_safe_level) {
    bluePulse();
  } else if(current_weight > low_safe_level && current_weight <= high_safe_level) {
    solidBlue();
  } else if(current_weight > high_safe_level && current_weight <= MAXIMUM_WATER_LEVEL) {
    solidGreen();
  } else if(current_weight > MAXIMUM_WATER_LEVEL) {
    redPulse();
  }

  if(isFirstReading){
    isFirstReading = false;
  }else{
    transmitReading(current_weight);
  }
  
  delay(LOOP_DELAY_MS);
}


void reboot(){
  delay(REBOOT_DELAY_MS);
  Serial.println("Rebooting...");
  ESP.restart();
}


void connectToWifi(){
  if(WiFi.status() == WL_CONNECTED){
    return;  
  }
  
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    reboot();
  }
  
  Serial.println("");
  Serial.println("WiFi connected"); 
}
