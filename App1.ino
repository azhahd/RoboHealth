#include <Wire.h>

const int m1p1 = 2, m1p2 = 3, m2p1 = 4, m2p2 = 5, m1pwm = 9, m2pwm = 10;

int echoPin = 12;
int trigPin = 11;

const int MPU_ = 0x68; // MPU_6050 I2C address
float AccX, AccY, AccZ;

void setup() {
  // put your setup code here, to run once:
  pinMode(m1p1, OUTPUT);
  pinMode(m1p2, OUTPUT);
  pinMode(m2p1, OUTPUT);
  pinMode(m2p2, OUTPUT);

  pinMode(m1pwm, OUTPUT);
  pinMode(m2pwm, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(9600);

  Wire.begin();
  Wire.beginTransmission(MPU_);
  Wire.write(0x6B);
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission
  Wire.beginTransmission(MPU_);
  Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(0x08);                  //Set the register bits as 00010000 (+/- 8g full scale range)
  Wire.endTransmission(true);
  //calculate_IMU_error();
  delay(20);

  
}

void loop() {
  // put your main code here, to run repeatedly:
    long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2)/29.1;
  Serial.println(distance);
  
  if(AccX < 0.2 && AccY < 0.2){
    analogWrite(m1pwm, 255);
    analogWrite(m2pwm, 255);

    digitalWrite(m1p1, HIGH);
    digitalWrite(m1p2, LOW);
    digitalWrite(m2p1, LOW);
    digitalWrite(m2p2, HIGH);
  }
  else if(distance < 75){
    analogWrite(m1pwm, 0);
    analogWrite(m2pwm, 255);

    digitalWrite(m1p1, LOW);
    digitalWrite(m1p2, HIGH);
    digitalWrite(m2p1, LOW);
    digitalWrite(m2p2, LOW);
    delay(500);
    
  }
  
  else{
    analogWrite(m1pwm, 255);
    analogWrite(m2pwm, 255);

    digitalWrite(m1p1, LOW);
    digitalWrite(m1p2, HIGH);
    digitalWrite(m2p1, HIGH);
    digitalWrite(m2p2, LOW);
    delay(500); 
  }

  // === Read acceleromter data === //
  Wire.beginTransmission(MPU_);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  //For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value

  // Print the values on the serial monitor

  AccX = AccX*2;
  AccY = AccY*2;
  AccZ = AccZ*2;

  AccX = AccX - 1.02771;
  AccY = AccY + 0.023;
  AccZ = AccZ - 0.07333;

  AccX = AccX*9.81;
  AccY = AccY*9.81;
  AccZ = AccZ*9.81;
  Serial.write(AccX);
  Serial.write(AccY);
  if(fabs(AccX) < 0.1) { AccX = 0.0;}
  if(fabs(AccY) < 0.1) { AccY = 0.0;}

  delay(10);
}
