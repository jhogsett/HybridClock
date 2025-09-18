### Testing Update

1. The new breathing ring pattern is too bright
    - the outer ring should never be brighter than the set maximum brightness of the original pattern (whatever that is)
2.  The new hour animation is far too subtle
    - unless you happen to be looking at the exact moment, it's easily missed
    - please suggest several alternatives, for example, a brief windmill rotation animation, or an obvious hue changes to the other hours LEDs, etc.
3. New hour-long color patterns are needed in addition to the original and the new breating one.
    - please suggest several alternatives
    - they need to be pleasant, non-fatiguing, non-annoying, not alot of change happening, subtle.
4. At the top of a new hour, a subtle less-noticeable calibration should be performed so ensure the hand points straight up
    - this should be a secondary calibration procedure, not replacing the existing one (but the existing one can be altered to support both uses)
    - in this special case it should skip the full rotation to find the sensor a second time, since the magnet should already be near the home position. 
    - it should do that last state of the calibration, where it finds the forward and back extents of the magnet sensing and then usees the mean and device specific adjustment
    - the LEDs shown during the regular calibration procedure should not be shown duriing this one (it is intended to be not noticeable)

### Development Plan

- Implement the above a separate developement cycles, each including:
    - developing the feature carefully and gradually in steps, with verification on the device and git commits on each successfully step forward
    - No git commits without first verifying on the device
    - No proceding forward to the next feature until the previous one is fully completed