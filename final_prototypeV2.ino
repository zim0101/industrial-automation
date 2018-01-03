//==========================================================
//include libraries:

#include "HX711.h"
//HX711 pin define
#define DOUT  3
#define CLK  2

HX711 scale(DOUT, CLK);
//calibration factor
#define calibration_factor 450

// pre-defined motor function
#define BRAKE 0
#define CW    1
#define CCW   2
#define MOTOR_1 0
// Definition of safety current
#define CS_THRESHOLD 15
//==========================================================
//-----------------FOLLOW THE PIN NUMBER ===================
//==========================================================
//define MOTOR 1 pin connection
#define MOTOR_A1_PIN 6
#define MOTOR_B1_PIN 7

#define CURRENT_SEN_1 A2
#define EN_PIN_1 A0

#define PWM_MOTOR_1 5

//define relay pin
#define relayPin 4

//define switches
#define limit_switch1 12
#define limit_switch2 13
//==========================================================
//default motor speed
// short usSpeed = 220;
unsigned short usMotor_Status = BRAKE;
//==========================================================

//define switch state
int switch_output1 = 0;
int switch_output2 = 0;
//==========================================================

int motorPin1 = 8;
int motorPin2 = 9;
int motorPin3 = 10;
int motorPin4 = 11;
int delayTime = 5;
int stepCount = 0;

void setup()
{
    // serial com baudrate = 9600
    Serial.begin(9600);

    scale.set_scale(calibration_factor);
    //scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0

    Serial.println("Readings:");

    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    pinMode(motorPin3, OUTPUT);
    pinMode(motorPin4, OUTPUT);

    //define motor pins as output
    pinMode(MOTOR_A1_PIN, OUTPUT);
    pinMode(MOTOR_B1_PIN, OUTPUT);
    pinMode(PWM_MOTOR_1, OUTPUT);
    pinMode(CURRENT_SEN_1, OUTPUT);
    pinMode(EN_PIN_1, OUTPUT);

    //define relaypin as output
    pinMode(relayPin, OUTPUT);

    //define switches as input
    pinMode(limit_switch1, INPUT);
    pinMode(limit_switch2, INPUT);

    digitalWrite(relayPin, LOW);

    Serial.print("System initializing [ |");
    for(int i = 0; i <= 8; i++)
    {
        Serial.print("|");
        if(i==8)
        {
            Serial.println("| ]");
        }

        delay(1000);
    }

    Serial.println("System is ready");

    Serial.print("Preparing Bluetooth communication: [");
    for(int i = 0; i <= 8; i++)
    {
        Serial.print("|");
        if(i==8)
        {
            Serial.println("| ]");
        }

        delay(1000);
    }

    Serial.println("Bluetooth communication is ready");
}

void loop()
{
    float previous_weight = 0;
    float new_weight = 0;
    float difference = 0;
    float weight = 0;
    float lastWeight = 0;
    float x;

    previous_weight = measureWeight();
    //============================ LS2 ACTIVATED ===========================
    turnON_Ls2();
    //============================ RUN stepper =============================
    stepperGo();
    //============================ SystemName: CHECK STATE==================
    if(measureWeight() < 30)
    {
        systemTurnOFF();
        while(1)
        {
            if(measureWeight() > 30)
            {
                //Serial.println("Weight == TRUE");
                break;
            }
        }
    }
    //======================== AUTOMATION PART ============================
    else if(measureWeight() > 30)
    {
        x = measureWeight() - 109;
        while(measureWeight() >= x)
        {
            weight = measureWeight();
            digitalWrite(relayPin, HIGH);
            Serial.print(weight); Serial.print("          ");
            Serial.println(x);
        }

        digitalWrite(relayPin, LOW);
    }
    //========================== 2 sec delay ===============================
    delay(2000);

    //=========================== LS1 is ON ================================
    turnON_Ls1();
    //========================= wait 60 sec ================================
    for(int x = 0; x <= 10; x++)
    {
        delay(950);
    }

    lastWeight = measureWeight();
    float weightDumped = previous_weight - lastWeight;
    Serial.print("Weight = "); Serial.print("                    ");
    Serial.print(measureWeight()); Serial.print("                    ");
    Serial.print("Weight dumped = ");  Serial.print("                    ");
    Serial.println(weightDumped);
}

