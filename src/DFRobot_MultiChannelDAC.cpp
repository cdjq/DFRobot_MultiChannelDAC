/*!
 * @file DFRobot_MultiChannelDAC.cpp
 * @brief DFRobot MultiChannel DAC Arduino library implementation.
 *
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author      DFRobot (github@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-11-16
 * @url         https://github.com/DFRobot/DFRobot_MultiChannelDAC
 */
#include "DFRobot_MultiChannelDAC.h"

#include <math.h>

DFRobot_MultiChannelDAC::DFRobot_MultiChannelDAC(TwoWire *wire)
  : DFRobot_RTU(),
    _wire(wire),
    _serial(nullptr),
    _isI2C(true),
    _addr(DFR_MCDAC_I2C_ADDR_MAX),
    _rtuId(DFR_MCDAC_RTU_DEFAULT_ID),
    _channels(0),
    _sku(0),
    _firmwareVersion(0),
    _enableMask(0) {
  for (uint8_t i = 0; i < DFR_MCDAC_MAX_CHANNELS; ++i) {
    _range[i] = eOutputRange10V;
  }
}

DFRobot_MultiChannelDAC::DFRobot_MultiChannelDAC(Stream *serial)
  : DFRobot_RTU(serial),
    _wire(nullptr),
    _serial(serial),
    _isI2C(false),
    _addr(0),
    _rtuId(DFR_MCDAC_RTU_DEFAULT_ID),
    _channels(0),
    _sku(0),
    _firmwareVersion(0),
    _enableMask(0) {
  setTimeoutTimeMs(100);
  for (uint8_t i = 0; i < DFR_MCDAC_MAX_CHANNELS; ++i) {
    _range[i] = eOutputRange10V;
  }
}

DFRobot_MultiChannelDAC::~DFRobot_MultiChannelDAC() {
}

/* I2C 探测：只写一个指针字节，能收到 ACK 就认为地址上有模块。 */
bool DFRobot_MultiChannelDAC::i2cProbe(uint8_t address) {
  if (_wire == nullptr) {
    return false;
  }
  _wire->beginTransmission(address);
  _wire->write(static_cast<uint8_t>(DFR_MCDAC_INPUT_VID & 0x7F));
  return _wire->endTransmission() == 0;
}

int DFRobot_MultiChannelDAC::readRegisterBlock(bool holding, uint16_t reg,
                                               uint16_t *values, uint8_t count) {
  if (count == 0 || values == nullptr) {
    return 0;
  }
  if (_isI2C) {
    if (_wire == nullptr || reg > 0x7F) {
      return -1;
    }
    const uint8_t pointer = static_cast<uint8_t>((holding ? DFR_MCDAC_I2C_HOLDING_BASE : 0x00) | reg);
    _wire->beginTransmission(_addr);
    _wire->write(pointer);
    if (_wire->endTransmission(false) != 0) {
      return -2;
    }
    const uint8_t received = _wire->requestFrom(_addr, static_cast<uint8_t>(count * 2u));
    if (received != static_cast<uint8_t>(count * 2u)) {
      return -3;
    }
    for (uint8_t i = 0; i < count; ++i) {
      const uint16_t high = static_cast<uint16_t>(_wire->read());
      const uint16_t low = static_cast<uint16_t>(_wire->read());
      values[i] = static_cast<uint16_t>((high << 8) | low);
    }
    return 0;
  }
  if (_serial == nullptr) {
    return -4;
  }
  const uint8_t result = holding
                             ? readHoldingRegister(_rtuId, reg, values, count)
                             : readInputRegister(_rtuId, reg, values, count);
  return (result == 0) ? 0 : static_cast<int>(result);
}

