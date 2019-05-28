//#include <Stepper.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h> // Include Encoder Library

#define COM_PIN 6

#define S_INT1 8
#define S_INT2 8
#define S_INT3 8
#define S_INT4 8

//Dunker
#define D_DIR 27
#define D_STP 26
#define D_EN 25

//Hope pins
#define S_DIR 31
#define S_STP 30
#define S_DC_EN 29
#define S_EN 28

#define MilkPump 10

#define DC_EN 8
#define PUMP_EN 8

#define DunkerSPR 2000
#define StirrerSPR 4076



LiquidCrystal_I2C lcd(0x27,20,4);

byte mode = 0; // cycles through the modes, initialized at 0 so that the 'START' is the first mode

/*Define Modes*/
#define sugarCubes 0
#define milkUsed 1
define modeDone 2

/*Setup Rotary Encoder*/
Encoder knob(2,3);
volatile unsigned int modeCount = 0;
byte eSwitch;

int sugarCount = 0;
int milkCount = 0;
int doneCount = 0;
int newSugarcount =0;
int newMilkcount =0;
int newDone;
int counts2Done = 1;


int offsetCount = 0;
long looptime = 20;
long looptime2 = 1000;
long looptime3 = 20;
long lastmil = 0;
long lastmil2 = 0;
long lastmil3 = 0;
int btnFlag =0;

int COM_STATE;
int Step = 0;

int done = 1;

//Stepper dunkerStepper(DunkerSPR, D_INT1, D_INT2, D_INT3, D_INT4);
//Stepper stirrerStepper(StirrerSPR, S_INT1, S_INT2, S_INT3, S_INT4);


void getMilk()
{
  if (btnFlag == 1)
    {
      milkCount = constrain(milkCount + 1, 0, 20);
      btnFlag = 0;
    }
  else if(btnFlag == -1)
  {
    milkCount = constrain(milkCount - 1, 0, 20);
    btnFlag = 0;
  }
}

void getSugar()
{
  if (btnFlag == 1)
    {
      sugarCount = constrain(sugarCount + 1, 0, 5);
      btnFlag = 0;
    }
  else if(btnFlag == -1)
  {
    sugarCount = constrain(sugarCount - 1, 0, 5);
    btnFlag = 0;
  }
}

void getDone()
{
  if (btnFlag == 1)
  {
    doneCount = constrain(doneCount + 1, 0, 5);

    btnFlag = 0;
  }
  else if(btnFlag == -1)
  {
    doneCount = constrain(doneCount - 1, 0, 5);
    btnFlag = 0;
  }
      if (doneCount == 5)
    {
      counts2Done = 0;
      //Serial.println(counts2Done);
      if((millis()-lastmil3) >= looptime3)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("SYSTEM GO!");
        lastmil3 = millis();
      }
    }
}

void menu()
{  
    if (eSwitch == LOW && mode < 2) 
    {
      if((millis()-lastmil2) >= looptime2)
      {
        mode++;
        modeCount = 0;
        lastmil2 = millis();
      }
    }
    if (eSwitch == LOW && mode >= 2)
    {
      mode = 0;
    }
    if (mode == 0)
    {
      newSugarcount = modeCount;
    }
    if (mode == 1)
    {
      newMilkcount = modeCount;
    }
}

void milk(void) 
{  
    lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Milk:");
    lcd.setCursor (0,1);
    lcd.print (milkCount);
    lcd.setCursor (9,1);
    lcd.print ("ml");
    //lcd.clear();
  //menu();
}

void sugar(void) 
{
    lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Sugar Cubes:");
    lcd.setCursor (0,1);
    lcd.print (sugarCount);  
    lcd.setCursor (9,1);
    lcd.print ("Cubes");
    //lcd.clear();
  //menu();
}

void done (void)
{
  lcd.clear();
  lcd.setCursor (0,0);
  lcd.print ("Rotate Clockwise to Start"); 
  lcd.setCursor (0,1);
  lcd.print (doneCount);
}

void setup()
{
    //dunkerStepper.setSpeed(8);
    //stirrerStepper.setSpeed(8);
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    
    //Dunker
    pinMode(D_STP, OUTPUT);
    pinMode(D_DIR, OUTPUT);
    pinMode(D_EN, OUTPUT);
    
    pinMode(COM_PIN, INPUT);

    //Hope
    pinMode(S_DIR, OUTPUT);
    pinMode(S_STP, OUTPUT);
    pinMode(S_DC_EN, OUTPUT);

    pinMode(MilkPump, OUTPUT);
    
}
void loop()
{
    switch (Step)
    {
    case 0: //Menu

//    while(done)
//    {
//        eSwitch = digitalRead(1);
//        menu();
//        
//        offsetCount = knob.read();
//        if (offsetCount == 4)
//        {
//            knob.write(0);
//            btnFlag = 1;
//        }
//        else if (offsetCount == -4)
//        {
//            knob.write(0);
//            btnFlag = -1;
//        }
//
//        switch (mode) 
//        {
//            case sugarCubes: 
//            {
//            getSugar();
//            break;
//            }
//            case milkUsed: 
//            {
//            getMilk();
//            break;
//            }
//            case modeDone:
//            {
//            getDone();
//            break;
//            }
//        }
//        
//        if((millis()-lastmil) >= looptime)
//        {
//            switch (mode) 
//            {
//            case sugarCubes: 
//            {
//                sugar();
//                break;
//            }
//            case milkUsed: 
//            {
//                milk();
//                break;
//            }
//            case modeDone:
//            {
//            done();
//            break;
//            }
//            }
//            lastmil = millis();
//        }
    

        COM_STATE = digitalRead(COM_PIN);
    
        COM_STATE =1;

        if(COM_STATE == 1)
        {   
          Step++;
        }

        
        break;
        
        case 1: //Sugar 

        
        Step++;
        break;
    
        case 2: //Teabag 
        
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
        
        
        Step++;
        break;
    
        case 3: //Milk

        
        
        Step++;
        break;
        
        case 4: //stur tea
        
        Step++;
        break;

        default:
            break;
        }

    }


