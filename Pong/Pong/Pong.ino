/*
To use this program, you need to connect one analog input device to A0 (player 1) and one to A1 (player 2), as well as a 2 pin button to D2 and GND
I'd recommend using potentiometers, but you could use anything you want, you'll just have to live with the consequences
*/

#include "Arduino_LED_Matrix.h"
#include <stdlib.h>
#include "EEPROM.h"

//SETUP
ArduinoLEDMatrix matrix;
uint32_t *frame;

signed char ballX, ballY, speedX, speedY;
byte won;
unsigned long delayTime;

const uint32_t winScreens[][3] = {
  {
    0xa80aac52,
		0xa000e0ea,
		0xeac8eae,
  },
  {
    0xa80aac52,
		0xa000e004,
		0xab4ab453,
  },
};

void InitGame(){
  unsigned long seed;
  EEPROM.get(127, seed);
  randomSeed(seed);
  ++seed;
  EEPROM.put(127, seed);

  ballX = 6;
  ballY = 4;
  speedX = random(2) == 1 ? -1 : 1;
  randomSeed(++seed);
  speedY = random(2) == 0 ? -1 : 1;

  won = 0;
  delayTime = 333333;
}

void setup(){
  //SETUP
  Serial.begin(115200);
  matrix.begin();
  frame = (uint32_t *)malloc(sizeof(uint32_t) * 3);

  pinMode(D2, INPUT_PULLUP);

  InitGame();
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
  unsigned long mcStart = micros();
  ClearFrame();

  //Get Input
  int player1raw = analogRead(A0);
  int player2raw = analogRead(A1);
  byte player1 = 0b111 << ((player1raw >> 7) - 1);
  if(player1 == 0) player1 = 0b11;
  byte player2 = 0b111 << ((player2raw >> 7) - 1);
  if(player2 == 0) player2 = 0b11;

  //Print Pedals
  WriteUint8_Vert(player1, 0);
  WriteUint8_Vert(player2, 11);

  //Move Ball
  ballX += speedX;
  ballY += speedY;

  //Top Bottom Collision
  if(ballY > 7){
    ballY = 6;
    speedY = -1;
  }
  if(ballY < 0){
    ballY = 1;
    speedY = 1;
  }

  //Pedal Collision
  if(ballX < 2){
    if(1 << ballY & player1){
      ballX = 1;
      speedX = 1;
    }
  }
  if(ballX > 9){
    if(1 << ballY & player2){
      ballX = 10;
      speedX = -1;
    }
  }

  //Check for Win
  if(!won){
    if(ballX < 0) won = 2;
    if(ballX > 11) won = 1;
  }

  //Print Ball
  SwitchPixelOn(ballX + (ballY * 12));

  //Win Screen
  if(won){
    frame[0] = winScreens[won - 1][0];
    frame[1] = winScreens[won - 1][1];
    frame[2] = winScreens[won - 1][2];
    if(digitalRead(D2) == LOW) InitGame();
  }

  matrix.loadFrame(frame);
  delayTime -= 1000;
  if(delayTime < 10000) delayTime = 10000;
  delayMicroseconds(delayTime - (micros() - mcStart));
}
