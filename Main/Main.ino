#include "Main.h"
#include<Encoder.h>
#include <PID_v1.h>


Encoder myEncLeft(ENCLA, ENCLB);
Encoder myEncRight(ENCRA, ENCRB);

double SetpointL, InputL, OutputL;
double KpL=5, KiL=2.5, KdL=0.001;

double SetpointR, InputR, OutputR;
double KpR=5, KiR=2.5, KdR=0.001;

PID LeftPID(&InputL, &OutputL, &SetpointL, KpL, KiL, KdL, DIRECT);
PID RightPID(&InputR, &OutputR, &SetpointR, KpR, KiR, KdR, DIRECT);

struct GloveData
{
    int DirectionL, DirectionR, SpeedL , SpeedR;
};


struct GloveData gd;



void IR() //For Hope
{


}

void Motor(int SpeedL, int SpeedR, int DirectionL, int DirectionR)
{
    SetpointL = SpeedL;
    SetpointR = SpeedR;

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

}

void Coms() //For Yashwin
{

}

void Modes()
{

}


void ConstantSpeed()
{
    Motor(gd.SpeedL,gd.SpeedR, gd.DirectionL, gd.DirectionR);
}

void FusionMode()
{

    Motor(gd.SpeedL,gd.SpeedR, gd.DirectionL, gd.DirectionR);
}


void GloveData()
{
    //Logic to determine speed and direction of motors 


    gd.DirectionL = CC;
    gd.DirectionR = CC;

    gd.SpeedL = 150;
    gd.SpeedR = 150;
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

            FusionMode();
            break;
        default:
            break;
    }

}




void setup()
{
    LeftPID.SetMode(AUTOMATIC);
    LeftPID.SetSampleTime(1);

    RightPID.SetMode(AUTOMATIC);
    RightPID.SetSampleTime(1);
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

