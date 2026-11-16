# DFRobot_MultiChannelDAC

DFR1328/DFR1329/DFR1332/DFR1333 4/8 通道同步电压输出 DAC 模块 Arduino 库。

## 简介

DFRobot 多通道 DAC 模块板载 RP2040，通过 SPI 驱动 DAC80504 或 DAC80508。主机 MCU 不直接操作 DAC 芯片，而是通过 I2C、UART 或 USB CDC 读写 General RTU 寄存器表。

- DFR1328：4 通道，0～10 V
- DFR1329：8 通道，0～10 V
- DFR1332：4 通道，0～2.5 V / 0～5 V
- DFR1333：8 通道，0～2.5 V / 0～5 V

每个通道可以独立设置、独立启用或停用，也可以单独更新；多个通道或全部通道可以用一次提交同步生效。

## 连接

### I2C

| 模块 | 主机 |
|------|------|
| Gravity SDA（GPIO0） | 主机 SDA |
| Gravity SCL（GPIO1） | 主机 SCL |
| GND | GND |
| VCC | 3.3 V 或 5 V |

I2C 地址由 ADDR0/ADDR1 决定，范围 0x40～0x43。出厂两脚外部上拉为高，默认地址 0x43。`begin()` 会依次探测 0x40～0x43。

模块上的 I2C/UART 拨码开关拨到 **I2C**（GPIO4 高电平，出厂默认）。

### UART

| 模块 | 主机 |
|------|------|
| Gravity TX（GPIO0） | 主机 RX |
| Gravity RX（GPIO1） | 主机 TX |
| GND | GND |
| VCC | 3.3 V 或 5 V |

出厂串口参数为 9600、无校验、2 停止位（8N2），General RTU 从站地址 0x0F。

模块上的 I2C/UART 拨码开关拨到 **UART**（GPIO4 低电平）。

### USB CDC

模块 USB 口接主机即可。USB CDC 与 UART 使用同一套 General RTU 帧和同一个从站地址，主机不需要匹配波特率。

## API

