#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

//  Variables
int Signal;                     // holds the incoming raw data. Signal value can range from 0-1024
const int PULSE_INPUT = A0;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int THRESHOLD = 515;   // Determine which Signal to "count as a beat", and which to ingore.
int LED13 = 13;                 //  The on-board Arduino LED
const int OUTPUT_TYPE = SERIAL_PLOTTER;
PulseSensorPlayground pulseSensor;


boolean peakDetected = false;
int bpm = 0;
int ibi = 0;

void setup() 
{
  Serial.begin(115200);

  // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) 
  {
    for(;;) 
    {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
}

// Main loop.
void loop() 
{
  delay(20);
  //pulseSensor.outputSample();
  if (pulseSensor.sawStartOfBeat()) 
  {
    //pulseSensor.outputBeat();
    bpm = pulseSensor.getBeatsPerMinute();
    ibi = pulseSensor.getInterBeatIntervalMs();
    Serial.println(ibi);
  }
}

// Peak detection function. Reads the signal coming from the sensor and returns true if there's a heartbeat.
boolean detectPeak()
{
  return false;
}
