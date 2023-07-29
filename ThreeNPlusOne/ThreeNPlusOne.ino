#include "Arduino_LED_Matrix.h"
#include <stdlib.h>
#include <EEPROM.h>

//SETUP
ArduinoLEDMatrix matrix;
size_t *apixel;
uint32_t *frame;

uint32_t currentStart, current, currentStep;

void setup(){
  //SETUP
  Serial.begin(115200);
  matrix.begin();
  frame = (uint32_t *)malloc(sizeof(uint32_t) * 3);

  EEPROM.get(0, currentStart);
  if(~currentStart == 0){
    currentStart = 1;
    EEPROM.put(0, currentStart);
  }
}

void SwitchPixelOn(size_t pixel){
  int n = pixel / 32;
  frame[n] = frame[n] | 0b1 << ((31 - pixel) % 32);
}
void SwitchPixelOff(size_t pixel){
  int n = pixel / 32;
  frame[n] = frame[n] & ~(0b1 << ((31 - pixel) % 32));
}
void ClearFrame(){
  frame[0] = 0;
  frame[1] = 0;
  frame[2] = 0;
}

void WriteUint8_Vert(uint8_t n, size_t row){
  for(int i = 0; i < 8; ++i){
    if(n >> i & 1 == 1) SwitchPixelOn(row + 12 * i);
    else SwitchPixelOff(row + 12 * i);
  }
}
void WriteUint32_Vert(uint32_t n, size_t row){
  for(int i = 0; i < 4; ++i){
    WriteUint8_Vert((uint8_t)(n >> (i * 8)), row + 3 - i);
  }
}

void loop(){
  current = currentStart;
  currentStep = 0;

  ClearFrame();
  WriteUint32_Vert(currentStart, 0);
  WriteUint32_Vert(current, 4);
  WriteUint32_Vert(currentStep, 8);
  matrix.loadFrame(frame);

  while(current != 1 /*&& current >= currentStart*/){
    unsigned long mcAlgoStart = micros();
    ++currentStep;
    if(current % 2 == 0) current /= 2;
    else current = current * 3 + 1;
    WriteUint32_Vert(current, 4);
    WriteUint32_Vert(currentStep, 8);
    matrix.loadFrame(frame);
    delayMicroseconds(33333 - (micros() - mcAlgoStart));
  }

  /*
  Serial.print(currentStart);
  Serial.print("\treached 1 in ");
  Serial.print(currentStep);
  Serial.print("\t steps\n");
  */
  ++currentStart;
  unsigned long mcWriteStart = micros();
  EEPROM.put(0, currentStart);
  delayMicroseconds(100000 - (micros() - mcWriteStart));
}