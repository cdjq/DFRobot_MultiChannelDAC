'''!
  @file set_all_channels_voltage.py
  @brief Set all DAC channels at once and update them synchronously
  @details Wire the module to the Raspberry Pi over I2C: SDA to SDA (pin 3),
  @n SCL to SCL (pin 5), GND to GND, VCC to 3.3 V or 5 V, then set the module
  @n switch to I2C. A multimeter on CH0～CH7 shows each channel output.
  @n The full scale is divided across the channels: an 8-channel 5 V board
  @n outputs 0.625 V ... 5.000 V, a 4-channel 5 V board 1.25 V ... 5.0 V.
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
    @brief Spread the full scale across all channels and read each one back
  '''
  print("DFRobot MultiChannel DAC - all channels")
  time.sleep(1)

  ret = dac.begin()
  if ret != 0:
    print("begin failed, code = %d" % ret)
    return

  channels = dac.get_channel_count()
  output_range = (DFRobot_MultiChannelDAC.OUTPUT_RANGE10V if dac.get_sku() < 2
                  else DFRobot_MultiChannelDAC.OUTPUT_RANGE5V)
  if dac.set_all_output_range(output_range) != 0:
    print("setAllOutputRange failed")
    return

  # Divide the current range: a 4-channel 5 V board gets 1.25-5 V, an 8-channel 5 V board
  # 0.625-5 V, and a 10 V board reaches its full scale the same way
  full_scale = 10.0 if output_range == DFRobot_MultiChannelDAC.OUTPUT_RANGE10V else 5.0
  voltages = [full_scale * (channel + 1) / channels for channel in range(channels)]
  ret = dac.set_all_voltage(voltages)
  if ret != 0:
    print("setAllVoltage failed, code = %d" % ret)
    return

  for channel in range(channels):
    print("CH%d = %.3f V" % (channel, dac.get_voltage(channel)))

  try:
    while True:
      time.sleep(1)
  except KeyboardInterrupt:
    print("stopped")


if __name__ == "__main__":
  main()
