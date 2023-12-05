#!/usr/bin/env python3
import unittest
from icecube.icetray import I3Tray

from icecube import icetray, dataclasses, dataio, trigger_sim
from icecube.trigger_sim import InjectDefaultDOMSets
from os.path import expandvars
import sys

infile = expandvars("$I3_TESTDATA/trigger-sim/triggersim_test.i3.zst")

config_ids = [1006,  # InIce SMT8
              1011,  # InIce DeepCore SMT3
              #24002, # InIce SLOP
              1007,  # InIce String
              21001, # InIce Volume
              102,   # IceTop SMT
              21002, # IceTop Volume
]

def hierarchy_to_dict(hierarchy, new=False):
    output = {cid:False for cid in config_ids}
    for trigger in hierarchy:
        cid = trigger.key.config_id
        if cid is None: continue
        
        cid = int(cid)
        if new and cid == 24002: 
            print('Found a 24002!')
        if cid not in config_ids: continue
        output[cid] = True
    return output
            

#-----------------------------------------------------------------------
# Make sure trigger-sim gives the same results as PDAQ for the subset of
# triggers that we actually simulate (ie, no minbias or calibration)
#-----------------------------------------------------------------------
class compare_to_pdaq(unittest.TestCase):
    def compare_to_pdaq(self):
        framenum = [0,]

        tray = I3Tray()
        tray.Add(dataio.I3Reader, FilenameList = [infile,])
        
        tray.Add(trigger_sim.TriggerSim, run_id = 1)
        
        def trigger_comparisons(frame, framenum):
            new_hierarchy = hierarchy_to_dict(frame["I3TriggerHierarchy"], new=True)
            old_hierarchy = hierarchy_to_dict(frame["I3TriggerHierarchy_pdaq"])
            for cid in config_ids:
                self.assertTrue(new_hierarchy[cid] == old_hierarchy[cid],
                                f"PDAQ vs trigger-sim comparison failed for config_id {cid} in frame {framenum[0]}!"
                                f"\n\t Trigger-sim: {new_hierarchy[cid]}, pdaq: {old_hierarchy[cid]}")
            framenum[0] += 1
            return
        
        tray.Add(trigger_comparisons,
                 framenum = framenum,
                 Streams = [icetray.I3Frame.DAQ,])

        tray.Execute()
            
        return

if __name__ == "__main__":
    test = compare_to_pdaq()
    test.compare_to_pdaq()
