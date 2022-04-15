#include <Arduino.h>

#include <SPI.h>
#include <MFRC522.h>

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
//****** RFID ********

#define RST_PIN 9
#define SS_PIN 10

String idUser = "";
//*********************

//***********Matrix LEDS ************************************************

int PINS[] = {8, 6, 5, 4};
int InterruptPins[] = {2, 3, 18, 19};

Adafruit_NeoMatrix matrix1 = Adafruit_NeoMatrix(8, 8, PINS[0],
                                                NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
                                                    NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                                                NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix matrix2 = Adafruit_NeoMatrix(8, 8, PINS[1],
                                                NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
                                                    NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                                                NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix matrix3 = Adafruit_NeoMatrix(8, 8, PINS[2],
                                                NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
                                                    NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                                                NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix matrix4 = Adafruit_NeoMatrix(8, 8, PINS[3],
                                                NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
                                                    NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                                                NEO_GRB + NEO_KHZ800);

Adafruit_NeoMatrix matrices[4] = {matrix1, matrix2, matrix3, matrix4};

byte cible[8] = {0x99, 0x66, 0x42, 0x99, 0x99, 0x82, 0x66, 0x99};
byte adn[8] = {0x5a, 0x89, 0x52, 0x89, 0x52, 0x89, 0x52, 0x99};
byte cercle[8] = {0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3c};
byte croix[8] = {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};
byte fusee[8] = {0x18, 0x3c, 0x3c, 0x3c, 0x3c, 0x24, 0x42, 0x81};
byte verre[8] = {0x24, 0x24, 0x24, 0x24, 0x18, 0x18, 0x18, 0x3c};

byte *symboles[6] = {cible, adn, cercle, croix, fusee, verre};
int numSymbole;
int winSymbole;
boolean isOver;

const uint16_t colors[] = {
    matrix1.Color(255, 0, 0), matrix1.Color(0, 255, 0), matrix1.Color(0, 0, 255)};

int delai;

//*****************************************************************

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

boolean interrupt;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details

  for (int i = 0; sizeof(matrices); i++)
  {
    matrices[i].begin();
    matrices[i].setTextWrap(false);
    matrices[i].setBrightness(40);
    matrices[i].setTextColor(colors[0]);
  }

  interrupt = false;
}

void drawSymbole(int matrix, int numSymbole)
{
  matrices[matrix].drawBitmap(0, 0, symboles[numSymbole], 8, 8, colors[random(2)]);
  matrices[matrix].show();
  delay(delai / matrix);
  matrices[matrix].clear();
}

void stopRollingFirstMatrix()
{
  interrupt = true;
  int winSymbole = numSymbole;
}

void stopRollingOtherMatrix()
{
  interrupt = true;
  if (numSymbole != winSymbole)
  {
    isOver = true;
  }
}

void playRound(int matrix, int delai)
{
  if (matrix == 0)
    attachInterrupt(digitalPinToInterrupt(InterruptPins[matrix]), stopRollingFirstMatrix, RISING);
  else
    attachInterrupt(digitalPinToInterrupt(InterruptPins[matrix]), stopRollingOtherMatrix, RISING);

  interrupt = false;
  numSymbole = 0;
  while (!interrupt)
  {
    drawSymbole(matrix, numSymbole);
    numSymbole++;
    if (numSymbole == 7)
    {
      numSymbole = 0;
    }
  }
  matrices[matrix].clear();
  matrices[matrix].drawBitmap(0, 0, symboles[numSymbole], 8, 8, colors[random(2)]);
  detachInterrupt(digitalPinToInterrupt(InterruptPins[matrix]));
  delay(2000);
}

void game1()
{
  isOver = false;
  delai = 1000;
  for (int i = 0; i < 3; i++)
  {
    playRound(i, delai / i);
    if (isOver)
    {
      break;
    }
  }

  if (isOver)
    Serial.println("Perdu.....");
  else
  {
    delay(5000);
    game1();
  }
}

void loop()
{
  // idUser = searchingForUser();

  delay(10000);
  Serial.println("Lancement de la partie...");

  game1();
}

String searchingForUser()
{
  while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
  {
  }

  String id = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);

    id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : " "));
    id.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  return id;
}
