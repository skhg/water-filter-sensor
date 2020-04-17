/*
 * CALIBRATION VALUES
 * 
 * All of these must be reset after disassembling and reassembling the base.
 */

/*
 * Turn on debug output to allow for calibration.
 * 
 * Values format:
 * MINIMUM_WATER_LEVEL, MAXIMUM_WATER_LEVEL, CURRENT_WEIGHT, CURRENT_WEIGHT_SMOOTHED_AVERAGE
 */
const boolean DEBUG_ON = true;

/*
 * The maximum weight of the system. Above this level, water will overflow. Updated by the data logger.
 * 
 * (Residual in top tank + full lower tank)
 */
int MAXIMUM_WATER_LEVEL = 121000;

/*
 * The weight of the system when tap no longer flows out. Updated by the data logger.
 * 
 * (Residual in top tank + residual in lower tank)
 */
int MINIMUM_WATER_LEVEL = 117000;








/*
 * USER PREFERENCES
 * 
 * Settings which can be adjusted depending on the desired display
 */

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
#define IP_PORT 8080









/*
 * SYSTEM CONFIGURATION
 * 
 * How the Arduino's physical devices are set up.
 */

/*
 * Pin positions for the HX-711 load sensor
 * 
 * These must be connected to analogue input pins.
 * 
 * Analogue input pins:
 * https://www.arduino.cc/reference/en/language/functions/analog-io/analogread/
 */
#define LOAD_SENSOR_DATA 14  // D5 on NodeMCU ESP8266 Lolin v3
#define LOAD_SENSOR_CLOCK 12 // D6 on NodeMCU ESP8266 Lolin v3

/*
 * Load sensor read interval
 * 
 * Reading the load sensor may take a few milliseconds and doing this on each cycle may be unnecessary.
 * This value allows you to read the sensor every n'th cycle
 */
const int LOAD_SENSOR_TRANSMIT_INTERVAL = 50;

/*
 * Screen dimensions
 */
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

/**
 * Delay before reboot
 */
#define REBOOT_DELAY_MS 1000

/**
 * How many seconds to wait before WiFi connection is established, or the system will reboot
 */
#define WIFI_CONNECT_TIMEOUT_SECONDS 30



 

/*
 * APPLICATION CODE
 * 
 * No need to make any changes below here
 */

#include <movingAvg.h>
#include "HX711.h"
#include <SPI.h>
#include <Wire.h>
#include<ESP8266WiFi.h> 
#include<home_wifi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

HX711 load_sensor;
WiFiClient client;
IPAddress server(IP_1,IP_2,IP_3,IP_4);
movingAvg smooth_weight(MOVING_AVERAGE_BUCKET_SIZE);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int transmit_cycle = 0;


void setup() {
  Serial.begin(9600);

  setupScreen();
  drawText("Booting...", 3);
  
  int range = MAXIMUM_WATER_LEVEL - MINIMUM_WATER_LEVEL;

  delay(1000); // Give the weight sensor a moment to initialise
  
  smooth_weight.begin();
  load_sensor.begin(LOAD_SENSOR_DATA, LOAD_SENSOR_CLOCK);

  while(!load_sensor.is_ready()){
    Serial.println("Waiting for load sensor...");
    delay(1000);
  }
}

void setupScreen() {
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
}

int readWeight() {

  if (load_sensor.is_ready()) {
    int curr_weight = (load_sensor.read() / 10);
    smooth_weight.reading(curr_weight);
  
    if(DEBUG_ON){
      Serial.print(MINIMUM_WATER_LEVEL);
      Serial.print(",");
      Serial.print(MAXIMUM_WATER_LEVEL);
      Serial.print(",");
      Serial.print(curr_weight);
      Serial.print(",");
      Serial.println(smooth_weight.getAvg());
    }
  }
    
  return smooth_weight.getAvg();

}

void drawText(String text, int textSize) {
  display.clearDisplay();

  display.setTextSize(textSize);
  display.setCursor(0,20);
  display.setTextColor(SSD1306_WHITE);
  display.println(text);

  display.display();
  delay(50);
}

int getCurrentFillPercent(int current_weight) {
  if(current_weight <= MINIMUM_WATER_LEVEL){
    return 0;
  }

  double fraction = ((double)current_weight - (double)MINIMUM_WATER_LEVEL) / ((double)MAXIMUM_WATER_LEVEL - (double)MINIMUM_WATER_LEVEL) * 100;

  return (int)fraction;
}

void loop() {
  connectToWifi();

  int currentWeight = readWeight();
  int fillPercent = getCurrentFillPercent(currentWeight);

  String percentText = String(fillPercent) + "%";
    
  drawText(percentText, 5);

  transmitReading(currentWeight, fillPercent);

}

void transmitReading(int weight, int fillPercent) {

  if(transmit_cycle == LOAD_SENSOR_TRANSMIT_INTERVAL){
    if(transmit_cycle >= LOAD_SENSOR_TRANSMIT_INTERVAL){
      transmit_cycle = 0;
    }

    String json = "{\"load\":"+String(weight)+",\"fillPercentage\":"+String(fillPercent)+"}";
    
    if (client.connect(server, IP_PORT)) {      
      // Make a HTTP request:
      client.println("POST /kitchen/water-filter HTTP/1.0");
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

void reboot(){
  delay(REBOOT_DELAY_MS);
  Serial.println("Rebooting...");
  ESP.restart();
}


void connectToWifi(){
  String wifiConnectionInfo = "Connectingto WiFi"; //Newline wrap on the OLED makes this appear correctly
  
  if(WiFi.status() == WL_CONNECTED){
    return;  
  }

  drawText(wifiConnectionInfo, 2);
  
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int connectAttempts = 0;
  int connectRetryInterval = 500;
  int rebootCountdown = WIFI_CONNECT_TIMEOUT_SECONDS * 1000;

  String dotsString = "";
  
  while (WiFi.status() != WL_CONNECTED) {

    if(dotsString.length() < 3){
      dotsString = dotsString + ".";
    }else{
      dotsString = "";
    }
    
    drawText(wifiConnectionInfo + dotsString, 2);
    delay(connectRetryInterval);
    
    Serial.print(".");

    rebootCountdown = rebootCountdown - connectRetryInterval;
    
    if(rebootCountdown < 0) {
      reboot();
    }
  }
  
  Serial.println("");
  Serial.println("WiFi connected"); 
}
