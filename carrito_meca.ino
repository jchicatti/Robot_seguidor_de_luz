#include <util/atomic.h> // For the ATOMIC_BLOCK macro
#include <NewPing.h>
#include <SharpDistSensor.h>

NewPing UltDer(14,15,60);
NewPing UltIzq(16,17,60);

SharpDistSensor sensor(A2,5);

int DisDer, DisIzq;
float distancia;

// Variables para encoders y motores
#define ENCA 2 // YELLOW
#define ENCB 3 // WHITE
#define ENCA2 7 // YELLOW
#define ENCB2 6 // WHITE
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
volatile int posi = 0; // specify posi as volatile: https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/volatile/
long prevT = 0;
float eprev = 0;
float eintegral = 0;
bool go;
// Variables para foto resistencias
#define FRIzq 1
#define FRDer 0
int ValIlumIzq = 0;
int ValIlumDer = 0; 
int BiasLuz =  26;
int difIlum = 0;
// Relación de velocidades es 216 para el derecho y 255 para el izquierdo


void setup() {
  Serial.begin(9600);
  sensor.setModel(SharpDistSensor::GP2Y0A41SK0F_5V_DS);

  // Ftoresistencias
  pinMode(FRIzq, INPUT);
  pinMode(FRDer, INPUT);

  // Rueda izquierda
  pinMode(ENCA,INPUT);
  pinMode(ENCB,INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA),readEncoder,RISING);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);

  // Rueda derecha
  pinMode(ENCA2,INPUT);
  pinMode(ENCB2,INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA2),readEncoder2,RISING);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  
}

void loop() {
  go = true;
  // Velocidades normales para ir derecho, ie, 100%
  int velDer = 216*1/2;
  int velIzq = 255*1/2;

  pared();
  
  ValIlumIzq = analogRead(FRIzq);
  ValIlumDer = analogRead(FRDer)+BiasLuz;

  difIlum = ValIlumIzq - ValIlumDer;
  
  if (difIlum > 0) {
    velDer = velDer + abs(difIlum)*2*0.84;  // 0.84 sale de la realción de 216/255. Un paso de 255 equivale a .84 de 216
    if (velDer > 216) {
      velDer = 216;
    }
  } else if (difIlum <0) {
    velIzq = velIzq + abs(difIlum)*2;
    if (velIzq > 255) {
      velIzq = 255;
    }
  }

  if (ValIlumIzq > 800 || ValIlumDer > 800){
    go = false;
  }

  distancia = getDistance();
  DisDer = UltDer.ping_cm();
  DisIzq = UltIzq.ping_cm();
  Serial.print("DisDer: ");
  Serial.print(DisDer);
  Serial.print("\tDisIzq: ");
  Serial.print(DisIzq);
  Serial.print("\tDisCen: ");
  Serial.println(distancia);
  delay(500);
  

//  Serial.print("Dif: ");
//  Serial.print(difIlum);
//  Serial.print("\tIzq: ");
//  Serial.print(ValIlumIzq);
//  Serial.print("\tVelIzq: ");
//  Serial.print(velIzq);
//  Serial.print("\tDer: ");
//  Serial.print(ValIlumDer);
//  Serial.print("\t VelDer:");
//  Serial.println(velDer);
  
  // signal the motor
  if (go == true){
    setMotor(1,velIzq,IN1,IN2);
    setMotor(1,velDer,IN3,IN4);
  } else {
    setMotor(1,0,IN1,IN2);
    setMotor(1,0,IN3,IN4);
  }
  
  
}

void setMotor(int dir, int pwr, int in1, int in2){
  if(dir == -1){
    analogWrite(in1, pwr);
    digitalWrite(in2,LOW);
  }
  else if(dir == 1){
    digitalWrite(in1,LOW);
    analogWrite(in2, pwr);
  }
  else{
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);
  }  
}

void pared() {
  distancia = getDistance();
  DisDer = UltDer.ping_cm();
  DisIzq = UltIzq.ping_cm();

  if(distancia < 15) {
    if(distancia > DisDer) {
    //giro izquierda
      setMotor(-1,255*1/2,IN1,IN2);
      setMotor(1,216*1/2,IN3,IN4);
      delay(250);
      setMotor(-1,0,IN1,IN2);
      setMotor(1,0,IN3,IN4);
    }
    else if (distancia > DisIzq) {
      //giro derecha
      setMotor(1,255*1/2,IN1,IN2);
      setMotor(-1,216*1/2,IN3,IN4);
      delay(250);
      setMotor(-1,0,IN1,IN2);
      setMotor(1,0,IN3,IN4);
    }
  }

}

float getDistance() {
  int minLimit = 80;
  int maxLimit = 530;

  float DisCen = 0;
  float data = analogRead(A2);

  if (data >= minLimit && data <= maxLimit) {
    DisCen = 2076 / (data - 11);
  }
  else {
    if(data < minLimit) {
      DisCen = 30;
    }
    else if (data > maxLimit) {
      DisCen = 4;
    }
  }

  return DisCen;
}

void readEncoder(){
  int b = digitalRead(ENCB);
  if(b > 0){
    posi++;
  }
  else{
    posi--;
  }
}

void readEncoder2(){
  int b = digitalRead(ENCB2);
  if(b > 0){
    posi++;
  }
  else{
    posi--;
  }
}
