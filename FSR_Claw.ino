#include "arduino_secrets.h"

//Leif Moller
//2026.07.17


#define FSR_PIN 34
static int Ro = 10; // 10 kΩ
static float Vcc = 3.3;

int fsrReading = 0;
int bufferIndex = 0;
double currentVal = 0;

float Rs, Vout;

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





//Data sheet gives force as a function of FSR resistances. Convert
float force(float currentVal){
  
  //Turn analog signal (0-4095) back to voltage (0-3.3)
  Vout = currentVal / 4095 * 3.3;

  if(Vout == 0) return 0;

  //Vout = Vcc * Ro / (Ro+Rs)
  Rs = ((Vcc * Ro) / Vout) - Ro;

  //F = (Rs / 271)^(-1/0.69)
  float F = pow((Rs/271),(-1.0/0.69));

  Serial.print("Vout:");
  Serial.println(Vout);
  Serial.print("Rs: ");
  Serial.println(Rs);


  return F;
}





void loop() {

  fsrReading = analogRead(FSR_PIN);

  Serial.print("FSR Reading: ");
  Serial.println(fsrReading);

  currentVal = movingAvg(fsrReading);

  float F = force(currentVal);

  Serial.print("Val: ");
  Serial.println(currentVal);

  Serial.print("Force: ");
  Serial.println(F);
  Serial.println();

  delay(1000);
}