int DFRobot_MultiChannelDAC::writeRegisterBlock(uint16_t reg, const uint16_t *values,
                                                uint8_t count) {
  if (count == 0 || values == nullptr) {
    return 0;
  }
  if (_isI2C) {
    if (_wire == nullptr || reg > 0x7F) {
      return -1;
    }
    _wire->beginTransmission(_addr);
    _wire->write(static_cast<uint8_t>(DFR_MCDAC_I2C_HOLDING_BASE | reg));
    for (uint8_t i = 0; i < count; ++i) {
      _wire->write(static_cast<uint8_t>(values[i] >> 8));
      _wire->write(static_cast<uint8_t>(values[i] & 0xFF));
    }
    return (_wire->endTransmission() == 0) ? 0 : -2;
  }
  if (_serial == nullptr || count > DFR_MCDAC_MAX_CHANNELS) {
    return -3;
  }
  uint16_t buffer[DFR_MCDAC_MAX_CHANNELS];
  for (uint8_t i = 0; i < count; ++i) {
    buffer[i] = values[i];
  }
  const uint8_t result = writeHoldingRegister(_rtuId, reg, buffer, count);
  return (result == 0) ? 0 : static_cast<int>(result);
}

int DFRobot_MultiChannelDAC::readInputRegister16(uint16_t reg, uint16_t *value) {
  return readRegisterBlock(false, reg, value, 1);
}

int DFRobot_MultiChannelDAC::readHoldingRegister16(uint16_t reg, uint16_t *value) {
  return readRegisterBlock(true, reg, value, 1);
}

int DFRobot_MultiChannelDAC::writeHoldingRegister16(uint16_t reg, uint16_t value) {
  return writeRegisterBlock(reg, &value, 1);
}

uint16_t DFRobot_MultiChannelDAC::rangeMaxMv(uint8_t range) const {
  switch (range) {
    case eOutputRange2_5V:
      return 2500;
    case eOutputRange5V:
      return 5000;
    case eOutputRange10V:
      return 10000;
    default:
      return 0;
  }
}

bool DFRobot_MultiChannelDAC::rangeSupported(uint8_t range) const {
  if (_sku < 2) {
    return range == eOutputRange10V;
  }
  return range == eOutputRange2_5V || range == eOutputRange5V;
}

uint16_t DFRobot_MultiChannelDAC::pidForSku(uint8_t sku) const {
  switch (sku) {
    case 1:
      return 0x0531;
    case 2:
      return 0x0534;
    case 3:
      return 0x0535;
    default:
      return 0x0530;
  }
}

int DFRobot_MultiChannelDAC::begin(void) {
  if (_isI2C) {
    if (_wire == nullptr) {
      return -1;
    }
    _wire->begin();
    bool found = false;
    for (uint8_t address = DFR_MCDAC_I2C_ADDR_BASE; address <= DFR_MCDAC_I2C_ADDR_MAX; ++address) {
      if (i2cProbe(address)) {
        _addr = address;
        found = true;
        break;
      }
    }
    if (!found) {
      return -2;
    }
  } else if (_serial == nullptr) {
    return -3;
  }

  uint16_t vid = 0;
  if (readInputRegister16(DFR_MCDAC_INPUT_VID, &vid) != 0 || vid != 0x3343) {
    return -4;
  }
  uint16_t pid = 0;
  if (readInputRegister16(DFR_MCDAC_INPUT_PID, &pid) != 0) {
    return -5;
  }
  uint16_t sku = 0;
  if (readInputRegister16(DFR_MCDAC_INPUT_BOARD_SKU, &sku) != 0 || sku > 3) {
    return -6;
  }
  uint16_t channels = 0;
  if (readInputRegister16(DFR_MCDAC_INPUT_CHANNEL_COUNT, &channels) != 0 ||
      (channels != 4 && channels != 8)) {
    return -7;
  }
  if (pid != pidForSku(static_cast<uint8_t>(sku))) {
    return -8;
  }

  _sku = static_cast<uint8_t>(sku);
  _channels = static_cast<uint8_t>(channels);
  (void)readInputRegister16(DFR_MCDAC_INPUT_FIRMWARE_VERSION, &_firmwareVersion);

  uint16_t enabled = 0;
  if (readInputRegister16(DFR_MCDAC_INPUT_CHANNEL_ENABLE, &enabled) == 0) {
    _enableMask = enabled;
  } else {
    _enableMask = static_cast<uint16_t>((1u << _channels) - 1u);
  }

  for (uint8_t ch = 0; ch < _channels; ++ch) {
    uint16_t range = 0;
    if (readInputRegister16(static_cast<uint16_t>(DFR_MCDAC_INPUT_CH1_RANGE + ch), &range) == 0 &&
        range <= eOutputRange10V) {
      _range[ch] = static_cast<uint8_t>(range);
    } else {
      _range[ch] = (_sku < 2) ? eOutputRange10V : eOutputRange5V;
    }
  }
  return 0;
}

