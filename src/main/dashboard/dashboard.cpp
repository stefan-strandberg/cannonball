#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "dashboard.hpp"

// Store 7 seg number formats
unsigned char numbers[] = { 0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6, 0x00 };

Dashboard dashboard;

/* Constructor */
Dashboard::Dashboard(void) {
}

Dashboard::~Dashboard(void) {
}

bool Dashboard::init(uint8_t addr) {

  _i2caddr = addr;
  _frame = 0;
  _module = wiringPiI2CSetup(_i2caddr);

  // Init LED states
  _ledStates[0] = 0x00;
  _ledStates[1] = 0x00;
  _ledStates[2] = 0x00;
  _ledStates[3] = 0x00;
  _ledStates[4] = 0x00;
  _ledStates[5] = 0x00;

  // shutdown
  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x00);

  delay(10);

  // out of shutdown
  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x01);

  // picture mode
  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_CONFIG, ISSI_REG_CONFIG_PICTUREMODE);

  // Display frame 
  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_PICTUREFRAME, _frame);

  // Disable unused leds
  for(uint8_t f=0; f<8; f++){
    writeRegister8(f, 0x00, 0xFF); // Tacho
    writeRegister8(f, 0x02, 0x1F); // Fuel
    writeRegister8(f, 0x04, 0x01); // Turbo
    writeRegister8(f, 0x06, 0xFF); // Speed 100s
    writeRegister8(f, 0x08, 0xFF); // Speed 10s
    writeRegister8(f, 0x0A, 0xFF); // Speed 1s
    writeRegister8(f, 0x0C, 0x00);
    writeRegister8(f, 0x0E, 0x00);
    writeRegister8(f, 0x10, 0x00);

    writeRegister8(f, 0x01, 0x00);
    writeRegister8(f, 0x03, 0x00);
    writeRegister8(f, 0x05, 0x00);
    writeRegister8(f, 0x07, 0x00);
    writeRegister8(f, 0x09, 0x00);
    writeRegister8(f, 0x0B, 0x00);
    writeRegister8(f, 0x0D, 0x00);
    writeRegister8(f, 0x0F, 0x00);
    writeRegister8(f, 0x11, 0x00);
  }

  // Don't sync audio
  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_AUDIOSYNC, 0x0);

  // Set inited
  _inited = true;

  // set each led to 0 PWM
  clearAll(); 

  return true;
}

void Dashboard::clearAll(void) {
  if (!_inited) return;
  clearTacho();
  clearFuel();
  clearTurbo();
  clearSpeed();
  return;
}

void Dashboard::updateTacho(uint8_t revs) {
  if (!_inited) return;
  if (revs < 0) revs = 0;
  if (revs > MAX_TACHO_REVS) revs = MAX_TACHO_REVS;
  
  for(uint8_t x=0; x<8; x++){
    if (x + 1 <= revs){
      drawPixel(x, DASH_TACHO_Y, DASH_LED_PWM);
    } else {
      drawPixel(x, DASH_TACHO_Y, 0);
    }
  }

  return;
}

void Dashboard::clearTacho(void) {
  updateTacho(0);
  return;
}

void Dashboard::updateFuel(uint8_t level) {
  if (!_inited) return;
  if (level < 0) level = 0;
  if (level > MAX_FUEL_LEVEL) level = MAX_FUEL_LEVEL;
  
  for(uint8_t x=0; x<5; x++){
    if (x + 1 <= level){
      drawPixel(x, DASH_FUEL_Y, DASH_LED_PWM);
    } else {
      drawPixel(x, DASH_FUEL_Y, 0);
    }
  }

  return;
}

void Dashboard::clearFuel(void) {
  updateFuel(0);
  return;
}

void Dashboard::updateTurbo(bool enabled) {
  if (!_inited) return;
  if (enabled){
    drawPixel(0, DASH_TURBO_Y, DASH_LED_PWM);
  } else {
    drawPixel(0, DASH_TURBO_Y, 0);
  }
  return;
}

void Dashboard::clearTurbo(void) {
  updateTurbo(false);
  return;
}

void Dashboard::updateSpeed(uint16_t speed) {
  if (!_inited) return;
  if (speed < 0) speed = 0;
  if (speed > 999) speed = 999;

  // Hundreds
  uint8_t next100 = DASH_NUMBER_EMPTY;
  if (speed > 99) next100 = speed / 100 % 10;
  drawNumber(DASH_SPEED_100_Y, next100);

  // Tens
  uint8_t next10 = DASH_NUMBER_EMPTY;
  if (speed > 9) next10 = speed / 10 % 10;
  drawNumber(DASH_SPEED_10_Y, next10);

  // Ones
  uint8_t next1 = DASH_NUMBER_EMPTY;
  if (speed > 0) next1 = speed % 10;
  drawNumber(DASH_SPEED_1_Y, next1);

  return;
}

void Dashboard::clearSpeed(void) {
  if (!_inited) return;
  drawNumber(DASH_SPEED_100_Y, DASH_NUMBER_EMPTY);
  drawNumber(DASH_SPEED_10_Y, DASH_NUMBER_EMPTY);
  drawNumber(DASH_SPEED_1_Y, DASH_NUMBER_EMPTY);
  return;
}

/*************/

void Dashboard::drawNumber(uint16_t y, uint8_t num) {
  if (!_inited) return;
  if (y != DASH_SPEED_100_Y && y != DASH_SPEED_10_Y && y != DASH_SPEED_1_Y) return;
  if (num < 0) num = 0;
  if (num > 10) num = 10;
  
  for(uint8_t x=0; x<8; x++){
    unsigned char mask = 0x80 >> x;
    if (numbers[num] & mask){
      drawPixel(x, y, DASH_SEVEN_SEG_PWM);
    } else {
      drawPixel(x, y, 0);
    }
  }

  return;
}

void Dashboard::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (!_inited) return;
  if ((x < 0) || (x >= 9)) return;
  if ((y < 0) || (y >= 9)) return;
  if (color > 255) color = 255; // PWM 8bit max
  
  uint8_t mask = 0x80 >> x;
  if (color > 0 && (_ledStates[y] & mask) != mask){
    setLEDPWM(_frame, x + y*16, color);
    _ledStates[y] |= mask;
  }
  if (color == 0 && (_ledStates[y] & mask) == mask){
    setLEDPWM(_frame, x + y*16, color);
    _ledStates[y] &= ~(mask);
  }

  return;
}

void Dashboard::selectBank(uint8_t b) {
  if (!_inited) return;
  if (b != _lastBank){
    wiringPiI2CWriteReg8(_module, ISSI_COMMANDREGISTER, b);
    _lastBank = b;
  }
}

void Dashboard::setLEDPWM(uint8_t bank, uint8_t lednum, uint8_t pwm) {
  if (!_inited) return;
  if (lednum >= 144) return;
  writeRegister8(bank, 0x24+lednum, pwm);
}

void Dashboard::writeRegister8(uint8_t bank, uint8_t reg, uint8_t data) {
  selectBank(bank);
  wiringPiI2CWriteReg8(_module, reg, data);
}

uint8_t  Dashboard::readRegister8(uint8_t bank, uint8_t reg) {
  selectBank(bank);
  return wiringPiI2CReadReg8 (_module, reg);
}
