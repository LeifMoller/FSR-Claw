#include "arduino_secrets.h"

//Leif Moller
//2026.07.17


#define FSR_PIN 34

int fsrReading = 0;
int bufferIndex = 0;
double currentVal = 0;

int fsrBuffer[5] = {0};

void setup() {
  Serial.begin(115200);
  delay(2000);
}

//Make an avg value of the last 5 readings to filter outliers
float movingAvg(int fsrReading){
  
  int sum = 0;

  fsrBuffer[bufferIndex] = fsrReading;
  bufferIndex = (bufferIndex + 1) % 5;

  for(int i = 0; i < 5; i++){
    sum += fsrBuffer[i];
  }

  return (float)sum / 5;

}

void loop() {

  fsrReading = analogRead(FSR_PIN);

  Serial.print("FSR Reading: ");
  Serial.println(fsrReading);

  currentVal = movingAvg(fsrReading);

  


  Serial.println(currentVal);

  delay(100);
}
