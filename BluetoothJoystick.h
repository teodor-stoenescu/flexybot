#ifndef _BLUETOOTH_JOYSTICK_H
#define _BLUETOOTH_JOYSTICK_H


#if ARDUINO >= 100
 #include "Arduino.h"
 #include "Print.h"
#else
 #include "WProgram.h"
#endif

#include "SoftwareSerial.h"

#define    STX          0x02
#define    ETX          0x03
#define    SLOW         750                            // Datafields refresh rate (ms)
#define    FAST         250                             // Datafields refresh rate (ms)

class BluetoothJoystick {
protected :
    byte buttonStatus;                                  // first Byte sent to Android device
    long sendInterval;                               // interval between Buttons status transmission (milliseconds)
    
    struct Variant {
        union {
            int asInt;
            float asFloat;
            String *asString;
        } data;
        byte type;
    };
    
    virtual Variant *GetCustomField(byte idx) {
        return NULL;
    }
    
    virtual void ButtonNotification(byte bStatus) { }
    virtual void JoystickNotification(int joyX, int joyY) { }
private :
    SoftwareSerial *port; 
    byte cmd[8];
    long previousMillis;                                // will store last time Buttons status was updated
    //String displayStatus = "lol";                          // message to Android device

    String GetButtonStatusString()  {
        String bStatus = "";
        for (int i = 0; i < 6; i++)  {
            bStatus += (buttonStatus & (B100000 >> i)) ? "1" : "0";
        }
        return bStatus;
    }
    
    void SendCustomField(Variant *v) {
        if (NULL == v) {
            port->print("N/A");
        }
        
        switch (v->type) {
            case 0:
                port->print(v->data.asInt);
                break;
            case 1:
                port->print(v->data.asFloat);
                break;
            case 2:
                port->print(*(v->data.asString));
                break;
        }
    }
    
    void GetJoystickState(byte data[8])    {
        int joyX = (data[1] - 48) * 100 + (data[2] - 48) * 10 + (data[3] - 48); // obtain the Int from the ASCII representation
        int joyY = (data[4] - 48) * 100 + (data[5] - 48) * 10 + (data[6] - 48);
        joyX = joyX - 200;                                                  // Offset to avoid
        joyY = joyY - 200;                                                  // transmitting negative numbers

        if (joyX < -100 || joyX > 100 || joyY < -100 || joyY > 100) {
            return; // commmunication error
        }
        
        JoystickNotification(joyX, joyY);
    }
    
    void SendBluetoothData()  {
        static long previousMillis = 0;
        long currentMillis = millis();
        if (currentMillis - previousMillis > sendInterval) {  // send data back to smartphone
            previousMillis = currentMillis;

            // Data frame transmitted back from Arduino to Android device:
            // < 0X02   Buttons state   0X01   DataField#1   0x04   DataField#2   0x05   DataField#3    0x03 >
            // < 0X02      "01011"      0X01     "120.00"    0x04     "-4500"     0x05  "Motor enabled" 0x03 >    // example
    
            port->print((char)STX);                                           // Start of Transmission
            port->print(GetButtonStatusString());   port->print((char)0x1);   // buttons status feedback
            SendCustomField(GetCustomField(0));     port->print((char)0x4);   // datafield #1
            SendCustomField(GetCustomField(1));     port->print((char)0x5);   // datafield #2
            SendCustomField(GetCustomField(2));                               // datafield #3
            port->print((char)ETX);                                           // End of Transmission
        }
}

public :
    BluetoothJoystick(SoftwareSerial *p) {
        port = p;
        memset(cmd, 0, sizeof(cmd));
        buttonStatus = 0;
        previousMillis = 0;
        sendInterval = SLOW;
    }
    
    void Setup() {
        port->begin(115200);
        port->print("$");
        port->print("$");
        port->print("$");
        delay(100);
        port->println("U,9600,N");
        port->begin(9600);  // Start bluetooth serial at 9600                            // 57600 = max value for softserial
    }

    void Loop() {
        if (port->available())  {                          // data received from smartphone
            delay(2);
            cmd[0] = port->read();
            if (cmd[0] == STX)  {
                int i = 1;
                while (port->available())  {
                    delay(1);
                    cmd[i] = port->read();
                    if (cmd[i] > 127 || i > 7) {
                        break; // Communication error
                    }
                    if ((cmd[i] == ETX) && (i == 2 || i == 7)) {
                        break; // Button or Joystick data
                    }
                    i++;
                }
                if (i == 2) {
                    ButtonNotification(cmd[1]);  // 3 Bytes  ex: < STX "C" ETX >
                } else if (i == 7) {
                    GetJoystickState(cmd);  // 6 Bytes  ex: < STX "200" "180" ETX >
                }
            }
        }
        SendBluetoothData();
    }
};




#endif
