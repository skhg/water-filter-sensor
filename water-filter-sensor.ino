#define blue_led 11           // the PWM pin the LED is attached to
#define red_led 10
#define green_led 9

#define test A5

// A6 not working
// A7 not working


int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by


// the setup routine runs once when you press reset:
void setup() {
  // declare pins to be an output:
  pinMode(blue_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);


  pinMode(test, OUTPUT);
}



// the loop routine runs over and over again forever:
void loop() {

  double masterScale = 0.5;
  
  double blueScale = 0.2;
  double redScale = 1;
  double greenScale = 0.25;

  analogWrite(test, 255);
  // set the brightness of pin 9:
  analogWrite(blue_led, brightness * masterScale * blueScale);
  analogWrite(red_led, brightness * masterScale * redScale);
  analogWrite(green_led, brightness * masterScale * greenScale);

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(30);
}
