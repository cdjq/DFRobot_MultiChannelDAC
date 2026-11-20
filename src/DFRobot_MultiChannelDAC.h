/*!
 * @file DFRobot_MultiChannelDAC.h
 * @brief DFRobot MultiChannel DAC Arduino library for DFR1328/DFR1329/DFR1332/DFR1333.
 * @details 通过 General RTU 产品寄存器访问模块：I2C 用 TwoWire 访问 0x40～0x43，UART/USB CDC 用 Stream；同一套 API 覆盖 4 通道和 8 通道、0～10 V 与 0～2.5 V/0～5 V 四种模块。
 *
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author      wxzed (xiao.wu@dfrobot.com)
 * @version     V1.1.0
 * @date        2026-11-20
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
#define DFR_MCDAC_HOLDING_COMMAND         0x0015 ///< 命令：1=全部停波形并回 0 V，2=全部按暂存值同步生效
#define DFR_MCDAC_HOLDING_WAVE_COMMAND    0x0016 ///< 波形命令：1=按掩码启动，2=按掩码停止
#define DFR_MCDAC_HOLDING_WAVE_MASK       0x0017 ///< 波形掩码，bit0=CH0
#define DFR_MCDAC_HOLDING_WAVE_CHANNEL    0x0018 ///< 波形参数所属通道，0～7
#define DFR_MCDAC_HOLDING_WAVE_BLOCK      0x0020 ///< 自定义波形块号，高 4 位通道，低 4 位块号
#define DFR_MCDAC_HOLDING_WAVE_DATA       0x0021 ///< 当前块的 16 个形状点

#define DFR_MCDAC_CMD_STOP_ALL            0x0001 ///< 命令值：全部通道停止波形并同步回 0 V
#define DFR_MCDAC_CMD_COMMIT_ALL          0x0002 ///< 命令值：全部通道按暂存值同步生效
#define DFR_MCDAC_WAVE_CMD_START          0x0001 ///< 波形命令：按掩码启动
#define DFR_MCDAC_WAVE_CMD_STOP           0x0002 ///< 波形命令：按掩码停止，电压停在最后一拍
#define DFR_MCDAC_WAVE_POINTS             256    ///< 自定义波形一个周期的点数
#define DFR_MCDAC_WAVE_FREQ_MAX_HZ        5000   ///< 波形频率上限，单位 Hz
#define DFR_MCDAC_RTU_WRITE_MAX           17     ///< 一帧最多写入的保持寄存器数，覆盖一块自定义波形

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
 * @enum eWaveform_t
 * @brief 波形类型。幅度是峰值，输出范围是偏置减幅度到偏置加幅度。
 */
