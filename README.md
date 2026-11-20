# DFRobot_MultiChannelDAC

[中文版](README_CN.md)

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

A 4/8-channel synchronous voltage-output DAC module library for DFR1328, DFR1329, DFR1332 and DFR1333.

The DFRobot MultiChannel DAC modules use an on-board RP2040 to drive a DAC80504 or DAC80508 over SPI. The host MCU does not touch the DAC directly: it reads and writes a General RTU register table over I2C, UART or USB CDC.

- DFR1328: 4 channels, 0–10 V
- DFR1329: 8 channels, 0–10 V
- DFR1332: 4 channels, 0–2.5 V / 0–5 V
- DFR1333: 8 channels, 0–2.5 V / 0–5 V

Each channel can be set independently, enabled or disabled, and updated alone. Several channels or all channels can be updated synchronously with one commit. Each channel can also output sine, square, triangle, sawtooth or a 256-point custom waveform, up to 5000 Hz.

After uploading `setSingleChannelVoltage` to the host board, open the serial port at 115200 8N1:

```
DFRobot MultiChannel DAC - single channel
firmware version: 0x…
board SKU: …
channel count: …
CH0 voltage: … V
```

### Connection

#### I2C

| Module | Host |
|--------|------|
| Gravity SDA (GPIO0) | Host SDA |
| Gravity SCL (GPIO1) | Host SCL |
| GND | GND |
| VCC | 3.3 V or 5 V |

The I2C address is selected by ADDR0/ADDR1 and is 0x40–0x43. With the factory pull-ups both pins are high, so the default address is 0x43. The `begin()` method scans 0x40–0x43 automatically.

Set the I2C/UART switch to **I2C** (GPIO4 high, the factory default).

#### UART

| Module | Host |
|--------|------|
| Gravity TX (GPIO0) | Host RX |
| Gravity RX (GPIO1) | Host TX |
| GND | GND |
| VCC | 3.3 V or 5 V |

The factory UART setting is 9600 baud, no parity, 2 stop bits (8N2), General RTU slave address 0x0F.

Set the I2C/UART switch to **UART** (GPIO4 low).

#### USB CDC

Connect the module USB port to the host. The USB CDC port uses the same General RTU frames and the same slave address. The host does not need to match a baud rate.

## Installation

- Search `DFRobot_MultiChannelDAC` in the Arduino Library Manager and install it.
- Or download the library ZIP and unzip it into the `libraries` folder of the Arduino IDE.

The library depends on `DFRobot_RTU`; install it the same way.

## Methods

