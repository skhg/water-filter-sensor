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
const boolean DEBUG_ON = false;

/*
 * The maximum weight of the system. Above this level, water will overflow.
 * 
 * (Residual in top tank + full lower tank)
 */
const int MAXIMUM_WATER_LEVEL = 9345;

/*
 * The weight of the system when tap no longer flows out.
 * 
 * (Residual in top tank + residual in lower tank)
 */
const int MINIMUM_WATER_LEVEL = 9179;








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








/*
 * SYSTEM CONFIGURATION
 * 
 * How the Arduino's physical devices are set up.
 */

/*
 * Pin positions for the RGB LED. These must be connected to PWM pins in order to vary brightness
 * 
 * PWM pin locations:
 * https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/
 */
#define BLUE_LED 9
#define GREEN_LED 10
#define RED_LED 11

/*
 * Pin positions for the HX-711 load sensor
 * 
 * These must be connected to analogue input pins.
 * 
 * Analogue input pins:
 * https://www.arduino.cc/reference/en/language/functions/analog-io/analogread/
 */
#define LOAD_SENSOR_DATA A0
#define LOAD_SENSOR_CLOCK A1

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
const double LED_BLUE_SCALE = 0.2;
const double LED_RED_SCALE = 1;
const double LED_GREEN_SCALE = 0.25;

/*
 * Load sensor read interval
 * 
 * Reading the load sensor may take a few milliseconds and doing this on each cycle may be unnecessary.
 * This value allows you to read the sensor every n'th cycle
 */
const int LOAD_SENSOR_READ_INTERVAL = 100;

/*
 * Loop delay in milliseconds
 * 
 * If your LED flashes too quickly or too slowly, adjusting this value will change the delay until the next cycle.
 */
const int LOOP_DELAY = 1;





 

/*
 * APPLICATION CODE
 * 
 * No need to make any changes below here
 */

#include <movingAvg.h>
#include <Q2HX711.h>

Q2HX711 load_sensor(LOAD_SENSOR_DATA, LOAD_SENSOR_CLOCK);

movingAvg smooth_weight(MOVING_AVERAGE_BUCKET_SIZE);

int low_safe_level;
int high_safe_level;
  
//state used only for controlling light pulse cycle
int pulse_brightness = 0;    // how bright the LED is during current cycle
int fadeAmount = 1;    // how many points to fade the LED by

//state used to determine when to read the weight sensor
int sensor_cycle = 0; // Current state of the cycle for sensor reading

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
  int full_brightness = 255;
  
  setColours(red, green, blue, full_brightness);
}

void rgbPulse(boolean red, boolean green, boolean blue) {

  setColours(red, green, blue, pulse_brightness);
  
  // change the brightness for next time through the loop:
  pulse_brightness = pulse_brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (pulse_brightness <= 0 || pulse_brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
}

void loop() {

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

  delay(LOOP_DELAY);

}
