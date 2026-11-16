/*!
 * @file DFRobot_MultiChannelDAC.h
 * @brief DFRobot MultiChannel DAC Arduino library for DFR1328/DFR1329/DFR1332/DFR1333.
 *
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author      DFRobot (github@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-11-16
 * @url         https://github.com/DFRobot/DFRobot_MultiChannelDAC
 */
#ifndef __DFROBOT_MULTICHANNELDAC_H
#define __DFROBOT_MULTICHANNELDAC_H ///< 头文件保护宏

#include <Arduino.h>
#include <Wire.h>
#include <DFRobot_RTU.h>

/* 产品寄存器地址，来自 product-registers.json 与 requirements.md 第 3 节。 */
#define DFR_MCDAC_INPUT_VID               0x0000 ///< 厂商 ID，只读，固定 0x3343
#define DFR_MCDAC_INPUT_PID               0x0001 ///< 产品 ID，只读：DFR1328=0x0530、DFR1329=0x0531、DFR1332=0x0534、DFR1333=0x0535
#define DFR_MCDAC_INPUT_HARDWARE_VERSION  0x0002 ///< 硬件版本，只读，0x1000 表示 V1.0.0.0
#define DFR_MCDAC_INPUT_FIRMWARE_VERSION  0x0003 ///< 固件版本，只读，0x1000 表示 V1.0.0.0
#define DFR_MCDAC_INPUT_BOARD_SKU         0x0004 ///< 板型：0=DFR1328、1=DFR1329、2=DFR1332、3=DFR1333
#define DFR_MCDAC_INPUT_CHANNEL_COUNT     0x0005 ///< 输出通道数：4 或 8
#define DFR_MCDAC_INPUT_STATUS            0x0006 ///< 状态，bit0=1 表示参考异常
#define DFR_MCDAC_INPUT_CH1_VOLTAGE_MV    0x0007 ///< CH1 当前生效电压，单位 mV，只读
#define DFR_MCDAC_INPUT_CH8_VOLTAGE_MV    0x000E ///< CH8 当前生效电压，单位 mV，只读
#define DFR_MCDAC_INPUT_CH1_RANGE         0x000F ///< CH1 当前量程代码，只读
#define DFR_MCDAC_INPUT_CH8_RANGE         0x0016 ///< CH8 当前量程代码，只读
#define DFR_MCDAC_INPUT_CHANNEL_ENABLE    0x0017 ///< 通道启用位图，只读，bit0=CH1

#define DFR_MCDAC_HOLDING_DEVICE_ADDRESS  0x0000 ///< RTU 从站地址，1～247，出厂 0x000F，重新上电生效
#define DFR_MCDAC_HOLDING_BAUDRATE        0x0001 ///< 波特率编号，0x0003=9600，重新上电生效
#define DFR_MCDAC_HOLDING_SERIAL_FORMAT   0x0002 ///< 串口格式，0x0003=无校验+2 停止位，重新上电生效
#define DFR_MCDAC_HOLDING_CH1_VOLTAGE_MV  0x0003 ///< CH1 目标电压，单位 mV，只暂存
#define DFR_MCDAC_HOLDING_CH8_VOLTAGE_MV  0x000A ///< CH8 目标电压，单位 mV，只暂存
#define DFR_MCDAC_HOLDING_CH1_RANGE       0x000B ///< CH1 量程代码，写入后该通道先回 0 V
#define DFR_MCDAC_HOLDING_CH8_RANGE       0x0012 ///< CH8 量程代码，写入后该通道先回 0 V
#define DFR_MCDAC_HOLDING_CHANNEL_ENABLE  0x0013 ///< 通道启用位图，bit0=CH1…bit7=CH8
#define DFR_MCDAC_HOLDING_COMMIT_MASK     0x0014 ///< 提交掩码，写入哪些位就让这些通道同时生效
#define DFR_MCDAC_HOLDING_COMMAND         0x0015 ///< 命令：1=全部回 0 V，2=全部按暂存值同步生效

#define DFR_MCDAC_CMD_STOP_ALL            0x0001 ///< 命令值：全部通道同步回 0 V
#define DFR_MCDAC_CMD_COMMIT_ALL          0x0002 ///< 命令值：全部通道按暂存值同步生效

#define DFR_MCDAC_I2C_ADDR_BASE           0x40   ///< I2C 从机地址基址，ADDR0/ADDR1 决定 0x40～0x43
#define DFR_MCDAC_I2C_ADDR_MAX            0x43   ///< I2C 从机地址上限
#define DFR_MCDAC_RTU_DEFAULT_ID          0x0F   ///< UART/USB CDC 的 General RTU 出厂从站地址
#define DFR_MCDAC_MAX_CHANNELS            8      ///< 8 通道板的最大通道数
#define DFR_MCDAC_I2C_HOLDING_BASE        0x80   ///< I2C 指针 0x80 + RTU 地址对应保持寄存器

