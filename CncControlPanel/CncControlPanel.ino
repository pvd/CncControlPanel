#include <PinChangeInt.h>

#define QUAD_A_PIN 2
#define QUAD_B_PIN 3

volatile int32_t irqCnt;
volatile int32_t count;
volatile uint8_t lastA;
volatile uint8_t lastB;
volatile uint8_t newA;
volatile uint8_t newB;

uint8_t irqPin;

void QuadUpdate()
{
  newA = digitalRead(QUAD_A_PIN);
  newB = digitalRead(QUAD_B_PIN);

irqCnt++;

  if ( newA != lastA )
  {
    if ( ( newA == 0 && newB == 0 ) || ( newA == 1 && newB == 1) ) 
    {
      count--;
    }
    else
    {
      count++;
    }
  }

  if ( newB != lastB )
  {
    if ( (newB == 1 && newA == 0) || (newB == 0 && newA == 1) )
    {
      count--;
    }
    else
    {
      count++;
    }
  }


  lastA = newA;
  lastB = newB;
}


void setup() 
{
  Serial.begin(115200);

  pinMode(QUAD_A_PIN, INPUT);
  pinMode(QUAD_B_PIN, INPUT);

  // enable internal pullups
  digitalWrite(QUAD_A_PIN, HIGH);
  digitalWrite(QUAD_B_PIN, HIGH);

  irqCnt = 0;
  count = 0;
  lastA = digitalRead(QUAD_A_PIN);
  lastB = digitalRead(QUAD_B_PIN);

  PCintPort::attachInterrupt(QUAD_A_PIN, &QuadUpdate, CHANGE);
  PCintPort::attachInterrupt(QUAD_B_PIN, &QuadUpdate, CHANGE);
}

void loop()
{
  Serial.print(irqCnt);
  Serial.print(",");
  Serial.print(count);
  Serial.println();
  
  delay(100);
  
}
