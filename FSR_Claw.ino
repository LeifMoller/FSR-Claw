#include "arduino_secrets.h"

//Leif Moller
//2026.07.17

//Circuit dfns
#define FSR_PIN 34
#define IN1 25  // close
#define IN2 26   // open
static int Ro = 10; // 10 kΩ
static float Vcc = 3.3; // 3.3V

int fsrReading = 0;
int bufferIndex = 0;
double currentVal = 0;

float Rs, Vout;

int fsrBuffer[5] = {0};


// PI stuff
// Kp and Ki arbitrary knobs. Adjust
float Kp = 0.05;
float Ki = 0.10;

float target = 0; 
float integral = 0;
const float PWM_MAX = 255;
unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

  //For PWM
  ledcAttach(IN1, 20000, 8);  // Arbitrary 20 kHz, 8-bit (for 256 step) PWM
  ledcAttach(IN2, 20000, 8);  

  lastTime = millis();        
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


// weighted error of P + I, with result btw/ 0 and 255
float piUpdate(float error, float dt){

  integral += error * dt;
  integral = constrain(integral, -PWM_MAX / Ki, PWM_MAX / Ki);  // anti-windup

  float u = Kp * error + Ki * integral;
  return constrain(u, -PWM_MAX, PWM_MAX);
}


// u > 0, closing. else, opening
void drive(float u){

  int duty = (int)fabs(u);

  if(u > 0){
    ledcWrite(IN1, duty);
    ledcWrite(IN2, 0);
  } else {
    ledcWrite(IN1, 0);
    ledcWrite(IN2, duty);
  }
}



void loop() {

  fsrReading = analogRead(FSR_PIN); 

  Serial.print("FSR Reading: ");
  Serial.println(fsrReading);

  currentVal = movingAvg(fsrReading);

  float F = force(currentVal);

  if(Serial.available()) target = Serial.readStringUntil('\n').toFloat();   // Read target

  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;

  if(!isfinite(F)) F = 10000; // if bad reading, set force high (but not infinite)

  if(target <= 0) integral = 0;           // if idle, reset integral so no carry through
  float u = piUpdate(target - F, dt);   // get error (P+I)
  drive(target > 0 ? u : 0);  // drive

  Serial.print("Val: ");
  Serial.println(currentVal);

  Serial.print("Force: ");
  Serial.println(F);
  Serial.println();

  delay(1000);
}