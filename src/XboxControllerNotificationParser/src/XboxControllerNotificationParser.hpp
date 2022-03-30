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
  uint16_t GD_GamePadX;  // Usage 0x00010030: X, Value = 0 to 65535
  uint16_t GD_GamePadY;  // Usage 0x00010031: Y, Value = 0 to 65535
  uint16_t GD_GamePadRx; // Usage 0x00010033: Rx, Value = 0 to 65535
  uint16_t GD_GamePadRy; // Usage 0x00010034: Ry, Value = 0 to 65535

  // Collection: CA:GamePad
  uint16_t GD_GamePadZ : 10; // Usage 0x00010032: Z, Value = 0 to 1023
  uint8_t : 6;               // Pad

  uint16_t GD_GamePadRz : 10; // Usage 0x00010035: Rz, Value = 0 to 1023
  uint8_t : 6;                // Pad

  uint8_t btnA : 1;      // Usage 0x00090001: Button 1 Primary/trigger, Value = 0 to 0
  uint8_t btnB : 1;      // Usage 0x00090002: Button 2 Secondary, Value = 0 to 0
  uint8_t btnX : 1;      // Usage 0x00090003: Button 3 Tertiary, Value = 0 to 0
  uint8_t btnY : 1;      // Usage 0x00090004: Button 4, Value = 0 to 0
  uint8_t btnLB : 1;     // Usage 0x00090005: Button 5, Value = 0 to 0
  uint8_t btnRB : 1;     // Usage 0x00090006: Button 6, Value = 0 to 0
  uint8_t btnSelect : 1; // Usage 0x00090007: Button 7, Value = 0 to 0
  uint8_t btnStart : 1;  // Usage 0x00090008: Button 8, Value = 0 to 0

  uint8_t btnXbox : 1; // Usage 0x00090009: Button 9, Value = 0 to 0
  uint8_t btnLS : 1;   // Usage 0x0009000A: Button 10, Value = 0 to 0
  uint8_t btnRS : 1;
  uint8_t : 5;                     // Pad
  uint8_t GD_GamePadHatSwitch : 4; // Usage 0x00010039: Hat switch, Value = 1 to 8, Physical = (Value - 1) x 45 in degrees
  uint8_t : 4;                     // Pad

  // Collection: CA:GamePad CP:SystemControl
  uint8_t GD_GamePadSystemControlSystemMainMenu : 1; // Usage 0x00010085: System Main Menu, Value = 0 to 1
  uint8_t : 7;                                       // Pad

  // Collection: CA:GamePad
  uint8_t GEN_GamePadBatteryStrength; // Usage 0x00060020: Battery Strength, Value = 0 to 255
} inputReport_t;

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
} outputReport_t;
typedef struct
{
  // No REPORT ID byte
  // Collection: CA:GamePad CL:
  uint8_t length;
  uint8_t data[16]; // Usage 0x000F0097: DC Enable Actuators, Value = 0 to 1
} input_report_raw_t;
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
