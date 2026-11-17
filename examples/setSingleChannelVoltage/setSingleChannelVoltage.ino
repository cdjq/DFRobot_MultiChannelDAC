/*!
 * @file setSingleChannelVoltage.ino
 * @brief Set one DAC channel and read it back.
 * @n Wire the module to the host over I2C: SDA to SDA, SCL to SCL, GND to GND, VCC to 3.3 V or 5 V,
 * @n then set the module switch to I2C. A multimeter on CH0 shows the output.
 * @n The serial port prints the firmware version, the board SKU, the channel count and CH0 voltage.
 * @n This is the main example: it initializes the module, reads the board type,
 * @n selects a valid range, sets CH0 voltage and reads the applied value back.
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author      wxzed (xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-11-16
 * @url https://github.com/DFRobot/DFRobot_MultiChannelDAC
 */
#include <Wire.h>
#include <DFRobot_MultiChannelDAC.h>

/* I2C 构造：begin() 会自动探测 0x40～0x43。 */
DFRobot_MultiChannelDAC dac;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("DFRobot MultiChannel DAC - single channel");

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
  ret = dac.setOutputRange(0, range);
  if (ret != 0) {
    Serial.print("setOutputRange failed, code = ");
    Serial.println(ret);
    return;
  }

  ret = dac.setVoltage(0, 5.0);
  if (ret != 0) {
    Serial.print("setVoltage failed, code = ");
    Serial.println(ret);
    return;
  }

  Serial.print("CH0 voltage: ");
  Serial.print(dac.getVoltage(0), 3);
  Serial.println(" V");
}

void loop() {
  delay(1000);
}
