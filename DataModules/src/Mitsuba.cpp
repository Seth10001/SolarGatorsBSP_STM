/*
 * Mitsuba.cpp
 *
 *  Created on: Jan 14, 2022
 *      Author: John Carr
 */

#include <Mitsuba.hpp>

namespace SolarGators {
namespace DataModules {

MitsubaRequest::MitsubaRequest(uint32_t id):
    DataModule(id, Request_Size, true)
{ }

MitsubaRequest::~MitsubaRequest()
{ }


void MitsubaRequest::SetRequests(bool frame0, bool frame1, bool frame2)
{
  requestFrame0 = frame0;
  requestFrame1 = frame1;
  requestFrame2 = frame2;
}

void MitsubaRequest::ClearRequests()
{
  requestFrame0 = false;
  requestFrame1 = false;
  requestFrame2 = false;
}

void MitsubaRequest::SetRequestAllFrames()
{
  requestFrame0 = true;
  requestFrame1 = true;
  requestFrame2 = true;
}

void MitsubaRequest::ToByteArray(uint8_t* buff) const
{
  buff[0] = 0;
  buff[0] |= static_cast<uint8_t>(requestFrame0) << 0;
  buff[0] |= static_cast<uint8_t>(requestFrame1) << 1;
  buff[0] |= static_cast<uint8_t>(requestFrame2) << 2;
}
void MitsubaRequest::FromByteArray(uint8_t* buff)
{
  requestFrame0 = buff[0] & (1 << 0);
  requestFrame1 = buff[0] & (1 << 1);
  requestFrame2 = buff[0] & (1 << 2);
}

MitsubaRx0::MitsubaRx0(uint32_t id):
    DataModule(id, Rx0_Size, true), battVoltage(0),battCurrent(0),
    battCurrentDir(false),motorCurrentPkAvg(0),FETtemp(0),
    motorRPM(0),PWMDuty(0),LeadAngle(0)
{ }

MitsubaRx0::~MitsubaRx0()
{ }

// Getters
uint16_t MitsubaRx0::GetBatteryVoltage() const
{
  return battVoltage;
}
uint16_t MitsubaRx0::GetBatteryCurrent() const
{
  return battCurrent;
}
bool MitsubaRx0::GetBatteryCurrentDir() const
{
  return battCurrentDir;
}
uint16_t MitsubaRx0::GetMotorCurrentPkAvg() const
{
  return motorCurrentPkAvg;
}
uint8_t MitsubaRx0::GetFetTemp() const
{
  return FETtemp;
}
uint16_t MitsubaRx0::GetMotorRPM() const
{
  return motorRPM;
}
uint16_t MitsubaRx0::GetPWMDuty() const
{
  return PWMDuty;
}
uint16_t MitsubaRx0::GetLeadAngle() const
{
  return LeadAngle;
}
// Converter Functions
void MitsubaRx0::ToByteArray(uint8_t* buff) const
{

  buff[0] = 0;
  buff[0] |= static_cast<uint8_t>(battVoltage);

  buff[1] = 0;
  buff[1] |= static_cast<uint8_t>(battVoltage >> 8);
  buff[1] |= static_cast<uint8_t>(static_cast<uint32_t>(battCurrent) << 2);

  buff[2] = 0;
  buff[2] |= static_cast<uint8_t>(battCurrent >> 6);
  buff[2] |= static_cast<uint8_t>(static_cast<uint32_t>(battCurrentDir) << 3);
  buff[2] |= static_cast<uint8_t>(motorCurrentPkAvg << 4);

  buff[3] = 0;
  buff[3] |= static_cast<uint8_t>(motorCurrentPkAvg >> 4);
  buff[3] |= static_cast<uint8_t>(static_cast<uint32_t>(FETtemp) << 6);


  uint32_t convMotorRPM = motorRPM * 1;
  buff[4] = 0;
  buff[4] |= static_cast<uint8_t>(FETtemp >> 2);
  buff[4] |= (static_cast<uint32_t>(convMotorRPM) & 0x1F) << 3;

  buff[5] = 0;
  buff[5] |= (static_cast<uint32_t>(convMotorRPM) & 0xFE0) >> 5;
  buff[5] |= (static_cast<uint8_t>(PWMDuty << 7));

  buff[6] = 0;
  buff[6] |= static_cast<uint8_t>(PWMDuty >> 1);

  buff[7] = 0;
  buff[7] |= static_cast<uint8_t>(PWMDuty >> 9);
  buff[7] |= static_cast<uint8_t>(static_cast<uint32_t>(LeadAngle) << 1);
}

void MitsubaRx0::FromByteArray(uint8_t* buff)
{
  uint32_t preBattVoltage = (static_cast<uint32_t>(buff[1] & 3) << 8) | (buff[0]);
  battVoltage = static_cast<uint16_t>(preBattVoltage);

  uint32_t preBattCurrent = (static_cast<uint32_t>(buff[2] & 7) << 6) | (buff[1] >> 2);
  battCurrent = static_cast<uint16_t>(preBattCurrent);

  battCurrentDir = static_cast<bool>(buff[2] & 8);

  uint32_t preMotorCurrent = static_cast<uint32_t>((buff[3] & 0x3F) << 4) | (buff[2] >> 4);
  motorCurrentPkAvg = static_cast<uint16_t>(preMotorCurrent);

  uint32_t preFETtemp = static_cast<uint32_t>((buff[4] & 7) << 2) | (buff[3] >> 6);
  FETtemp = static_cast<uint8_t>(preFETtemp);

  uint32_t preMotorRPM = (static_cast<uint32_t>(buff[5] & 0x7F) << 5) | (buff[4] >> 3);
  motorRPM = static_cast<uint16_t>(preMotorRPM);

  uint32_t preDuty = (static_cast<uint32_t>(buff[7] & 1) << 9) | (buff[6] << 1) | (buff[5] >> 7);
  PWMDuty = static_cast<uint16_t>(preDuty);

  LeadAngle = static_cast<uint8_t>((buff[7] >> 1));
}

MitsubaRx1::MitsubaRx1(uint32_t id):
    DataModule(id, Rx1_Size, true)
{ }

MitsubaRx1::~MitsubaRx1()
{ }

// Getters
bool MitsubaRx1::GetPowerMode() const
{
  return powerMode;
}
bool MitsubaRx1::GetMcMode() const
{
  return MCmode;
}
uint16_t MitsubaRx1::GetAcceleratorPosition() const
{
  return AcceleratorPosition;
}
uint16_t MitsubaRx1::GetRegenVrPosition() const
{
  return regenVRposition;
}
uint8_t MitsubaRx1::GetDigitSwitchPosition() const
{
  return digitSWposition;
}
uint16_t MitsubaRx1::GetOutTargetVal() const
{
  return outTargetVal;
}
uint8_t MitsubaRx1::GetDriveActStat() const
{
  return driveActStat;
}
bool MitsubaRx1::GetRegenStat() const
{
  return regenStat;
}
// Converter Functions
void MitsubaRx1::ToByteArray(uint8_t* buff) const
{
  buff[0] = 0;
  buff[0] |= static_cast<uint8_t>(powerMode);
  buff[0] |= static_cast<uint8_t>(MCmode) << 1;
  buff[0] |= static_cast<uint8_t>(AcceleratorPosition << 2);

  buff[1] = 0;
  buff[1] |= static_cast<uint8_t>(AcceleratorPosition >> 6);
  buff[1] |= static_cast<uint8_t>(regenVRposition << 4);

  buff[2] = 0;
  buff[2] |= static_cast<uint8_t>(regenVRposition >> 4);
  buff[2] |= static_cast<uint8_t>(static_cast<uint32_t>(digitSWposition) << 6);

  buff[3] = 0;
  buff[3] |= static_cast<uint8_t>(digitSWposition >> 2);
  buff[3] |= static_cast<uint8_t>(outTargetVal << 2);

  buff[4] = 0;
  buff[4] |= static_cast<uint8_t>(outTargetVal >> 6);
  buff[4] |= static_cast<uint8_t>(driveActStat << 4);
  buff[4] |= static_cast<uint8_t>(static_cast<uint8_t>(regenStat) << 6);
}
void MitsubaRx1::FromByteArray(uint8_t* buff)
{
  powerMode = buff[0] & 1;

  MCmode = (buff[0] >> 1) & 1;

  uint32_t preAccelPos = static_cast<uint32_t>((buff[1] & 0xF) << 6) | (buff[0] >> 2);
  AcceleratorPosition = static_cast<uint16_t>(preAccelPos);

  uint32_t preRegenVRposition = static_cast<uint32_t>((buff[2] & 0x3F) << 4) | (buff[1] >> 4);
  regenVRposition = static_cast<uint16_t>(preRegenVRposition);

  uint32_t preDigitSWposition = static_cast<uint32_t>((buff[3] & 0x3) << 2) | (buff[2] >> 6);
  digitSWposition = static_cast<uint8_t>(preDigitSWposition);

  uint32_t preOutTargetVal = static_cast<uint32_t>((buff[4] & 0xF) << 6) | (buff[3] >> 2);
  outTargetVal = static_cast<uint16_t>(preOutTargetVal);

  driveActStat = static_cast<uint8_t>((buff[4] >> 4) & 3);

  regenStat = static_cast<bool>((buff[4] >> 6) & 1);
}

MitsubaRx2::MitsubaRx2(uint32_t id):
    DataModule(id, Rx2_Size, true)
{ }

MitsubaRx2::~MitsubaRx2()
{ }

// Getters
bool MitsubaRx2::GetAdSensorError() const
{
  return adSensorError;
}
bool MitsubaRx2::GetMotorSensorUError() const
{
  return motorCurrSensorUError;
}
bool MitsubaRx2::GetMotorCurrSensorWError() const
{
  return motorCurrSensorWError;
}
bool MitsubaRx2::GetFetThermError() const
{
  return fetThermError;
}
bool MitsubaRx2::GetBattVoltSensorError() const
{
  return battVoltSensorError;
}
bool MitsubaRx2::GetBattCurrSensorError() const
{
  return battCurrSensorError;
}
bool MitsubaRx2::GetBattCurrSensorAdjError() const
{
  return battCurrSensorAdjError;
}
bool MitsubaRx2::GetMotorCurrSensorAdjError() const
{
  return motorCurrSensorAdjError;
}
bool MitsubaRx2::GetAccelPosError() const
{
  return accelPosError;
}
bool MitsubaRx2::GetContVoltSensorError() const
{
  return contVoltSensorError;
}
bool MitsubaRx2::GetPowerSystemError() const
{
  return powerSystemError;
}
bool MitsubaRx2::GetOverCurrError() const
{
  return overCurrError;
}
bool MitsubaRx2::GetOverVoltError() const
{
  return overVoltError;
}
bool MitsubaRx2::GetOverCurrLimit() const
{
  return overCurrLimit;
}
bool MitsubaRx2::GetMotorSystemError() const
{
  return motorSystemError;
}
bool MitsubaRx2::GetMotorLock() const
{
  return motorLock;
}
bool MitsubaRx2::GetHallSensorShort() const
{
  return hallSensorShort;
}
bool MitsubaRx2::GetHallSensorOpen() const
{
  return hallSensorOpen;
}
uint8_t MitsubaRx2::GetOverHeatLevel() const
{
  return overHeatLevel;
}
// Converter Functions
void MitsubaRx2::ToByteArray(uint8_t* buff) const
{
  uint32_t convOverHeadLevel = overHeatLevel;

  buff[0] = 0;
  buff[0] |= static_cast<uint8_t>(adSensorError) << 0;
  buff[0] |= static_cast<uint8_t>(motorCurrSensorUError) << 1;
  buff[0] |= static_cast<uint8_t>(motorCurrSensorWError) << 2;
  buff[0] |= static_cast<uint8_t>(fetThermError) << 3;
  buff[0] |= static_cast<uint8_t>(battVoltSensorError) << 5;
  buff[0] |= static_cast<uint8_t>(battCurrSensorError) << 6;
  buff[0] |= static_cast<uint8_t>(battCurrSensorAdjError) << 7;

  buff[1] = 0;
  buff[1] |= static_cast<uint8_t>(motorCurrSensorAdjError) << 0;
  buff[1] |= static_cast<uint8_t>(accelPosError) << 1;
  buff[1] |= static_cast<uint8_t>(contVoltSensorError) << 3;

  buff[2] = 0;
  buff[2] |= static_cast<uint8_t>(powerSystemError) << 0;
  buff[2] |= static_cast<uint8_t>(overCurrError) << 1;
  buff[2] |= static_cast<uint8_t>(overVoltError) << 3;
  buff[2] |= static_cast<uint8_t>(overCurrLimit) << 5;

  buff[3] = 0;
  buff[3] |= static_cast<uint8_t>(motorSystemError) << 0;
  buff[3] |= static_cast<uint8_t>(motorLock) << 1;
  buff[3] |= static_cast<uint8_t>(hallSensorShort) << 2;
  buff[3] |= static_cast<uint8_t>(hallSensorOpen) << 3;

  buff[4] = static_cast<uint32_t>(convOverHeadLevel) & 0x3;
}
void MitsubaRx2::FromByteArray(uint8_t* buff)
{
  adSensorError      = buff[0] & (1 << 0);
  motorCurrSensorUError  = buff[0] & (1 << 1);
  motorCurrSensorWError  = buff[0] & (1 << 2);
  fetThermError      = buff[0] & (1 << 3);
  battVoltSensorError    = buff[0] & (1 << 5);
  battCurrSensorError    = buff[0] & (1 << 6);
  battCurrSensorAdjError   = buff[0] & (1 << 7);

  motorCurrSensorAdjError  = buff[1] & (1 << 0);
  accelPosError      = buff[1] & (1 << 1);
  contVoltSensorError    = buff[1] & (1 << 3);

  powerSystemError     = buff[2] & (1 << 0);
  overCurrError      = buff[2] & (1 << 1);
  overVoltError      = buff[2] & (1 << 3);
  overCurrLimit      = buff[2] & (1 << 5);

  motorSystemError     = buff[3] & (1 << 0);
  motorLock        = buff[3] & (1 << 1);
  hallSensorShort      = buff[3] & (1 << 2);
  hallSensorOpen     = buff[3] & (1 << 3);

  overHeatLevel      = buff[4] & 0x3;
}

} /* namespace DataModules */
} /* namespace SolarGators */
