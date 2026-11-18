'''!
  @file two_point_calibration.py
  @brief Two-point calibration example for one DAC channel
  @details Wire the module to the Raspberry Pi over I2C: SDA to SDA (pin 3),
  @n SCL to SCL (pin 5), GND to GND, VCC to 3.3 V or 5 V, then set the module
  @n switch to I2C. Connect a multimeter to CH0.
  @n The script outputs 10 % and 90 % of the selected range and asks for the
  @n measured values, then applies the slope and offset to later settings.
  @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license The MIT License (MIT)
  @author wxzed
  @version V1.0.0
  @date 2026-11-18
'''

import sys
import time

sys.path.append('..')

from DFRobot_MultiChannelDAC import DFRobot_MultiChannelDAC

dac = DFRobot_MultiChannelDAC()


def read_meter_voltage():
  '''!
    @brief Wait for a float typed on the keyboard (the multimeter reading in V)
    @return float Multimeter reading
  '''
  while True:
    try:
      return float(input())
    except ValueError:
      continue


def main():
  '''!
    @brief Take multimeter readings at 10 % and 90 % of the range, compute slope and
    @n offset, then output 20 %/50 %/80 % with the calibrated values
  '''
  print("DFRobot MultiChannel DAC - two point calibration")
  time.sleep(1)

  ret = dac.begin()
  if ret != 0:
    print("begin failed, code = %d" % ret)
    return

  output_range = (DFRobot_MultiChannelDAC.OUTPUT_RANGE10V if dac.get_sku() < 2
                  else DFRobot_MultiChannelDAC.OUTPUT_RANGE5V)
  if dac.set_output_range(0, output_range) != 0:
    print("setOutputRange failed")
    return

  full_scale = 10.0 if output_range == DFRobot_MultiChannelDAC.OUTPUT_RANGE10V else 5.0
  low_target = full_scale * 0.1
  high_target = full_scale * 0.9

  print("Setting CH0 to %.3f V. Enter the measured voltage:" % low_target)
  dac.set_voltage(0, low_target)
  time.sleep(1)
  low_measured = read_meter_voltage()

  print("Setting CH0 to %.3f V. Enter the measured voltage:" % high_target)
  dac.set_voltage(0, high_target)
  time.sleep(1)
  high_measured = read_meter_voltage()

  slope = (high_measured - low_measured) / (high_target - low_target)
  offset = low_measured - slope * low_target
  print("slope = %.6f" % slope)
  print("offset = %.6f" % offset)

  targets = [full_scale * 0.2, full_scale * 0.5, full_scale * 0.8]
  for target in targets:
    command = (target - offset) / slope
    if dac.set_voltage(0, command) != 0:
      print("calibrated setVoltage failed")
      return
    print("target %.3f V -> command %.3f V, readback %.3f V"
          % (target, command, dac.get_voltage(0)))
    time.sleep(1)

  try:
    while True:
      time.sleep(1)
  except KeyboardInterrupt:
    print("stopped")


if __name__ == "__main__":
  main()
