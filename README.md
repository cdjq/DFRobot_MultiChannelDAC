# DFRobot_MultiChannelDAC

A 4/8-channel synchronous voltage-output DAC module library for DFR1328, DFR1329, DFR1332 and DFR1333.

## Introduction

The DFRobot MultiChannel DAC modules use an on-board RP2040 to drive a DAC80504 or DAC80508 over SPI. The host MCU does not touch the DAC directly: it reads and writes a General RTU register table over I2C, UART or USB CDC.

- DFR1328: 4 channels, 0–10 V
- DFR1329: 8 channels, 0–10 V
- DFR1332: 4 channels, 0–2.5 V / 0–5 V
- DFR1333: 8 channels, 0–2.5 V / 0–5 V

Each channel can be set independently, enabled or disabled, and updated alone. Several channels or all channels can be updated synchronously with one commit.

## Connection

### I2C

| Module | Host |
|--------|------|
| Gravity SDA (GPIO0) | Host SDA |
| Gravity SCL (GPIO1) | Host SCL |
| GND | GND |
| VCC | 3.3 V or 5 V |

The I2C address is selected by ADDR0/ADDR1 and is 0x40–0x43. With the factory pull-ups both pins are high, so the default address is 0x43. The `begin()` method scans 0x40–0x43 automatically.

Set the I2C/UART switch to **I2C** (GPIO4 high, the factory default).

### UART

| Module | Host |
|--------|------|
| Gravity TX (GPIO0) | Host RX |
| Gravity RX (GPIO1) | Host TX |
| GND | GND |
| VCC | 3.3 V or 5 V |

The factory UART setting is 9600 baud, no parity, 2 stop bits (8N2), General RTU slave address 0x0F.

Set the I2C/UART switch to **UART** (GPIO4 low).

### USB CDC

Connect the module USB port to the host. The USB CDC port uses the same General RTU frames and the same slave address. The host does not need to match a baud rate.

## API

