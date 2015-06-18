#define VERSION     "\n\nAndroTest V2.0 - @kas2014\ndemo for V5.x App"

// V2.0  changed to pure ASCII Communication Protocol ** not backward compatible **
// V1.4  improved communication errors handling
// V1.3  renamed for publishing, posted on 09/05/2014
// V1.2  Text display   ** not backward compatible **
// V1.1  Integer display
// V1.0  6 buttons + 4 data char implemented

// Demo setup:
// Button #1 controls pin #13 LED
// Button #4 toggle datafield display rate
// Button #5 configured as "push" button (momentary)
// Other buttons display demo message

// Arduino pin#2 to TX BlueTooth module
// Arduino pin#3 to RX BlueTooth module
// make sure your BT board is set @57600 bps
// better remove SoftSerial for PWM based projects

#include <Math.h>
#include "SoftwareSerial.h"
#include "BluetoothJoystick.h"

#define    ledPin       13
#define    SPEED_LEFT        160
#define    SPEED_RIGHT       160


SoftwareSerial mySerial(4,2);
//BluetoothJoystick btApp(&mySerial);

class MyBt : public BluetoothJoystick {
private :
    String displayStatus;
    Variant v3;
protected :
    virtual Variant *GetCustomField(byte idx) {
        if (2 == idx) {
            return &v3;
        }
        return NULL;
    }

    virtual void ButtonNotification(byte bStatus) {
        switch (bStatus) {
            // -----------------  BUTTON #1  -----------------------
            case 'A':
                buttonStatus |= B000001;        // ON
                Serial.println("\n** Button_1: ON **");
                // your code...
                displayStatus = "LED <ON>";
                Serial.println(displayStatus);
                digitalWrite(ledPin, HIGH);
                break;
            case 'B':
                buttonStatus &= B111110;        // OFF
                Serial.println("\n** Button_1: OFF **");
                // your code...
                displayStatus = "LED <OFF>";
                Serial.println(displayStatus);
                digitalWrite(ledPin, LOW);
                break;
  
            // -----------------  BUTTON #2  -----------------------
            case 'C':
                buttonStatus |= B000010;        // ON
                Serial.println("\n** Button_2: ON **");
                // your code...
                displayStatus = "Button2 <ON>";
                Serial.println(displayStatus);
                break;
            case 'D':
                buttonStatus &= B111101;        // OFF
                Serial.println("\n** Button_2: OFF **");
                // your code...
                displayStatus = "Button2 <OFF>";
                Serial.println(displayStatus);
                break;
  
            // -----------------  BUTTON #3  -----------------------
            case 'E':
                buttonStatus |= B000100;        // ON
                Serial.println("\n** Button_3: ON **");
                // your code...
                displayStatus = "Motor #1 enabled"; // Demo text message
                Serial.println(displayStatus);
                break;
            case 'F':
                buttonStatus &= B111011;      // OFF
                Serial.println("\n** Button_3: OFF **");
                // your code...
                displayStatus = "Motor #1 stopped";
                Serial.println(displayStatus);
                break;
  
            // -----------------  BUTTON #4  -----------------------
            case 'G':
                buttonStatus |= B001000;       // ON
                Serial.println("\n** Button_4: ON **");
                // your code...
                displayStatus = "Datafield update <FAST>";
                Serial.println(displayStatus);
                sendInterval = FAST;
                break;
            case 'H':
                buttonStatus &= B110111;    // OFF
                Serial.println("\n** Button_4: OFF **");
                // your code...
                displayStatus = "Datafield update <SLOW>";
                Serial.println(displayStatus);
                sendInterval = SLOW;
                break;
  
            // -----------------  BUTTON #5  -----------------------
            case 'I':           // configured as momentary button
                //      buttonStatus |= B010000;        // ON
                Serial.println("\n** Button_5: ++ pushed ++ **");
                // your code...
                displayStatus = "Button5: <pushed>";
                break;
                //   case 'J':
                //     buttonStatus &= B101111;        // OFF
                //     // your code...
                //     break;
      
            // -----------------  BUTTON #6  -----------------------
            case 'K':
                buttonStatus |= B100000;        // ON
                Serial.println("\n** Button_6: ON **");
                // your code...
                displayStatus = "Button6 <ON>"; // Demo text message
                break;
            case 'L':
                buttonStatus &= B011111;        // OFF
                Serial.println("\n** Button_6: OFF **");
                // your code...
                displayStatus = "Button6 <OFF>";
                break;
        }
        // ---------------------------------------------------------------
    }
    virtual void JoystickNotification(int joyX, int joyY) {
        if ((0 == joyX) && (0 == joyY)) {
            go(0, 0);
            return;
        }
      
        double spd = sqrt(joyX * joyX + joyY * joyY);
        double dx = joyX / spd, dy = joyY / spd;
        int spdSel = spd / 20;
        double dir = atan2(dy, dx);
        int dirSel = (((dir / M_PI) + (0.04166)) * 12) + 11;
        
        Serial.print("spd: "); Serial.print(spdSel); Serial.print("  dir:"); Serial.println(dirSel);
      
        const double pwrLookup[] = {0.6, 0.7, 0.8, 0.9, 1.0};
      
        const double leftPwr[] = {
          -0.86603, -0.96593, -1.00000, -0.96593, -0.86603, -0.70711,
          -0.50000, -0.25882,  0.00000,  0.25882,  0.50000,  0.70711,
           0.86603,  0.96593,  1.00000,  0.96593,  0.86603,  0.70711,
           0.50000,  0.25882,  0.00000, -0.25882, -0.50000, -0.70711
        };
        
        const double rightPwr[] = {
           0.50000,  0.25882,  0.00000, -0.25882, -0.50000, -0.70711,
          -0.86603, -0.96593, -1.00000, -0.96593, -0.86603, -0.70711,
          -0.50000, -0.25882,  0.00000,  0.25882,  0.50000,  0.70711,
           0.86603,  0.96593,  1.00000,  0.96593,  0.86603,  0.70711
        };
        
        go((int)(leftPwr[dirSel] * pwrLookup[spdSel] * SPEED_LEFT),
           (int)(rightPwr[dirSel] * pwrLookup[spdSel] * SPEED_RIGHT));
    }
public :
    MyBt(SoftwareSerial *p) : BluetoothJoystick(p) {
        v3.type = 2;
        v3.data.asString = &displayStatus;
    }
};

