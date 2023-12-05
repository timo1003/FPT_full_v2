from icecube import icetray, dataclasses


class LaunchDowngrader(icetray.I3Module):
    """
    This module duplicates an IceTopRawData object in the frame.
    The duplicate is exactly the same as the input except that all
    launches that have the LC bit off are discarded and those that
    have the LC bit on are 'downgraded', turning the LC bit off.
    """
    def __init__(self, ctx):
        icetray.I3Module.__init__(self, ctx)
        self.AddParameter("Launches",
                          "Input raw data in the frame",
                          "IceTopRawData")
        self.AddParameter("Output",
                          "Where to put the result",
                          "IceTopRawDataRedundant")
        self.AddOutBox("OutBox")

    def Configure(self):
        self.in_name = self.GetParameter("Launches")
        self.out_name = self.GetParameter("Output")

    def DAQ(self, frame):
        if self.in_name in frame:
            raw_data = frame[self.in_name]
            redundant_data = dataclasses.I3DOMLaunchSeriesMap()
            for dom_key, launches in raw_data.iteritems():
                extra = dataclasses.I3DOMLaunchSeries()
                for launch in launches:
                    if launch.lc_bit:
                        laun = dataclasses.I3DOMLaunch(launch)
                        laun.lc_bit = False
                        extra.append(laun)
                # Only add this DOMKey to the map if it contains something
                if len(extra) > 0:
                    icetray.i3logging.log_debug("Creating redundant data, length=%d"%len(extra))
                    redundant_data[dom_key] = extra
                else:
                    icetray.i3logging.log_debug("Found some Launches, but length=0")
            if len(redundant_data) > 0:
                frame.Put(self.out_name, redundant_data)
        else:
            icetray.i3logging.log_debug("Did not find the launches %s in the frame."%self.in_name)
        self.PushFrame(frame)


@icetray.traysegment
def ExtractHLCsAsSLCs(tray, name):
    """
    Segment to add a list of HLC pulses that have been processed like
    SLCs. It starts from the raw data, discards pulses with LC
    bit off, takes the pulses with LC bit on and turns it off, and
    extracts the pulses.

    This segment is only to get each DOM's SLC calibration
    constants. Most users do not need to use it.
    """
    from icecube import icetray
    icetray.load('WaveCalibrator', False)
    icetray.load('tpx', False)
    icetray.load('vemcal', False)

    # create a raw data without SLCs and with HLCs 'demoted'.
    # Default input = "IceTopRawData"
    # Default output = "IceTopRawDataRedundant"
    # KR To-Do: ONLY run this *if* IceTopRawData exists
    tray.AddModule(LaunchDowngrader, name + '_downgrade_ice_top_launches',
                   Launches='CleanIceTopRawData'
                   )

    # From now on, this does pulse extraction.
    tray.AddModule('I3WaveCalibrator', name + '_CalibrateIceTop',
                   Launches='IceTopRawDataRedundant',
                   Waveforms='CalibratedIceTopWaveforms_2',
                   WaveformRange='CalibratedIceTopWaveformRange_2',
                   If=lambda fr: 'IceTopRawDataRedundant' in fr,
                   )

    tray.AddModule('I3WaveformSplitter', name + '_IceTopSplitter',
                   Input='CalibratedIceTopWaveforms_2',
                   HLC_ATWD='OfflineCalibratedIceTopATWD_HLC_2',
                   HLC_FADC='OfflineCalibratedIceTopFADC_HLC_2',
                   SLC='OfflineCalibratedIceTopATWD_SLC_2',
                   # ! put all maps in the frame, even if they are empty
                   Force=True,
                   # ! keep highest non-saturated gain one
                   PickUnsaturatedATWD=True,
                   If=lambda fr: 'IceTopRawDataRedundant' in fr,
                   )

    tray.AddModule('I3TopSLCPulseExtractor', name + '_SLCPulseExtractor',
                   Waveforms='OfflineCalibratedIceTopATWD_SLC_2',
                   PEPulses='IceTopHLCasSLCPEPulses_forSLCcal',
                   VEMPulses='',
                   If=lambda fr: 'IceTopRawDataRedundant' in fr,
                   )

    # This module grabs the HLC/SLC/Waveforms, and stores what we'll need later
    # in a lightweight frame object
    tray.AddModule('I3ITSLCCalExtractor', name + '_slccal_extract',
                   # use the "PE" output version, not the "VEM" version
                   IceTopRegularHLCPulses='IceTopPulses_HLC',
                   IceTopDowngradedPulses='IceTopHLCasSLCPEPulses_forSLCcal',
                   IceTopWaveforms='OfflineCalibratedIceTopATWD_SLC_2',
                   # the actual output of this traysegment to keep in the frame
                   SLCCalData='I3ITSLCCalData',
                   If=lambda fr: 'IceTopRawDataRedundant' in fr
                   )

    # Cleanup of frame objects no longer needed
    my_garbage = ['IceTopRawDataRedundant',
                  'CalibratedIceTopWaveforms_2',
                  'CalibratedIceTopWaveformRange_2',
                  'OfflineCalibratedIceTopATWD_HLC_2',
                  'OfflineCalibratedIceTopFADC_HLC_2',
                  'OfflineCalibratedIceTopATWD_SLC_2',
                  'IceTopHLCasSLCPEPulses_forSLCcal',
                  ]

    tray.Add("Delete", name + '_cleanup',
             keys=my_garbage)
