#include "xInputNotificationParser.hpp"

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

  joyMin = joyLHori < joyMin ? joyLHori : joyMin;
  joyMax = joyLHori > joyMax ? joyLHori : joyMax;

  joyMin = joyLVert < joyMin ? joyLVert : joyMin;
  joyMax = joyLVert > joyMax ? joyLVert : joyMax;

  joyMin = joyRHori < joyMin ? joyRHori : joyMin;
  joyMax = joyRHori > joyMax ? joyRHori : joyMax;

  joyMin = joyRVert < joyMin ? joyRVert : joyMin;
  joyMax = joyRVert > joyMax ? joyRVert : joyMax;

  joyMid = (joyMin + joyMax) / 2;

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
  outOfDate = true;
  return 0;
}

string XboxControllerNotificationParser::toString()
{
  return string("");
}
