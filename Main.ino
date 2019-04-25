#include "Main.h"

void IR() //For Hope
{


}

void Motor(int Speed, int Direction)
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
    Motor(150, CC);
}

void FusionMode()
{

}


void GloveData()
{

}

void Menu(int mode)
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


}

void loop()
{
    Coms();
    IR();

    Menu();

}

