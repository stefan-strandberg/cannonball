/***************************************************************************
    Process Outputs.

    - Cabinet Vibration & Hydraulic Movement
    - Brake & Start Lamps
    - Coin Chute Outputs
    
    The Deluxe Motor code is also used by the force-feedback haptic system.

    One thing to note is that this code was originally intended to drive
    a moving hydraulic cabinet, not to be mapped to a haptic device.

    Therefore, it's not perfect when used in this way, but the results
    aren't bad :)
    
    Copyright Chris White.
    See license.txt for more details.

    [MPB] Completely stripped back for use with simple haptic motor
***************************************************************************/

#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "engine/outrun.hpp"
#include "engine/ocrash.hpp"
#include "engine/oferrari.hpp"
#include "engine/ooutputs.hpp"

OOutputs::OOutputs(void)
{

}

OOutputs::~OOutputs(void)
{
}


// Initalize Moving Cabinet Motor
// Source: 0xECE8
void OOutputs::init()
{
  _module = wiringPiI2CSetup(DRV2605_ADDR);

  uint8_t id = readRegister8(DRV2605_REG_STATUS);
  
  writeRegister8(DRV2605_REG_MODE, 0x00); // out of standby
  
  writeRegister8(DRV2605_REG_RTPIN, 0x00); // no real-time-playback
  
  writeRegister8(DRV2605_REG_WAVESEQ1, 1); // strong click
  writeRegister8(DRV2605_REG_WAVESEQ2, 0);
  
  writeRegister8(DRV2605_REG_OVERDRIVE, 0); // no overdrive
  
  writeRegister8(DRV2605_REG_SUSTAINPOS, 0);
  writeRegister8(DRV2605_REG_SUSTAINNEG, 0);
  writeRegister8(DRV2605_REG_BREAK, 0);
  writeRegister8(DRV2605_REG_AUDIOMAX, 0x64);
  
  // ERM open loop
  
  // turn off N_ERM_LRA
  writeRegister8(DRV2605_REG_FEEDBACK, readRegister8(DRV2605_REG_FEEDBACK) & 0x7F);
  // turn on ERM_OPEN_LOOP
  writeRegister8(DRV2605_REG_CONTROL3, readRegister8(DRV2605_REG_CONTROL3) | 0x20);

  // Run in realtime mode
  setMode(DRV2605_MODE_REALTIME);

  // Turn off by default
  setRealtimeValue(0x00);
}

void OOutputs::tick()
{
    // If game isn't running, then just exit
    if (outrun.game_state != GS_INGAME)
    {
        setRealtimeValue(0x00);
        return;
    }

    const uint16_t speed = oinitengine.car_increment >> 16;
    uint16_t index = 0;

    // Car Crashing: Diable Motor once speed below 10
    if (ocrash.crash_counter)
    {
        if (speed > 10) {
            setRealtimeValue(0xFF);
        } else {
            setRealtimeValue(0x00);
        }
        return;
    }

    // Car skidding
    //if (ocrash.skid_counter) {

      //setRealtimeValue(0x3F);
      //return;

    //}

    // Normal car movement
    if (speed < 10)
    {
        setRealtimeValue(0x00);
        return;
    } 

    // Wheels are off road
    if (oferrari.wheel_state != OFerrari::WHEELS_ON)
    {
        setRealtimeValue(0xFF);
        return;

    } else {

      // Wheels are on road but slipping
      if (oferrari.is_slipping){
        setRealtimeValue(0xFF);
        return;
      }

      // Wheels spinning
      if (oferrari.car_state == OFerrari::CAR_SMOKE)
      {
          setRealtimeValue(0xFF);
          return;
      }

    }

    setRealtimeValue(0x00);
    return;

    // Car Normal
    /*else if (!ocrash.skid_counter)
    {
        if (speed < 10 || oferrari.wheel_state == OFerrari::WHEELS_ON)
        {
            setRealtimeValue(0x00);
            return;
        }  

        if (speed > 140)      index = 5;
        else if (speed > 100) index = 4;
        else if (speed > 60)  index = 3;
        else if (speed > 20)  index = 2;
        else                  index = 1;

        if (index > _vCounter)
        {
            _vCounter = 0;
            setRealtimeValue(0x00);
        }
        else
        {
            _vCounter++;
            setRealtimeValue(0x7F);
        }
        return;
    }

    // 0xEC7A calc_crash_skid:
    if (speed > 90)      index = 4;
    else if (speed > 70) index = 3;
    else if (speed > 50) index = 2;
    else if (speed > 30) index = 1;
    if (index > _vCounter)
    {
        _vCounter = 0;
        setRealtimeValue(0x00);
    }
    else
    {
        _vCounter++;
        setRealtimeValue(0x7F);
    }*/
}

// ------------------------------------------------------------------------------------------------
// Private
// ------------------------------------------------------------------------------------------------

void OOutputs::setMode(uint8_t mode) {
  writeRegister8(DRV2605_REG_MODE, mode);
}

void OOutputs::setRealtimeValue(uint8_t rtp) {
    if (rtp != _current_v){
        writeRegister8(DRV2605_REG_RTPIN, rtp);
        _current_v = rtp;
    }
}

void OOutputs::writeRegister8(uint8_t reg, uint8_t val) {
  wiringPiI2CWriteReg8(_module, reg, val);
}

uint8_t  OOutputs::readRegister8(uint8_t reg) {
  return wiringPiI2CReadReg8 (_module, reg);
}