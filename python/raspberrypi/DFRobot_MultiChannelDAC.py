'''!
  @file DFRobot_MultiChannelDAC.py
  @brief Drive the DFR1328/DFR1329/DFR1332/DFR1333 multi-channel DAC modules
  @details The module runs its own firmware and exposes a General RTU register
  @n table. Wire the module over I2C (SDA/SCL/GND/VCC, address 0x40-0x43) or
  @n over UART (module TX to the Pi RXD, module RX to the Pi TXD, GND, VCC).
  @n A multimeter on the channel output shows the voltage that was set.
  @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license The MIT License (MIT)
  @author wxzed
  @version V1.0.0
  @date 2026-11-18
'''

import smbus

try:
  from DFRobot_RTU import DFRobot_RTU
except ImportError:  # DFRobot_RTU.py is only needed in UART mode
  DFRobot_RTU = None


class DFRobot_MultiChannelDAC(object):
  '''!
    @brief 4/8-channel synchronous voltage-output DAC driver; I2C and UART share one class
  '''

  ## Vendor ID, read only, always 0x3343 (from DFRobot_MultiChannelDAC.h)
  _REG_INPUT_VID = 0x0000
  ## Product ID, read only: DFR1328=0x0530, DFR1329=0x0531, DFR1332=0x0534, DFR1333=0x0535
  _REG_INPUT_PID = 0x0001
  ## Hardware version, read only, 0x1000 means V1.0.0.0
  _REG_INPUT_HARDWARE_VERSION = 0x0002
  ## Firmware version, read only, 0x1000 means V1.0.0.0
  _REG_INPUT_FIRMWARE_VERSION = 0x0003
  ## Board type: 0=DFR1328, 1=DFR1329, 2=DFR1332, 3=DFR1333
  _REG_INPUT_BOARD_SKU = 0x0004
  ## Output channel count: 4 or 8
  _REG_INPUT_CHANNEL_COUNT = 0x0005
  ## Status, bit0=1 means a reference alarm
  _REG_INPUT_STATUS = 0x0006
  ## CH1 applied voltage in mV, read only; CH1 to CH8 occupy 0x0007 to 0x000E
  _REG_INPUT_CH1_VOLTAGE_MV = 0x0007
  ## CH8 applied voltage in mV, read only
  _REG_INPUT_CH8_VOLTAGE_MV = 0x000E
  ## CH1 range code, read only; CH1 to CH8 occupy 0x000F to 0x0016
  _REG_INPUT_CH1_RANGE = 0x000F
  ## CH8 range code, read only
  _REG_INPUT_CH8_RANGE = 0x0016
  ## Channel enable bitmap, read only, bit0=CH1
  _REG_INPUT_CHANNEL_ENABLE = 0x0017

  ## RTU slave address, 1 to 247, factory 0x0F, takes effect after power cycle
  _REG_HOLDING_DEVICE_ADDRESS = 0x0000
  ## Baud rate code, 0x0003=9600, takes effect after power cycle
  _REG_HOLDING_BAUDRATE = 0x0001
  ## Serial format, 0x0003=no parity with 2 stop bits, takes effect after power cycle
  _REG_HOLDING_SERIAL_FORMAT = 0x0002
  ## CH1 target voltage in mV, staged only; CH1 to CH8 occupy 0x0003 to 0x000A
  _REG_HOLDING_CH1_VOLTAGE_MV = 0x0003
  ## CH8 target voltage in mV, staged only
  _REG_HOLDING_CH8_VOLTAGE_MV = 0x000A
  ## CH1 range code, the channel drops to 0 V when written; CH1 to CH8 occupy 0x000B to 0x0012
  _REG_HOLDING_CH1_RANGE = 0x000B
  ## CH8 range code
  _REG_HOLDING_CH8_RANGE = 0x0012
  ## Channel enable bitmap, bit0=CH1 ... bit7=CH8
  _REG_HOLDING_CHANNEL_ENABLE = 0x0013
  ## Commit mask, the channels whose bits are set take effect together
  _REG_HOLDING_COMMIT_MASK = 0x0014
  ## Command: 1=all channels back to 0 V, 2=all channels apply the staged values
  _REG_HOLDING_COMMAND = 0x0015

  ## Range 0-2.5 V, DFR1332/DFR1333 only (Arduino eOutputRange2_5V)
  OUTPUT_RANGE2_5V = 0
  ## Range 0-5 V, DFR1332/DFR1333 only
  OUTPUT_RANGE5V = 1
  ## Range 0-10 V, DFR1328/DFR1329 only
  OUTPUT_RANGE10V = 2

  ## I2C address base, ADDR0/ADDR1 select 0x40 to 0x43
  _I2C_ADDR_BASE = 0x40
  ## Highest I2C address
  _I2C_ADDR_MAX = 0x43
  ## Factory UART slave address
  _RTU_DEFAULT_ID = 0x0F
  ## Largest channel count of the 8-channel boards
  _MAX_CHANNELS = 8
  ## I2C pointer 0x80 + RTU address selects the holding registers
  _I2C_HOLDING_BASE = 0x80

  ## Command value: all channels back to 0 V together
  _CMD_STOP_ALL = 0x0001
  ## Command value: all channels apply the staged values together
  _CMD_COMMIT_ALL = 0x0002

  ## DFRobot_RTU exception code: illegal function
  RTU_EXCEPTION_ILLEGAL_FUNCTION = 0x01
  ## Illegal data address
  RTU_EXCEPTION_ILLEGAL_DATA_ADDRESS = 0x02
  ## Illegal data value
  RTU_EXCEPTION_ILLEGAL_DATA_VALUE = 0x03
  ## Slave failure
  RTU_EXCEPTION_SLAVE_FAILURE = 0x04
  ## CRC error
  RTU_EXCEPTION_CRC_ERROR = 0x08
  ## Receive error
  RTU_RECV_ERROR = 0x09
  ## Memory error
  RTU_MEMORY_ERROR = 0x0A
  ## Address error
  RTU_ID_ERROR = 0x0B

  def __init__(self, i2c_bus=1, address=_I2C_ADDR_MAX, baud=None, bits=8, parity=0, stopbit=2):
    '''!
      @brief Create the module object
      @details I2C mode matches DFRobot_MultiChannelDAC(TwoWire *wire = &Wire) and
      @n begin() probes 0x40-0x43. UART mode matches DFRobot_MultiChannelDAC(Stream *serial)
      @n and hands the port to DFRobot_RTU, which opens /dev/ttyAMA0.
      @param i2c_bus I2C bus number, usually 1 on a Raspberry Pi
      @param address I2C address, 0x40-0x43
      @param baud UART baud rate; None selects I2C
      @param bits UART data bits
      @param parity UART parity, 0 means none
      @param stopbit UART stop bits, the module ships with 2
    '''
    self._rtu = None
    self._use_i2c = baud is None
    if self._use_i2c is False:
      if DFRobot_RTU is None:
        raise ImportError('UART mode needs DFRobot_RTU.py next to this file')
      self._rtu = DFRobot_RTU(baud, bits, parity, stopbit)
      self._rtu.set_timout_time_s(0.1)

    self._i2c_bus = i2c_bus
    self._addr = address
    self._bus = None
    self._rtu_id = self._RTU_DEFAULT_ID
    self._channels = 0
    self._sku = 0
    self._firmware_version = 0
    self._enable_mask = 0
    self._range = [self.OUTPUT_RANGE10V] * self._MAX_CHANNELS

  def _read_register_block(self, holding, reg, count):
    '''!
      @brief Read count 16-bit registers in one go
      @param holding True for holding registers, False for input registers
      @param reg Start register address
      @param count Number of registers
      @return ret, values
      @retval 0 Success, values holds the read data
      @retval -1 I2C not open or address above 0x7F
      @retval -2 I2C pointer write failed
      @retval -3 I2C data read failed
      @retval -4 UART not open
      @retval other DFRobot_RTU exception code in UART mode
    '''
    if count == 0:
      return 0, []
    if self._use_i2c is True:
      if self._bus is None or reg > 0x7F:
        return -1, []
      pointer = (self._I2C_HOLDING_BASE if holding is True else 0x00) | reg
      try:
        self._bus.write_byte(self._addr, pointer)
      except OSError:
        return -2, []
      try:
        data = self._bus.read_i2c_block_data(self._addr, pointer, count * 2)
      except OSError:
        return -3, []
      if len(data) != count * 2:
        return -3, []
      return 0, [(data[2 * i] << 8) | data[2 * i + 1] for i in range(count)]

    if self._rtu is None:
      return -4, []
    if holding is True:
      reply = self._rtu.read_holding_registers(self._rtu_id, reg, count)
    else:
      reply = self._rtu.read_input_registers(self._rtu_id, reg, count)
    if reply is None or len(reply) != 1 + count * 2 or reply[0] != 0:
      return (reply[0] if reply else self.RTU_RECV_ERROR), [0] * count
    return 0, [(reply[1 + 2 * i] << 8) | reply[2 + 2 * i] for i in range(count)]

  def _write_register_block(self, reg, values):
    '''!
      @brief Write a block of 16-bit registers
      @param reg Start register address
      @param values Values to write
      @return int
      @retval 0 Success
      @retval -1 I2C not open or address above 0x7F
      @retval -2 I2C write failed
      @retval -3 UART not open or more than 8 values
      @retval other DFRobot_RTU exception code in UART mode
    '''
    count = len(values)
    if count == 0:
      return 0
    if self._use_i2c is True:
      if self._bus is None or reg > 0x7F:
        return -1
      payload = []
      for value in values:
        payload += [(value >> 8) & 0xFF, value & 0xFF]
      try:
        self._bus.write_i2c_block_data(self._addr, self._I2C_HOLDING_BASE | reg, payload)
      except OSError:
        return -2
      return 0
    if self._rtu is None or count > self._MAX_CHANNELS:
      return -3
    data = []
    for value in values:
      data += [(value >> 8) & 0xFF, value & 0xFF]
    return self._rtu.write_holding_registers(self._rtu_id, reg, data)

  def _read_input_register16(self, reg):
    '''!
      @brief Read one input register
      @param reg Register address
      @return ret, value
      @retval 0 Success, value holds the read data
      @retval other Communication failure
    '''
    ret, values = self._read_register_block(False, reg, 1)
    return ret, (values[0] if values else 0)

  def _read_holding_register16(self, reg):
    '''!
      @brief Read one holding register
      @param reg Register address
      @return ret, value
      @retval 0 Success, value holds the read data
      @retval other Communication failure
    '''
    ret, values = self._read_register_block(True, reg, 1)
    return ret, (values[0] if values else 0)

  def _write_holding_register16(self, reg, value):
    '''!
      @brief Write one holding register
      @param reg Register address
      @param value Value to write
      @return int 0 on success, other values are communication failures
    '''
    return self._write_register_block(reg, [value])

  def _write_voltage_mv(self, channel, mv):
    '''!
      @brief Write the channel target voltage in mV and commit that channel
      @param channel Channel number
      @param mv Target voltage in mV
      @return int 0 on success, other values are communication failures
    '''
    ret = self._write_holding_register16(self._REG_HOLDING_CH1_VOLTAGE_MV + channel, mv)
    if ret != 0:
      return ret
    return self._write_holding_register16(self._REG_HOLDING_COMMIT_MASK, 1 << channel)

  def _i2c_probe(self, address):
    '''!
      @brief Write a pointer byte to an address; an ACK means a module is there
      @param address I2C address
      @return bool True when the address acknowledges
    '''
    if self._bus is None:
      return False
    try:
      self._bus.write_byte(address, self._REG_INPUT_VID & 0x7F)
    except OSError:
      return False
    return True

  def _range_supported(self, output_range):
    '''!
      @brief Whether the current board supports this range
      @param output_range Range code
      @return bool True when supported
    '''
    if self._sku < 2:
      return output_range == self.OUTPUT_RANGE10V
    return output_range in (self.OUTPUT_RANGE2_5V, self.OUTPUT_RANGE5V)

  def _range_max_mv(self, output_range):
    '''!
      @brief Full-scale voltage of a range
      @param output_range Range code
      @return int Full-scale voltage in mV
    '''
    if output_range == self.OUTPUT_RANGE2_5V:
      return 2500
    if output_range == self.OUTPUT_RANGE5V:
      return 5000
    if output_range == self.OUTPUT_RANGE10V:
      return 10000
    return 0

  def _pid_for_sku(self, sku):
    '''!
      @brief Product ID that belongs to a board type
      @param sku Board type number
      @return int Product ID
    '''
    if sku == 1:
      return 0x0531
    if sku == 2:
      return 0x0534
    if sku == 3:
      return 0x0535
    return 0x0530

  def begin(self):
    '''!
      @brief Initialize the module and cache board type, channel count, firmware version and ranges
      @return int
      @retval 0 Success
      @retval -1 I2C open failure
      @retval -2 Nothing answers on 0x40-0x43
      @retval -3 UART not open
      @retval -4 VID mismatch
      @retval -5 PID read failure
      @retval -6 SKU read failure
      @retval -7 Channel count is not 4/8
      @retval -8 PID does not match the SKU
    '''
    if self._use_i2c is True:
      try:
        self._bus = smbus.SMBus(self._i2c_bus)
      except (OSError, IOError):
        return -1
      found = False
      for address in range(self._I2C_ADDR_BASE, self._I2C_ADDR_MAX + 1):
        if self._i2c_probe(address) is True:
          self._addr = address
          found = True
          break
      if found is False:
        return -2
    elif self._rtu is None:
      return -3

    ret, vid = self._read_input_register16(self._REG_INPUT_VID)
    if ret != 0 or vid != 0x3343:
      return -4
    ret, pid = self._read_input_register16(self._REG_INPUT_PID)
    if ret != 0:
      return -5
    ret, sku = self._read_input_register16(self._REG_INPUT_BOARD_SKU)
    if ret != 0 or sku > 3:
      return -6
    ret, channels = self._read_input_register16(self._REG_INPUT_CHANNEL_COUNT)
    if ret != 0 or channels not in (4, 8):
      return -7
    if pid != self._pid_for_sku(sku):
      return -8

    self._sku = sku
    self._channels = channels
    ret, self._firmware_version = self._read_input_register16(self._REG_INPUT_FIRMWARE_VERSION)
    ret, enabled = self._read_input_register16(self._REG_INPUT_CHANNEL_ENABLE)
    if ret == 0:
      self._enable_mask = enabled
    else:
      self._enable_mask = (1 << self._channels) - 1
    for channel in range(self._channels):
      ret, value = self._read_input_register16(self._REG_INPUT_CH1_RANGE + channel)
      if ret == 0 and value <= self.OUTPUT_RANGE10V:
        self._range[channel] = value
      else:
        self._range[channel] = (self.OUTPUT_RANGE10V if self._sku < 2 else self.OUTPUT_RANGE5V)
    return 0

  def set_output_range(self, channel, output_range):
    '''!
      @brief Set the range of one channel; the channel first outputs 0 V
      @param channel Channel number 0-3 or 0-7
      @param output_range Range the board supports
      @return int
      @retval 0 Success
      @retval -1 Illegal channel
      @retval -2 Illegal or unsupported range
      @retval other Communication failure
    '''
    if self._channels == 0 or channel >= self._channels:
      return -1
    if self._range_supported(output_range) is False:
      return -2
    ret = self._write_holding_register16(self._REG_HOLDING_CH1_RANGE + channel, int(output_range))
    if ret != 0:
      return ret
    self._range[channel] = int(output_range)
    return 0

  def set_all_output_range(self, output_range):
    '''!
      @brief Set all channels to the same range
      @param output_range Range the board supports
      @return int
      @retval 0 Success
      @retval -1 Not initialized
      @retval -2 Illegal or unsupported range
      @retval other Communication failure
    '''
    if self._channels == 0:
      return -1
    if self._range_supported(output_range) is False:
      return -2
    values = [int(output_range)] * self._channels
    ret = self._write_register_block(self._REG_HOLDING_CH1_RANGE, values)
    if ret != 0:
      return ret
    for channel in range(self._channels):
      self._range[channel] = int(output_range)
    return 0

  def set_voltage(self, channel, voltage):
    '''!
      @brief Set one channel voltage and update it immediately
      @param channel Channel number 0-3 or 0-7
      @param voltage Output voltage in V, limited by the channel range
      @return int
      @retval 0 Success
      @retval -1 Illegal channel
      @retval -2 Negative voltage
      @retval -3 Above the channel range
      @retval other Communication failure
    '''
    if self._channels == 0 or channel >= self._channels:
      return -1
    if voltage < 0:
      return -2
    mv = int(voltage * 1000.0 + 0.5)
    if mv < 0 or mv > self._range_max_mv(self._range[channel]):
      return -3
    return self._write_voltage_mv(channel, mv)

  def set_sync_voltages(self, channels, voltages):
    '''!
      @brief Set several channels and make them take effect together
      @param channels List of channel numbers, duplicates are rejected
      @param voltages Matching list of voltages in V
      @return int
      @retval 0 Success
      @retval -1 Not initialized or empty list
      @retval -2 Illegal length
      @retval -3 Illegal channel
      @retval -4 Duplicate channel
      @retval -5 Negative voltage
      @retval -6 Above the channel range
      @retval other Communication failure
    '''
    if self._channels == 0 or channels is None or voltages is None:
      return -1
    length = len(channels)
    if length == 0 or length > self._channels or len(voltages) < length:
      return -2
    values = [0] * self._MAX_CHANNELS
    used = 0
    for index in range(length):
      channel = channels[index]
      if channel >= self._channels:
        return -3
      bit = 1 << channel
      if used & bit:
        return -4
      if voltages[index] < 0:
        return -5
      mv = int(voltages[index] * 1000.0 + 0.5)
      if mv < 0 or mv > self._range_max_mv(self._range[channel]):
        return -6
      used |= bit
      values[channel] = mv
    for index in range(length):
      channel = channels[index]
      ret = self._write_holding_register16(self._REG_HOLDING_CH1_VOLTAGE_MV + channel, values[channel])
      if ret != 0:
        return ret
    return self._write_holding_register16(self._REG_HOLDING_COMMIT_MASK, used)

  def set_all_voltage(self, voltages):
    '''!
      @brief Set all channels and make them take effect together
      @param voltages Voltages in V ordered CH0, CH1, ...; length equals the channel count
      @return int
      @retval 0 Success
      @retval -1 Not initialized or empty list
      @retval -2 Length does not match the channel count
      @retval -3 Negative voltage
      @retval -4 Above the channel range
      @retval other Communication failure
    '''
    if self._channels == 0 or voltages is None:
      return -1
    if len(voltages) != self._channels:
      return -2
    values = [0] * self._MAX_CHANNELS
    for channel in range(self._channels):
      if voltages[channel] < 0:
        return -3
      mv = int(voltages[channel] * 1000.0 + 0.5)
      if mv < 0 or mv > self._range_max_mv(self._range[channel]):
        return -4
      values[channel] = mv
    ret = self._write_register_block(self._REG_HOLDING_CH1_VOLTAGE_MV, values)
    if ret != 0:
      return ret
    return self._write_holding_register16(self._REG_HOLDING_COMMIT_MASK, (1 << self._channels) - 1)

  def set_channel_enabled(self, channel, enabled):
    '''!
      @brief Enable or disable one channel
      @param channel Channel number 0-3 or 0-7
      @param enabled True to enable, False to disable
      @return int
      @retval 0 Success
      @retval -1 Illegal channel
      @retval other Communication failure
    '''
    if self._channels == 0 or channel >= self._channels:
      return -1
    mask = self._enable_mask
    bit = 1 << channel
    if enabled is True:
      mask |= bit
    else:
      mask &= ~bit
    ret = self._write_holding_register16(self._REG_HOLDING_CHANNEL_ENABLE, mask & 0xFFFF)
    if ret == 0:
      self._enable_mask = mask
    return ret

  def stop_output(self):
    '''!
      @brief Set all channels to 0 V together, keeping range and enable state
      @return int 0 on success, other values are communication failures
    '''
    return self._write_holding_register16(self._REG_HOLDING_COMMAND, self._CMD_STOP_ALL)

  def read_firmware_version(self):
    '''!
      @brief Read the firmware version, e.g. 0x1000 means V1.0.0.0
      @return int Firmware version, or the cached value (0 before the first read)
    '''
    ret, value = self._read_input_register16(self._REG_INPUT_FIRMWARE_VERSION)
    if ret == 0:
      self._firmware_version = value
    return self._firmware_version

  def get_sku(self):
    '''!
      @brief Read the board type
      @return int
      @retval 0 DFR1328
      @retval 1 DFR1329
      @retval 2 DFR1332
      @retval 3 DFR1333
      @retval 0xFF Read failure
    '''
    ret, value = self._read_input_register16(self._REG_INPUT_BOARD_SKU)
    if ret != 0 or value > 3:
      return 0xFF
    return value

  def get_channel_count(self):
    '''!
      @brief Read the channel count
      @return int 4 or 8, 0 before begin()
    '''
    return self._channels

  def get_voltage(self, channel):
    '''!
      @brief Read the applied voltage of one channel
      @param channel Channel number 0-3 or 0-7
      @return float Voltage in V, 0 for an illegal channel or a read failure
    '''
    if self._channels == 0 or channel >= self._channels:
      return 0.0
    ret, value = self._read_input_register16(self._REG_INPUT_CH1_VOLTAGE_MV + channel)
    if ret != 0:
      return 0.0
    return value / 1000.0

  def get_range(self, channel):
    '''!
      @brief Read the current range of one channel
      @param channel Channel number 0-3 or 0-7
      @return int Range code; OUTPUT_RANGE10V for an illegal channel or a read failure
    '''
    if self._channels == 0 or channel >= self._channels:
      return self.OUTPUT_RANGE10V
    ret, value = self._read_input_register16(self._REG_INPUT_CH1_RANGE + channel)
    if ret != 0 or value > self.OUTPUT_RANGE10V:
      return self.OUTPUT_RANGE10V
    return value

  def get_channel_enabled(self, channel):
    '''!
      @brief Read the enable state of one channel
      @param channel Channel number 0-3 or 0-7
      @return bool True if enabled; False for an illegal channel or a read failure
    '''
    if self._channels == 0 or channel >= self._channels:
      return False
    ret, value = self._read_input_register16(self._REG_INPUT_CHANNEL_ENABLE)
    if ret == 0:
      self._enable_mask = value
    return (self._enable_mask & (1 << channel)) != 0

  def get_status(self):
    '''!
      @brief Read the module status bitmap
      @return int Status bitmap, bit0=1 means a DAC reference alarm; 0 on a read failure
    '''
    ret, value = self._read_input_register16(self._REG_INPUT_STATUS)
    if ret != 0:
      return 0
    return value
