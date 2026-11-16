/*!
 * @file setAllChannelsVoltage.ino
 * @brief Set all DAC channels at once and update them synchronously.
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

  Serial.println("DFRobot MultiChannel DAC - all channels");
  int ret = dac.begin();
  if (ret != 0) {
    Serial.print("begin failed, code = ");
    Serial.println(ret);
    return;
  }

  const uint8_t channels = dac.getChannelCount();
  eOutputRange_t range = (dac.getSKU() < 2) ? eOutputRange10V : eOutputRange5V;
  if (dac.setAllOutputRange(range) != 0) {
    Serial.println("setAllOutputRange failed");
    return;
  }

  float voltages[DFR_MCDAC_MAX_CHANNELS] = {0};
  for (uint8_t ch = 0; ch < channels; ++ch) {
    voltages[ch] = 1.0f + (float)ch; /* CH0=1.0 V, CH1=2.0 V, ... */
  }
  ret = dac.setAllVoltage(voltages, channels);
  if (ret != 0) {
    Serial.print("setAllVoltage failed, code = ");
    Serial.println(ret);
    return;
  }

  for (uint8_t ch = 0; ch < channels; ++ch) {
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
