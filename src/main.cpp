#include <Arduino.h>
#include <AccelStepper.h>

#define DIR_PIN  13
#define STEP_PIN 12
#define EN_PIN 8
#define HOME_PIN -1 // TODO: Set the correct pin for the home switch, or set to -1 if not used


AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

String command = "";

void homing()
{
  digitalWrite(EN_PIN, LOW);
  stepper.move(-1000000);
  stepper.setSpeed(10000);
  while(stepper.distanceToGo() != 0)
  {
    stepper.run();
#if HOME_PIN != -1
    if(digitalRead(HOME_PIN) == LOW)
    {
      stepper.stop();
      stepper.setCurrentPosition(0);
      Serial.println("HOMED");
      digitalWrite(EN_PIN, HIGH);
      return;
    }
#endif
    if(Serial.available() > 0 && Serial.read() == 'S')
    {
      stepper.stop();
      Serial.println("HOMING ABORTED");
      digitalWrite(EN_PIN, HIGH);
      return;
    }
  }
#if HOME_PIN == -1
  stepper.setCurrentPosition(0);
  Serial.println("HOMED (NO SWITCH)");
#endif
  digitalWrite(EN_PIN, HIGH);
}

void process(String command) // Msteps;speed; 
{
  command.trim();
  if(command.startsWith("M"))
  {
    digitalWrite(EN_PIN, LOW);
    command = command.substring(1);
    int deli1 = command.indexOf(';');
    int deli2 = command.indexOf(';',deli1 + 1);

    if (deli1 == -1 || deli2 == -1)
    {
      Serial.println("Err");
      return;
    }
    long steps = command.substring(0, deli1).toInt();
    long speed = command.substring(deli1 + 1, deli2).toInt();
    if (speed <= 0)
    {
      Serial.println("Err");
      return;
    }
    stepper.move(steps);
    stepper.setSpeed(speed);
    while(stepper.distanceToGo() != 0)
    {
      stepper.runSpeedToPosition();
      if(Serial.available() > 0)
      {
        char c = Serial.read();
        if(c == 'S')
        {
          stepper.stop();
          break;
        }
        if(c == 'C')
        {
          Serial.println(stepper.currentPosition());
        }
      }
    }
    Serial.println("DONE");
    digitalWrite(EN_PIN, HIGH);
  }
  else
  {
    Serial.println("Err");
  }
}

void setup() 
{
  Serial.begin (115200);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);
  stepper.setMaxSpeed(11000);
  homing();
  Serial.println("Stepper Ready");
}


void loop() 
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();
    if(c == '\n'|| c == '\r')
    {
      if(command.length() > 0)
      {
        process(command);
        command = "";
      }
    }
    else
    {
      command += c;
    }
  }  
}