int DFRobot_MultiChannelDAC::setOutputRange(uint8_t channel, eOutputRange_t range) {
  if (_channels == 0 || channel >= _channels) {
    return -1;
  }
  if (!rangeSupported(static_cast<uint8_t>(range))) {
    return -2;
  }
  const int result = writeHoldingRegister16(
      static_cast<uint16_t>(DFR_MCDAC_HOLDING_CH1_RANGE + channel),
      static_cast<uint16_t>(range));
  if (result != 0) {
    return result;
  }
  _range[channel] = static_cast<uint8_t>(range);
  return 0;
}

int DFRobot_MultiChannelDAC::setAllOutputRange(eOutputRange_t range) {
  if (_channels == 0) {
    return -1;
  }
  if (!rangeSupported(static_cast<uint8_t>(range))) {
    return -2;
  }
  uint16_t values[DFR_MCDAC_MAX_CHANNELS];
  for (uint8_t ch = 0; ch < _channels; ++ch) {
    values[ch] = static_cast<uint16_t>(range);
  }
  const int result = writeRegisterBlock(DFR_MCDAC_HOLDING_CH1_RANGE, values, _channels);
  if (result != 0) {
    return result;
  }
  for (uint8_t ch = 0; ch < _channels; ++ch) {
    _range[ch] = static_cast<uint8_t>(range);
  }
  return 0;
}

int DFRobot_MultiChannelDAC::writeVoltageMv(uint8_t channel, uint16_t mv) {
  const int result = writeHoldingRegister16(
      static_cast<uint16_t>(DFR_MCDAC_HOLDING_CH1_VOLTAGE_MV + channel), mv);
  if (result != 0) {
    return result;
  }
  return writeHoldingRegister16(DFR_MCDAC_HOLDING_COMMIT_MASK,
                                static_cast<uint16_t>(1u << channel));
}

int DFRobot_MultiChannelDAC::setVoltage(uint8_t channel, float voltage) {
  if (_channels == 0 || channel >= _channels) {
    return -1;
  }
  if (voltage < 0.0f) {
    return -2;
  }
  const long mv = static_cast<long>(voltage * 1000.0f + 0.5f);
  if (mv < 0 || mv > rangeMaxMv(_range[channel])) {
    return -3;
  }
  return writeVoltageMv(channel, static_cast<uint16_t>(mv));
}

int DFRobot_MultiChannelDAC::setSyncVoltages(const uint8_t channels[],
                                             const float voltages[],
                                             uint8_t length) {
  if (_channels == 0 || channels == nullptr || voltages == nullptr) {
    return -1;
  }
  if (length == 0 || length > _channels) {
    return -2;
  }
  uint16_t mv[DFR_MCDAC_MAX_CHANNELS];
  uint16_t used = 0;
  for (uint8_t i = 0; i < length; ++i) {
    const uint8_t channel = channels[i];
    if (channel >= _channels) {
      return -3;
    }
    const uint16_t bit = static_cast<uint16_t>(1u << channel);
    if ((used & bit) != 0) {
      return -4;
    }
    if (voltages[i] < 0.0f) {
      return -5;
    }
    const long value = static_cast<long>(voltages[i] * 1000.0f + 0.5f);
    if (value < 0 || value > rangeMaxMv(_range[channel])) {
      return -6;
    }
    used = static_cast<uint16_t>(used | bit);
    mv[channel] = static_cast<uint16_t>(value);
  }
  for (uint8_t i = 0; i < length; ++i) {
    const uint8_t channel = channels[i];
    const int result = writeHoldingRegister16(
        static_cast<uint16_t>(DFR_MCDAC_HOLDING_CH1_VOLTAGE_MV + channel),
        mv[channel]);
    if (result != 0) {
      return result;
    }
  }
  return writeHoldingRegister16(DFR_MCDAC_HOLDING_COMMIT_MASK, used);
}

