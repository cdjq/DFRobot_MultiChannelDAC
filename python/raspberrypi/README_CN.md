# DFRobot_MultiChannelDAC

[English Version](./README.md)

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

DFR1328 / DFR1329 / DFR1332 / DFR1333 4/8 通道同步电压输出 DAC 模块的树莓派
Python 库。

模块自带固件，主机不直接操作 DAC 芯片，而是通过 I2C 或 UART 读写一套
General RTU 寄存器表。

- DFR1328：4 通道，0～10 V
- DFR1329：8 通道，0～10 V
- DFR1332：4 通道，0～2.5 V / 0～5 V
- DFR1333：8 通道，0～2.5 V / 0～5 V

### 连接

I2C：

| 模块 | 树莓派 |
|------|--------|
| Gravity SDA（GPIO0） | SDA（GPIO2，3 号脚） |
| Gravity SCL（GPIO1） | SCL（GPIO3，5 号脚） |
| GND | GND |
| VCC | 3.3 V 或 5 V |

UART：

| 模块 | 树莓派 |
|------|--------|
| Gravity TX（GPIO0） | RXD（GPIO15，10 号脚） |
| Gravity RX（GPIO1） | TXD（GPIO14，8 号脚） |
| GND | GND |
| VCC | 3.3 V 或 5 V |

模块上的 I2C/UART 拨码开关要和接线一致。I2C 地址由 ADDR0/ADDR1 决定，范围
0x40～0x43（出厂外部上拉为高，默认 0x43），`begin()` 会依次探测 0x40～0x43。
出厂串口参数为 9600、无校验、2 停止位（8N2），General RTU 从站地址 0x0F。

运行 `examples/set_single_channel_voltage.py`，终端会打印：

```
DFRobot MultiChannel DAC - single channel
firmware version: 0x1000
board SKU: 3
channel count: 8
CH0 voltage: 5.000 V
```

其余示例是 `set_sync_channels_voltage.py`、`set_all_channels_voltage.py` 和
`two_point_calibration.py`。

## 库安装

把 `DFRobot_MultiChannelDAC.py` 和 `examples` 目录拷到树莓派上，例如
`/home/pi/DFRobot_MultiChannelDAC`。

```bash
sudo pip3 install smbus pyserial
```

用 `sudo raspi-config` 打开 I2C（Interface Options → I2C），然后重启。

UART 模式还要把现成的 `DFRobot_RTU.py`（DFRobot_RTU 库的
`python/raspberrypi/DFRobot_RTU.py`）放到 `DFRobot_MultiChannelDAC.py` 同目录，
串口帧、CRC 和超时都由它处理，本库不重写。

## 方法

