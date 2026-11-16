/*!
 * @file setSyncChannelsVoltage.ino
 * @brief Set several DAC channels and update them synchronously.
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author      DFRobot (github@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-11-16
 */
#include <Wire.h>
#include <DFRobot_MultiChannelDAC.h>

DFRobot_MultiChannelDAC dac;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("DFRobot MultiChannel DAC - sync channels");
  int ret = dac.begin();
  if (ret != 0) {
    Serial.print("begin failed, code = ");
    Serial.println(ret);
    return;
  }

  eOutputRange_t range = (dac.getSKU() < 2) ? eOutputRange10V : eOutputRange5V;
  if (dac.setAllOutputRange(range) != 0) {
    Serial.println("setAllOutputRange failed");
    return;
  }

  /* 通道编号 0～3，与 PCB 丝印 CH0～CH3 一致；未选中的通道保持原来的输出。 */
  const uint8_t channels[] = {0, 1, 2};
  const float voltages[] = {1.0, 2.5, 4.0};
  ret = dac.setSyncVoltages(channels, voltages, 3);
  if (ret != 0) {
    Serial.print("setSyncVoltages failed, code = ");
    Serial.println(ret);
    return;
  }

  for (uint8_t ch = 0; ch <= 2; ++ch) {
    Serial.print("CH");
    Serial.print(ch);
    Serial.print(" = ");
    Serial.print(dac.getVoltage(ch), 3);
    Serial.println(" V");
  }
}

void loop() {
  delay(1000);
}