int DFRobot_MultiChannelDAC::setAllVoltage(const float voltage[], uint8_t length) {
  if (_channels == 0 || voltage == nullptr) {
    return -1;
  }
  if (length != _channels) {
    return -2;
  }
  uint16_t values[DFR_MCDAC_MAX_CHANNELS];
  for (uint8_t ch = 0; ch < _channels; ++ch) {
    if (voltage[ch] < 0.0f) {
      return -3;
    }
    const long mv = static_cast<long>(voltage[ch] * 1000.0f + 0.5f);
    if (mv < 0 || mv > rangeMaxMv(_range[ch])) {
      return -4;
    }
    values[ch] = static_cast<uint16_t>(mv);
  }
  const int result = writeRegisterBlock(DFR_MCDAC_HOLDING_CH1_VOLTAGE_MV, values, _channels);
  if (result != 0) {
    return result;
  }
  return writeHoldingRegister16(DFR_MCDAC_HOLDING_COMMIT_MASK,
                                static_cast<uint16_t>((1u << _channels) - 1u));
}

int DFRobot_MultiChannelDAC::setChannelEnabled(uint8_t channel, bool enabled) {
  if (_channels == 0 || channel >= _channels) {
    return -1;
  }
  uint16_t mask = _enableMask;
  const uint16_t bit = static_cast<uint16_t>(1u << channel);
  if (enabled) {
    mask = static_cast<uint16_t>(mask | bit);
  } else {
    mask = static_cast<uint16_t>(mask & ~bit);
  }
  const int result = writeHoldingRegister16(DFR_MCDAC_HOLDING_CHANNEL_ENABLE, mask);
  if (result == 0) {
    _enableMask = mask;
  }
  return result;
}

int DFRobot_MultiChannelDAC::stopOutput(void) {
  return writeHoldingRegister16(DFR_MCDAC_HOLDING_COMMAND, DFR_MCDAC_CMD_STOP_ALL);
}

uint16_t DFRobot_MultiChannelDAC::readFirmwareVersion(void) {
  uint16_t value = 0;
  if (readInputRegister16(DFR_MCDAC_INPUT_FIRMWARE_VERSION, &value) == 0) {
    _firmwareVersion = value;
  }
  return _firmwareVersion;
}

uint8_t DFRobot_MultiChannelDAC::getSKU(void) {
  uint16_t value = 0;
  if (readInputRegister16(DFR_MCDAC_INPUT_BOARD_SKU, &value) != 0 || value > 3) {
    return 0xFF;
  }
  return static_cast<uint8_t>(value);
}

uint8_t DFRobot_MultiChannelDAC::getChannelCount(void) {
  return _channels;
}

float DFRobot_MultiChannelDAC::getVoltage(uint8_t channel) {
  if (_channels == 0 || channel >= _channels) {
    return 0.0f;
  }
  uint16_t value = 0;
  if (readInputRegister16(
          static_cast<uint16_t>(DFR_MCDAC_INPUT_CH1_VOLTAGE_MV + channel), &value) != 0) {
    return 0.0f;
  }
  return static_cast<float>(value) / 1000.0f;
}

eOutputRange_t DFRobot_MultiChannelDAC::getRange(uint8_t channel) {
  if (_channels == 0 || channel >= _channels) {
    return eOutputRange10V;
  }
  uint16_t value = 0;
  if (readInputRegister16(
          static_cast<uint16_t>(DFR_MCDAC_INPUT_CH1_RANGE + channel), &value) != 0 ||
      value > eOutputRange10V) {
    return eOutputRange10V;
  }
  return static_cast<eOutputRange_t>(value);
}

bool DFRobot_MultiChannelDAC::getChannelEnabled(uint8_t channel) {
  if (_channels == 0 || channel >= _channels) {
    return false;
  }
  uint16_t value = 0;
  if (readInputRegister16(DFR_MCDAC_INPUT_CHANNEL_ENABLE, &value) == 0) {
    _enableMask = value;
  }
  return (_enableMask & (1u << channel)) != 0;
}

uint16_t DFRobot_MultiChannelDAC::getStatus(void) {
  uint16_t value = 0;
  (void)readInputRegister16(DFR_MCDAC_INPUT_STATUS, &value);
  return value;
}
