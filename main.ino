//----HEADER DEFINES----//
#include <CapacitiveSensor.h>
#include "pitches.h"
#include <Stepper.h>

//----PIN DEFINES----//

#define COMMON_PIN      2    // The common 'send' pin for all keys
#define STROBE_PIN      8   // The LED pin
#define BUZZER_PIN      A4   // The output pin for the piezo buzzer
#define FREQUENCY_PIN   12 // The frequency pin
#define MAG_PIN         11
#define TILT_PIN        35
#define NUM_OF_SAMPLES  10   // Higher number whens more delay but more consistent readings
#define CAP_THRESHOLD   150  // Capactive reading that triggers a note (adjust to fit your needs)
#define NUM_OF_KEYS     4    // Number of keys that are on the keyboard

//----VARIABLE DEFINES----//

int secretSequence [5] = {3, 1, 2, 0, 3};
int sequence [5] = {0, 0, 0, 0, 0};
int sequenceIndex = 0;
int ontime, offtime, duty;
float frequency;
float NORMAL_FREQ = 8700;
const int stepsPerRevolution = 2048;
int notes[]={NOTE_C4,NOTE_D4,NOTE_E4,NOTE_F4}; // C-Major scale
bool tiltState = false;

unsigned long debounceDelay = 50; 

//----OBJECT DEFINES----//

#define CS(Y) CapacitiveSensor(2, Y) // This macro creates a capacitance "key" sensor object for each key on the piano keyboard
CapacitiveSensor keys[] = {CS(3), CS(4), CS(5), CS(6)}; // Defines the pins that the keys are connected to

Stepper myStepper(stepsPerRevolution, 53, 49, 51, 47);

//----SETUP----//

void setup() { 
  for(int i=0; i<4; ++i) {
    keys[i].set_CS_AutocaL_Millis(0xFFFFFFFF);
  }
  pinMode(FREQUENCY_PIN, INPUT);
  pinMode(TILT_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT); 
  pinMode(STROBE_PIN, OUTPUT);
  pinMode(MAG_PIN, OUTPUT);
  digitalWrite(TILT_PIN, HIGH);
  digitalWrite(MAG_PIN, HIGH);
  digitalWrite(STROBE_PIN, LOW);
  myStepper.setSpeed(5);
  Serial.begin(9600);  
  }


//---MAIN LOOP---//

void loop() {    

  for (int i = 0; i < 4; ++i) {
    // If the capacitance reading is greater than the threshold, play a note:
    if(keys[i].capacitiveSensor(NUM_OF_SAMPLES) > CAP_THRESHOLD) {
      tone(BUZZER_PIN, notes[i]); // Plays the note corresponding to the key pressed
      sequence[sequenceIndex] = i;
      delay(500);
      noTone(BUZZER_PIN);
      sequenceIndex += 1;
    }
  }
  
  if (sequenceIndex == 5) {
    sequenceIndex = 0;
    bool equal = true;
    for (int i = 0; i < 5; i++){
      if (secretSequence[i] != sequence[i]){
        equal = false;
      }  
    } 

    if (equal) {
      digitalWrite(STROBE_PIN, HIGH);
      delay(2000);
      digitalWrite(STROBE_PIN, LOW);
      checkFrequency();
      digitalWrite(MAG_PIN, LOW);
      checkMotion();
      doSteps();
    }
  }
}


//----INDUCTOR FREQUENCY CHECK----//

void checkFrequency(){

  bool readFrequency = true;
  while (readFrequency){
    ontime = pulseIn(FREQUENCY_PIN, HIGH);
    offtime = pulseIn(FREQUENCY_PIN, LOW);
    frequency = 1000000 / (ontime + offtime);
    if (frequency < NORMAL_FREQ){
      tone(BUZZER_PIN, 1000);
      delay(500);
      noTone(BUZZER_PIN);
      readFrequency = false;
    }
  }
}

//----MOTION CHECK----//

void checkMotion(){
  bool readMotion = true;
  while (readMotion){

    if (digitalRead(TILT_PIN)){
      delay(debounceDelay);
      if (digitalRead(TILT_PIN)){
        tiltState = true;
      }
    }
    if (tiltState){
      tone(BUZZER_PIN, 500);
      delay(500);
      tone(BUZZER_PIN, 1500);
      delay(500);
      tone(BUZZER_PIN, 2500);
      delay(500);
      noTone(BUZZER_PIN);
      readMotion = false;
    }

  }
}

//----TURN MOTOR----//

void doSteps(){
  myStepper.step(-stepsPerRevolution / 4);
  delay(1000);
  myStepper.step(stepsPerRevolution / 4);
}