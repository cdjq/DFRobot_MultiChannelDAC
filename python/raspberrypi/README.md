# DFRobot_MultiChannelDAC

[中文版](./README_CN.md)

## Product Link

- [DFRobot_MultiChannelDAC](https://github.com/DFRobot/DFRobot_MultiChannelDAC)

## Table of Contents

- [Product Link](#product-link)
- [Table of Contents](#table-of-contents)
- [Summary](#summary)
- [Installation](#installation)
- [Methods](#methods)
- [Compatibility](#compatibility)
- [History](#history)
- [Credits](#credits)

## Summary

A Raspberry Pi Python library for the DFR1328 / DFR1329 / DFR1332 / DFR1333
4/8-channel synchronous voltage-output DAC modules.

The module runs its own firmware: the host never touches the DAC chip, it reads
and writes a General RTU register table over I2C or UART.

- DFR1328: 4 channels, 0–10 V
- DFR1329: 8 channels, 0–10 V
- DFR1332: 4 channels, 0–2.5 V / 0–5 V
- DFR1333: 8 channels, 0–2.5 V / 0–5 V

### Connection

I2C:

| Module | Raspberry Pi |
|--------|--------------|
| Gravity SDA (GPIO0) | SDA (GPIO2, pin 3) |
| Gravity SCL (GPIO1) | SCL (GPIO3, pin 5) |
| GND | GND |
| VCC | 3.3 V or 5 V |

UART:

| Module | Raspberry Pi |
|--------|--------------|
| Gravity TX (GPIO0) | RXD (GPIO15, pin 10) |
| Gravity RX (GPIO1) | TXD (GPIO14, pin 8) |
| GND | GND |
| VCC | 3.3 V or 5 V |

Set the I2C/UART switch on the module to **I2C** or **UART** to match the
wiring. The I2C address is 0x40–0x43 (factory pull-ups → 0x43) and `begin()`
probes 0x40–0x43 automatically. The factory UART setting is 9600 8N2 with
General RTU slave address 0x0F.

Run `examples/set_single_channel_voltage.py` and the terminal prints:

```
DFRobot MultiChannel DAC - single channel
firmware version: 0x1000
board SKU: 3
channel count: 8
CH0 voltage: 5.000 V
```

The examples also cover `set_sync_channels_voltage.py`,
`set_all_channels_voltage.py` and `two_point_calibration.py`.

## Installation

Copy `DFRobot_MultiChannelDAC.py` and the `examples` folder to the Raspberry Pi,
for example into `/home/pi/DFRobot_MultiChannelDAC`.

```bash
sudo pip3 install smbus pyserial
```

Enable I2C with `sudo raspi-config` (Interface Options → I2C) and reboot.

For UART mode also copy the existing `DFRobot_RTU.py`
(`python/raspberrypi/DFRobot_RTU.py` of the DFRobot_RTU library) next to
`DFRobot_MultiChannelDAC.py`; the serial frames, CRC and timeout are handled
there, not in this library.

## Methods

```python
'''!
  @fn __init__
  @brief Create the module object
  @details I2C mode matches DFRobot_MultiChannelDAC(TwoWire *wire = &Wire) and
  @n begin() probes 0x40-0x43. UART mode matches DFRobot_MultiChannelDAC(Stream *serial)
  @n and hands the port to DFRobot_RTU, which opens /dev/ttyAMA0.
  @param i2c_bus I2C bus number, usually 1 on a Raspberry Pi
  @param address I2C address, 0x40–0x43
  @param baud UART baud rate; None selects I2C
  @param bits UART data bits
  @param parity UART parity, 0 means none
  @param stopbit UART stop bits, the module ships with 2
'''
def __init__(self, i2c_bus=1, address=0x43, baud=None, bits=8, parity=0, stopbit=2)

'''!
  @fn begin
  @brief Initialize the module and cache board type, channel count, firmware version and ranges
  @return int
  @retval 0 Success
  @retval -1 I2C open failure
  @retval -2 Nothing answers on 0x40–0x43
  @retval -3 UART not open
  @retval -4 VID mismatch
  @retval -5 PID read failure
  @retval -6 SKU read failure
  @retval -7 Channel count is not 4/8
  @retval -8 PID does not match the SKU
'''
def begin(self)

'''!
  @fn set_output_range
  @brief Set the range of one channel; the channel first outputs 0 V
  @param channel Channel number 0–3 or 0–7
  @param output_range Range the board supports
  @return int
  @retval 0 Success
  @retval -1 Illegal channel
  @retval -2 Illegal or unsupported range
  @retval other Communication failure
'''
def set_output_range(self, channel, output_range)

'''!
  @fn set_all_output_range
  @brief Set all channels to the same range
  @param output_range Range supported by the board
  @return int
  @retval 0 Success
  @retval -1 Not initialized
  @retval -2 Illegal or unsupported range
  @retval other Communication failure
'''
def set_all_output_range(self, output_range)

'''!
  @fn set_voltage
  @brief Set one channel voltage and update it immediately
  @param channel Channel number 0–3 or 0–7
  @param voltage Output voltage in V, limited by the channel range
  @return int
  @retval 0 Success
  @retval -1 Illegal channel
  @retval -2 Negative voltage
  @retval -3 Above the channel range
  @retval other Communication failure
'''
def set_voltage(self, channel, voltage)

'''!
  @fn set_sync_voltages
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
def set_sync_voltages(self, channels, voltages)

'''!
  @fn set_all_voltage
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
def set_all_voltage(self, voltages)

'''!
  @fn set_channel_enabled
  @brief Enable or disable one channel
  @param channel Channel number 0–3 or 0–7
  @param enabled True to enable, False to disable
  @return int
  @retval 0 Success
  @retval -1 Illegal channel
  @retval other Communication failure
'''
def set_channel_enabled(self, channel, enabled)

'''!
  @fn stop_output
  @brief Set all channels to 0 V together, keeping range and enable state
  @return int 0 on success, other values are communication failures
'''
def stop_output(self)

'''!
  @fn read_firmware_version
  @brief Read the firmware version, e.g. 0x1000 means V1.0.0.0
  @return int Firmware version, or the cached value (0 before the first read)
'''
def read_firmware_version(self)

'''!
  @fn get_sku
  @brief Read the board type
  @return int
  @retval 0 DFR1328
  @retval 1 DFR1329
  @retval 2 DFR1332
  @retval 3 DFR1333
  @retval 0xFF Read failure
'''
def get_sku(self)

'''!
  @fn get_channel_count
  @brief Read the channel count
  @return int 4 or 8, 0 before begin()
'''
def get_channel_count(self)

'''!
  @fn get_voltage
  @brief Read the applied voltage of one channel
  @param channel Channel number 0–3 or 0–7
  @return float Voltage in V, 0 for an illegal channel or a read failure
'''
def get_voltage(self, channel)

'''!
  @fn get_range
  @brief Read the current range of one channel
  @param channel Channel number 0–3 or 0–7
  @return int Range code; OUTPUT_RANGE10V for an illegal channel or a read failure
'''
def get_range(self, channel)

'''!
  @fn get_channel_enabled
  @brief Read the enable state of one channel
  @param channel Channel number 0–3 or 0–7
  @return bool True if enabled; False for an illegal channel or a read failure
'''
def get_channel_enabled(self, channel)

'''!
  @fn get_status
  @brief Read the module status bitmap
  @return int Status bitmap, bit0=1 means a DAC reference alarm; 0 on a read failure
'''
def get_status(self)
```

### Examples

- `set_single_channel_voltage.py`: initialize, print board type, set CH0 range
  and voltage, read them back (main example).
- `set_sync_channels_voltage.py`: update three channels synchronously.
- `set_all_channels_voltage.py`: update all channels, dividing the full scale
  across the channels.
- `two_point_calibration.py`: use a multimeter to calibrate CH0 at 10 % and
  90 % of full scale.

```bash
cd python/raspberrypi/examples
sudo python3 set_single_channel_voltage.py
```

## Compatibility

| MCU | Work Well | Work Wrong | Untested | Remarks |
|-----|-----------|------------|----------|---------|
| Raspberry Pi 4B | | | √ | |

## History

- 2026/11/18 - V1.0.0 version

## Credits

Written by wxzed, 2026. [DFRobot](https://www.dfrobot.com)
