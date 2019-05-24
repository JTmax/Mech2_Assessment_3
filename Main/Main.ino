#include "Main.h"
#include <Encoder.h>
#include <PID_v1.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

int testing = 1;
int distance = 0;
int reqSent = 0;
int initial = 0;
int pos = 0;
long lastmillis =0;
int flagSet = 0;
long imil = 0;
String msg1;
int x_offset = 0;
int killsw =0;

IntervalTimer motorTimer;

Encoder myEncLeft(ENCLA,ENCLB);
Encoder myEncRight(ENCRA, ENCRB);

int userSetVal =0;

double SetpointL, InputL, OutputL;
double KpL=1.22, KiL=1.4, KdL=0;

double SetpointR, InputR, OutputR;
double KpR =1.26, KiR=1.6, KdR=0;

PID LeftPID(&InputL, &OutputL, &SetpointL, KpL, KiL, KdL, DIRECT);
PID RightPID(&InputR, &OutputR, &SetpointR, KpR, KiR, KdR, DIRECT);

struct MotorData
{
    int DirectionL, DirectionR, SetSpeedL , SetSpeedR;
    int CurSpeedL, CurSpeedR, FilteredL =0;
};

struct EncoderData 
{
    long OldLPos =0, OldRPos =0, NewLPos =0, NewRPos=0, DeltaL=0, DeltaR=0;
};


struct MotorData MD;
struct EncoderData EncData;

int MotorSpeedLoopTime = 25000; //In micro seconds
long LastSpeedLoop = 0;

int ScreenRefreshTime = 100; //In mili seconds
long LastScreenLoop =0;

void killSwitch()
{
    killsw = 1;
}

void LCDscreen()
{
    if((millis()-LastScreenLoop) >= ScreenRefreshTime)
    {
        lcd.clear();
        lcd.print("SL:"+(String)MD.CurSpeedL + " RPM");
        
        lcd.setCursor(0,1 );
        lcd.print("SR:"+(String)MD.CurSpeedR + " RPM");

        lcd.setCursor(9,0);
        lcd.print("S:"+(String)userSetVal);
        
        LastScreenLoop = millis();
    }
}

void Motor(int SetSpeedL, int SetSpeedR, int DirectionL, int DirectionR)
{
    SetpointL = SetSpeedL;
    SetpointR = SetSpeedR;

    LeftPID.Compute();
    RightPID.Compute();

    //Left Motor
    if(DirectionL == CC)
    {
        digitalWrite(INA1,HIGH);
        digitalWrite(INA2,LOW);
    }
    else
    {
        digitalWrite(INA1,LOW);
        digitalWrite(INA2,HIGH);
    }
    
    //Right Motor
    if(DirectionR == CC)
    {
        digitalWrite(INB1,HIGH);
        digitalWrite(INB2,LOW);
    }
    else
    {
        digitalWrite(INB1,LOW);
        digitalWrite(INB2,HIGH);
    }

    analogWrite(PWMA, OutputL);
    analogWrite(PWMB, OutputR);

}

void EncoderData()
{

    EncData.NewLPos = myEncLeft.read();
    EncData.NewRPos = myEncRight.read();
    
    EncData.DeltaL = EncData.NewLPos - EncData.OldLPos; //Gets new encoder postion 
    EncData.DeltaR = EncData.NewRPos - EncData.OldRPos;
    
    EncData.OldLPos = EncData.NewLPos;
    EncData.OldRPos = EncData.NewRPos; 
        
    if(EncData.NewLPos ==0)
    {
        MD.CurSpeedL =0;
    }
    else
    {
        MD.CurSpeedL = abs(60000000/((1156.68/EncData.DeltaL)*(MotorSpeedLoopTime))); //Calcualtes current motor speed
    }  

    if(EncData.NewRPos ==0)
    {
        MD.CurSpeedR =0;
    }
    else
    {
        MD.CurSpeedR = abs(60000000/((1156.68/EncData.DeltaR)*(MotorSpeedLoopTime))); 
    }

    InputL = MD.CurSpeedL; //PID Input speed
    InputR = MD.CurSpeedR; //PID Input speed 

}


String Coms() //For Yashwin
{
    //Parse serial data from bluetooth module 
      String serData;

      analogWrite(PWMA, 0);
      analogWrite(PWMB, 0);
      
      Serial1.println("1");

      while(1)
      {
          delay(1);
          if(Serial1.available() > 0)
          {
            char c = Serial1.read();
            //Serial.println(c, DEC);
            if(c == '|')
            {
              Serial.println(serData);
              return(serData);
            }
            else
            {
              serData += c;
            }
          }
        
      }
}

void ConstantSpeed()
{
    Motor(MD.SetSpeedL,MD.SetSpeedR, MD.DirectionL, MD.DirectionR);
}


int Move(int SetSpeed, int setSteps)
{

    if(abs(myEncRight.read()) >= setSteps)
    {  
        MD.SetSpeedL = 0;
        MD.SetSpeedR = 0;
        
        myEncLeft.write(0); //reset encoder ticks
        myEncRight.write(0);
        
        return(1);
    }
    else 
    {
        //Serial.println((String)myEncLeft.read());
        MD.SetSpeedL =  SetSpeed;
        MD.SetSpeedR = SetSpeed;
        
    }

    return(0);
}