/**
 * @enum eOutputRange_t
 * @brief 输出量程。DFR1328/DFR1329 只支持 10 V；DFR1332/DFR1333 支持 2.5 V 和 5 V。
 */
typedef enum {
  eOutputRange2_5V = 0, /**< 0～2.5 V，仅 DFR1332/DFR1333 */
  eOutputRange5V = 1,   /**< 0～5 V，仅 DFR1332/DFR1333 */
  eOutputRange10V = 2   /**< 0～10 V，仅 DFR1328/DFR1329 */
} eOutputRange_t;

/**
 * @class DFRobot_MultiChannelDAC
 * @brief 4/8 通道同步电压输出 DAC 模块驱动。同一套 API 支持 I2C、UART 和 USB CDC。
 * @n 模块侧固件完成 DAC 配置和同步更新；本库只通过产品寄存器访问模块。
 */
class DFRobot_MultiChannelDAC : public DFRobot_RTU {
public:
  /**
   * @brief I2C 构造。begin() 会依次探测 0x40～0x43。
   * @param wire Wire 对象指针，默认 &Wire。
   * @n I2C 地址由模块 ADDR0/ADDR1 拨码决定，外部上拉默认 0x43。
   */
  DFRobot_MultiChannelDAC(TwoWire *wire = &Wire);

  /**
   * @brief UART/USB CDC 构造，使用 DFRobot General RTU 协议。
   * @param serial 已打开的 Stream 对象指针，例如 &Serial1 或 &Serial。
   * @n 模块出厂 9600 8N2、从站地址 0x0F；主机需先按该参数打开串口。USB CDC 忽略波特率。
   */
  DFRobot_MultiChannelDAC(Stream *serial);

  ~DFRobot_MultiChannelDAC();

  /**
   * @brief 初始化模块，读取板型、通道数和固件版本。
   * @return 0 表示成功；非 0 表示探测失败、VID/PID 不符或板型不支持。
   * @n 初始化后全部通道输出 0 V；DFR1328/DFR1329 默认量程 0～10 V，DFR1332/DFR1333 默认 0～5 V。
   */
  int begin(void);

  /**
   * @brief 设置一个通道的输出量程。
   * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印 CH0～CH3 / CH0～CH7 一致。
   * @param range 量程：eOutputRange2_5V、eOutputRange5V、eOutputRange10V；只有板型支持的档位才有效。
   * @return 0 表示成功；非 0 表示通道、量程非法或通信失败。
   * @n 修改量程时该通道先输出 0 V，切换后保持 0 V；目标电压同时清 0。
   */
  int setOutputRange(uint8_t channel, eOutputRange_t range);

  /**
   * @brief 把全部通道设置为同一量程。
   * @param range 量程：eOutputRange2_5V、eOutputRange5V、eOutputRange10V；只有板型支持的档位才有效。
   * @return 0 表示成功；非 0 表示量程非法或通信失败。
   * @n 调用时全部通道先同步回 0 V，再完成量程切换。
   */
  int setAllOutputRange(eOutputRange_t range);

  /**
   * @brief 设置一个通道的输出电压并立即更新。
   * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印 CH0～CH3 / CH0～CH7 一致。
   * @param voltage 输出电压，单位 V，范围由该通道当前量程决定。
   * @return 0 表示成功；非 0 表示通道、电压非法或通信失败。
   * @n 只更新指定通道；连续调用多个单通道接口不保证同时生效。
   */
  int setVoltage(uint8_t channel, float voltage);

  /**
   * @brief 一次设置多个指定通道，并使这些通道同步生效。
   * @param channels 通道编号数组，编号 0～3 或 0～7；不允许重复通道。
   * @param voltages 电压数组，与 channels 数组逐一对应，单位 V。
   * @param length 数组长度，1～通道数；由它决定读取多少个元素。
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
   * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印 CH0～CH3 / CH0～CH7 一致。
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

private:
  int readInputRegister16(uint16_t reg, uint16_t *value);
  int readHoldingRegister16(uint16_t reg, uint16_t *value);
  int writeHoldingRegister16(uint16_t reg, uint16_t value);
  int readRegisterBlock(bool holding, uint16_t reg, uint16_t *values, uint8_t count);
  int writeRegisterBlock(uint16_t reg, const uint16_t *values, uint8_t count);
  int writeVoltageMv(uint8_t channel, uint16_t mv);
  bool i2cProbe(uint8_t address);
  bool rangeSupported(uint8_t range) const;
  uint16_t rangeMaxMv(uint8_t range) const;
  uint16_t pidForSku(uint8_t sku) const;

  TwoWire *_wire;
  Stream *_serial;
  bool _isI2C;
  uint8_t _addr;
  uint8_t _rtuId;
  uint8_t _channels;
  uint8_t _sku;
  uint16_t _firmwareVersion;
  uint16_t _enableMask;
  uint8_t _range[DFR_MCDAC_MAX_CHANNELS];
};

#endif