void turnON_Ls2()
{
    int flag = 0;
    float weight;
    int globalSwitch_info = checkSwitch_state();
    //make: LS2 == ON
    if(checkSwitch_state() == 0 || checkSwitch_state() == 1)
    {
        while(1)
        {
            Forward();
            if(checkSwitch_state() == 2)
            {
                Serial.println("LS2 == ON");
                flag = 1;
                Stop();
                break;
            }
        }
    }
    else if(checkSwitch_state() == 2)
    {
        flag = 1;
        Stop();
    }
}

void systemTurnOFF()
{
    Stop();
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
}
void turnON_Ls1()
{
    int flag = 0;
    float weight;
    int globalSwitch_info = checkSwitch_state();

    if(checkSwitch_state() == 2 || checkSwitch_state() == 0)
    {
        while(1)
        {
            Reverse();
            if(checkSwitch_state() == 1)
            {
                Serial.println("LS1 == ON");
                flag = 0;
                Stop();
                break;
            }
        }
    }
}

void stepperGo()
{
  for(int j = 0; j <=2000; j++)
  {
    digitalWrite(motorPin4, HIGH);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin1, LOW);
    delay(delayTime);
    digitalWrite(motorPin4, LOW);
    digitalWrite(motorPin3, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin1, LOW);
    delay(delayTime);
    digitalWrite(motorPin4, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin2, HIGH);
    digitalWrite(motorPin1, LOW);
    delay(delayTime);
    digitalWrite(motorPin4, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin1, HIGH);
    delay(delayTime);
  }
}
//==========================================================
float measureWeight()
{
    float zIndex = scale.get_units();
    float yIndex = zIndex *(-2);
    float weight = zIndex + yIndex - 600;
    Serial.println(weight);

    return weight;
}
//==========================================================
int checkSwitch_state()
{
    int switchFlag = 0;

    switch_output1 = digitalRead(limit_switch1);
    switch_output2 = digitalRead(limit_switch2);
    delay(10);

    if(switch_output1 == 1 && switch_output2 ==1)
    {
        //Serial.println("LS1 == OFF and LS2 ==OFF");
        switchFlag = 0;
    }
    else if(switch_output1 == 0 && switch_output2 == 1)
    {
        //Serial.println("LS1 == ON");
        switchFlag = 1;
    }
    else if(switch_output2 == 0 && switch_output1 ==1)
    {
        //Serial.println("LS2 == ON");
        switchFlag = 2;
    }

    return switchFlag;
}
//==========================================================
void Stop()
{
    Serial.print("STOPED");
    digitalWrite(EN_PIN_1, LOW);
    digitalWrite(MOTOR_A1_PIN, LOW);
    digitalWrite(MOTOR_B1_PIN, LOW);
    analogWrite(PWM_MOTOR_1, 0);
}
//===========================================================
void Forward()
{
    Serial.print("Forward");
    digitalWrite(EN_PIN_1, HIGH);
    digitalWrite(MOTOR_B1_PIN, HIGH);
    analogWrite(PWM_MOTOR_1, 250);
    digitalWrite(MOTOR_A1_PIN, LOW);
}
//===========================================================
void Reverse()
{
    Serial.print("Backward");
    digitalWrite(EN_PIN_1, HIGH);
    digitalWrite(MOTOR_A1_PIN, HIGH);
    analogWrite(PWM_MOTOR_1, 250);
    digitalWrite(MOTOR_B1_PIN, LOW);
}
//===========================================================
