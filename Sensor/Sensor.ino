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
const int PULSE_INPUT0 = A0;
const int PULSE_INPUT1 = A1;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int THRESHOLD = 520;   // Determine which Signal to "count as a beat", and which to ingore.
int LED13 = 13;                 //  The on-board Arduino LED
const int OUTPUT_TYPE = SERIAL_PLOTTER;
PulseSensorPlayground pulseSensor(2);

int bpm0 = 0;
int ibi0 = 0;
int sequence0[LENGTH];
int encryptedSequence0[LENGTH/GROUP];
int step0 = 0;
bool match0 = false;

int bpm1 = 0;
int ibi1 = 0;
int sequence1[LENGTH];
int encryptedSequence1[LENGTH/GROUP];
int step1 = 0;
bool match1 = false;

void encrypt(String sensor, int sequence[LENGTH], int (&encryptedSequence)[LENGTH/GROUP]);

void setup() 
{
  Serial.begin(115200);

  // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT0, 0);
  pulseSensor.blinkOnPulse(PULSE_BLINK, 0);
  pulseSensor.analogInput(PULSE_INPUT1, 1);
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
  if (pulseSensor.sawStartOfBeat(0)) 
  {
    pulseSensor.outputBeat(0);
    bpm0 = pulseSensor.getBeatsPerMinute(0);
    ibi0 = pulseSensor.getInterBeatIntervalMs(0);
    
    if (step0 != LENGTH)
    {
      sequence0[step0] = ibi0;
      step0++;
    }
    else
    {
      step0 = 0;
      encrypt("0", sequence0, encryptedSequence0);
      match0 = true;
    }
  }
  if (pulseSensor.sawStartOfBeat(1)) 
  {
    pulseSensor.outputBeat(1);
    bpm1 = pulseSensor.getBeatsPerMinute(1);
    ibi1 = pulseSensor.getInterBeatIntervalMs(1);
    
    if (step1 != LENGTH)
    {
      sequence1[step1] = ibi1;
      step1++;
    }
    else
    {
      step1 = 0;
      encrypt("1", sequence1, encryptedSequence1);
      match1 = true;
    }
  }
  if (match0 && match1)
  {
    int distance = 0;
    for (int i = 0; i < LENGTH/GROUP; i++)
    {
      Serial.print(encryptedSequence0[i]);
      Serial.print("-");
      Serial.print(encryptedSequence1[i]);
      Serial.print(" ");
      Serial.println(hammingDistance(encryptedSequence0[i], encryptedSequence1[i])); 
      distance += hammingDistance(encryptedSequence0[i], encryptedSequence1[i]);
    }
    Serial.println();
    Serial.print("Hamming distance: ");
    Serial.println(distance);
    match0 = false;
    match1 = false;
  }
}

void encrypt(String sensor, int sequence[LENGTH], int (&encryptedSequence)[LENGTH/GROUP])
{
  Serial.println("Sensor " + sensor);
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
    encryptedSequence[i] = grayEncodedSequence[i];
    Serial.print(grayEncodedSequence[i], BIN);
    Serial.print(" ");
  }
  Serial.println();
}

int hammingDistance(int n1, int n2) 
{ 
    int x = n1 ^ n2; 
    int setBits = 0; 
  
    while (x > 0) { 
        setBits += x & 1; 
        x >>= 1; 
    } 
  
    return setBits; 
}
