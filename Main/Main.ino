#include "Main.h"
#include <Encoder.h>
#include <PID_v1.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27,20,4);

Encoder myEncLeft(ENCLA, ENCLB);
Encoder myEncRight(ENCRA, ENCRB);

double SetpointL, InputL, OutputL;
double KpL=5, KiL=2.5, KdL=0.001;

double SetpointR, InputR, OutputR;
double KpR=5, KiR=2.5, KdR=0.001;

PID LeftPID(&InputL, &OutputL, &SetpointL, KpL, KiL, KdL, DIRECT);
PID RightPID(&InputR, &OutputR, &SetpointR, KpR, KiR, KdR, DIRECT);

struct MotorData
{
    int DirectionL, DirectionR, SetSpeedL , SetSpeedR;
    float CurSpeedL, CurSpeedR, FilteredL =0;
};

struct EncoderData 
{
    long OldLPos =0, OldRPos =0, NewLPos =0, NewRPos=0, DeltaL=0, DeltaR=0;
};

struct MotorData MD;
struct EncoderData EncData;


int MotorSpeedLoopTime = 10; //In micro seconds
long LastSpeedLoop = 0;

int ScreenRefreshTime = 100; //In mili seconds
long LastScreenLoop =0;

void IR() //For Hope
{


}

float Filter(float prevSpeed, float CurrentSpeed)
{
    int filter = 1000;

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
    if(DirectionR == CCW)
    {
        digitalWrite(INA1,HIGH);
        digitalWrite(INA2,LOW);
    }
    else
    {
        digitalWrite(INA1,LOW);
        digitalWrite(INA2,HIGH);
    }
    
    analogWrite(PWMA, OutputL);
    analogWrite(PWMB, OutputR);
}

void Encoder()
{
    if((millis()-LastSpeedLoop) >= MotorSpeedLoopTime)
    {
        EncData.NewLPos = myEncLeft.read();
        EncData.NewRPos = myEncRight.read();

        EncData.DeltaL = EncData.NewLPos - EncData.OldLPos;
        EncData.DeltaR = EncData.NewRPos - EncData.OldRPos;

        MD.CurSpeedL = 60000/((1156.68/EncData.DeltaL)*(MotorSpeedLoopTime));
        MD.CurSpeedR = 60000/((1156.68/EncData.DeltaR)*(MotorSpeedLoopTime));

        MD.FilteredL = Filter(MD.FilteredL, MD.CurSpeedL);

        InputL = MD.CurSpeedL; //PID Input speed
        InputR = MD.CurSpeedR; //PID Input speed 

        EncData.OldLPos = EncData.NewLPos;
        EncData.OldRPos = EncData.NewRPos;

        LastSpeedLoop = millis();
    }

}

void Coms() //For Yashwin
{

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
    MD.DirectionR = CC;

    MD.SetSpeedL = 150;
    MD.SetSpeedR = 150;
}

void Mode(int mode)
{
    switch (mode)
    {
        //Constant Speed Mode
        case 1:
            //ConstantSpeed();
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
        lcd.print("SL:"+(String)MD.FilteredL);
        
        lcd.setCursor(0,1 );
        lcd.print("SD:"+(String)MD.CurSpeedR);

        LastScreenLoop = millis();
    }

}


void setup()
{
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
    
    //Encoder Pins modes 
    pinMode(ENCLA,INPUT);
    pinMode(ENCLB,INPUT);
    pinMode(ENCRA,INPUT);
    pinMode(ENCRB,INPUT);
}

void loop()
{   
    LeftPID.Compute();
    RightPID.Compute();

    Encoder(); //Get current motor speed

    Coms(); //Get data from bluetooth
    
    GloveData(); //Compute Glove data

    IR(); //Get IR sensor reading

    Mode(1);

}

