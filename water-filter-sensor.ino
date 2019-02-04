//User-configurable settings

// The maximum weight of the system. Above this level, water will overflow.
const int MAXIMUM_WATER_LEVEL = 800;

// The weight of the system when tap no longer flows out.
const int MINIMUM_WATER_LEVEL = 550;

// The fill level (0.0 to 1.0) below which we should alert to refill
const double LOW_SAFE_WATER_FRACTION = 0.3;

// The highest fill level (0.0 to 1.0) we aim to reach when we refill
const double HIGH_SAFE_WATER_FRACTION = 0.8;

// A calibration value that differs for every sensor. Whatever the sensor reads when no weight is applied
//Whatever is displayed in serial monitor when the plate is empty must be ADDED to this number
const int TARE_VALUE = 8486;

// Enable for calibration
const boolean DEBUG_ON = false;


// No need to make any changes below here

#include <movingAvg.h>
#include <Q2HX711.h>

// The following must be connected to PWM pins in order to vary brightness
#define blue_led 9
#define red_led 11
#define green_led 10
// Note to self: On my Arduino board, A6 and A7 are not working. Do not use

#define load_data_pin A0
#define load_clock_pin A1

Q2HX711 load_sensor(load_data_pin, load_clock_pin);

movingAvg smooth_weight(20);

int low_safe_level;
int high_safe_level;

//scaling factors for the relative brightness of the RGB LED's
double masterScale = 1;
double blueScale = 0.2;
double redScale = 1;
double greenScale = 0.25;
  
//state used only for controlling light pulse cycle
int pulse_brightness = 0;    // how bright the LED is during current cycle
int fadeAmount = 1;    // how many points to fade the LED by

//state used to determine when to read the weight sensor
int sensor_cycle = 0; // Current state of the cycle for sensor reading
int sensor_interval = 100; // Read the sensor every n'th cycle (deal with a sensor that reads slowly)

// the setup routine runs once when you press reset:
void setup() {

  int range = MAXIMUM_WATER_LEVEL - MINIMUM_WATER_LEVEL;
  low_safe_level = (range * LOW_SAFE_WATER_FRACTION) + MINIMUM_WATER_LEVEL;
  high_safe_level = (range * HIGH_SAFE_WATER_FRACTION) + MINIMUM_WATER_LEVEL;
  
  // LED pins are all outputs
  pinMode(blue_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);

  Serial.begin(9600);

  smooth_weight.begin();
}

int readWeight() {

  if(sensor_cycle == sensor_interval){
    if(sensor_cycle >= sensor_interval){
      sensor_cycle = 0;
    }

    int curr_weight = (load_sensor.read() / 1000) - TARE_VALUE;
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
    analogWrite(blue_led, brightness * masterScale * blueScale);
  }else{
    analogWrite(blue_led, 0);
  }
  if(red) {
    analogWrite(red_led, brightness * masterScale * redScale);
  }else{
    analogWrite(red_led, 0);
  }
  if(green) {
    analogWrite(green_led, brightness * masterScale * greenScale);
  }else{
    analogWrite(green_led, 0);
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

// the loop routine runs over and over again forever:
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

  delay(1);

}
