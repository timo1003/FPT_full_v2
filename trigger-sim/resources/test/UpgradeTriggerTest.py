#!/usr/bin/env python3
from icecube.icetray import I3Tray

from icecube import icetray, dataclasses, dataio, trigger_sim

from os.path import expandvars
import sys

from modules.inice_test_modules import TestSource, TestSourcePulses, TestModule

gcd_file = expandvars("$I3_TESTDATA/GCD/GeoCalibDetectorStatus_ICUpgrade.v55.mixed_mergedGeo.V5.i3.bz2")
trigger_config_id = 1018 # Hijack this one just for this test

#-----------------------------------------------------------------------
# Test MeasurementMode = 0 (ie, all LC pulses for each PMT)
#-----------------------------------------------------------------------
tray = I3Tray()
tray.AddModule("I3InfiniteSource", prefix=gcd_file, stream=icetray.I3Frame.DAQ)

tray.AddModule(TestSourcePulses, NPMTs = 50)
tray.AddModule(TestSource, NDOMs = 8)

tray.Add(trigger_sim.TriggerSim, "",
         run_id = 1)

tray.AddModule(TestModule, TriggerName = 'I3TriggerHierarchy',
               TriggerTypeID = dataclasses.VOLUME,
               TriggerConfigID = trigger_config_id,
               TriggerSourceID = dataclasses.IN_ICE_PULSES)
                              
tray.Execute(100)
print("PASSED (MeasurementMode 0)")

