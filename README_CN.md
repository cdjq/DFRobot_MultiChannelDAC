# DFRobot_MultiChannelDAC

[English Version](README.md)

## 产品链接

- [DFRobot_MultiChannelDAC](https://github.com/DFRobot/DFRobot_MultiChannelDAC)

## 目录

- [产品链接](#产品链接)
- [目录](#目录)
- [概述](#概述)
- [库安装](#库安装)
- [方法](#方法)
- [兼容性](#兼容性)
- [历史](#历史)
- [创作者](#创作者)

## 概述

DFR1328/DFR1329/DFR1332/DFR1333 4/8 通道同步电压输出 DAC 模块 Arduino 库。

DFRobot 多通道 DAC 模块板载 RP2040，通过 SPI 驱动 DAC80504 或 DAC80508。主机 MCU 不直接操作 DAC 芯片，而是通过 I2C、UART 或 USB CDC 读写 General RTU 寄存器表。

- DFR1328：4 通道，0～10 V
- DFR1329：8 通道，0～10 V
- DFR1332：4 通道，0～2.5 V / 0～5 V
- DFR1333：8 通道，0～2.5 V / 0～5 V

每个通道可以独立设置、独立启用或停用，也可以单独更新；多个通道或全部通道可以用一次提交同步生效。每路还可以输出正弦、方波、三角、锯齿或 256 点自定义波形，频率最高 5000 Hz。

烧录 `setSingleChannelVoltage` 后，用 115200 8N1 打开串口会看到：

```
DFRobot MultiChannel DAC - single channel
firmware version: 0x…
board SKU: …
channel count: …
CH0 voltage: … V
```

### 连接

#### I2C

| 模块 | 主机 |
|------|------|
| Gravity SDA（GPIO0） | 主机 SDA |
| Gravity SCL（GPIO1） | 主机 SCL |
| GND | GND |
| VCC | 3.3 V 或 5 V |

I2C 地址由 ADDR0/ADDR1 决定，范围 0x40～0x43。出厂两脚外部上拉为高，默认地址 0x43。`begin()` 会依次探测 0x40～0x43。

模块上的 I2C/UART 拨码开关拨到 **I2C**（GPIO4 高电平，出厂默认）。

#### UART

| 模块 | 主机 |
|------|------|
| Gravity TX（GPIO0） | 主机 RX |
| Gravity RX（GPIO1） | 主机 TX |
| GND | GND |
| VCC | 3.3 V 或 5 V |

出厂串口参数为 9600、无校验、2 停止位（8N2），General RTU 从站地址 0x0F。

模块上的 I2C/UART 拨码开关拨到 **UART**（GPIO4 低电平）。

#### USB CDC

模块 USB 口接主机即可。USB CDC 与 UART 使用同一套 General RTU 帧和同一个从站地址，主机不需要匹配波特率。

## 库安装

- 在 Arduino 库管理器中搜索 `DFRobot_MultiChannelDAC` 并安装。
- 或下载库压缩包，解压到 Arduino IDE 的 `libraries` 目录。

本库依赖 `DFRobot_RTU`，请按同样方式安装。

## 方法

```cpp
/**
 * @fn DFRobot_MultiChannelDAC
 * @brief I2C 构造。begin() 会依次探测 0x40～0x43。
 * @param wire Wire 对象指针，默认 &Wire。
 * @n I2C 地址由 ADDR0/ADDR1 拨码决定，出厂外部上拉默认 0x43。
 */
DFRobot_MultiChannelDAC(TwoWire *wire = &Wire);

/**
 * @fn DFRobot_MultiChannelDAC
 * @brief UART/USB CDC 构造，使用 DFRobot General RTU 协议。
 * @param serial 已打开的 Stream 对象，例如 &Serial1 或 &Serial。
 * @n 出厂 9600 8N2、从站地址 0x0F；USB CDC 忽略波特率。
 */
DFRobot_MultiChannelDAC(Stream *serial);

/**
 * @fn begin
 * @brief 初始化模块，读取板型、通道数和固件版本。
 * @return 0 表示成功；非 0 表示探测失败、VID/PID 不符或板型不支持。
 * @n 不改变通道输出和量程；模块上电后默认全部通道 0 V，DFR1328/DFR1329 默认 0～10 V，DFR1332/DFR1333 默认 0～5 V。
 */
int begin(void);

/**
 * @fn setOutputRange
 * @brief 设置一个通道的输出量程。
 * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印 CH0～CH3 一致。
 * @param range 量程，只有板型支持的档位才有效。
 * - eOutputRange2_5V：0～2.5 V，仅 DFR1332/DFR1333。
 * - eOutputRange5V：0～5 V，仅 DFR1332/DFR1333。
 * - eOutputRange10V：0～10 V，仅 DFR1328/DFR1329。
 * @return 0 表示成功；非 0 表示通道、量程非法或通信失败。
 * @n 修改量程时该通道先输出 0 V，切换后保持 0 V；目标电压同时清 0。
 */
int setOutputRange(uint8_t channel, eOutputRange_t range);

/**
 * @fn setAllOutputRange
 * @brief 把全部通道设置为同一量程。
 * @param range 量程，只有板型支持的档位才有效。
 * - eOutputRange2_5V：0～2.5 V，仅 DFR1332/DFR1333。
 * - eOutputRange5V：0～5 V，仅 DFR1332/DFR1333。
 * - eOutputRange10V：0～10 V，仅 DFR1328/DFR1329。
 * @return 0 表示成功；非 0 表示量程非法或通信失败。
 * @n 调用时全部通道先同步回 0 V，再完成量程切换。
 */
int setAllOutputRange(eOutputRange_t range);

/**
 * @fn setVoltage
 * @brief 设置一个通道的输出电压并立即更新。
 * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印 CH0～CH3 一致。
 * @param voltage 输出电压，单位 V，范围由该通道当前量程决定。
 * @return 0 表示成功；非 0 表示通道、电压非法或通信失败。
 * @n 只更新指定通道；连续调用多个单通道接口不保证同时生效。
 */
int setVoltage(uint8_t channel, float voltage);

/**
 * @fn setSyncVoltages
 * @brief 一次设置多个指定通道，并使这些通道同步生效。
 * @param channels 通道编号数组，编号 0～3 或 0～7；不允许重复通道。
 * @param voltages 电压数组，与 channels 数组逐一对应，单位 V。
 * @param length 数组长度，1～通道数。
 * @return 0 表示成功；非 0 表示通道重复、通道或电压非法、通信失败。
 * @n 未选中的通道保持原来的输出；任一参数错误时所有通道都不更新。
 */
int setSyncVoltages(const uint8_t channels[], const float voltages[], uint8_t length);

/**
 * @fn setAllVoltage
 * @brief 一次设置全部通道，并使全部通道同步生效。
 * @param voltage 电压数组，按 CH0、CH1……排列，单位 V。
 * @param length 数组长度，4 通道模块填 4，8 通道模块填 8。
 * @return 0 表示成功；非 0 表示长度、电压非法或通信失败。
 * @n 任一数值错误时全部通道都不更新。
 */
int setAllVoltage(const float voltage[], uint8_t length);

/**
 * @fn setChannelEnabled
 * @brief 独立启用或停用一个通道。
 * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印 CH0～CH3 一致。
 * @param enabled true 表示启用，false 表示停用。
 * @return 0 表示成功；非 0 表示通道非法或通信失败。
 * @n 停用后输出约 0 V，但不是高阻态；重新启用后恢复该通道最后一次设置的目标电压。
 */
int setChannelEnabled(uint8_t channel, bool enabled);

/**
 * @fn stopOutput
 * @brief 将全部通道同步设置为 0 V，并停止全部波形。
 * @return 0 表示成功；非 0 表示通信失败。
 * @n 不改变量程和通道启用状态；后续调用 setVoltage() 或 setAllVoltage() 即可恢复输出。
 */
int stopOutput(void);

/**
 * @fn setWaveform
 * @brief 写入一路波形参数，不启动播放。
 * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印一致。
 * @param type 波形类型。
 * - eWaveformDc：直流，输出等于偏置。
 * - eWaveformSine：正弦。
 * - eWaveformSquare：方波，占空比有效。
 * - eWaveformTriangle：三角。
 * - eWaveformSawtooth：锯齿。
 * - eWaveformCustom：自定义，使用 writeCustomWave() 写入的 256 点。
 * @param freqHz 频率，单位 Hz，0～5000。0 表示停在当前相位。
 * @param amplitude 峰值幅度，单位 V。输出范围是偏置减幅度到偏置加幅度，必须落在当前量程内。
 * @param offset 偏置，单位 V。
 * @param dutyPercent 方波占空比，0～100，其他波形忽略。默认 50。
 * @param phaseDeg 起始相位，0～360，默认 0。
 * @return 0 表示成功；非 0 表示通道、参数超出范围或通信失败。
 * @n 正在播放时写入会立即改变这一路的参数，相位累加器不重置。要从头播放，再调用 startWaveform()。
 */
int setWaveform(uint8_t channel, eWaveform_t type, float freqHz, float amplitude, float offset, float dutyPercent = 50, float phaseDeg = 0);

/**
 * @fn writeCustomWave
 * @brief 写入一路自定义波形的 256 个形状点。
 * @param channel 通道编号，0～3 或 0～7。
 * @param samples 256 个点。0 对应偏置减幅度，32768 对应偏置，65535 对应偏置加幅度。
 * @return 0 表示成功；非 0 表示通道非法、指针为空或通信失败。
 * @n 需先用 setWaveform() 把类型设为 eWaveformCustom。点数据掉电不保存。
 */
int writeCustomWave(uint8_t channel, const uint16_t samples[256]);

/**
 * @fn startWaveform
 * @brief 按给定通道一起启动波形，相位同时清零。
 * @param channels 通道编号数组，0～3 或 0～7，不允许重复。
 * @param length 数组长度，1～通道数。
 * @return 0 表示成功；非 0 表示通道非法、重复或通信失败。
 * @n 两路配合的波形必须放在同一次调用里。单路可直接传通道号：startWaveform(0)。
 */
int startWaveform(const uint8_t channels[], uint8_t length);
int startWaveform(uint8_t channel);

/**
 * @fn stopWaveform
 * @brief 停止给定通道的波形，电压停在最后一拍。
 * @param channels 通道编号数组，0～3 或 0～7，不允许重复。
 * @param length 数组长度，1～通道数。
 * @return 0 表示成功；非 0 表示通道非法、重复或通信失败。
 * @n 不会回到 0 V。要全部归零，调用 stopOutput()。单路可直接传通道号：stopWaveform(0)。
 */
int stopWaveform(const uint8_t channels[], uint8_t length);
int stopWaveform(uint8_t channel);

/**
 * @fn readFirmwareVersion
 * @brief 读取固件版本号。
 * @return 固件版本，例如 0x1000 表示 V1.0.0.0；读取失败返回 0。
 * @n 数据来自模块输入寄存器 0x0003，不触发主动上报。
 */
uint16_t readFirmwareVersion(void);

/**
 * @fn getSKU
 * @brief 读取板型编号。
 * @return 0=DFR1328、1=DFR1329、2=DFR1332、3=DFR1333；读取失败返回 0xFF。
 * @n 板型由模块 SKU 引脚识别；当前硬件未实现时模块固定返回 0。
 */
uint8_t getSKU(void);

/**
 * @fn getChannelCount
 * @brief 读取输出通道数。
 * @return 4 或 8；未初始化时返回 0。
 * @n 由 begin() 读取的输入寄存器 0x0005 缓存。
 */
uint8_t getChannelCount(void);

/**
 * @fn getVoltage
 * @brief 读取一个通道当前生效的输出电压。
 * @param channel 通道编号，0～3 或 0～7。
 * @return 电压，单位 V；通道非法或读取失败返回 0。
 * @n 数据来自模块输入寄存器 0x0007～0x000E；停用的通道读 0。
 */
float getVoltage(uint8_t channel);

/**
 * @fn getRange
 * @brief 读取一个通道当前量程。
 * @param channel 通道编号，0～3 或 0～7。
 * @return 量程枚举；通道非法或读取失败返回 eOutputRange10V。
 * @n 数据来自模块输入寄存器 0x000F～0x0016。
 */
eOutputRange_t getRange(uint8_t channel);

/**
 * @fn getChannelEnabled
 * @brief 读取一个通道的启用状态。
 * @param channel 通道编号，0～3 或 0～7。
 * @return true 表示启用，false 表示停用或读取失败。
 * @n 数据来自模块输入寄存器 0x0017 的对应位。
 */
bool getChannelEnabled(uint8_t channel);

/**
 * @fn getStatus
 * @brief 读取模块状态。
 * @return 状态位图，bit0=1 表示 DAC 参考异常；读取失败返回 0。
 * @n 数据来自模块输入寄存器 0x0006。
 */
uint16_t getStatus(void);
```

通道编号从 0 开始，与 PCB 丝印 CH0～CH3 / CH0～CH7 一致。`setSyncVoltages()` 由 `length` 指定数组长度，例如 `{0, 1, 2}` 配 `length = 3`。

### 示例

- `setSingleChannelVoltage`：初始化、读取板型、设置 CH0 量程和电压并读回。
- `setSyncChannelsVoltage`：多个指定通道同步更新。
- `setAllChannelsVoltage`：全部通道同步更新。
- `twoPointCalibration`：用万用表在 10% 和 90% 两点校准 CH0。
- `setWaveform`：CH0 输出 10 Hz 正弦，CH0 与 CH1 同时启动。

## 兼容性

| 主板 | 通过 | 未通过 | 未测试 | 备注 |
|------|------|--------|--------|------|
| Arduino UNO | | | √ | |
| Arduino MEGA 2560 | | | √ | |
| ESP32 | | | √ | |
| Raspberry Pi Pico（RP2040） | | | √ | |

模块固件为 DFRobot General RTU 2.0。

## 历史

- 2026/11/16 - V1.0.0 版本
- 2026/11/20 - V1.1.0 版本，增加波形参数、自定义形状和同步启动/停止接口

## 创作者

由 wxzed 编写（xiao.wu@dfrobot.com），2026。[DFRobot](https://www.dfrobot.com)
