#ifndef __XBOX_CONTROLLER_NOTIFICATION_PARSER_H__
#define __XBOX_CONTROLLER_NOTIFICATION_PARSER_H__

#include <iostream>
using namespace std;
#define XBOX_CONTROLLER_ERROR_INVALID_LENGTH 1

//--------------------------------------------------------------------------------
// Generic Desktop Page inputReport (Device --> Host)
//--------------------------------------------------------------------------------

typedef struct
{
    // No REPORT ID byte
    // Collection: CA:GamePad CP:
    uint16_t joyLX;  // Usage 0x00010030: X, Value = 0 to 65535
    uint16_t joyLYt; // Usage 0x00010031: Y, Value = 0 to 65535
    uint16_t joyRX;  // Usage 0x00010033: Rx, Value = 0 to 65535
    uint16_t joyRY;  // Usage 0x00010034: Ry, Value = 0 to 65535

    // Collection: CA:GamePad
    uint16_t LT : 10; // Usage 0x00010032: LT, Value = 0 to 1023
    uint16_t : 6;     // Pad

    uint16_t RT : 10; // Usage 0x00010035: RT, Value = 0 to 1023
    uint16_t : 6;     // Pad

    /**
     * none:        00000000:0
     * up:          00000001:1
     * up+right:    00000010:2
     * right:       00000011:3
     * down+right:  00000100:4
     * down:        00000101:5
     * down+left:   00000110:6
     * left:        00000111:7
     * up+left:     00001000:8
     */
    uint8_t btnDir : 4; // Value = 0 to 8
    uint8_t : 4;        // Pad

    uint8_t btnA : 1;  // Value = 0 to 1
    uint8_t btnB : 1;  // Value = 0 to 1
    uint8_t : 1;       // Pad
    uint8_t btnX : 1;  // Value = 0 to 1
    uint8_t btnY : 1;  // Value = 0 to 1
    uint8_t : 1;       // Pad
    uint8_t btnLB : 1; // Value = 0 to 1
    uint8_t btnRB : 1; // Value = 0 to 1

    uint8_t : 2;           // Pad
    uint8_t btnSelect : 1; // Value = 0 to 0
    uint8_t btnStart : 1;  // Value = 0 to 0
    uint8_t btnXbox : 1;   // Value = 0 to 0
    uint8_t btnLS : 1;     // Value = 0 to 0
    uint8_t btnRS : 1;     //
    uint8_t : 1;           // Pad

    uint8_t btnShare : 1;
    uint8_t : 7; // Pad
} XboxController_input_report_t;

//--------------------------------------------------------------------------------
// Physical Interface Device Page outputReport (Device <-- Host)
//--------------------------------------------------------------------------------
typedef struct
{
    // No REPORT ID byte
    // Collection: CA:GamePad CL:
    uint8_t PID_GamePadDcEnableActuators : 4; // Usage 0x000F0097: DC Enable Actuators, Value = 0 to 1
    uint8_t : 4;                              // Pad
    uint8_t PID_GamePadMagnitude[4];          // Usage 0x000F0070: Magnitude, Value = 0 to 100
    uint8_t PID_GamePadDuration;              // Usage 0x000F0050: Duration, Value = 0 to 255, Physical = Value in 10⁻² s units
    uint8_t PID_GamePadStartDelay;            // Usage 0x000F00A7: Start Delay, Value = 0 to 255, Physical = Value in 10⁻² s units
    uint8_t PID_GamePadLoopCount;             // Usage 0x000F007C: Loop Count, Value = 0 to 255
} XboxController_outputReport_t;

class XboxControllerNotificationParser
{
public:
    XboxControllerNotificationParser();

    bool btnA, btnB, btnX, btnY;
    bool btnShare, btnStart, btnSelect, btnXbox;
    bool btnLB, btnRB, btnLS, btnRS;
    bool btnDirUp, btnDirLeft, btnDirRight, btnDirDown;
    uint16_t joyLHori, joyLVert, joyRHori, joyRVert, trigLT, trigRT;
    uint8_t update(uint8_t *data, size_t length);
    string toString();

    static const uint16_t maxJoy = 0xffff;
};

#endif
