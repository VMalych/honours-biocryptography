#define USE_ARDUINO_INTERRUPTS true
#define LENGTH 6
#define GROUP 2
#define MIN 0
#define MAX 2048
#define STEP 4
#define BITLENGTH 2
#include <PulseSensorPlayground.h>
//  Variables
int Signal;                     // holds the incoming raw data. Signal value can range from 0-1024
const int PULSE_INPUT1 = A0;
const int PULSE_INPUT2 = A1;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int THRESHOLD = 515;   // Determine which Signal to "count as a beat", and which to ingore.
int LED13 = 13;                 //  The on-board Arduino LED
const int OUTPUT_TYPE = SERIAL_PLOTTER;
PulseSensorPlayground pulseSensor(2);

int bpm = 0;
int ibi = 0;
int sequence[LENGTH];
int step = 0;

void setup() 
{
  Serial.begin(115200);

  // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT1, 0);
  pulseSensor.blinkOnPulse(PULSE_BLINK, 0);
  pulseSensor.analogInput(PULSE_INPUT2, 1);
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
  pulseSensor.outputSample();
  if (pulseSensor.sawStartOfBeat()) 
  {
    pulseSensor.outputBeat();
    bpm = pulseSensor.getBeatsPerMinute();
    ibi = pulseSensor.getInterBeatIntervalMs();
    
    if (step != LENGTH)
    {
      sequence[step] = ibi;
      step++;
    }
    else
    {
      step = 0;
      encrypt();
    }
  }
}

void encrypt()
{
  //Print the IPI sequence
  Serial.println("IPI Sequence");
  for (int i = 0; i < LENGTH; i++)
  {
    Serial.print(sequence[i]);
    Serial.print(" ");
  }
  Serial.println();

  //Group the IPI sequence
  int groupedSequence[LENGTH/GROUP];
  int j = 0;
  Serial.println("Grouped");
  for (int i = 0; i < LENGTH; i += 2)
  {
    groupedSequence[j] = sequence[i] + sequence[i + 1];
    Serial.print(groupedSequence[j]);
    Serial.print(" ");
    j++;
  }
  Serial.println();
  
  //Quantize the IPI sequence
  Serial.println("Quantized");
  int quantizedSequence[LENGTH/GROUP];
  for (int i = 0; i < LENGTH/GROUP; i++)
  {
    for (int j = 0; j < (MAX-MIN)/STEP; j++)
    {
      if (groupedSequence[i] < MIN + (j + 1) * STEP)
      {
        quantizedSequence[i] = j % (int)pow(2, BITLENGTH);
        break;
      }
    }
    Serial.print(quantizedSequence[i]);
    Serial.print(" ");
  }
  Serial.println();
  
  //Gray code the IPI sequence
  Serial.println("Binary");
  for (int i = 0; i < LENGTH/GROUP; i++)
  {
    Serial.print(quantizedSequence[i], BIN);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("Gray encoded binary");
  int grayEncodedSequence[LENGTH/GROUP];
  for (int i = 0; i < LENGTH/GROUP; i++)
  {
    grayEncodedSequence[i] = (quantizedSequence[i]>>1) ^ quantizedSequence[i];
    Serial.print(grayEncodedSequence[i], BIN);
    Serial.print(" ");
  }
  Serial.println();
}
