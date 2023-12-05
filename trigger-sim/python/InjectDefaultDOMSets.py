#!/usr/bin/env python3
import copy
from icecube.icetray import I3Tray

from icecube import icetray, dataclasses, dataio
from icecube.icetray import logging
from icecube.trigger_sim import GetDefaultDOMSets

class InjectDefaultDOMSets(icetray.I3Module):
    """
    This module adds the default DOMSets information
    to the DetectorStatus frame. This will be the same
    data that older versions of trigger-sim used.
    
    If new definitions as passed via NewDefinitionsMap,
    they will be added to the DOMSets written to the frame.
    This can be used to define DOMSets for future detector
    extensions.

    The module will not overwrite existing information.
    If a DOMSet object is found, new definitions will be
    appended to it and the updated results will be pushed
    to the frame.
    """    
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddParameter('DOMSetsName', 'Name of the DOMSets frame object', 'DOMSets')
        self.AddParameter("NewDefinitionMap", 
                          "A map of a new DOMSet ID to a boolean function f(OMKey, I3OMGeo)"
                          " that determines whether an individual DOM should be included"
                          " in the new DOMSet.",
                          {})
        self.AddOutBox("OutBox")
        
    def Configure(self):
        self.domSetsName = self.GetParameter('DOMSetsName')
        self.definitions = self.GetParameter("NewDefinitionMap")

    def DetectorStatus(self, frame):
        if self.domSetsName in frame:
            logging.log_warn(f" *** There already is a frame object named {self.domSetsName} "
                             " in your DetectorStatus frame. InjectDefaultDOMSets will append to it.")
            domsets = frame[self.domSetsName]
            del frame[self.domSetsName]
        else:
            domsets = dataclasses.I3MapKeyVectorInt()

        merged_definitions = self.GetDefaultDOMSetDefinitions()
        for domset_id, definition in self.definitions.items():
            if domset_id in merged_definitions.keys():
                logging.log_fatal(f"You've set a new domset definition {domset_id}, but that already exists in the"
                                  " default set of definitions. Trying to redefine an existing DOMSet is a terrible"
                                  " idea that will only lead to headaches, so we will not allow it in trigger-sim."
                                  " Please choose a new ID for your domset.")
            merged_definitions[domset_id] = definition

        # Ensure every trigger has it's domset defined
        triggers = frame['I3DetectorStatus'].trigger_status
        for config, status in triggers:
            trigger_domset = status.trigger_settings.get('domSet', None)
            if trigger_domset is None: 
                logging.log_warn(f"Trigger {config} has no domSet set! It will default to using DOMSet 11 (ie, all Gen1 in-ice)")
            elif int(trigger_domset) not in merged_definitions.keys():
                logging.log_fatal(f"Trigger {config} is attempting to use DOMSet {trigger_domset}, but that is not"
                                  " part of the default definitions or in your configured NewDefinitionMap. Please"
                                  " define what this is supposed to and pass it into trigger-sim's InjectDefaultDOMSets"
                                  " NewDefinitionMap configuration option.")
 
        # We need to add new domsets. Because the default domsets don't include
        # new Upgrade/Gen2 modules, we'll need to loop over something different.
        # Let's grab something more reliable.
        frame[self.domSetsName] = self.GetDomSets(frame['I3Geometry'].omgeo,
                                                  merged_definitions,
                                                  domsets)
        self.PushFrame(frame)
        return

    @staticmethod
    def GetDomSets(omgeo, domset_definitions, base_domsets = None):
        """
        Static method used to create a DOMSets definition from an OMGeo and a dictionary
          of DOMSet IDs to lambda functions. This is intended to be used for the python
          interface of trigger-sim in trigger testing.
        
        Args:
         - omgeo: An I3OMGeo object which will be looped through in order to label each DOM
            with the appropriate DOMSet IDs.
         - domset_definitions: A dict{int: lambda omkey, omgeo} used to define the doms
            associated with each DOMSet.
         - base_domsets: A pre-existing DOMSet definition that you would like to use as a base
            for building the DOMSet map. No values in this base_domsets will be overwritten or
            modified in-place.
        
        Returns:
         - domsets: And I3MapKeyVectorInt used to map from an OMKey to a vector of DOMSet IDs.
        """
        if base_domsets is None:
            domsets = dataclasses.I3MapKeyVectorInt()
        else:
            domsets = copy.deepcopy(base_domsets)
        
        for dom in omgeo.keys():
            current_ids = domsets.get(dom, [])
            for domset_id, f in domset_definitions.items():
                if f(dom, omgeo[dom]) and domset_id not in current_ids: 
                    current_ids.append(domset_id)
            domsets[dom] = current_ids
        return domsets        
    
    @staticmethod
    def GetDefaultDOMSetDefinitions():
        default_domsets = {}
        
        # DomSet 2: Inice
        default_domsets[2] = lambda omkey, omgeo: (1 <= omkey.string <= 78) and (1 <= omkey.om <= 60)
        
        # DomSet 3: IceTop full configuration (IT-81)
        default_domsets[3] = lambda omkey, omgeo: (1 <= omkey.string <= 81) and (61 <= omkey.om <= 64)
        
        # DomSet 4: lower part of DeepCore strings (om>=41) plus lower part of 7 inice strings
        domset4_dc_strings = (79, 80, 81, 82, 83, 84, 85, 86)
        domset4_ic_strings = (26, 27, 35, 36, 37, 45, 46)
        default_domsets[4] = lambda omkey, omgeo: (((omkey.string in domset4_dc_strings) and (11 <= omkey.om <= 60)) or
                                                   ((omkey.string in domset4_ic_strings) and (41 <= omkey.om <= 60)))
        
        # DomSet 5: lower part of DeepCore strings (om>=39) plus lower part of 7 inice strings
        domset5_dc_strings = (79, 80, 81, 82, 83, 84, 85, 86)
        domset5_ic_strings = (26, 27, 35, 36, 37, 45, 46)
        default_domsets[5] = lambda omkey, omgeo: (((omkey.string in domset5_dc_strings) and (11 <= omkey.om <= 60)) or
                                                   ((omkey.string in domset5_ic_strings) and (39 <= omkey.om <= 60)))
        
        # DomSet 6: lower part of DeepCore strings (om>=39) plus lower part of 12 inice strings
        domset6_dc_strings = (79, 80, 81, 82, 83, 84, 85, 86)
        domset6_ic_strings = (25, 26, 27, 34, 35, 36, 37, 44, 45, 46, 47, 54)
        default_domsets[6] = lambda omkey, omgeo: (((omkey.string in domset6_dc_strings) and (11 <= omkey.om <= 60)) or
                                                   ((omkey.string in domset6_ic_strings) and (39 <= omkey.om <= 60)))
        
        # DOMSet 7: Scintillators
        domset7_stations = [12, 62]
        default_domsets[7] = lambda omkey, omgeo: (omkey.string in domset7_stations) and (65 <= omkey.om <= 66)
        
        # DOMSet 8 (for IceTop Volume (a.k.a. 2-Station) trigger.  
        domset8_stations = (26, 36, 46, 79, 80, 81)
        default_domsets[8] = lambda omkey, omgeo: (omkey.string in domset8_stations) and (omkey.om in [61+(omkey.string==26), 63])
        
        #  DOMSet 9: IceACT
        default_domsets[9] = lambda omkey, omgeo: (omkey.string == 0) and (omkey.om == 1)
        
        # DOMSet 10: DMIce
        default_domsets[10] = lambda omkey, omgeo: (omkey.string == 0) and (2 <= omkey.om <= 5)
        
        # DomSet 11: InIce full configuration (IC-86)
        default_domsets[11] = lambda omkey, omgeo: (1 <= omkey.string <= 86) and (1 <= omkey.om <= 60)

        return default_domsets