```cpp
/**
 * @brief I2C constructor. begin() scans 0x40–0x43.
 * @param wire Wire object, defaults to &Wire.
 * @n I2C address is set by ADDR0/ADDR1; factory pull-ups make it 0x43.
 */
DFRobot_MultiChannelDAC(TwoWire *wire = &Wire);

/**
 * @brief UART/USB CDC constructor using DFRobot General RTU.
 * @param serial Opened Stream, for example &Serial1 or &Serial.
 * @n Factory UART is 9600 8N2, slave address 0x0F. USB CDC ignores baud rate.
 */
DFRobot_MultiChannelDAC(Stream *serial);

/**
 * @brief Initialize the module and read board type, channel count and firmware version.
 * @return 0 on success; non-zero on probe, VID/PID or board-type failure.
 * @n All channels output 0 V after init. Default range is 0–10 V on DFR1328/DFR1329 and 0–5 V on DFR1332/DFR1333.
 */
int begin(void);

/**
 * @brief Set the output range of one channel.
 * @param channel Channel number, 0–3 or 0–7, matching PCB silkscreen CH0–CH3.
 * @param range eOutputRange2_5V, eOutputRange5V or eOutputRange10V; only ranges supported by the board are valid.
 * @return 0 on success; non-zero on illegal channel/range or communication failure.
 * @n The channel first outputs 0 V, then switches range and stays at 0 V. The target voltage is cleared.
 */
int setOutputRange(uint8_t channel, eOutputRange_t range);

/**
 * @brief Set all channels to the same output range.
 * @param range eOutputRange2_5V, eOutputRange5V or eOutputRange10V; only ranges supported by the board are valid.
 * @return 0 on success; non-zero on illegal range or communication failure.
 * @n All channels first return to 0 V, then switch range together.
 */
int setAllOutputRange(eOutputRange_t range);

/**
 * @brief Set one channel voltage and update it immediately.
 * @param channel Channel number, 0–3 or 0–7, matching PCB silkscreen CH0–CH3.
 * @param voltage Output voltage in V, limited by the current range of that channel.
 * @return 0 on success; non-zero on illegal channel/voltage or communication failure.
 * @n Only the selected channel is updated. Consecutive single-channel calls are not synchronous.
 */
int setVoltage(uint8_t channel, float voltage);

/**
 * @brief Set several channels and update them synchronously.
 * @param channels Channel array with numbers 0–3 or 0–7; duplicates are rejected.
 * @param voltages Voltage array in V, matching channels one by one.
 * @param length Array length, 1 to the channel count.
 * @return 0 on success; non-zero on duplicate/illegal channel, illegal voltage or communication failure.
 * @n Unselected channels keep their previous output. If any parameter is invalid, no channel is updated.
 */
int setSyncVoltages(const uint8_t channels[], const float voltages[], uint8_t length);

/**
 * @brief Set all channels and update them synchronously.
 * @param voltage Voltage array in V, ordered CH0, CH1, ...
 * @param length Array length: 4 for a 4-channel module, 8 for an 8-channel module.
 * @return 0 on success; non-zero on illegal length/voltage or communication failure.
 * @n If any value is invalid, no channel is updated.
 */
int setAllVoltage(const float voltage[], uint8_t length);

/**
 * @brief Enable or disable one channel.
 * @param channel Channel number, 0–3 or 0–7, matching PCB silkscreen CH0–CH3.
 * @param enabled true to enable, false to disable.
 * @return 0 on success; non-zero on illegal channel or communication failure.
 * @n A disabled channel outputs about 0 V, not high impedance. Re-enabling restores its last target voltage.
 */
int setChannelEnabled(uint8_t channel, bool enabled);

/**
 * @brief Set all channels to 0 V synchronously.
 * @return 0 on success; non-zero on communication failure.
 * @n Range and enable state are unchanged. A later setVoltage() or setAllVoltage() restores output.
 */
int stopOutput(void);

/**
 * @brief Read the firmware version.
 * @return Firmware version, for example 0x1000 for V1.0.0.0; 0 on read failure.
 * @n The value comes from input register 0x0003; there is no active report.
 */
uint16_t readFirmwareVersion(void);

/**
 * @brief Read the board type.
 * @return 0=DFR1328, 1=DFR1329, 2=DFR1332, 3=DFR1333; 0xFF on read failure.
 * @n Board type comes from the SKU pins; current hardware returns 0 when the pins are not implemented.
 */
uint8_t getSKU(void);

/**
 * @brief Read the output channel count.
 * @return 4 or 8; 0 before begin().
 * @n Cached from input register 0x0005 read by begin().
 */
uint8_t getChannelCount(void);

/**
 * @brief Read the applied voltage of one channel.
 * @param channel Channel number, 0–3 or 0–7.
 * @return Voltage in V; 0 for an illegal channel or read failure.
 * @n The value comes from input registers 0x0007–0x000E; a disabled channel reads 0.
 */
float getVoltage(uint8_t channel);

/**
 * @brief Read the current range of one channel.
 * @param channel Channel number, 0–3 or 0–7.
 * @return Range enum; eOutputRange10V for an illegal channel or read failure.
 * @n The value comes from input registers 0x000F–0x0016.
 */
eOutputRange_t getRange(uint8_t channel);

/**
 * @brief Read the enable state of one channel.
 * @param channel Channel number, 0–3 or 0–7.
 * @return true if enabled, false if disabled or read failed.
 * @n The value comes from the matching bit of input register 0x0017.
 */
bool getChannelEnabled(uint8_t channel);

/**
 * @brief Read the module status.
 * @return Status bitmap, bit0=1 means DAC reference alarm; 0 on read failure.
 * @n The value comes from input register 0x0006.
 */
uint16_t getStatus(void);
```

Channel numbers start at 0 and match the PCB silkscreen CH0–CH3 / CH0–CH7. `setSyncVoltages()` takes the channel array and its length, for example `{0, 1, 2}` with `length = 3`.

## Examples

- `setSingleChannelVoltage`: initialize, read board type, set CH0 range and voltage, read it back.
- `setSyncChannelsVoltage`: update several channels synchronously.
- `setAllChannelsVoltage`: update all channels synchronously.
- `twoPointCalibration`: use a multimeter to calibrate CH0 at 10 % and 90 % of full scale.

## Compatibility

- MCU: AVR, ESP32, RP2040 and other Arduino-compatible boards with `Wire` or `Stream`.
- Module firmware: DFRobot General RTU 2.0.
- Library dependency: `DFRobot_RTU`.

## History

- 2026/11/16 - V1.0.0: first release for DFR1328/DFR1329/DFR1332/DFR1333.

## Credits

Written by DFRobot (github@dfrobot.com).
