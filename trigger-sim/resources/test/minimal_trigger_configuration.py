#!/usr/bin/env python3

import os
from icecube import icetray, dataclasses, dataio
from icecube import trigger_sim, phys_services
from icecube.icetray import I3Tray
from os.path import expandvars

tray = I3Tray()

i3_testdata = expandvars("$I3_TESTDATA") 
gcd_file = i3_testdata + "/GCD/GeoCalibDetectorStatus_2016.57531_V0.i3.gz"

tray.AddModule("I3InfiniteSource", prefix = gcd_file)               

tray.Add(trigger_sim.TriggerSim, "",
         run_id = 1)

tray.Execute(10)