void Mode(int mode)
{
    switch (mode)
    {
        //Constant Speed Mode
        case 1:

            ConstantSpeed();
            break;
        //Fusion mode
        case 2:

            //FusionMode();
            break;
        default:
            break;
    }
    //Print Data to screen 
}

int RotateToCup(String msg)
{
    String FoundFlag;
    String xOffset;

    FoundFlag = msg.substring(0, msg.indexOf(","));
    xOffset = msg.substring(msg.indexOf(",")+1);

    if(FoundFlag.toInt() == 0)
    {
        initial = 1; //reset 
        pos = 0; //reset

        return(0);
    }
    else
    {
        int offsetval;
        
        offsetval = xOffset.toInt();
        if(offsetval <0)
        {
            MD.DirectionL = CCW;
            MD.DirectionR = CC;
        }
        else if(offsetval == 0)
        {
            pos++;
        }
        else
        {
            MD.DirectionL = CC;
            MD.DirectionR = CCW;
        }

        return(offsetval);
    }
    
}

int nbDelay(long imillis, int setDelay)
{
    if(millis() - imillis >= setDelay)
    {
        return(1);
    }
    else
    {
        MD.SetSpeedL = 0;
        MD.SetSpeedR = 0;
    }
    
    return(0);
}

void serialData()
{
    Serial.print((String)MD.CurSpeedL + " ");
    Serial.print((String)MD.CurSpeedR + " ");
    Serial.print((String)SetpointL + " ");
    Serial.println((String)SetpointL + " ");
}

void setup()
{
    Serial.begin(9600); //Debug serial port
    Serial1.begin(9600); //Pi serial port
    
    lcd.init();
    lcd.backlight();

    LeftPID.SetMode(AUTOMATIC);
    LeftPID.SetSampleTime(1);

    RightPID.SetMode(AUTOMATIC);
    RightPID.SetSampleTime(1);

    //Motor Driver Pin modes
    pinMode(INA1,OUTPUT);
    pinMode(INA2,OUTPUT);
    pinMode(INB1,OUTPUT);
    pinMode(INB2,OUTPUT);
    pinMode(PWMA,OUTPUT);
    pinMode(PWMB,OUTPUT);

    pinMode(KILL_SWITCH, INPUT); 
    
    attachInterrupt(KILL_SWITCH, killSwitch, RISING);

    analogWriteFrequency(PWMA, 58593.75);
    analogWriteFrequency(PWMB, 58593.75);

    motorTimer.begin(EncoderData, MotorSpeedLoopTime);
}


void loop()
{   
    if((millis() - lastmillis) >= 30)
    {
        //serialData();
        //Serial.print(pos);
        //Serial.print(" ");
        //Serial.println(MD.SetSpeedL);

        //Serial4.println("4");
        lastmillis = millis();

    }

    switch (pos)
    {
        case 0: //Search for cup
            MD.DirectionL = CC;
            MD.DirectionR = CCW;
    
            flagSet = Move(35,340);
            
            if(flagSet == 1)
            {
                pos++;
                imil = millis();
            }
            break;
        
        case 1: //Get prediction from pi 

            Serial.println("Com Sent: 1");
            msg1 = Coms();
            pos++;
            break;
            
        case 2: //Rotate towards cup
            
            if(initial == 0) //will only do this once
            {
                x_offset = RotateToCup(msg1);
                initial = 1;
            }
            else
            {
                flagSet = Move(16, x_offset);
                
                if(flagSet == 1)
                {
                    pos++;
                }
            }
            break;

        case 3: //Move towards cup
            
            //Set motor directions
            MD.DirectionL = CC; 
            MD.DirectionR = CC;

            if(killsw != 1) //Keep moving untill the killSwitch is pressed
            {
                MD.SetSpeedL = 16;
                MD.SetSpeedR = 16;
            }
            else
            {
                MD.SetSpeedL = 0;
                MD.SetSpeedR = 0;

                pos++;
            }
            break;

//      case 2:
//        
//            if(flagSet == 1 && reqSent == 0)
//            {
//                MD.DirectionL = CC;
//                MD.DirectionR = CC;
//                MD.SetSpeedL = 16;
//                MD.SetSpeedR = 16;
//                //pos = 0;
//            }
//            else if(flagSet == 0 && reqSent == 0)
//            {
//                pos = 0;
//            }
//            break;
//            
//        case 4:
//            MD.DirectionL = CC;
//            MD.DirectionR = CCW;
//            flagSet = Move(25,1000);
//            if(flagSet == 1)
//            {
//                pos++;
//                imil = millis();
//            }
//            break;
//        
//        case 5:
//            flagSet = nbDelay(imil,1000);
//            if(flagSet == 1)
//            {
//                pos++;
//            }
//            break;
//
//        case 6:
//            MD.DirectionL = CC;
//            MD.DirectionR = CC;
//            flagSet = Move(25,5000);
//            if(flagSet == 1)
//            {
//                pos++;
//                imil = millis();
//            }
//            break;
//            
        
        default:
            break;
    }

    LCDscreen();
    ConstantSpeed();
}

