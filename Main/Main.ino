#include "Main.h"
#include <Encoder.h>
#include <PID_v1.h>
#include <LiquidCrystal_I2C.h>
#include <NewPing.h>

LiquidCrystal_I2C lcd(0x27,20,4);
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

int testing = 1;
int distance = 0;

IntervalTimer motorTimer;
IntervalTimer pingTimer;

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

struct SixDofData 
{
    int pitch, yaw, roll, accel_x, accel_y, accel_z;
};

struct SixDofData gSense;
struct MotorData MD;
struct EncoderData EncData;

int MotorSpeedLoopTime = 25000; //In micro seconds
long LastSpeedLoop = 0;

int ScreenRefreshTime = 100; //In mili seconds
long LastScreenLoop =0;


float Filter(float prevSpeed, float CurrentSpeed)
{
    int filter = 5000;

    float FilteredVal = (prevSpeed + (CurrentSpeed * filter)) / (filter + 1);

    return(FilteredVal);
}

void distData()
{
  distance = sonar.ping_cm();
}

void Motor(int SetSpeedL, int SetSpeedR, int DirectionL, int DirectionR)
{
    SetpointL = SetSpeedL;
    SetpointR = SetSpeedR;

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


}

void EncoderD()
{
    //userSetVal = userSetSpeed.read();

    EncData.NewLPos = myEncLeft.read();
    EncData.NewRPos = myEncRight.read();
    
    EncData.DeltaL = EncData.NewLPos - EncData.OldLPos; //Gets new encoder postion 
    EncData.DeltaR = EncData.NewRPos - EncData.OldRPos;
    
    EncData.OldLPos = EncData.NewLPos;
    EncData.OldRPos = EncData.NewRPos; 
    //myEncLeft.write(0); //reset encoder ticks
    //myEncRight.write(0);
        
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

    //distance = sonar.ping_cm();

}


void Coms() //For Yashwin
{
    //Parse serial data from bluetooth module 

    //Set struct data
    gSense.pitch= 0;
    gSense.yaw =0;
    gSense.roll =0;
    gSense.accel_x =0;
    gSense.accel_y =0;
    gSense.accel_z =0;

}

void ConstantSpeed()
{
    Motor(MD.SetSpeedL,MD.SetSpeedR, MD.DirectionL, MD.DirectionR);
}

void FusionMode()
{

    Motor(MD.SetSpeedL,MD.SetSpeedR, MD.DirectionL, MD.DirectionR);
}


void GloveData()
{
    //Logic to determine speed and direction of motors 
    MD.DirectionL = CC;
    MD.DirectionR = CCW;
    MD.SetSpeedL =  20;
    MD.SetSpeedR = 20;
}

void AccelData()
{
//test
}

int Move(int setSpeed, int setSteps)
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
        MD.SetSpeedL =  setSpeed;
        MD.SetSpeedR = setSpeed;
        
    }

    return(0);
}


void Mode(int mode)
{
    switch (mode)
    {
        //Constant Speed Mode
        case 1:

            analogWrite(PWMA, OutputL);
            analogWrite(PWMB, OutputR);
            
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
    if((millis()-LastScreenLoop) >= ScreenRefreshTime)
    {
        lcd.clear();
        lcd.print("SL:"+(String)MD.CurSpeedL + " RPM");
        
        lcd.setCursor(0,1 );
        lcd.print("SR:"+(String)MD.CurSpeedR + " RPM");

        lcd.setCursor(9,0);
        lcd.print("S:"+(String)userSetVal);

        lcd.setCursor(9,1);
        lcd.print("D:"+(String)distance);
        
        LastScreenLoop = millis();
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
    Serial.begin(9600);
    Serial4.begin(9600);
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

    analogWriteFrequency(PWMA, 58593.75);
    analogWriteFrequency(PWMB, 58593.75);

    motorTimer.begin(EncoderD, MotorSpeedLoopTime);
    //pingTimer.begin(distData, 35000);
}

long lastmillis =0;
int flagSet = 0;
long imil = 0;

int pos = 0;
void loop()
{   
    LeftPID.Compute();
    RightPID.Compute();

    if((millis() - lastmillis) >= 30)
    {
        //serialData();
        //Serial.print(pos);
        //Serial.print(" ");
        //Serial.println(MD.SetSpeedL);

        Serial4.println("4");
        lastmillis = millis();

      if(Serial4.available())
      {
        char c = Serial4.read();
        Serial.println(c);
      }
    }


    
    Coms(); //Get data from bluetooth
    
    //GloveData(); //Compute Glove data
    switch (pos)
    {
        case 0:
            MD.DirectionL = CC;
            MD.DirectionR = CCW;
    
            flagSet = Move(25,1000);
            if(flagSet == 1)
            {
                pos++;
                imil = millis();
            }
            break;
        
        case 1:

            flagSet = nbDelay(imil,1000);
            if(flagSet == 1)
            {
                pos++;
            }
            break;
            
        case 2:
            MD.DirectionL = CC;
            MD.DirectionR = CCW;
            flagSet = Move(25,1000);
            if(flagSet == 1)
            {
                pos++;
                imil = millis();
            }
            break;
            
        case 3:
            flagSet = nbDelay(imil,1000);
            if(flagSet == 1)
            {
                pos++;
            }
            break;
            
        case 4:
            MD.DirectionL = CC;
            MD.DirectionR = CCW;
            flagSet = Move(25,1000);
            if(flagSet == 1)
            {
                pos++;
                imil = millis();
            }
            break;
        
        case 5:
            flagSet = nbDelay(imil,1000);
            if(flagSet == 1)
            {
                pos++;
            }
            break;

        case 6:
            MD.DirectionL = CC;
            MD.DirectionR = CC;
            flagSet = Move(25,5000);
            if(flagSet == 1)
            {
                pos++;
                imil = millis();
            }
            break;
            
            
        default:
            break;
    }
    

    //IR(); //Get IR sensor reading
    
    Mode(1);
}