```cpp
/**
 * @brief I2C 构造。begin() 会依次探测 0x40～0x43。
 * @param wire Wire 对象指针，默认 &Wire。
 * @n I2C 地址由 ADDR0/ADDR1 拨码决定，出厂外部上拉默认 0x43。
 */
DFRobot_MultiChannelDAC(TwoWire *wire = &Wire);

/**
 * @brief UART/USB CDC 构造，使用 DFRobot General RTU 协议。
 * @param serial 已打开的 Stream 对象，例如 &Serial1 或 &Serial。
 * @n 出厂 9600 8N2、从站地址 0x0F；USB CDC 忽略波特率。
 */
DFRobot_MultiChannelDAC(Stream *serial);

/**
 * @brief 初始化模块，读取板型、通道数和固件版本。
 * @return 0 表示成功；非 0 表示探测失败、VID/PID 不符或板型不支持。
 * @n 初始化后全部通道输出 0 V；DFR1328/DFR1329 默认 0～10 V，DFR1332/DFR1333 默认 0～5 V。
 */
int begin(void);

/**
 * @brief 设置一个通道的输出量程。
 * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印 CH0～CH3 一致。
 * @param range eOutputRange2_5V、eOutputRange5V 或 eOutputRange10V；只有板型支持的档位才有效。
 * @return 0 表示成功；非 0 表示通道、量程非法或通信失败。
 * @n 修改量程时该通道先输出 0 V，切换后保持 0 V；目标电压同时清 0。
 */
int setOutputRange(uint8_t channel, eOutputRange_t range);

/**
 * @brief 把全部通道设置为同一量程。
 * @param range eOutputRange2_5V、eOutputRange5V 或 eOutputRange10V；只有板型支持的档位才有效。
 * @return 0 表示成功；非 0 表示量程非法或通信失败。
 * @n 调用时全部通道先同步回 0 V，再完成量程切换。
 */
int setAllOutputRange(eOutputRange_t range);

/**
 * @brief 设置一个通道的输出电压并立即更新。
 * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印 CH0～CH3 一致。
 * @param voltage 输出电压，单位 V，范围由该通道当前量程决定。
 * @return 0 表示成功；非 0 表示通道、电压非法或通信失败。
 * @n 只更新指定通道；连续调用多个单通道接口不保证同时生效。
 */
int setVoltage(uint8_t channel, float voltage);

/**
 * @brief 一次设置多个指定通道，并使这些通道同步生效。
 * @param channels 通道编号数组，编号 0～3 或 0～7；不允许重复通道。
 * @param voltages 电压数组，与 channels 数组逐一对应，单位 V。
 * @param length 数组长度，1～通道数。
 * @return 0 表示成功；非 0 表示通道重复、通道或电压非法、通信失败。
 * @n 未选中的通道保持原来的输出；任一参数错误时所有通道都不更新。
 */
int setSyncVoltages(const uint8_t channels[], const float voltages[], uint8_t length);

/**
 * @brief 一次设置全部通道，并使全部通道同步生效。
 * @param voltage 电压数组，按 CH0、CH1……排列，单位 V。
 * @param length 数组长度，4 通道模块填 4，8 通道模块填 8。
 * @return 0 表示成功；非 0 表示长度、电压非法或通信失败。
 * @n 任一数值错误时全部通道都不更新。
 */
int setAllVoltage(const float voltage[], uint8_t length);

/**
 * @brief 独立启用或停用一个通道。
 * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印 CH0～CH3 一致。
 * @param enabled true 表示启用，false 表示停用。
 * @return 0 表示成功；非 0 表示通道非法或通信失败。
 * @n 停用后输出约 0 V，但不是高阻态；重新启用后恢复该通道最后一次设置的目标电压。
 */
int setChannelEnabled(uint8_t channel, bool enabled);

/**
 * @brief 将全部通道同步设置为 0 V。
 * @return 0 表示成功；非 0 表示通信失败。
 * @n 不改变量程和通道启用状态；后续调用 setVoltage() 或 setAllVoltage() 即可恢复输出。
 */
int stopOutput(void);

/**
 * @brief 读取固件版本号。
 * @return 固件版本，例如 0x1000 表示 V1.0.0.0；读取失败返回 0。
 * @n 数据来自模块输入寄存器 0x0003，不触发主动上报。
 */
uint16_t readFirmwareVersion(void);

/**
 * @brief 读取板型编号。
 * @return 0=DFR1328、1=DFR1329、2=DFR1332、3=DFR1333；读取失败返回 0xFF。
 * @n 板型由模块 SKU 引脚识别；当前硬件未实现时模块固定返回 0。
 */
uint8_t getSKU(void);

/**
 * @brief 读取输出通道数。
 * @return 4 或 8；未初始化时返回 0。
 * @n 由 begin() 读取的输入寄存器 0x0005 缓存。
 */
uint8_t getChannelCount(void);

/**
 * @brief 读取一个通道当前生效的输出电压。
 * @param channel 通道编号，0～3 或 0～7。
 * @return 电压，单位 V；通道非法或读取失败返回 0。
 * @n 数据来自模块输入寄存器 0x0007～0x000E；停用的通道读 0。
 */
float getVoltage(uint8_t channel);

/**
 * @brief 读取一个通道当前量程。
 * @param channel 通道编号，0～3 或 0～7。
 * @return 量程枚举；通道非法或读取失败返回 eOutputRange10V。
 * @n 数据来自模块输入寄存器 0x000F～0x0016。
 */
eOutputRange_t getRange(uint8_t channel);

/**
 * @brief 读取一个通道的启用状态。
 * @param channel 通道编号，0～3 或 0～7。
 * @return true 表示启用，false 表示停用或读取失败。
 * @n 数据来自模块输入寄存器 0x0017 的对应位。
 */
bool getChannelEnabled(uint8_t channel);

/**
 * @brief 读取模块状态。
 * @return 状态位图，bit0=1 表示 DAC 参考异常；读取失败返回 0。
 * @n 数据来自模块输入寄存器 0x0006。
 */
uint16_t getStatus(void);
```

通道编号从 0 开始，与 PCB 丝印 CH0～CH3 / CH0～CH7 一致。`setSyncVoltages()` 由 `length` 指定数组长度，例如 `{0, 1, 2}` 配 `length = 3`。

## 示例

- `setSingleChannelVoltage`：初始化、读取板型、设置 CH0 量程和电压并读回。
- `setSyncChannelsVoltage`：多个指定通道同步更新。
- `setAllChannelsVoltage`：全部通道同步更新。
- `twoPointCalibration`：用万用表在 10% 和 90% 两点校准 CH0。

## 兼容性

- MCU：AVR、ESP32、RP2040 等带 `Wire` 或 `Stream` 的 Arduino 兼容板。
- 模块固件：DFRobot General RTU 2.0。
- 库依赖：`DFRobot_RTU`。

## 历史

- 2026/11/16 - V1.0.0：DFR1328/DFR1329/DFR1332/DFR1333 首次发布。

## 致谢

由 DFRobot 编写（github@dfrobot.com）。
