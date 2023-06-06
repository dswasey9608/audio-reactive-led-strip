**Table of Contents**
- [Summary](#summary)
- [Potential Additions](#potential-additions)

# Summary
This is a library that was originally designed to utilize ESP8266, Raspberry Pi, or Blink
Sticks to do audio-visualization on several types of LED strips. I have taken a look at 
the code coming back to using it after a long while and noticing some updates I could 
potentially make it order to make the system more readable and usable.

If I am fortunate I may be able to port everything into a newer version of Python,
assuming that all the the dependencies have something that supports something like
Python 3.10. 

Some key additions I would like to make would include are listed below, and their status
will be marked off as I make progress.

# Potential Additions
- [ ] Port to Python 3.10
- [ ] Make system more modular by defining classes
  - [ ] Use class variables instead of globals
  - [ ] Convert all Python files into classes 
- [ ] Make a Python script to automatically populate `ws2812_controller.ino` with updated
  information relative to `config.py`
