#include <SoftwareSerial.h>
#include <BMDSDIControl.h>

const int                shieldAddress = 0x6E;
BMD_SDICameraControl_I2C sdiCameraControl(shieldAddress);

#define SSerialRX       4
#define SSerialTX       7
#define panright        5
#define panleft         6
#define tiltup         10
#define tiltdown       11

SoftwareSerial RS485Serial(SSerialRX, SSerialTX);

byte byteReceived[7];
int byteNumber = 0;
//int apertureOrdinal = 0;
//float fstop = 6.0;

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino Serial Servo Control with Pelco D");
  Serial.println();  

  RS485Serial.begin(2400);
  
  sdiCameraControl.begin();
  sdiCameraControl.setOverride(true);

  pinMode(panright, OUTPUT);
  pinMode(panleft, OUTPUT);
  pinMode(tiltup, OUTPUT);
  pinMode(tiltdown, OUTPUT);
  digitalWrite(panright, LOW);    
  digitalWrite(panleft, LOW);
  digitalWrite(tiltup, LOW);
  digitalWrite(tiltdown, LOW);
}

void loop() {
  if (RS485Serial.available() > 0) {
    byteReceived[byteNumber ++] = RS485Serial.read();
    if ( byteReceived[0] != 0xFF ) {
      byteNumber = 0;
    }
  }
  if ( byteNumber > 6 ) {
    handlePelcoCommand();
    byteNumber = 0;
  }
}

void handlePelcoCommand() {
  printPelcoHex();
  Serial.print(F("camera "));
  Serial.print(byteReceived[1], DEC);
  Serial.print(F(": "));
  handlePan();
  handleTilt();
  handleZoom();
  handleFocus();
  handleIris();
  isAutoScan();
  Serial.println("");
}

void handlePan() {
  if (bitRead(byteReceived[3], 1)) {
    Serial.print(F("pan right: "));
    Serial.print(byteReceived[4], DEC);
    digitalWrite(panright, HIGH);
    digitalWrite(panleft, LOW);
  } else if (bitRead(byteReceived[3], 2)) {
    Serial.print(F("pan left: "));
    Serial.print(byteReceived[4], DEC);
    digitalWrite(panleft, HIGH);
    digitalWrite(panright, LOW);
  } else {
    Serial.print(F("pan stop"));
    digitalWrite(panright, LOW);
    digitalWrite(panleft, LOW);
  }
  Serial.print(F(", "));
}

void handleTilt() {
  if (bitRead(byteReceived[3], 3)) {
    Serial.print(F("tilt up: "));
    Serial.print(byteReceived[5], DEC);
    digitalWrite(tiltup, HIGH);
    digitalWrite(tiltdown, LOW);
  } else if (bitRead(byteReceived[3], 4)) {
    Serial.print(F("tilt down: "));
    Serial.print(byteReceived[5], DEC);
    digitalWrite(tiltdown, HIGH);
    digitalWrite(tiltup, LOW);
  } else {
    Serial.print(F("tilt stop"));
    digitalWrite(tiltup, LOW);
    digitalWrite(tiltdown, LOW);
  }
  Serial.print(F(", "));
}

void handleZoom() {
  if (bitRead(byteReceived[3], 5)) {
    Serial.print(F("zoom tele"));
    sdiCameraControl.writeCommandFixed16(byteReceived[1], 0x00, 0x09, 0x00, 0.1);
  } else if (bitRead(byteReceived[3], 6)) {
    Serial.print(F("zoom wide"));
    sdiCameraControl.writeCommandFixed16(byteReceived[1], 0x00, 0x09, 0x00, -0.1);
  } else {
    Serial.print(F("zoom stop"));
    sdiCameraControl.writeCommandFixed16(byteReceived[1], 0x00, 0x09, 0x00, 0.0);
  }
  Serial.print(F(", "));
}

void handleFocus() {
  if (bitRead(byteReceived[3], 7)) {
    Serial.print(F("focus far"));
    sdiCameraControl.writeCommandFixed16(byteReceived[1], 0x00, 0x00, 0x01, 0.01);
  } else if (bitRead(byteReceived[2], 0)) {
    Serial.print(F("focus near"));
    sdiCameraControl.writeCommandFixed16(byteReceived[1], 0x00, 0x00, 0x01, -0.01);
  } else {
    Serial.print(F("focus stop"));
  }
  Serial.print(F(", "));
}

void handleIris() {
  if (bitRead(byteReceived[2], 1)) {
    Serial.print(F("iris open"));
    sdiCameraControl.writeCommandInt64(byteReceived[1], 0x00, 0x04, 0x01, -1);
  } else if (bitRead(byteReceived[2], 2)) {
    Serial.print(F("iris close"));
    sdiCameraControl.writeCommandInt64(byteReceived[1], 0x00, 0x04, 0x01, 1);
  } else {
    Serial.print(F("iris stop"));
  }
  Serial.print(F(", "));

//  if (bitRead(byteReceived[2], 1)) {
//    if (apertureOrdinal > 0) {
//      apertureOrdinal--;
//    }
//  } else if (bitRead(byteReceived[2], 2)) {
//    apertureOrdinal++;
//  }
//  sdiCameraControl.writeCommandInt64(byteReceived[1], 0x00, 0x04, 0x00, apertureOrdinal);

//  if (bitRead(byteReceived[2], 1)) {
//    if (fstop > -1.0) {
//      fstop -= 0.25;
//    }
//  } else if (bitRead(byteReceived[2], 2)) {
//    if (fstop < 16.0) {
//      fstop += 0.25;
//    }
//  }
//  sdiCameraControl.writeCommandFixed16(byteReceived[1], 0x00, 0x02, 0x00, fstop);
}

void isAutoScan() {
  // This command is not intended to be used as autofocus, but it's how I use it
  if (byteReceived[2] == 0x90) {
    sdiCameraControl.writeCommandVoid(byteReceived[1], 0x00, 0x01);
    Serial.print(F("autofocus"));
  }
}

void printPelcoHex() {
  Serial.print(byteReceived[0], HEX);
  Serial.print(F("-"));
  Serial.print(byteReceived[1], HEX);
  Serial.print(F("-"));
  Serial.print(byteReceived[2], HEX);
  Serial.print(F("-"));
  Serial.print(byteReceived[3], HEX);
  Serial.print(F("-"));
  Serial.print(byteReceived[4], HEX);
  Serial.print(F("-"));
  Serial.print(byteReceived[5], HEX);
  Serial.print(F("-"));
  Serial.print(byteReceived[6], HEX);
  Serial.println("");
}

