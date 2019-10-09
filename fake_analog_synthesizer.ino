// Auduino, the Lo-Fi granular synthesiser
//
// by Peter Knight, Tinker.it http://tinker.it
//
// Help:      http://code.google.com/p/tinkerit/wiki/Auduino
// More help: http://groups.google.com/group/auduino
//
// Analog in 0: Grain 1 pitch
// Analog in 1: Grain 2 decay
// Analog in 2: Grain 1 decay
// Analog in 3: Grain 2 pitch
// Analog in 4: Grain repetition frequency
//
// Digital 3: Audio out (Digital 11 on ATmega8)
//
/*
  Calibration
 
 Demonstrates one techinque for calibrating sensor input.  The
 sensor readings during the first five seconds of the sketch
 execution define the minimum and maximum of expected values
 attached to the sensor pin.
 
 The sensor minumum and maximum initial values may seem backwards.
 Initially, you set the minimum high and listen for anything 
 lower, saving it as the new minumum. Likewise, you set the
 maximum low and listen for anything higher as the new maximum.
 
 The circuit:
 * Analog sensor (potentiometer will do) attached to analog input 0
 * LED attached from digital pin 9 to ground
 
 created 29 Oct 2008
 By David A Mellis
 Modified 17 Jun 2009
 By Tom Igoe
 
 http://arduino.cc/en/Tutorial/Calibration
 
 This example code is in the public domain.
 
 */

// AUDUINO code STARTS
#include <avr/io.h>
#include <avr/interrupt.h>

uint16_t syncPhaseAcc;
uint16_t syncPhaseInc;
uint16_t grainPhaseAcc;
uint16_t grainPhaseInc;
uint16_t grainAmp;
uint8_t grainDecay;
uint16_t grain2PhaseAcc;
uint16_t grain2PhaseInc;
uint16_t grain2Amp;
uint8_t grain2Decay;

// Map Analogue channels
#define SYNC_CONTROL         (4)
#define GRAIN_FREQ_CONTROL   (3)
#define GRAIN_DECAY_CONTROL  (2)
#define GRAIN2_FREQ_CONTROL  (1)
#define GRAIN2_DECAY_CONTROL (0)

// ATMEGA 328
//    Output is on pin 3
//
#define PWM_PIN       3
#define PWM_VALUE     OCR2B
#define LED_PIN       13
#define LED_PORT      PORTB
#define LED_BIT       5
#define PWM_INTERRUPT TIMER2_OVF_vect
#endif
// AUDUINO code ENDS


// SWITCH, LDR & LEDs - START
// LDR
//#define LDRSWITCH (4)
// switch replaced by external interrupt
volatile int LDRswitchState = LOW;
#define LDR_PIN  (5)
#define LDRLED_PIN  (9)
//Callibration variables
int LDRValue = 0;   // the sensor value
int LDRMin = 1023;  // minimum sensor value
int LDRMax = 0;     // maximum sensor value

// PWM_VALUE LED
#define FRQLED_PIN (11) // not as consistent as pin 13      

// mapmode LED
#define mapModeLED_PIN (10)    // the number of the LED pin  
int mapModeLEDState = LOW;     // ledState used to set the LED
long previousMillis = 0;       // will store last time LED was updated
int BlinkRate = 5;            // no of blinks per second i.e. fps - empirically tested as just slow enough to count
int BlinkCount = 0;           // variable to store no of blinks
int BlinkLoopLength = 14;    // err... blink loop length

// SWITCH, LDR & LEDs - ENDS

void audioOn() {
  // Set up PWM to 31.25kHz, phase accurate
  TCCR2A = _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = _BV(TOIE2);
#endif
}


void setup() {
  
  pinMode(PWM_PIN,OUTPUT);
  audioOn();
  pinMode(LDRLED_PIN,OUTPUT);
  pinMode(FRQLED_PIN,OUTPUT);   
  pinMode(mapModeLED_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT);
  attachInterrupt(0, LDRswitched, CHANGE);

  // Calibration
  //Serial.begin(9600);
  // turn on LED to signal the start of the calibration period:
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  // calibrate during the first five seconds 
  while (millis() < 5000) {
    LDRValue = analogRead(LDR_PIN);
    // record the maximum sensor value
    if (LDRValue > LDRMax) {
      LDRMax = LDRValue;
    }
    // record the minimum sensor value
    if (LDRValue < LDRMin) {
      LDRMin = LDRValue;
    }
  }
  // signal the end of the calibration period
  digitalWrite(9, LOW);
  // END calibration code

}


void LDRswitched()
{
  LDRswitchState = !LDRswitchState;
}


void loop() {
  // The loop is pretty simple - it just updates the parameters for the oscillators.
  //BEGIN Calibration
  // read the sensor:
  LDRValue = analogRead(LDR_PIN);
  // apply the calibration to the sensor reading
  LDRValue = map(LDRValue, LDRMin, LDRMax, 0, 1023);
  // in case the sensor value is outside the range seen during calibration
  LDRValue = constrain(LDRValue, 0, 1023);
  //    // fade the LED using the calibrated value: (this moved below)
  //    analogWrite(LDRLED_PIN, LDRValue); /*  TOO SLOW  */
  //    Serial.println(LDRValue);
  //END Calibration
  
  //input from pots
  grainPhaseInc  = mapPhaseInc(analogRead(GRAIN_FREQ_CONTROL)) / 2;
  grainDecay     = analogRead(GRAIN_DECAY_CONTROL) / 8;
  grain2PhaseInc = mapPhaseInc(analogRead(GRAIN2_FREQ_CONTROL)) / 2;
  grain2Decay    = analogRead(GRAIN2_DECAY_CONTROL) / 4;

  digitalWrite(FRQLED_PIN, PWM_VALUE);

}


SIGNAL(PWM_INTERRUPT)
{
  uint8_t value;
  uint16_t output;

  syncPhaseAcc += syncPhaseInc;
  if (syncPhaseAcc < syncPhaseInc) {
    // Time to start the next grain
    grainPhaseAcc = 0;
    grainAmp = 0x7fff;
    grain2PhaseAcc = 0;
    grain2Amp = 0x7fff;
    LED_PORT ^= 1 << LED_BIT; // Faster than using digitalWrite
  }

  // Increment the phase of the grain oscillators
  grainPhaseAcc += grainPhaseInc;
  grain2PhaseAcc += grain2PhaseInc;

  // Convert phase into a triangle wave
  value = (grainPhaseAcc >> 7) & 0xff;
  if (grainPhaseAcc & 0x8000) value = ~value;
  // Multiply by current grain amplitude to get sample
  output = value * (grainAmp >> 8);

  // Repeat for second grain
  value = (grain2PhaseAcc >> 7) & 0xff;
  if (grain2PhaseAcc & 0x8000) value = ~value;
  output += value * (grain2Amp >> 8);

  // Make the grain amplitudes decay by a factor every sample (exponential decay)
  grainAmp -= (grainAmp >> 8) * grainDecay;
  grain2Amp -= (grain2Amp >> 8) * grain2Decay;

  // Scale output to the available range, clipping if necessary
  output >>= 9;
  if (output > 255) output = 255;

  // Output to PWM (this is faster than using analogWrite)  
  PWM_VALUE = output;
}