```cpp
/**
 * @fn DFRobot_MultiChannelDAC
 * @brief I2C constructor. begin() scans 0x40–0x43.
 * @param wire Wire object, defaults to &Wire.
 * @n I2C address is set by ADDR0/ADDR1; factory pull-ups make it 0x43.
 */
DFRobot_MultiChannelDAC(TwoWire *wire = &Wire);

/**
 * @fn DFRobot_MultiChannelDAC
 * @brief UART/USB CDC constructor using DFRobot General RTU.
 * @param serial Opened Stream, for example &Serial1 or &Serial.
 * @n Factory UART is 9600 8N2, slave address 0x0F. USB CDC ignores baud rate.
 */
DFRobot_MultiChannelDAC(Stream *serial);

/**
 * @fn begin
 * @brief Initialize the module and read board type, channel count and firmware version.
 * @return 0 on success; non-zero on probe, VID/PID or board-type failure.
 * @n Output and range are unchanged by begin(). All channels are 0 V after the module powers up. Default range is 0–10 V on DFR1328/DFR1329 and 0–5 V on DFR1332/DFR1333.
 */
int begin(void);

/**
 * @fn setOutputRange
 * @brief Set the output range of one channel.
 * @param channel Channel number, 0–3 or 0–7, matching PCB silkscreen CH0–CH3.
 * @param range Output range; only ranges supported by the board are valid.
 * - eOutputRange2_5V: 0–2.5 V, DFR1332/DFR1333 only.
 * - eOutputRange5V: 0–5 V, DFR1332/DFR1333 only.
 * - eOutputRange10V: 0–10 V, DFR1328/DFR1329 only.
 * @return 0 on success; non-zero on illegal channel/range or communication failure.
 * @n The channel first outputs 0 V, then switches range and stays at 0 V. The target voltage is cleared.
 */
int setOutputRange(uint8_t channel, eOutputRange_t range);

/**
 * @fn setAllOutputRange
 * @brief Set all channels to the same output range.
 * @param range Output range; only ranges supported by the board are valid.
 * - eOutputRange2_5V: 0–2.5 V, DFR1332/DFR1333 only.
 * - eOutputRange5V: 0–5 V, DFR1332/DFR1333 only.
 * - eOutputRange10V: 0–10 V, DFR1328/DFR1329 only.
 * @return 0 on success; non-zero on illegal range or communication failure.
 * @n All channels first return to 0 V, then switch range together.
 */
int setAllOutputRange(eOutputRange_t range);

/**
 * @fn setVoltage
 * @brief Set one channel voltage and update it immediately.
 * @param channel Channel number, 0–3 or 0–7, matching PCB silkscreen CH0–CH3.
 * @param voltage Output voltage in V, limited by the current range of that channel.
 * @return 0 on success; non-zero on illegal channel/voltage or communication failure.
 * @n Only the selected channel is updated. Consecutive single-channel calls are not synchronous.
 */
int setVoltage(uint8_t channel, float voltage);

/**
 * @fn setSyncVoltages
 * @brief Set several channels and update them synchronously.
 * @param channels Channel array with numbers 0–3 or 0–7; duplicates are rejected.
 * @param voltages Voltage array in V, matching channels one by one.
 * @param length Array length, 1 to the channel count.
 * @return 0 on success; non-zero on duplicate/illegal channel, illegal voltage or communication failure.
 * @n Unselected channels keep their previous output. If any parameter is invalid, no channel is updated.
 */
int setSyncVoltages(const uint8_t channels[], const float voltages[], uint8_t length);

/**
 * @fn setAllVoltage
 * @brief Set all channels and update them synchronously.
 * @param voltage Voltage array in V, ordered CH0, CH1, ...
 * @param length Array length: 4 for a 4-channel module, 8 for an 8-channel module.
 * @return 0 on success; non-zero on illegal length/voltage or communication failure.
 * @n If any value is invalid, no channel is updated.
 */
int setAllVoltage(const float voltage[], uint8_t length);

/**
 * @fn setChannelEnabled
 * @brief Enable or disable one channel.
 * @param channel Channel number, 0–3 or 0–7, matching PCB silkscreen CH0–CH3.
 * @param enabled true to enable, false to disable.
 * @return 0 on success; non-zero on illegal channel or communication failure.
 * @n A disabled channel outputs about 0 V, not high impedance. Re-enabling restores its last target voltage.
 */
int setChannelEnabled(uint8_t channel, bool enabled);

/**
 * @fn stopOutput
 * @brief Set all channels to 0 V synchronously and stop every waveform.
 * @return 0 on success; non-zero on communication failure.
 * @n Range and enable state are unchanged. A later setVoltage() or setAllVoltage() restores output.
 */
int stopOutput(void);

/**
 * @fn setWaveform
 * @brief Write one channel's waveform parameters without starting playback.
 * @param channel Channel number, 0–3 or 0–7, matching the PCB silkscreen.
 * @param type Waveform type.
 * - eWaveformDc: DC, output equals the offset.
 * - eWaveformSine: sine.
 * - eWaveformSquare: square, duty cycle applies.
 * - eWaveformTriangle: triangle.
 * - eWaveformSawtooth: sawtooth.
 * - eWaveformCustom: custom, using the 256 points from writeCustomWave().
 * @param freqHz Frequency in Hz, 0–5000. 0 holds the current phase.
 * @param amplitude Peak amplitude in V. The output spans offset minus amplitude to offset plus amplitude and must stay inside the current range.
 * @param offset Offset in V.
 * @param dutyPercent Square-wave duty, 0–100. Ignored by other types. Default 50.
 * @param phaseDeg Start phase, 0–360. Default 0.
 * @return 0 on success; non-zero on an illegal channel or parameter, or communication failure.
 * @n Writing while a channel is playing changes its parameters immediately and does not reset the phase accumulator. Call startWaveform() to restart from the beginning.
 */
int setWaveform(uint8_t channel, eWaveform_t type, float freqHz, float amplitude, float offset, float dutyPercent = 50, float phaseDeg = 0);

/**
 * @fn writeCustomWave
 * @brief Write the 256 shape points of one custom waveform.
 * @param channel Channel number, 0–3 or 0–7.
 * @param samples 256 points. 0 is offset minus amplitude, 32768 is the offset, 65535 is offset plus amplitude.
 * @return 0 on success; non-zero on an illegal channel, a null pointer or communication failure.
 * @n Call setWaveform() with eWaveformCustom first. The points are not kept across power-off.
 */
int writeCustomWave(uint8_t channel, const uint16_t samples[256]);

/**
 * @fn startWaveform
 * @brief Start the listed channels together and clear their phase at the same time.
 * @param channels Channel array, 0–3 or 0–7, with no duplicates.
 * @param length Array length, 1 to the channel count.
 * @return 0 on success; non-zero on an illegal or duplicate channel, or communication failure.
 * @n Waveforms that must stay aligned belong in one call. One channel can be started as startWaveform(0).
 */
int startWaveform(const uint8_t channels[], uint8_t length);
int startWaveform(uint8_t channel);

/**
 * @fn stopWaveform
 * @brief Stop the listed channels and hold the last sample.
 * @param channels Channel array, 0–3 or 0–7, with no duplicates.
 * @param length Array length, 1 to the channel count.
 * @return 0 on success; non-zero on an illegal or duplicate channel, or communication failure.
 * @n This does not return the output to 0 V. Call stopOutput() to zero every channel. One channel can be stopped as stopWaveform(0).
 */
int stopWaveform(const uint8_t channels[], uint8_t length);
int stopWaveform(uint8_t channel);

/**
 * @fn readFirmwareVersion
 * @brief Read the firmware version.
 * @return Firmware version, for example 0x1000 for V1.0.0.0; 0 on read failure.
 * @n The value comes from input register 0x0003; there is no active report.
 */
uint16_t readFirmwareVersion(void);

/**
 * @fn getSKU
 * @brief Read the board type.
 * @return 0=DFR1328, 1=DFR1329, 2=DFR1332, 3=DFR1333; 0xFF on read failure.
 * @n Board type comes from the SKU pins; current hardware returns 0 when the pins are not implemented.
 */
uint8_t getSKU(void);

/**
 * @fn getChannelCount
 * @brief Read the output channel count.
 * @return 4 or 8; 0 before begin().
 * @n Cached from input register 0x0005 read by begin().
 */
uint8_t getChannelCount(void);

/**
 * @fn getVoltage
 * @brief Read the applied voltage of one channel.
 * @param channel Channel number, 0–3 or 0–7.
 * @return Voltage in V; 0 for an illegal channel or read failure.
 * @n The value comes from input registers 0x0007–0x000E; a disabled channel reads 0.
 */
float getVoltage(uint8_t channel);

/**
 * @fn getRange
 * @brief Read the current range of one channel.
 * @param channel Channel number, 0–3 or 0–7.
 * @return Range enum; eOutputRange10V for an illegal channel or read failure.
 * @n The value comes from input registers 0x000F–0x0016.
 */
eOutputRange_t getRange(uint8_t channel);

/**
 * @fn getChannelEnabled
 * @brief Read the enable state of one channel.
 * @param channel Channel number, 0–3 or 0–7.
 * @return true if enabled, false if disabled or read failed.
 * @n The value comes from the matching bit of input register 0x0017.
 */
bool getChannelEnabled(uint8_t channel);

/**
 * @fn getStatus
 * @brief Read the module status.
 * @return Status bitmap, bit0=1 means DAC reference alarm; 0 on read failure.
 * @n The value comes from input register 0x0006.
 */
uint16_t getStatus(void);
```

Channel numbers start at 0 and match the PCB silkscreen CH0–CH3 / CH0–CH7. `setSyncVoltages()` takes the channel array and its length, for example `{0, 1, 2}` with `length = 3`.

### Examples

- `setSingleChannelVoltage`: initialize, read board type, set CH0 range and voltage, read it back.
- `setSyncChannelsVoltage`: update several channels synchronously.
- `setAllChannelsVoltage`: update all channels synchronously.
- `twoPointCalibration`: use a multimeter to calibrate CH0 at 10 % and 90 % of full scale.
- `setWaveform`: output a 10 Hz sine on CH0, and start CH0 and CH1 together.

## Compatibility

| MCU | Work Well | Work Wrong | Untested | Remarks |
|-----|-----------|------------|----------|---------|
| Arduino UNO | | | √ | |
| Arduino MEGA 2560 | | | √ | |
| ESP32 | | | √ | |
| Raspberry Pi Pico (RP2040) | | | √ | |

## History

- 2026/11/16 - V1.0.0 version
- 2026/11/20 - V1.1.0 version. Adds waveform parameters, custom shapes, and synchronous start/stop.

## Credits

Written by wxzed (xiao.wu@dfrobot.com), 2026. [DFRobot](https://www.dfrobot.com)
