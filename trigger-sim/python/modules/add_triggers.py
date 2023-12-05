from icecube import icetray
from icecube import dataclasses

from icecube.dataclasses import TypeID
from icecube.dataclasses import Map_TriggerKey_I3TriggerStatus as TriggerConfigMap
from icecube.dataclasses import TriggerKey, I3TriggerStatus
from icecube.icetray import I3Units
import logging

def DefineSMT(self, SourceID, ConfigID,
              DOMSet, TimeWindow, Multiplicity,
              ReadoutMinus=5000*I3Units.ns,
              ReadoutPlus=5000*I3Units.ns,
              ReadoutOffset=0):
    """                                                                                                                                                                                                          
    Create a configuration for a new SMT trigger
    """
    key = TriggerKey(SourceID, TypeID.SIMPLE_MULTIPLICITY, ConfigID)

    config = I3TriggerStatus()
    config.trigger_name = "SimpleMajorityTrigger"

    subdetector = config.Subdetector.INICE
    if SourceID == dataclasses.SourceID.ICE_TOP:
        subdetector = config.Subdetector.ICETOP
        
    config.readout_settings[subdetector] = I3TriggerStatus.I3TriggerReadoutConfig()
    config.readout_settings[subdetector].readout_time_minus = ReadoutMinus
    config.readout_settings[subdetector].readout_time_plus = ReadoutPlus
    config.readout_settings[subdetector].readout_time_offset = ReadoutOffset

    config.trigger_settings['domSet'] = str(DOMSet)
    config.trigger_settings['timeWindow'] = str(TimeWindow)
    config.trigger_settings['threshold'] = str(Multiplicity)

    self[key] = config
    return

def DefineStringTrigger(self, SourceID, ConfigID,
                        DOMSet, TimeWindow, Multiplicity, CoherenceLength,
                        ReadoutMinus=5000*I3Units.ns, 
                        ReadoutPlus=5000*I3Units.ns, 
                        ReadoutOffset=0):
    """
    Create a configuration for a new string trigger
    """
    key = TriggerKey(SourceID, TypeID.SIMPLE_MULTIPLICITY, ConfigID)

    config = I3TriggerStatus()
    config.trigger_name = "ClusterTrigger"

    subdetector = config.Subdetector.INICE
    if SourceID == dataclasses.SourceID.ICE_TOP:
        subdetector = config.Subdetector.ICETOP
        
    config.readout_settings[subdetector] = I3TriggerStatus.I3TriggerReadoutConfig()
    config.readout_settings[subdetector].readout_time_minus = ReadoutMinus
    config.readout_settings[subdetector].readout_time_plus = ReadoutPlus
    config.readout_settings[subdetector].readout_time_offset = ReadoutOffset

    config.trigger_settings['domSet'] = str(DOMSet)
    config.trigger_settings['timeWindow'] = str(TimeWindow)
    config.trigger_settings['multiplicity'] = str(Multiplicity)
    config.trigger_settings['coherenceLength'] = str(CoherenceLength)

    self[key] = config
    return

def DefineVolumeTrigger(self, SourceID, ConfigID,
                        DOMSet, TimeWindow, Multiplicity, 
                        SimpleMultiplicity, Radius, Height,
                        ReadoutMinus=5000*I3Units.ns, 
                        ReadoutPlus=5000*I3Units.ns, 
                        ReadoutOffset=0):
    """
    Create a configuration for a new volume trigger
    """
    key = TriggerKey(SourceID, TypeID.VOLUME, ConfigID)
    
    config = I3TriggerStatus()
    config.trigger_name = "CylinderTrigger"

    subdetector = config.Subdetector.INICE
    if SourceID == dataclasses.SourceID.ICE_TOP:
        subdetector = config.Subdetector.ICETOP
        
    config.readout_settings[subdetector] = I3TriggerStatus.I3TriggerReadoutConfig()
    config.readout_settings[subdetector].readout_time_minus = ReadoutMinus
    config.readout_settings[subdetector].readout_time_plus = ReadoutPlus
    config.readout_settings[subdetector].readout_time_offset = ReadoutOffset

    config.trigger_settings['domSet'] = str(DOMSet)
    config.trigger_settings['timeWindow'] = str(TimeWindow)
    config.trigger_settings['multiplicity'] = str(Multiplicity)
    config.trigger_settings['simpleMultiplicity'] = str(SimpleMultiplicity)
    config.trigger_settings['radius'] = str(Radius)
    config.trigger_settings['height'] = str(Height)

    self[key] = config
    return

TriggerConfigMap.DefineSMT = DefineSMT
TriggerConfigMap.DefineStringTrigger = DefineStringTrigger
TriggerConfigMap.DefineVolumeTrigger = DefineVolumeTrigger

class AddTriggers(icetray.I3Module) :
    """
    An I3Module that can be used to define new triggers to run. 
    This is intended to be used for new triggering studies in 
    the Upgrade and Gen2 detectors. The new triggers are created
    externally, then passed in through this module. Trigger configs
    are added to the I3DetectorStatus, but will not overwrite any 
    existing triggers already in the frame. 

    An example is available at $I3_SRC/trigger-sim/python/__init__.py
    """
    def __init__(self, context):
        icetray.I3Module.__init__(self,context)

        self.AddOutBox("OutBox")
        self.AddParameter("Triggers",
                          "A map of new triggers to add to the ones defined in the GCD file.",
                          TriggerConfigMap())
        return

    def Configure(self):
        self.additionalTriggers = self.GetParameter("Triggers")

        if len(self.additionalTriggers.keys()) == 0:
            logging.log_warn("No triggers provided. AddTriggers module will be nothing.")
        return

    def DetectorStatus(self, frame):
        detstat = frame['I3DetectorStatus']
        newTrigger_config = self.additionalTriggers.copy()
        newTrigger_config.update(detstat.trigger_status)
        detstat.trigger_status.update(newTrigger_config)
        del frame['I3DetectorStatus']
        frame['I3DetectorStatus'] = detstat
        self.PushFrame(frame)
        return 
