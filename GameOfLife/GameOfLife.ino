#include "Arduino_LED_Matrix.h"
#include <stdlib.h>
#include <EEPROM.h>

//SETUP
ArduinoLEDMatrix matrix;
size_t *apixel;
uint32_t *frame;

unsigned char show[8][12], calc[8][12], hist[16][8][12], histc;

void RandomPos(){
  unsigned long seed;
  EEPROM.get(127, seed);
  randomSeed(seed);
  ++seed;
  EEPROM.put(127, seed);

  for(int i = 0; i < 8; ++i){
    for(int j = 0; j < 12; ++j){
      show[i][j] = random(2) == 0 ? 1 : 0;
      for(int k = 0; k < 16; ++k){
        hist[k][i][j] = 0;
      }
    }
  }
}

void CurtainClose(){
  for(int i = 0; i < 12; ++i){
    unsigned long mcStartSub = micros();
    WriteUint8_Vert(0xff, i);
    matrix.loadFrame(frame);
    delayMicroseconds(50000 - (micros() - mcStartSub));
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

unsigned char NM(unsigned char a, unsigned char b){
  int c = a - 1;
  if(c < 0) return b - 1;
  return c % b;
}

unsigned char Match(unsigned char a[8][12], unsigned char b[8][12]){
  for(int i = 0; i < 8; ++i){
    for(int j = 0; j < 12; ++j){
      if(a[i][j] != b[i][j]) return 0;
    }
  }
  return 1;
}
unsigned char InHist(){
  for(int k = 0; k < 16; ++k){
    if(Match(show, hist[k])) return 1;
  }
  return 0;
}



void setup(){
  //SETUP
  Serial.begin(115200);
  matrix.begin();
  frame = (uint32_t *)malloc(sizeof(uint32_t) * 3);

  RandomPos();
}

void loop(){
  long mcStart = micros();
  ++histc;

  for(int i = 0; i < 8; ++i){
    for(int j = 0; j < 12; ++j){
      calc[i][j] = show[i][j];
      hist[histc % 16][i][j] = show[i][j];
    }
  }
  for(int i = 0; i < 8; ++i){
    for(int j = 0; j < 12; ++j){
      int n = calc[NM(i, 8)][NM(j, 12)]
            + calc[NM(i, 8)][j]
            + calc[NM(i, 8)][(j + 1) % 12]
            + calc[i][NM(j, 12)]
            + calc[i][(j + 1) % 12]
            + calc[(i + 1) % 8][NM(j, 12)]
            + calc[(i + 1) % 8][j]
            + calc[(i + 1) % 8][(j + 1) % 12];
      if(calc[i][j]){
        if(n == 2 || n == 3) show[i][j] = 1;
        else show[i][j] = 0;
      }
      else{
        if(n == 3) show[i][j] = 1;
        else show[i][j] = 0;
      }
    }
  }

  ClearFrame();
  for(int i = 0; i < 8; ++i){
    for(int j = 0; j < 12; ++j){
      if(show[i][j]) SwitchPixelOn(i * 12 + j);
    }
  }
  matrix.loadFrame(frame);

  delayMicroseconds(100000 - (micros() - mcStart));

  if(InHist()){
    RandomPos();
    CurtainClose();
    delayMicroseconds(1000000 - (micros() - mcStart));
  }
}
