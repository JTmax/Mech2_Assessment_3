#include "Main.h"
#include <Encoder.h>
#include <PID_v1.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

//GLOBAL VARIABLES//

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
int mapping = 0;
int Done =0;

/////////////////////////

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

//      analogWrite(PWMA, 0);
//      analogWrite(PWMB, 0);
      
      Serial1.println("1");
      Serial.println("Com Sent: 1");
      
      while(1)
      {
          delay(1);
          if(Serial1.available() > 0)
          {
            char c = Serial1.read();
            if(c == '|')
            {
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
        MD.SetSpeedL =  SetSpeed;
        MD.SetSpeedR = SetSpeed;   
    }

    return(0);
}

int RotateToCup(String msg)
{
    String FoundFlag;
    String xOffset;

    FoundFlag = msg.substring(0, msg.indexOf(","));
    xOffset = msg.substring(msg.indexOf(",")+1);
    
    Serial.println("Flag:" + FoundFlag);
    
    if(FoundFlag.toInt() == 0)
    {
        pos = 0; //reset

        return(0);
    }
    else
    {
        int offsetval;
        
        Serial.print("Offset:" + offsetval);
        
        offsetval = xOffset.toInt();
        if(offsetval < 0)
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

        return(abs(offsetval));
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

    //Hope
    pinMode(S_DIR, OUTPUT);
    pinMode(S_STP, OUTPUT);
    pinMode(S_DC_EN, OUTPUT);

    //Dunker
    pinMode(D_STP, OUTPUT);
    pinMode(D_DIR, OUTPUT);
    pinMode(D_EN, OUTPUT);

    pinMode(KILL_SWITCH, INPUT); 
    
    attachInterrupt(KILL_SWITCH, killSwitch, FALLING);

    analogWriteFrequency(PWMA, 58593.75);
    analogWriteFrequency(PWMB, 58593.75);

    motorTimer.begin(EncoderData, MotorSpeedLoopTime);
}

void loop()
{   
    if((millis() - lastmillis) >= 30) //Serial Print loop 30 ms
    {
        //serialData();
        //Serial.println((String)killsw);
        lastmillis = millis();
    }

    switch (pos)
    {
        case 0: //Search for cup
            
            //Serial.println("In case: 0 (Searching for cup)");
            
            initial=0;
            x_offset = 0;
            
            MD.DirectionL = CC;
            MD.DirectionR = CCW;
    
            flagSet = Move(35,340);
                        
            if(flagSet == 1)
            {
                pos++;
                imil = millis();
            }

            ConstantSpeed();
            break;
        
        case 1: //Get prediction from pi 
            
            //Serial.println("In case: 1 (Getting Prediction from Pi)" );

            MD.SetSpeedL = 0;
            MD.SetSpeedR = 0;
            
            while(MD.CurSpeedL != 0)
            {
              ConstantSpeed();
            }
                        
            msg1 = Coms(); //Coms is a blocking functions
            pos++;
            break;
            
        case 2: //Rotate towards cup
            
            if(initial == 0) //will only do this once
            {
                //Serial.println("In case: 2/1 (Parsing returned serial data)");
                
                x_offset = RotateToCup(msg1);
                
                initial = 1;
            }
            else
            {
                //Serial.println("In case: 2/2 (Rotating towards cup)" );
                ConstantSpeed();
                
                mapping = x_offset * 0.633; //Map px displacment to excoder value
                
                flagSet = Move(30, mapping);
                
                if(flagSet == 1)
                {
                    pos++;
                }
            }
            break;

        case 3: //Move towards cup

            //Serial.println("In case: 3 (Driving towards cup)");
            
            //Set motor directions
            MD.DirectionL = CCW; 
            MD.DirectionR = CCW;

            if(killsw == 0) //Keep moving untill the killSwitch is pressed
            {
                MD.SetSpeedL = 30;
                MD.SetSpeedR = 30;

                ConstantSpeed();
            }
            else
            {
                //Serial.println("In case: 3 (Docked with cup)");
                
                MD.SetSpeedL = 0;
                MD.SetSpeedR = 0;
                
                while(MD.CurSpeedL != 0)
                {
                  ConstantSpeed();
                }

                pos++;
            }
            break;

         //BLOCKING FUNCTIONS//
            
         case 4: //Milk
            //Serial.println("In case: 4");

            //ADD CODE
            Done = 1;

            while(Done == 1)
            {


                
                Done = 0;
            }

            /////////
           
            pos++;
            break;

         case 5: //Dunker
            //Serial.println("In case: 5");
            int state;
            digitalWrite(D_STP, LOW);
            digitalWrite(D_DIR, LOW);
            digitalWrite(D_EN, HIGH);

            //ADD CODE
            for(int x = 1; x<30; x++)  //Loop the forward stepping enough times for motion to be visible
            {
                //Read direction pin state and change it
                state = digitalRead(D_DIR);

                if(state == HIGH)
                {
                    digitalWrite(D_DIR, LOW);
                }
                else if(state == LOW)
                {
                    digitalWrite(D_DIR,HIGH);
                }
 
                for(int y= 1; y<2000; y++)
                {
                    digitalWrite(D_STP,HIGH); //Trigger one step
                    delay(1);
                    digitalWrite(D_STP,LOW); //Pull step pin low so it can be triggered again
                    delay(1);
                }
            }

            /////////
            
            pos++;
            break;

         case 6: //Hope
            //Serial.println("In case: 6");

            digitalWrite(S_STP, LOW);
            digitalWrite(S_DIR, LOW);
            digitalWrite(S_EN, HIGH);
            digitalWrite(S_DC_EN, LOW);

            //ADD CODE
            digitalWrite(S_DIR, LOW);
            
            for (int x = 1; x < 4096; x++)
            {
            digitalWrite(S_STP, HIGH);
            delayMicroseconds(500);
            digitalWrite(S_STP, LOW);
            delayMicroseconds(500);
            }

            delay(2000);
            analogWrite(S_DC_EN, 75); //Turm on dc motor

            delay(10000);
            analogWrite(S_DC_EN, 0); //Turn off dc motor

            digitalWrite(S_DIR, HIGH); //Change directions

            for (int y = 1; y < 4096; y++)
            {
            digitalWrite(S_STP, HIGH);
            delayMicroseconds(500);
            digitalWrite(S_STP, LOW);
            delayMicroseconds(500);
            }
            /////////
            
            pos++;

            break;
         
         case 7: //Move back play beep
            
//            MD.DirectionL = CC;
//            MD.DirectionR = CC;
//    
//            flagSet = Move(35,500);
//            
//            if(flagSet == 1)
//            {
//                pos++;
//            }
//            
            break;
    
//        case 8:
//            flagSet = nbDelay(imil,1000);
//            if(flagSet == 1)
//            {
//                pos++;
//            }
//            break;

        default:
            break;
    }

    LCDscreen();
    //ConstantSpeed();
}

