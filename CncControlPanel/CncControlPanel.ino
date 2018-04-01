#define ledPin 13

int inByte = 0;
int outByte = 0;

void setup() 
{
  DDRB = DDRB | 0b00111111; // pins 8-13 as output
  PORTB = 0;
  Serial.begin(115200);
}

void loop()
{
  // send data only when you receive data:
  if (Serial.available() > 0) 
  {
    // read the incoming byte:
    inByte = Serial.read();

    if ( digitalRead(2) == false )
    {
      outByte++;
      if ( outByte > 20 )
      {
        outByte = 0;
      }
    }
    Serial.write(outByte);
  }

  PORTB = inByte;
}
