#include<Wire.h>

const int MPU = 0x68; //pino aberto 0X68 , pino ligado em 3,3V 0x69

//Variaveis globais
float acelX, acelY, acelZ, acelNew, acelDelta, acelMax = 0;
bool flagZero = false, flagMax = false, flagCollision = true;
byte entryCounter = 0, entryCounterActual = 0, i = 0;
float acelDeltaMax[10] = {};
unsigned long hitBreak, actualTime;
int timeWithoutHit, collisionTime;

void setup(){
  Serial.begin(9600);         //inicia a comunicação serial
  Wire.begin();                 //inicia I2C
  Wire.beginTransmission(MPU);  //Inicia transmissão para o endereço do MPU
  Wire.write(0x6B);

  //Inicializa o MPU-6050
  Wire.write(0);
  Wire.endTransmission(true);
}

void loop(){
  Wire.beginTransmission(MPU);      //transmite
  Wire.write(0x3B);                 // Endereço 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);     //Finaliza transmissão

  Wire.requestFrom(MPU, 14, true); //requisita bytes

  //Armazena o valor dos sensores nas variaveis correspondentes
  acelX = Wire.read() << 8 | Wire.read(); //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  acelY = Wire.read() << 8 | Wire.read(); //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  acelZ = Wire.read() << 8 | Wire.read(); //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  acelDelta = accelerationCalculation(acelZ);
  printValues(true);
}

float accelerationCalculation(float acelZ){
  acelNew = (9.80665 * acelZ) / 17545;
  if (acelZ > 17540) acelDelta = acelNew - 9.80665;
  else acelDelta = 9.80665 - acelNew;
  return acelDelta;
}

void printValues(bool filter) {
  if (!filter) {
    Serial.print("Z axis acceleration: "); Serial.println(acelZ);
    Serial.print("Z-axis acceleration value: "); Serial.print(acelNew); Serial.println("m/s**2");
    Serial.print("Delta value of z-axis acceleration: "); Serial.print(acelDelta); Serial.println("m/s**2");
    Serial.println("");
  }
  else {
    if ((acelZ < 0.50) and (!flagZero)) {
      Serial.println("Body without acceleration.");
      flagZero = true;
    }
    else if ((acelZ >= 0.50) and (flagZero)) {
      if (flagCollision) {
        actualTime = millis();
      }
      Serial.print("Body acceleranting at: "); Serial.print(acelDelta); Serial.println("m/s**2");
      flagZero = false;
      entryCounter++;
      acelDeltaMax[i] = acelDelta;
      i++;
    }
    if (entryCounterActual != entryCounter) {
      entryCounterActual = entryCounter;
      hitBreak = millis();
    }
    else {
      timeWithoutHit = millis() - hitBreak;
      if (timeWithoutHit > 3000) {
        Serial.println("No Hit!");
        hitBreak = millis();
        for (byte index = 0; index < i; index++) {
          Serial.println(acelDeltaMax[index]);

          if (acelDeltaMax[index] > acelMax) acelMax = acelDeltaMax[index];
          flagMax = true;
        }
        i = 0;
        entryCounter = 0;
        if (flagMax) {
          collisionTime = millis() - actualTime - 3000;
          Serial.print("Maximum acceleration: "); Serial.print(acelMax); Serial.println("m/s**2");
          Serial.print("Collision Time: "); Serial.print(collisionTime); Serial.println(" milliseconds");
          flagMax = false;
          acelMax = 0;
        }
      }
    }
  }
}
