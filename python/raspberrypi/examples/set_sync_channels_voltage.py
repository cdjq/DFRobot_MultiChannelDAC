'''!
  @file set_sync_channels_voltage.py
  @brief Set several DAC channels and update them synchronously
  @details Wire the module to the Raspberry Pi over I2C: SDA to SDA (pin 3),
  @n SCL to SCL (pin 5), GND to GND, VCC to 3.3 V or 5 V, then set the module
  @n switch to I2C. A multimeter on CH0～CH2 shows the three outputs.
  @n CH0, CH1 and CH2 change together to 1.0 V, 2.5 V and 4.0 V; CH3 keeps 0 V.
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
    @brief Output 1.0 V, 2.5 V and 4.0 V on three channels together and read each back
  '''
  print("DFRobot MultiChannel DAC - sync channels")
  time.sleep(1)

  ret = dac.begin()
  if ret != 0:
    print("begin failed, code = %d" % ret)
    return

  output_range = (DFRobot_MultiChannelDAC.OUTPUT_RANGE10V if dac.get_sku() < 2
                  else DFRobot_MultiChannelDAC.OUTPUT_RANGE5V)
  if dac.set_all_output_range(output_range) != 0:
    print("setAllOutputRange failed")
    return

  # Channel numbers are 0-3, matching the PCB silkscreen CH0-CH3; unselected channels keep their output
  channels = [0, 1, 2]
  voltages = [1.0, 2.5, 4.0]
  ret = dac.set_sync_voltages(channels, voltages)
  if ret != 0:
    print("setSyncVoltages failed, code = %d" % ret)
    return

  for channel in range(3):
    print("CH%d = %.3f V" % (channel, dac.get_voltage(channel)))

  try:
    while True:
      time.sleep(1)
  except KeyboardInterrupt:
    print("stopped")


if __name__ == "__main__":
  main()
