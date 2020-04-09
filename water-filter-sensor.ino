/*
 * CALIBRATION VALUES
 * 
 * All of these must be reset after disassembling and reassembling the base.
 */

/*
 * Turn on debug output to allow for calibration.
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
const int MAXIMUM_WATER_LEVEL = 121000;

/*
 * The weight of the system when tap no longer flows out.
 * 
 * (Residual in top tank + residual in lower tank)
 */
const int MINIMUM_WATER_LEVEL = 117000;








/*
 * USER PREFERENCES
 * 
 * Settings which can be adjusted depending on the desired display
 */

/*
 * The fractional fill level (0.0 to 1.0) below which we should alert to refill
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
const int LOAD_SENSOR_READ_INTERVAL = 100;

/*
 * Screen dimensions
 */
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)




 

/*
 * APPLICATION CODE
 * 
 * No need to make any changes below here
 */

#include <movingAvg.h>
#include "HX711.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

HX711 load_sensor;

movingAvg smooth_weight(MOVING_AVERAGE_BUCKET_SIZE);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int low_safe_level;
int high_safe_level;

void setup() {

  int range = MAXIMUM_WATER_LEVEL - MINIMUM_WATER_LEVEL;
  low_safe_level = (range * LOW_SAFE_WATER_FRACTION) + MINIMUM_WATER_LEVEL;
  high_safe_level = (range * HIGH_SAFE_WATER_FRACTION) + MINIMUM_WATER_LEVEL;

  Serial.begin(9600);

  delay(1000);
  
  smooth_weight.begin();
  load_sensor.begin(LOAD_SENSOR_DATA, LOAD_SENSOR_CLOCK);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  while(!load_sensor.is_ready()){
    Serial.println("Waiting for load sensor...");
    delay(1000);
  }
}

int readWeight() {

  if (load_sensor.is_ready()) {
    int curr_weight = (load_sensor.read() / 10);
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
    
  return smooth_weight.getAvg();

}

void drawText(String text) {
  display.clearDisplay();

//  display.setTextSize(1);             // Normal 1:1 pixel scale
//  display.setTextColor(SSD1306_WHITE);        // Draw white text
//  display.setCursor(0,0);             // Start at top-left corner
//  display.println(F("Hello, world!"));
//
//  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
//  display.println(3.141592);

  display.setTextSize(5);             // Draw 2X-scale text
  display.setCursor(0,20); 
  display.setTextColor(SSD1306_WHITE);
  display.println(text);
//  display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
  delay(50);
}

int getCurrentFillPercent() {
  int current_weight = readWeight();

  if(current_weight <= MINIMUM_WATER_LEVEL){
    return 0;
  }

  double fraction = ((double)current_weight - (double)MINIMUM_WATER_LEVEL) / ((double)MAXIMUM_WATER_LEVEL - (double)MINIMUM_WATER_LEVEL) * 100;

  return (int)fraction;
}

void loop() {

//  int current_weight = readWeight();

  int fillPercent = getCurrentFillPercent();

  String percentText = String(fillPercent) + "%";
    
  drawText(percentText);
  
//  if(current_weight <= low_safe_level) {
//    bluePulse();
//  } else if(current_weight > low_safe_level && current_weight <= high_safe_level) {
//    solidBlue();
//  } else if(current_weight > high_safe_level && current_weight <= MAXIMUM_WATER_LEVEL) {
//    solidGreen();
//  } else if(current_weight > MAXIMUM_WATER_LEVEL) {
//    redPulse();
//  }

//  delay(LOOP_DELAY);

}
