#define USE_ARDUINO_INTERRUPTS true
#define LENGTH 8 //Sequence length
#define GROUP 2  //Grouping size
#define MIN 0    //Minimum IPI value
#define MAX 10000 //Maximum IPI value
#define STEP 4 //Step size for quantisation
#define BITLENGTH 2 //Bit length of a quantised and binarised sequence
#include <PulseSensorPlayground.h>
//  Variables
int Signal;                     // holds the incoming raw data. Signal value can range from 0-1024
const int PULSE_INPUT0 = A0;    //Input pin for the first sensor
const int PULSE_INPUT1 = A1;    //Input pin for the second sensor
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int THRESHOLD = 520;   // Determine which Signal to "count as a beat", and which to ingore.
const int OUTPUT_TYPE = SERIAL_PLOTTER;
PulseSensorPlayground pulseSensor(2);

int bpm0 = 0; //BPM of first sensor
int ibi0 = 0; //IPI of first sensor
int sequence0[LENGTH]; //First IPI sequence
int encryptedSequence0[LENGTH/GROUP]; //First encrypted IPI sequence
int step0 = 0;
bool match0 = false; //First sequence ready

int bpm1 = 0; //BPM of second sensor
int ibi1 = 0; //IPI of second sensor
int sequence1[LENGTH]; //Second IPI sequence
int encryptedSequence1[LENGTH/GROUP]; //Second encrypted IPI sequence
int step1 = 0;
bool match1 = false; //Second sequence ready

void encrypt(String sensor, int sequence[LENGTH], int (&encryptedSequence)[LENGTH/GROUP]);

void setup() 
{
  Serial.begin(115200);

  //Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT0, 0);
  pulseSensor.blinkOnPulse(PULSE_BLINK, 0);
  pulseSensor.analogInput(PULSE_INPUT1, 1);
  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  //Now that everything is ready, start reading the PulseSensor signal.
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
  if (pulseSensor.sawStartOfBeat(0)) 
  {
    pulseSensor.outputBeat(0);
    bpm0 = pulseSensor.getBeatsPerMinute(0);
    ibi0 = pulseSensor.getInterBeatIntervalMs(0);

    //Once enough IPI received, encrypt the sequence
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

    //Once enough IPI received, encrypt the sequence
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

  //When both IPI sequences are ready
  if (match0 && match1)
  {
    int distance = 0;
    for (int i = 0; i < LENGTH/GROUP; i++)
    {
      distance += hammingDistance(encryptedSequence0[i], encryptedSequence1[i]);
    }

    //CSV printing
    for (int i = 0; i < LENGTH/GROUP; i++)
    {
      Serial.print(encryptedSequence0[i], BIN);

    }
    Serial.print(",");
    for (int i = 0; i < LENGTH/GROUP; i++)
    {
      Serial.print(encryptedSequence1[i], BIN);
    }
    Serial.print(",");
    Serial.println(distance);
    match0 = false;
    match1 = false;
  }
}

//Encryption
void encrypt(String sensor, int sequence[LENGTH], int (&encryptedSequence)[LENGTH/GROUP])
{
  //Group the IPI sequence
  int groupedSequence[LENGTH/GROUP];
  int j = 0;
  for (int i = 0; i < LENGTH; i += 2)
  {
    groupedSequence[j] = sequence[i] + sequence[i + 1];
    j++;
  }
  
  //Quantize the IPI sequence
  //Serial.println("Quantized");
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
  }
  
  //Gray code the IPI sequence
  int grayEncodedSequence[LENGTH/GROUP];
  for (int i = 0; i < LENGTH/GROUP; i++)
  {
    grayEncodedSequence[i] = (quantizedSequence[i]>>1) ^ quantizedSequence[i];
    encryptedSequence[i] = grayEncodedSequence[i];
  }
}

//Hamming distance calculation
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