typedef enum {
  eWaveformDc = 0,       /**< 直流，输出等于偏置 */
  eWaveformSine = 1,     /**< 正弦 */
  eWaveformSquare = 2,   /**< 方波，占空比有效 */
  eWaveformTriangle = 3, /**< 三角 */
  eWaveformSawtooth = 4, /**< 锯齿 */
  eWaveformCustom = 5    /**< 自定义，使用 writeCustomWave() 写入的 256 点 */
} eWaveform_t;

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
   * @n 不改变通道输出和量程；模块上电后默认全部通道 0 V，DFR1328/DFR1329 默认量程 0～10 V，DFR1332/DFR1333 默认 0～5 V。
   */
  int begin(void);

  /**
   * @brief 设置一个通道的输出量程。
   * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印 CH0～CH3 / CH0～CH7 一致。
   * @param range 量程，只有板型支持的档位才有效。
   * - eOutputRange2_5V：0～2.5 V，仅 DFR1332/DFR1333。
   * - eOutputRange5V：0～5 V，仅 DFR1332/DFR1333。
   * - eOutputRange10V：0～10 V，仅 DFR1328/DFR1329。
   * @return 0 表示成功；非 0 表示通道、量程非法或通信失败。
   * @n 修改量程时该通道先输出 0 V，切换后保持 0 V；目标电压同时清 0。
   */
  int setOutputRange(uint8_t channel, eOutputRange_t range);

  /**
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
   * @brief 将全部通道同步设置为 0 V，并停止全部波形。
   * @return 0 表示成功；非 0 表示通信失败。
   * @n 不改变量程和通道启用状态；后续调用 setVoltage() 或 setAllVoltage() 即可恢复输出。
   */
  int stopOutput(void);

  /**
   * @brief 写入一路波形参数，不启动播放。
   * @param channel 通道编号，0～3 或 0～7，与 PCB 丝印一致。
   * @param type 波形类型。
   * @param freqHz 频率，单位 Hz，0～5000。0 表示停在当前相位。
   * @param amplitude 峰值幅度，单位 V。直流波形不使用这个值。
   * @param offset 偏置，单位 V。输出范围是偏置减幅度到偏置加幅度，必须落在当前量程内。
   * @param dutyPercent 方波占空比，0～100，其他波形忽略。默认 50。
   * @param phaseDeg 起始相位，0～360，默认 0。
   * @return 0 表示成功；非 0 表示通道、参数超出范围或通信失败。
   * @n 正在播放时写入会立即改变这一路的参数，相位累加器不重置。要从头播放，再调用 startWaveform()。
   */
  int setWaveform(uint8_t channel, eWaveform_t type, float freqHz, float amplitude, float offset,
                  float dutyPercent = 50.0f, float phaseDeg = 0.0f);

  /**
   * @brief 写入一路自定义波形的 256 个形状点。
   * @param channel 通道编号，0～3 或 0～7。
   * @param samples 256 个点。0 对应偏置减幅度，32768 对应偏置，65535 对应偏置加幅度。
   * @return 0 表示成功；非 0 表示通道非法、指针为空或通信失败。
   * @n 需先用 setWaveform() 把类型设为 eWaveformCustom。点数据掉电不保存。
   */
  int writeCustomWave(uint8_t channel, const uint16_t samples[DFR_MCDAC_WAVE_POINTS]);

  /**
   * @brief 按给定通道一起启动波形，相位同时清零。
   * @param channels 通道编号数组，0～3 或 0～7，不允许重复。
   * @param length 数组长度，1～通道数。
   * @return 0 表示成功；非 0 表示通道非法、重复或通信失败。
   * @n 两路配合的波形必须放在同一次调用里。模块会检查每路的偏置和幅度是否落在量程内，任一不合格则都不启动。
   */
  int startWaveform(const uint8_t channels[], uint8_t length);

  /**
   * @brief 启动一路波形。
   * @param channel 通道编号，0～3 或 0～7。
   * @return 0 表示成功；非 0 表示通道非法或通信失败。
   */
  int startWaveform(uint8_t channel);

  /**
   * @brief 停止给定通道的波形，电压停在最后一拍。
   * @param channels 通道编号数组，0～3 或 0～7，不允许重复。
   * @param length 数组长度，1～通道数。
   * @return 0 表示成功；非 0 表示通道非法、重复或通信失败。
   * @n 不会回到 0 V。要全部归零，调用 stopOutput()。
   */
  int stopWaveform(const uint8_t channels[], uint8_t length);

  /**
   * @brief 停止一路波形，电压停在最后一拍。
   * @param channel 通道编号，0～3 或 0～7。
   * @return 0 表示成功；非 0 表示通道非法或通信失败。
   */
  int stopWaveform(uint8_t channel);

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

protected:
  int readInputRegister16(uint16_t reg, uint16_t *value);
  int readHoldingRegister16(uint16_t reg, uint16_t *value);
  int writeHoldingRegister16(uint16_t reg, uint16_t value);
  int readRegisterBlock(bool holding, uint16_t reg, uint16_t *values, uint8_t count);
  int writeRegisterBlock(uint16_t reg, const uint16_t *values, uint8_t count);
  int writeVoltageMv(uint8_t channel, uint16_t mv);
  int waveformMask(const uint8_t channels[], uint8_t length, uint16_t *mask) const;
  int waveformCommand(uint16_t command, const uint8_t channels[], uint8_t length);
  bool i2cProbe(uint8_t address);
  bool rangeSupported(uint8_t range) const;
  uint16_t rangeMaxMv(uint8_t range) const;
  uint16_t pidForSku(uint8_t sku) const;

private:
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
