from icecube._trigger_sim import *  # noqa: F403

from icecube import icetray
from icecube.dataclasses import I3OMGeo
from icecube.trigger_sim.InjectDefaultDOMSets import InjectDefaultDOMSets

@icetray.traysegment
def ExtensionsTriggers(tray, name):
    """
    Define domsets and triggers for both the Upgrade and Gen2.
    This acts as both an example tray segment for adding triggering
    information to the GCD file and as a temporary standard method
    for testing new triggers.

    Arguments:
      tray: The I3Tray where we will add I3Modules
      name: An arbitrary name for this segment. Will not be used, but is required.
    """
    #====================================
    # Add some custom triggers for testing
    #====================================
    from icecube.icetray import I3Units
    from icecube.dataclasses import SourceID
    from icecube.trigger_sim.modules import add_triggers

    domset_definitions =  {}
    newTriggers = add_triggers.TriggerConfigMap()

    # An example of an Upgrade trigger. This only uses mDOMs and DEggs in a
    # "physics region" defined in sharepoint
    def physics_region(omkey, omgeo):
        if omkey.string < 87: return False
        if omkey.string > 93: return False
        if omgeo.omtype not in [I3OMGeo.OMType.PDOM, I3OMGeo.OMType.mDOM, I3OMGeo.OMType.DEgg]: return False
        pos_z = omgeo.position.z

        # Only include doms in the physics region using numbers from sharepoint
        # https://uwprod.sharepoint.com/:x:/r/sites/icecubeupgrade/_layouts/15/Doc.aspx?sourcedoc=%7BE97DF61B-4A99-41D7-A4E4-A3464802B8C4%7D&file=Upgrade%20Strings%20CMD.xlsx
        icecube_center = 1948.07*I3Units.m
        top = icecube_center - 2163*I3Units.m
        bottom = icecube_center - 2430*I3Units.m
        return (pos_z >= bottom) & (pos_z <= top)

    domset_definitions[1000] = physics_region
    newTriggers.DefineVolumeTrigger(SourceID.IN_ICE_PULSES,
                                    ConfigID = 1018,
                                    DOMSet = 1000,
                                    TimeWindow = 1750*I3Units.ns,
                                    Multiplicity = 8,
                                    SimpleMultiplicity = 8,
                                    Radius = 60000,
                                    Height = 60000)

    # And an example of a Gen2 trigger
    domset_definitions[1002] = lambda omkey, omgeo: omkey.string>=1000
    newTriggers.DefineVolumeTrigger(SourceID.IN_ICE_PULSES,
                                    ConfigID = 1023,
                                    DOMSet = 1002,
                                    TimeWindow = 1750*I3Units.ns,
                                    Multiplicity = 20,
                                    SimpleMultiplicity = 20,
                                    Radius = 60000,
                                    Height = 60000)

    #====================================
    # And actually add them
    #====================================
    tray.Add(add_triggers.AddTriggers,
             Triggers = newTriggers)

    tray.Add(InjectDefaultDOMSets,
             NewDefinitionMap = domset_definitions)

    return


@icetray.traysegment
def TriggerSim(tray,
               name,
               run_id = None,
               prune = True,
               time_shift = True,
               time_shift_args = None,
               filter_mode = True,
               **kwargs):
    """
    Configure triggers according to the GCD file.

    NB: This segment no longer supports the ULEE trigger, so now only
    IC59 and later configurations.

    Arguments:
        tray: Standard for segments.
        name: Standard for segments.
        run_id: The run id.
        prune: Whether to remove launches outside the readout windows.  Nearly everyone will want to keep this set at True.  It makes simulation look more like data.
        time_shift: Whether to time shift time-like frame objects.  Nearly everyone will want to keep this set at True.  It makes simulation look more like data.
        time_shift_args: dict that's forwarded to the I3TimeShifter module.
        filter_mode: Whether to filter frames that do not trigger.

    This ignores AMANDA triggers and only supports the following modules:

    * SimpleMajorityTrigger
    * ClusterTrigger
    * CylinderTrigger
    * SlowMonopoleTrigger

    This segment also includes the I3GlobalTriggerSim as well as the
    I3Pruner and I3TimeShifter.  Its job is to make simulation look
    like data that the pole filters get.
    """

    if time_shift_args is None:
        time_shift_args = {}

    if run_id is None :
        icetray.logging.log_fatal("You have to set run_id to a valid number.")

    #====================================
    # Define upgrade and gen2 triggers
    # These are harmless to add to gen1 simulation
    # since none of the Gen1 DOMs will fall into
    # the Gen2/Upgrade DOMSets anyway.
    #
    # The price we pay is processing time to loop through
    # the hits two more times.
    # This seems like a reasonable payoff for the
    # simplification
    #====================================
    tray.Add(ExtensionsTriggers, "AddExtensionsTriggerDefinitions")

    #====================================
    # Run the triggers. SLOP is awkward and
    # uses a different format than other triggers,
    # so just run it separately for now.
    #====================================
    tray.AddModule("I3TriggerSimModule",
                   InIcePulses = "I3RecoPulseSeriesMapExtensions")

    tray.AddModule("I3GlobalTriggerSim",name + "_global_trig",
                   RunID = run_id,
                   FilterMode = filter_mode)

    if prune :
        tray.AddModule("I3Pruner")

    if time_shift :
        from icecube.trigger_sim.modules.time_shifter import I3TimeShifter
        tray.AddModule(I3TimeShifter, **time_shift_args)