MyBt btApp(&mySerial);

int MOTOR2_PIN1 = 3;
int MOTOR2_PIN2 = 5;
int MOTOR1_PIN1 = 6;
int MOTOR1_PIN2 = 9;


void setup()  {
  
  pinMode(MOTOR1_PIN1, OUTPUT);
  pinMode(MOTOR1_PIN2, OUTPUT);
  pinMode(MOTOR2_PIN1, OUTPUT);
  pinMode(MOTOR2_PIN2, OUTPUT);

  Serial.begin(9600);
  
  btApp.Setup();
  
  pinMode(ledPin, OUTPUT);
  Serial.println(VERSION);
  while (mySerial.available())  mySerial.read();        // empty RX buffer
}

void loop() {
    btApp.Loop();
}

void getButtonState(int bStatus)  {
  
}

void go(int speedLeft, int speedRight) {
  if (speedLeft > 0) {
    analogWrite(MOTOR1_PIN1, speedLeft);
    analogWrite(MOTOR1_PIN2, 0);
  }
  else {
    analogWrite(MOTOR1_PIN1, 0);
    analogWrite(MOTOR1_PIN2, -speedLeft);
  }

  if (speedRight > 0) {
    analogWrite(MOTOR2_PIN1, speedRight);
    analogWrite(MOTOR2_PIN2, 0);
  } else {
    analogWrite(MOTOR2_PIN1, 0);
    analogWrite(MOTOR2_PIN2, -speedRight);
  }
}






