/*!
 * @file setWaveform.ino
 * @brief Output a sine wave, then start two channels together.
 * @n Wire the module to the host over I2C: SDA to SDA, SCL to SCL, GND to GND, VCC to 3.3 V or 5 V,
 * @n then set the module switch to I2C. A scope or multimeter on CH0 and CH1 shows the output.
 * @n The serial port prints the firmware version, the board SKU and the channel count.
 * @n setWaveform() only stores the parameters. Channels listed in one startWaveform() call
 * @n clear their phase together. stopWaveform() holds the last sample; stopOutput() returns every channel to 0 V.
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author      wxzed (xiao.wu@dfrobot.com)
 * @version     V1.1.0
 * @date        2026-11-20
 * @url https://github.com/DFRobot/DFRobot_MultiChannelDAC
 */
#include <Wire.h>
#include <DFRobot_MultiChannelDAC.h>

/* I2C 构造：begin() 会自动探测 0x40～0x43。 */
DFRobot_MultiChannelDAC dac;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("DFRobot MultiChannel DAC - waveform");

  int ret = dac.begin();
  if (ret != 0) {
    Serial.print("begin failed, code = ");
    Serial.println(ret);
    return;
  }

  Serial.print("firmware version: 0x");
  Serial.println(dac.readFirmwareVersion(), HEX);
  Serial.print("board SKU: ");
  Serial.println(dac.getSKU());
  Serial.print("channel count: ");
  Serial.println(dac.getChannelCount());

  /* DFR1328/DFR1329 只支持 0～10 V；DFR1332/DFR1333 支持 0～2.5 V 和 0～5 V。 */
  eOutputRange_t range = (dac.getSKU() < 2) ? eOutputRange10V : eOutputRange5V;
  float amplitude = (range == eOutputRange10V) ? 2.5f : 1.25f;
  float offset = amplitude;

  ret = dac.setOutputRange(0, range);
  if (ret != 0) {
    Serial.print("setOutputRange failed, code = ");
    Serial.println(ret);
    return;
  }
  ret = dac.setOutputRange(1, range);
  if (ret != 0) {
    Serial.print("setOutputRange failed, code = ");
    Serial.println(ret);
    return;
  }

  /* 10 Hz 正弦。幅度是峰值，偏置等于幅度时输出从 0 V 摆到满量程的一半。 */
  ret = dac.setWaveform(0, eWaveformSine, 10.0f, amplitude, offset);
  if (ret != 0) {
    Serial.print("setWaveform CH0 failed, code = ");
    Serial.println(ret);
    return;
  }
  ret = dac.setWaveform(1, eWaveformSine, 10.0f, amplitude, offset, 50.0f, 180.0f);
  if (ret != 0) {
    Serial.print("setWaveform CH1 failed, code = ");
    Serial.println(ret);
    return;
  }

  const uint8_t channels[] = {0, 1};
  ret = dac.startWaveform(channels, 2);
  if (ret != 0) {
    Serial.print("startWaveform failed, code = ");
    Serial.println(ret);
    return;
  }

  Serial.println("CH0 and CH1 sine started");
}

void loop() {
  delay(1000);
}
