#!/usr/bin/env python3
import unittest
from icecube.icetray import I3Tray

from icecube import icetray, dataclasses, dataio, trigger_sim
from icecube.trigger_sim import InjectDefaultDOMSets
from os.path import expandvars
import sys

gcd_file = expandvars("$I3_TESTDATA/GCD/GeoCalibDetectorStatus_ICUpgrade.v55.mixed_mergedGeo.V5.i3.bz2")

#-----------------------------------------------------------------------
# Make sure we get an error if we try to modify the default domsets
#-----------------------------------------------------------------------
class domset_tests(unittest.TestCase):
    def do_not_overwrite_domsets(self):
        #====================================
        # Trying to define a new domset using 
        # an ID that's already defined should
        # result in a RuntimeError.
        #====================================
        tray = I3Tray()
        tray.AddModule("I3InfiniteSource", prefix=gcd_file, stream=icetray.I3Frame.DAQ)
        
        definitions = {2: lambda omkey, omgeo: False}
        tray.Add(InjectDefaultDOMSets,
                 NewDefinitionMap = definitions)
        
        with self.assertRaises(RuntimeError):
            tray.Execute(10)
        return
            
    def triggers_domsets_must_be_defined(self):
        #====================================
        # Add a custom trigger for testing, but
        # do *not* add the necessary domset. This
        # should result in a RuntimeError
        #====================================
        from icecube.icetray import I3Units
        from icecube.dataclasses import SourceID
        from icecube.trigger_sim.modules import add_triggers
        newTriggers = add_triggers.TriggerConfigMap()
        newTriggers.DefineSMT(SourceID.IN_ICE_PULSES,
                              ConfigID = 42,
                              DOMSet = 42,
                              TimeWindow = 100*I3Units.ns,
                              Multiplicity = 8)

        #====================================
        # Actually add them
        #====================================
        tray = I3Tray()
        tray.AddModule("I3InfiniteSource", prefix=gcd_file, stream=icetray.I3Frame.DAQ)
        tray.Add(add_triggers.AddTriggers,
                 Triggers = newTriggers)
        
        # But don't add the domset 42!
        tray.Add(InjectDefaultDOMSets)
        
        with self.assertRaises(RuntimeError):
            tray.Execute(10)
            
        return

if __name__ == '__main__':
    test = domset_tests()

    test.do_not_overwrite_domsets()
    test.triggers_domsets_must_be_defined()
