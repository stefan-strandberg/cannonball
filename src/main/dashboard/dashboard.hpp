#ifndef _DASHBOARD_H_
#define _DASHBOARD_H_

#include <stdint.h>
#include <wiringPiI2C.h>

#define ISSI_ADDR_DEFAULT 0x74

#define ISSI_REG_CONFIG  0x00
#define ISSI_REG_CONFIG_PICTUREMODE 0x00
#define ISSI_REG_CONFIG_AUTOPLAYMODE 0x08
#define ISSI_REG_CONFIG_AUDIOPLAYMODE 0x18

#define ISSI_CONF_PICTUREMODE 0x00
#define ISSI_CONF_AUTOFRAMEMODE 0x04
#define ISSI_CONF_AUDIOMODE 0x08

#define ISSI_REG_PICTUREFRAME  0x01

#define ISSI_REG_SHUTDOWN 0x0A
#define ISSI_REG_AUDIOSYNC 0x06

#define ISSI_COMMANDREGISTER 0xFD
#define ISSI_BANK_FUNCTIONREG 0x0B    // helpfully called 'page nine'

#define DASH_SEVEN_SEG_PWM 255
#define DASH_LED_PWM 120

#define DASH_TACHO_Y 0
#define DASH_FUEL_Y 1
#define DASH_TURBO_Y 2
#define DASH_SPEED_100_Y 3
#define DASH_SPEED_10_Y 4
#define DASH_SPEED_1_Y 5

#define DASH_NUMBER_EMPTY 10

class Dashboard {
 public:
  static const uint8_t MAX_TACHO_REVS = 8;
  static const uint8_t MAX_FUEL_LEVEL = 5;

  Dashboard(void);
  ~Dashboard(void);

  bool init(uint8_t addr = ISSI_ADDR_DEFAULT);
  void clearAll(void);

  void updateTacho(uint8_t revs);
  void clearTacho(void);

  void updateFuel(uint8_t level);
  void clearFuel(void);

  void updateTurbo(bool enabled);
  void clearTurbo(void);

  void updateSpeed(uint16_t speed);
  void clearSpeed(void);

 protected:
  void selectBank(uint8_t bank);
  void drawNumber(uint16_t y, uint8_t num);
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void setLEDPWM(uint8_t bank, uint8_t lednum, uint8_t pwm);
  void writeRegister8(uint8_t bank, uint8_t reg, uint8_t data);
  uint8_t readRegister8(uint8_t bank, uint8_t reg);
  uint8_t _i2caddr, _frame, _lastBank, _last100, _last10, _last1;
  unsigned char _ledStates[6];
  bool _inited;
  int _module;
};

extern Dashboard dashboard;

#endif
