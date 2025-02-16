#!/usr/bin/env python3
import os

from icecube.icetray import I3Tray
from icecube import icetray
from icecube import dataclasses
from icecube import dataio
from icecube.trigger_sim.modules.time_shifter import I3TimeShifter

filename = os.path.expandvars("$I3_TESTDATA/sim/corsika.F2K010001_IC59_slim.i3.gz")

tray = I3Tray()
tray.AddModule("I3Reader","read",\
               filename = filename, \
               SkipKeys = ["TimeShift"] )

tray.AddModule(I3TimeShifter,"shift")
    
tray.Execute()

