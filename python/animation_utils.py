# This is a module designed for general purpose animations and patterns across an LED strip
# of a given size
import numpy as np

class RainbowStrip:
  # R: tuple = (255, 0, 0)    # Red
  # Y: tuple = (255, 255, 0)  # Yellow
  # G: tuple = (0, 255, 0)    # Green
  # C: tuple = (0, 255, 255)  # Cyan
  # B: tuple = (0, 0, 255)    # Blue
  # P: tuple = (255, 0, 255)  # Purple
  N_CYCLE_COLORS = 6        # Number of colors to explicitly represent in the cycle

  def __init__(self, n_leds, n_cycles, scroll):
    """
    Initialize the rainbow!

    Args:
    - n_leds: int || Number of LEDs in the strip
    - n_cycles: int || Number of cycles to have per number of leds provided
    - scroll: bool || Whether to scroll the colors
    """
    self.n_leds = n_leds
    self.n_cycles = n_cycles
    self.scroll = scroll
    self.lut = np.zeros((3, self.n_leds))    # Use this to define table for values

    # Create cycles of the rainbow across some number of leds
    pixels_per_cycle = n_leds // n_cycles
    pixels_per_color = 0
    if pixels_per_cycle <= self.N_CYCLE_COLORS:
      pixels_per_color = 1
    else:
      pixels_per_color = pixels_per_cycle // self.N_CYCLE_COLORS

    # Write pixel values for increases and decreases
    pos_ramp = np.linspace(0, 255 - 255 // self.N_CYCLE_COLORS, pixels_per_color)
    neg_ramp = np.linspace(0 + 255 // self.N_CYCLE_COLORS, 255, pixels_per_color)
    neg_ramp = neg_ramp[::-1]
    constant_on = np.ones(pixels_per_color)*255
    constant_off = np.zeros(pixels_per_color)

    # Write out to cycles now
    self.lut[0] = np.zeros(self.n_leds)
    self.lut[1] = np.zeros(self.n_leds)
    self.lut[2] = np.zeros(self.n_leds)

    print("Pixels per color = " + str(pixels_per_color))
    print("Pixels per cycle = " + str(pixels_per_cycle))

    remainder = pixels_per_cycle % self.N_CYCLE_COLORS

    for i in range(0, n_cycles):
      offset = i * pixels_per_cycle

      # Red
      self.lut[0][0+offset:pixels_per_color+offset] = constant_on
      self.lut[0][pixels_per_color+offset:pixels_per_color*2+offset] = neg_ramp
      self.lut[0][pixels_per_color*2+offset:pixels_per_color*3+offset] = constant_off
      self.lut[0][pixels_per_color*3+offset:pixels_per_color*4+offset] = constant_off
      self.lut[0][pixels_per_color*4+offset:pixels_per_color*5+offset] = pos_ramp
      self.lut[0][pixels_per_color*5+offset:pixels_per_color*6+offset] = constant_on
      self.lut[0][pixels_per_cycle*(i+1)-remainder:pixels_per_cycle*(i+1)] = np.ones(remainder)*255
      
      # Green
      self.lut[1][0+offset:pixels_per_color+offset] = pos_ramp
      self.lut[1][pixels_per_color+offset:pixels_per_color*2+offset] = constant_on
      self.lut[1][pixels_per_color*2+offset:pixels_per_color*3+offset] = constant_on
      self.lut[1][pixels_per_color*3+offset:pixels_per_color*4+offset] = neg_ramp
      self.lut[1][pixels_per_color*4+offset:pixels_per_color*5+offset] = constant_off
      self.lut[1][pixels_per_color*5+offset:pixels_per_color*6+offset] = constant_off
      self.lut[1][pixels_per_cycle*(i+1)-remainder:pixels_per_cycle*(i+1)] = np.zeros(remainder)


      # Blue
      self.lut[2][0+offset:pixels_per_color+offset] = constant_off
      self.lut[2][pixels_per_color+offset:pixels_per_color*2+offset] = constant_off
      self.lut[2][pixels_per_color*2+offset:pixels_per_color*3+offset] = pos_ramp
      self.lut[2][pixels_per_color*3+offset:pixels_per_color*4+offset] = constant_on
      self.lut[2][pixels_per_color*4+offset:pixels_per_color*5+offset] = constant_on
      self.lut[2][pixels_per_color*5+offset:pixels_per_color*6+offset] = neg_ramp
      self.lut[2][pixels_per_cycle*(i+1)-remainder:pixels_per_cycle*(i+1)] = np.zeros(remainder)


  def update(self, y):
    """
    Provide the RGB values for the ouptut to the LEDs based on the power in y
    Assume that y comes in normalized
    """
    return y*self.lut



