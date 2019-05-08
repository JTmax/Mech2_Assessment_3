#include "Main.h"
#include <Encoder.h>
#include <PID_v1.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

int testing = 1;

IntervalTimer motorTimer;

Encoder myEncLeft(ENCLA,ENCLB);
Encoder myEncRight(ENCRA, ENCRB);
Encoder userSetSpeed(29,30);

int userSetVal =0;

double SetpointL, InputL, OutputL;
double KpL=1.22, KiL=1.4, KdL=0;

double SetpointR, InputR, OutputR;
double KpR =1.22, KiR=1.4, KdR=0;

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
    userSetVal = userSetSpeed.read();

    EncData.NewLPos = myEncLeft.read();
    EncData.NewRPos = myEncRight.read();
    
    myEncLeft.write(0); //reset encoder ticks
    myEncRight.write(0);
        
    if(EncData.NewLPos ==0)
    {
        MD.CurSpeedL =0;
    }
    else
    {
        MD.CurSpeedL = abs(60000000/((1156.68/EncData.NewLPos)*(MotorSpeedLoopTime))); //Calcualtes current motor speed
    }  

    if(EncData.NewRPos ==0)
    {
        MD.CurSpeedR =0;
    }
    else
    {
        MD.CurSpeedR = abs(60000000/((1156.68/EncData.NewRPos)*(MotorSpeedLoopTime))); 
    }

    InputL = MD.CurSpeedL; //PID Input speed
    InputR = MD.CurSpeedR; //PID Input speed 

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
    Motor(userSetVal, userSetVal, MD.DirectionL, MD.DirectionR);
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
        LastScreenLoop = millis();
    }

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

}

long lastmillis =0;

void loop()
{   
    LeftPID.Compute();
    RightPID.Compute();

    if((millis() - lastmillis) >= 30)
    {
        serialData();
        lastmillis = millis();
    }

    Coms(); //Get data from bluetooth
    
    GloveData(); //Compute Glove data

    //IR(); //Get IR sensor reading
    
    Mode(1);
}

