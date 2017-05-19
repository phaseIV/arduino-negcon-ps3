#include "UnoJoy.h"
#include <PS2X_lib.h>

// uncomment for debug messages
//#define PRINTDEBUG

#define PS2_DAT        11  //14    
#define PS2_CMD        9  //15
#define PS2_SEL        8  //16
#define PS2_CLK        10  //17

#define pressures   false
#define rumble      false

PS2X ps2x; 

int error = 0;
byte type = 0;
byte vibrate = 0;

boolean myL1 = false;
boolean myR1 = false;
boolean myPadUp = false;
boolean myStart = false;
boolean myTriangle = false;
boolean mySquare = false;
boolean myCircle = false;
boolean myCross = false;
boolean mySelect = false;
boolean myHome = false;
boolean myWipeoutPad = true;
boolean result = true;

int myLimit = 100;
int mySteer = 128;
int myDeadzone = 12;

#ifdef PRINTDEBUG
unsigned long start;
unsigned long end;
unsigned long delta;
int counter = 0;
#endif


void setup()
{
    setupPins();
    setupUnoJoy();
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);

#ifdef PRINTDEBUG
    start = millis();
#endif
}

void loop()
{
    dataForController_t controllerData = getControllerData();
    setControllerData(controllerData);

#ifdef PRINTDEBUG
    end = millis();
    delta = end - start;
    if( delta > 1000 ) {
        start = millis();
        Serial.println(counter);
        counter = 0;
    }
#endif
}

void setupPins(void)
{
    for (int i = 2; i <= 12; i++){
        pinMode(i, INPUT);
        digitalWrite(i, HIGH);
    }
    pinMode(A4, INPUT);
    digitalWrite(A4, HIGH);
    pinMode(A5, INPUT);
    digitalWrite(A5, HIGH);
}

dataForController_t getControllerData(void)
{  
    dataForController_t controllerData = getBlankDataForController();
    result = ps2x.read_gamepad(false, vibrate);

#ifdef PRINTDEBUG
    //if(result)
        counter ++;
#endif

    if(ps2x.Button(PSB_TRIANGLE)) myTriangle = true; else myTriangle = false;
    if(ps2x.Button(PSB_CIRCLE)) myCircle = true; else myCircle = false;
    if(ps2x.Button(PSB_R1)) myR1 = true; else myR1 = false;
    if(ps2x.Button(PSB_START)) myStart = true; else myStart = false;
    if(ps2x.Button(PSB_PAD_UP)) myPadUp = true; else myPadUp = false;
    if(ps2x.Analog(PSS_LY) > myLimit) myL1 = true; else myL1 = false;
    if(ps2x.Analog(PSS_RY) > myLimit) myCross = true; else myCross = false;
    if(ps2x.Analog(PSS_LX) > myLimit) mySquare = true; else mySquare = false;

    // select button key combo (is it needed?)
    if(ps2x.Button(PSB_START) && ps2x.Button(PSB_PAD_UP)) {
        myStart = false;
        myPadUp = false;
        mySelect = true;
    } else {
        mySelect = false;
    }

    // home button key combo
    if(ps2x.Button(PSB_START) && ps2x.ButtonPressed(PSB_R1) && ps2x.Analog(PSS_LY) > myLimit) {
        myStart = false;
        myR1 = false;
        myL1 = false;
        myHome = true;
    } else {
        myHome = false;
    }

    // swap directional pad layout key combo
    if(ps2x.Button(PSB_START) && ps2x.ButtonPressed(PSB_R1)) {
        myWipeoutPad = !myWipeoutPad;
#ifdef PRINTDEBUG
        Serial.println("DPAD layout swapped");
#endif
    }

    //tune twist values
    mySteer = ps2x.Analog(PSS_RX);
    mySteer -= 128;
    //chop deadzone
    if(mySteer < 0) mySteer -= myDeadzone;
    if(mySteer > 0) mySteer += myDeadzone;
    //reduce maximum twist
    mySteer *= 1.65;
    //validate values
    mySteer += 128;
    if(mySteer < 0) mySteer = 0;
    if(mySteer > 255) mySteer = 255;


#ifdef PRINTDEBUG
    if(ps2x.Button(PSB_START)) Serial.println("Start is being held");
    if(ps2x.Button(PSB_PAD_UP)) Serial.println("UP held");
    if(ps2x.Button(PSB_PAD_RIGHT)) Serial.println("RIGHT held");
    if(ps2x.Button(PSB_PAD_LEFT)) Serial.println("LEFT held");
    if(ps2x.Button(PSB_PAD_DOWN)) Serial.println("DOWN held");
    if(ps2x.Button(PSB_TRIANGLE)) Serial.println("Triangle held");
    if(ps2x.Button(PSB_CIRCLE)) Serial.println("Circle held");
    if(ps2x.Button(PSB_SQUARE)) Serial.println("Square held");
    if(ps2x.Button(PSB_CROSS)) Serial.println("Cross held");

    if(ps2x.Button(PSB_R1)) // print stick values if R1 is pressed
    {
        Serial.print("Stick Values:");
        Serial.print(ps2x.Analog(PSS_LY), DEC); //Left stick, Y axis. Other options: LX, RY, RX
        Serial.print(",");
        Serial.print(ps2x.Analog(PSS_LX), DEC);
        Serial.print(",");
        Serial.print(ps2x.Analog(PSS_RY), DEC);
        Serial.print(",");
        Serial.print(ps2x.Analog(PSS_RX), DEC);
        Serial.print(",");
        Serial.println(mySteer);
    }
#endif

    // remap righthand buttons, to race with digital buttons
    controllerData.triangleOn = myCross; //myTriangle; //ps2x.Button(PSB_TRIANGLE);
    controllerData.circleOn = mySquare; //myCircle; //ps2x.Button(PSB_CIRCLE);
    controllerData.squareOn = myCircle; //mySquare; //ps2x.Button(PSB_SQUARE);
    controllerData.crossOn = myTriangle; //myCross; //ps2x.Button(PSB_CROSS);

    // directional pad behaves differently in-game (Wipout HD/Fury)
    if(!myWipeoutPad) {
        controllerData.dpadUpOn = myPadUp;
        controllerData.dpadDownOn = ps2x.Button(PSB_PAD_DOWN);
        controllerData.dpadLeftOn = ps2x.Button(PSB_PAD_LEFT);
        controllerData.dpadRightOn = ps2x.Button(PSB_PAD_RIGHT);
    } else {
        controllerData.dpadUpOn = ps2x.Button(PSB_PAD_RIGHT);
        controllerData.dpadDownOn = myPadUp;
        controllerData.dpadLeftOn = ps2x.Button(PSB_PAD_DOWN);
        controllerData.dpadRightOn = ps2x.Button(PSB_PAD_LEFT);
    }

    controllerData.l1On = myL1;
    controllerData.r1On = myR1;
    controllerData.selectOn = mySelect;
    controllerData.startOn = myStart;
    controllerData.homeOn = myHome;

    controllerData.leftStickX = mySteer;
    controllerData.leftStickY = 128;
    controllerData.rightStickX = 128;
    controllerData.rightStickY = 128;

    return controllerData;
}
