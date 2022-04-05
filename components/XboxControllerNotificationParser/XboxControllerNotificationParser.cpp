#include "XboxControllerNotificationParser.hpp"

#define XBOX_CONTROLLER_DATA_LEN 16
#define XBOX_CONTROLLER_INDEX_BUTTONS_DIR 12
#define XBOX_CONTROLLER_INDEX_BUTTONS_MAIN 13
#define XBOX_CONTROLLER_INDEX_BUTTONS_CENTER 14
#define XBOX_CONTROLLER_INDEX_BUTTONS_SHARE 15

XboxControllerNotificationParser::XboxControllerNotificationParser()
{
  btnA = btnB = btnX = btnY = false;
  btnShare = btnStart = btnSelect = btnXbox = false;
  btnLB = btnRB = btnLS = btnRS = false;
  btnDirUp = btnDirLeft = btnDirRight = btnDirDown = false;
  joyLHori = joyLVert = joyRHori = joyRVert = 0xffff / 2;
  trigLT = trigRT = 0;
}
// fd 8d 78 84 7a 83 bc 83 00 00 00 00 00 a0 d0 fd
uint8_t XboxControllerNotificationParser::update(uint8_t *data, size_t length)
{
  if (length != 16)
  {
    return XBOX_CONTROLLER_ERROR_INVALID_LENGTH;
  }
  uint8_t btnBits;

  joyLHori = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
  joyLVert = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
  joyRHori = (uint16_t)data[4] | ((uint16_t)data[5] << 8);
  joyRVert = (uint16_t)data[6] | ((uint16_t)data[7] << 8);
  trigLT = (uint16_t)data[8] | ((uint16_t)data[9] << 8);
  trigRT = (uint16_t)data[10] | ((uint16_t)data[11] << 8);

  btnBits = data[XBOX_CONTROLLER_INDEX_BUTTONS_DIR];
  btnDirUp = btnBits == 1 || btnBits == 2 || btnBits == 8;
  btnDirRight = 2 <= btnBits && btnBits <= 4;
  btnDirDown = 4 <= btnBits && btnBits <= 6;
  btnDirLeft = 6 <= btnBits && btnBits <= 8;

  btnBits = data[XBOX_CONTROLLER_INDEX_BUTTONS_MAIN];
  btnA = btnBits & 0b00000001;
  btnB = btnBits & 0b00000010;
  btnX = btnBits & 0b00001000;
  btnY = btnBits & 0b00010000;
  btnLB = btnBits & 0b01000000;
  btnRB = btnBits & 0b10000000;

  btnBits = data[XBOX_CONTROLLER_INDEX_BUTTONS_CENTER];
  btnSelect = btnBits & 0b00000100;
  btnStart = btnBits & 0b00001000;
  btnXbox = btnBits & 0b00010000;
  btnLS = btnBits & 0b00100000;
  btnRS = btnBits & 0b01000000;

  btnBits = data[XBOX_CONTROLLER_INDEX_BUTTONS_SHARE];
  btnShare = btnBits & 0b00000001;
  return 0;
}

string XboxControllerNotificationParser::toString()
{
  string str = string("") +
               "btnY: " + to_string(btnY) + " " +
               "btnX: " + to_string(btnX) + " " +
               "btnB: " + to_string(btnB) + " " +
               "btnA: " + to_string(btnA) + " " +
               "btnLB: " + to_string(btnLB) + " " +
               "btnRB: " + to_string(btnRB) + "\n" +
               "btnSelect: " + to_string(btnSelect) + " " +
               "btnStart: " + to_string(btnStart) + " " +
               "btnXbox: " + to_string(btnXbox) + " " +
               "btnShare: " + to_string(btnShare) + " " +
               "btnLS: " + to_string(btnLS) + " " +
               "btnRS: " + to_string(btnRS) + "\n" +
               "btnDirUp: " + to_string(btnDirUp) + " " +
               "btnDirRight: " + to_string(btnDirRight) + " " +
               "btnDirDown: " + to_string(btnDirDown) + " " +
               "btnDirLeft: " + to_string(btnDirLeft) + "\n"
                                                        "joyLHori: " +
               to_string(joyLHori) + "\n" +
               "joyLVert: " + to_string(joyLVert) + "\n" +
               "joyRHori: " + to_string(joyRHori) + "\n" +
               "joyRVert: " + to_string(joyRVert) + "\n" +
               "trigLT: " + to_string(trigLT) + "\n" +
               "trigRT: " + to_string(trigRT) + "\n";
  return str;
}