```python
'''!
  @fn __init__
  @brief 建立模块对象
  @details I2C 模式对应 DFRobot_MultiChannelDAC(TwoWire *wire = &Wire)，
  @n begin() 会探测 0x40～0x43；UART 模式对应 DFRobot_MultiChannelDAC(Stream *serial)，
  @n 串口交给 DFRobot_RTU 打开 /dev/ttyAMA0。
  @param i2c_bus I2C 总线号，树莓派一般是 1
  @param address I2C 从机地址，0x40～0x43
  @param baud 串口波特率；None 走 I2C
  @param bits UART 数据位
  @param parity UART 校验位，0 表示无校验
  @param stopbit UART 停止位，模块出厂为 2
'''
def __init__(self, i2c_bus=1, address=0x43, baud=None, bits=8, parity=0, stopbit=2)

'''!
  @fn begin
  @brief 初始化模块，读取板型、通道数、固件版本和每个通道的量程
  @return int
  @retval 0 成功
  @retval -1 I2C 打开失败
  @retval -2 0x40～0x43 都没有应答
  @retval -3 UART 未打开
  @retval -4 VID 不符
  @retval -5 PID 读失败
  @retval -6 板型读失败
  @retval -7 通道数不是 4/8
  @retval -8 PID 与板型不匹配
'''
def begin(self)

'''!
  @fn set_output_range
  @brief 设置一个通道的量程，该通道会先输出 0 V
  @param channel 通道编号 0～3 或 0～7
  @param output_range 板型支持的量程
  @return int
  @retval 0 成功
  @retval -1 通道非法
  @retval -2 量程非法或不被板型支持
  @retval other 通信失败
'''
def set_output_range(self, channel, output_range)

'''!
  @fn set_all_output_range
  @brief 把全部通道设置为同一量程
  @param output_range 板型支持的量程
  @return int
  @retval 0 成功
  @retval -1 未初始化
  @retval -2 量程非法或不被板型支持
  @retval other 通信失败
'''
def set_all_output_range(self, output_range)

'''!
  @fn set_voltage
  @brief 设置一个通道的输出电压并立即更新
  @param channel 通道编号 0～3 或 0～7
  @param voltage 输出电压，单位 V，受该通道量程限制
  @return int
  @retval 0 成功
  @retval -1 通道非法
  @retval -2 电压为负
  @retval -3 超当前量程
  @retval other 通信失败
'''
def set_voltage(self, channel, voltage)

'''!
  @fn set_sync_voltages
  @brief 一次设置多个通道并让它们同步生效
  @param channels 通道编号列表，不允许重复
  @param voltages 与 channels 对应的电压列表，单位 V
  @return int
  @retval 0 成功
  @retval -1 未初始化或空列表
  @retval -2 长度不合法
  @retval -3 通道非法
  @retval -4 通道重复
  @retval -5 电压为负
  @retval -6 超量程
  @retval other 通信失败
'''
def set_sync_voltages(self, channels, voltages)

'''!
  @fn set_all_voltage
  @brief 一次设置全部通道并让它们同步生效
  @param voltages 按 CH0、CH1…… 排列的电压列表，长度等于通道数
  @return int
  @retval 0 成功
  @retval -1 未初始化或空列表
  @retval -2 长度与通道数不符
  @retval -3 电压为负
  @retval -4 超量程
  @retval other 通信失败
'''
def set_all_voltage(self, voltages)

'''!
  @fn set_channel_enabled
  @brief 启用或停用一个通道
  @param channel 通道编号 0～3 或 0～7
  @param enabled True 启用，False 停用
  @return int
  @retval 0 成功
  @retval -1 通道非法
  @retval other 通信失败
'''
def set_channel_enabled(self, channel, enabled)

'''!
  @fn stop_output
  @brief 将全部通道同步设置为 0 V，不改变量程和启用状态
  @return int 0 成功，其它通信失败
'''
def stop_output(self)

'''!
  @fn read_firmware_version
  @brief 读取固件版本，例如 0x1000 表示 V1.0.0.0
  @return int 固件版本，读失败返回缓存值（首次读取前是 0）
'''
def read_firmware_version(self)

'''!
  @fn get_sku
  @brief 读取板型编号
  @return int
  @retval 0 DFR1328
  @retval 1 DFR1329
  @retval 2 DFR1332
  @retval 3 DFR1333
  @retval 0xFF 读失败
'''
def get_sku(self)

'''!
  @fn get_channel_count
  @brief 读取输出通道数
  @return int 4 或 8，begin() 之前返回 0
'''
def get_channel_count(self)

'''!
  @fn get_voltage
  @brief 读取一个通道当前生效的输出电压
  @param channel 通道编号 0～3 或 0～7
  @return float 电压，单位 V；通道非法或读失败返回 0
'''
def get_voltage(self, channel)

'''!
  @fn get_range
  @brief 读取一个通道当前量程
  @param channel 通道编号 0～3 或 0～7
  @return int 量程代码；通道非法或读失败返回 OUTPUT_RANGE10V
'''
def get_range(self, channel)

'''!
  @fn get_channel_enabled
  @brief 读取一个通道的启用状态
  @param channel 通道编号 0～3 或 0～7
  @return bool True 启用；通道非法或读失败返回 False
'''
def get_channel_enabled(self, channel)

'''!
  @fn get_status
  @brief 读取模块状态位图
  @return int 状态位图，bit0=1 表示 DAC 参考异常；读失败返回 0
'''
def get_status(self)
```

### 示例

- `set_single_channel_voltage.py`：初始化、打印板型、设置 CH0 量程和电压并读回（主示例）。
- `set_sync_channels_voltage.py`：三个通道同步更新。
- `set_all_channels_voltage.py`：全部通道同步更新，按量程在通道间均分。
- `two_point_calibration.py`：用万用表在 10% 和 90% 两点校准 CH0。

```bash
cd python/raspberrypi/examples
sudo python3 set_single_channel_voltage.py
```

## 兼容性

| 主板 | 通过 | 未通过 | 未测试 | 备注 |
|------|------|--------|--------|------|
| 树莓派 4B | | | √ | |

## 历史

- 2026/11/18 - V1.0.0 版本

## 创作者

Written by wxzed, 2026. [DFRobot](https://www.dfrobot.com)
