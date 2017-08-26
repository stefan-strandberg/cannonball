#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "Dashboard.hpp"

// Store 7 seg number formats
unsigned char numbers[] = { 0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6, 0x00 };

/* Constructor */
Dashboard::Dashboard() {
}

bool Dashboard::begin(uint8_t addr) {

  _i2caddr = addr;
  _frame = 0;
  _module = wiringPiI2CSetup(_i2caddr);

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

  // set each led to 0 PWM
  clearAll(); 

  // Don't sync audio
  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_AUDIOSYNC, 0x0);

  return true;
}

void Dashboard::clearAll(void) {
  clearTacho();
  clearFuel();
  clearTurbo();
  clearSpeed();
  return;
}

void Dashboard::updateTacho(uint8_t revs) {
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
  if (speed < 0) speed = 0;
  if (speed > 999) speed = 999;
  
  // Hundreds
  if (speed > 99){
    uint8_t hundreds = speed / 100 % 10;
    drawNumber(DASH_SPEED_100_Y, hundreds);
  } else {
    drawNumber(DASH_SPEED_100_Y, DASH_NUMBER_EMPTY);
  }

  // Tens
  if (speed > 9){
    uint8_t tens = speed / 10 % 10;
    drawNumber(DASH_SPEED_10_Y, tens);
  } else {
    drawNumber(DASH_SPEED_10_Y, DASH_NUMBER_EMPTY);
  }

  // Ones
  if (speed > 0){
    uint8_t ones = speed % 10;
    drawNumber(DASH_SPEED_1_Y, ones);
  } else {
    drawNumber(DASH_SPEED_1_Y, DASH_NUMBER_EMPTY);
  }

  return;
}

void Dashboard::clearSpeed(void) {
  drawNumber(DASH_SPEED_100_Y, DASH_NUMBER_EMPTY);
  drawNumber(DASH_SPEED_10_Y, DASH_NUMBER_EMPTY);
  drawNumber(DASH_SPEED_1_Y, DASH_NUMBER_EMPTY);
  return;
}

/*************/

void Dashboard::drawNumber(uint16_t y, uint8_t num) {
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
  if ((x < 0) || (x >= 16)) return;
  if ((y < 0) || (y >= 9)) return;
  if (color > 255) color = 255; // PWM 8bit max
  
  setLEDPWM(_frame, x + y*16, color);

  return;
}

void Dashboard::selectBank(uint8_t b) {
  wiringPiI2CWriteReg8(_module, ISSI_COMMANDREGISTER, b);
}

void Dashboard::setLEDPWM(uint8_t bank, uint8_t lednum, uint8_t pwm) {
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
