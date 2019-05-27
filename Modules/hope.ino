#include <Stepper.h>
int x;
int y;
int dir = 3;
int stp = 2;
int dcmotor = 6;
int state;
int delayms = 500; //speed of stepper motor
int travsbobs = 0;

void setup()
{
  pinMode(dir, OUTPUT);
  pinMode(stp, OUTPUT);
  digitalWrite(dir, LOW);
  pinMode(dcmotor, OUTPUT);
}

void loop()
{

  if (travsbobs == 0)
  {
    digitalWrite(dir, LOW);
    analogWrite(dcmotor, 0);

    for (x = 1; x < 4096; x++)
    {
      digitalWrite(stp, HIGH);
      delayMicroseconds(delayms);
      digitalWrite(stp, LOW);
      delayMicroseconds(delayms);
    }

    delay(2000);
    analogWrite(dcmotor, 75);
    delay(10000);
    analogWrite(dcmotor, 0);
    digitalWrite(dir, HIGH);

    for (y = 1; y < 4096; y++)
    {
      digitalWrite(stp, HIGH);
      delayMicroseconds(delayms);
      digitalWrite(stp, LOW);
      delayMicroseconds(delayms);
    }
    travsbobs = 1;
  }




}