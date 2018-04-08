#include <Encoder.h>

#define PIN_MPG_X_A 2
#define PIN_MPG_X_B 3

struct TStatus
{
  int8_t  sync;
  int32_t mpgXcount;
  int32_t mpgYcount;
  int32_t mpgZcount;
  int8_t  portA;
  int8_t  portB;
  int8_t  portC;
  int8_t  portD;
};


Encoder mpgX(PIN_MPG_X_A, PIN_MPG_X_B);
TStatus statusRec;
int     inByte;

void setup() 
{
  Serial.begin(115200);

  pinMode(8, INPUT);
  pinMode(9, INPUT);
  
  mpgX.write(0);

  statusRec.sync  = 0x55;
  statusRec.mpgXcount = 0;
  statusRec.mpgYcount = 0;
  statusRec.mpgZcount = 0;
  statusRec.portA = 0;
  statusRec.portB = 0; 
  statusRec.portC = 0;
  statusRec.portD = 0;
}

void loop()
{
  // send data only when you receive data:
  if (Serial.available() > 0) 
  {
    // Wait for incoming data;
    inByte = Serial.read();

    // Update the status record and transmit it 
    statusRec.mpgXcount = mpgX.read();
    statusRec.portB     = digitalRead(8);  
    Serial.write(reinterpret_cast<char*>(&statusRec), sizeof(statusRec));
  }
}
