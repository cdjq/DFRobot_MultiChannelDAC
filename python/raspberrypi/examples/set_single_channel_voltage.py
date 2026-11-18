'''!
  @file set_single_channel_voltage.py
  @brief Set one DAC channel and read it back
  @details Wire the module to the Raspberry Pi over I2C: SDA to SDA (pin 3),
  @n SCL to SCL (pin 5), GND to GND, VCC to 3.3 V or 5 V, then set the module
  @n switch to I2C. A multimeter on CH0 shows the output.
  @n The script prints the firmware version, the board SKU, the channel count
  @n and the CH0 voltage, then keeps the output on until Ctrl+C.
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


def main():
  '''!
    @brief Initialize the module, set the CH0 range and 5 V output, then read it back
  '''
  print("DFRobot MultiChannel DAC - single channel")
  time.sleep(1)

  ret = dac.begin()
  if ret != 0:
    print("begin failed, code = %d" % ret)
    return

  print("firmware version: 0x%X" % dac.read_firmware_version())
  print("board SKU: %d" % dac.get_sku())
  print("channel count: %d" % dac.get_channel_count())

  # DFR1328/DFR1329 support 0-10 V only; DFR1332/DFR1333 support 0-2.5 V and 0-5 V
  output_range = (DFRobot_MultiChannelDAC.OUTPUT_RANGE10V if dac.get_sku() < 2
                  else DFRobot_MultiChannelDAC.OUTPUT_RANGE5V)
  ret = dac.set_output_range(0, output_range)
  if ret != 0:
    print("setOutputRange failed, code = %d" % ret)
    return

  ret = dac.set_voltage(0, 5.0)
  if ret != 0:
    print("setVoltage failed, code = %d" % ret)
    return

  print("CH0 voltage: %.3f V" % dac.get_voltage(0))

  try:
    while True:
      time.sleep(1)
  except KeyboardInterrupt:
    print("stopped")


if __name__ == "__main__":
  main()
