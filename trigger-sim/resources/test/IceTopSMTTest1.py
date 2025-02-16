#!/usr/bin/env python3
from icecube.icetray import I3Tray

from icecube import icetray, dataclasses, dataio, trigger_sim

from os.path import expandvars
import sys

from modules.inice_test_modules import TestSource, TestModule

tray = I3Tray()

gcd_file = expandvars("$I3_TESTDATA/GCD/GeoCalibDetectorStatus_2013.56429_V1.i3.gz")
tray.AddModule("I3InfiniteSource", prefix=gcd_file, stream=icetray.I3Frame.DAQ)

tray.AddModule(TestSource,
    NDOMs = 2,
    DOMType = dataclasses.I3OMGeo.IceTop
    )

tray.Add(trigger_sim.TriggerSim, "",
         run_id = 1)

#this event should not trigger
tray.AddModule(TestModule, Trigger = False )
               
tray.Execute(100)


