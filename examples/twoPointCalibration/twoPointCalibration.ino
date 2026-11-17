/*!
 * @file twoPointCalibration.ino
 * @brief Two-point calibration example for one DAC channel.
 * @n Wire the module to the host over I2C: SDA to SDA, SCL to SCL, GND to GND, VCC to 3.3 V or 5 V,
 * @n then set the module switch to I2C. Connect a multimeter to CH0. The sketch outputs 10 % and 90 % of
 * @n the selected range, asks for the measured values, then applies the slope and offset to later voltage settings.
 * @n The serial port asks for each measured voltage in V, then prints "slope = …" and "offset = …",
 * @n followed by one line per test point, for example "target … V -> command … V, readback … V".
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author      wxzed (xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-11-16
 * @url https://github.com/DFRobot/DFRobot_MultiChannelDAC
 */
#include <Wire.h>
#include <DFRobot_MultiChannelDAC.h>

DFRobot_MultiChannelDAC dac;

/* 等待串口输入一个浮点数（万用表读数，单位 V）。 */
static float readMeterVoltage(void) {
  while (Serial.available() == 0) {
    delay(10);
  }
  return Serial.parseFloat();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("DFRobot MultiChannel DAC - two point calibration");
  int ret = dac.begin();
  if (ret != 0) {
    Serial.print("begin failed, code = ");
    Serial.println(ret);
    return;
  }

  eOutputRange_t range = (dac.getSKU() < 2) ? eOutputRange10V : eOutputRange5V;
  if (dac.setOutputRange(0, range) != 0) {
    Serial.println("setOutputRange failed");
    return;
  }
  const float fullScale = (range == eOutputRange10V) ? 10.0f : 5.0f;
  const float lowTarget = fullScale * 0.1f;
  const float highTarget = fullScale * 0.9f;

  Serial.print("Setting CH0 to ");
  Serial.print(lowTarget, 3);
  Serial.println(" V. Enter the measured voltage:");
  dac.setVoltage(0, lowTarget);
  delay(1000);
  const float lowMeasured = readMeterVoltage();

  Serial.print("Setting CH0 to ");
  Serial.print(highTarget, 3);
  Serial.println(" V. Enter the measured voltage:");
  dac.setVoltage(0, highTarget);
  delay(1000);
  const float highMeasured = readMeterVoltage();

  const float slope = (highMeasured - lowMeasured) / (highTarget - lowTarget);
  const float offset = lowMeasured - slope * lowTarget;
  Serial.print("slope = ");
  Serial.println(slope, 6);
  Serial.print("offset = ");
  Serial.println(offset, 6);

  const float targets[] = {fullScale * 0.2f, fullScale * 0.5f, fullScale * 0.8f};
  for (uint8_t i = 0; i < 3; ++i) {
    const float command = (targets[i] - offset) / slope;
    if (dac.setVoltage(0, command) != 0) {
      Serial.println("calibrated setVoltage failed");
      return;
    }
    Serial.print("target ");
    Serial.print(targets[i], 3);
    Serial.print(" V -> command ");
    Serial.print(command, 3);
    Serial.print(" V, readback ");
    Serial.print(dac.getVoltage(0), 3);
    Serial.println(" V");
    delay(1000);
  }
}

void loop() {
  delay(1000);
}
