'''
IceTop filter tray segment.

This segment defines the IceTopSelectedStations_22 filters

To apply this filter, this segment:
- Counts the HLCTank hits of a given I3RecoPulsesSeries (inputRecoPulses) 
  if they are from a station of the stationlist (stationList) and
  checks if both tanks are hit if required(bothTanksHit)
- Filters events based on the given HLC threshold.    (requiredCountsforFilter)

Returns:
- I3Bool in IceTopSplit P frames named by outFiltername (standard: IceTop_SouthwestNHLCTank1_22) with
    filter results per-split

This filter also will save the following objects for later use/archiving:
- Number of HLC counts used for the filter (standard: IceTopHLCCount_SW)
'''
from icecube import dataclasses, icetray
from icecube.filterscripts import filter_globals
from icecube.phys_services.which_split import which_split

@icetray.traysegment
def IceTopFilterSelectedStations(tray, name='icetop_filter',
                                 inputRecoPulses=filter_globals.HLCTankPulses,
                                 requiredCountsforFilter=1,
                                 stationList=['33','34','43'],
                                 bothTanksHit= False,
                                 outHLCCountname='IceTopHLCCount_SW',
                                 outFiltername='IceTop_SouthwestNHLCTank1_22'):

    def IceTopFilterSelect(frame, inputCount='IceTopHLCCount_SW',
                           requiredCounts=1,
                           outputName='IceTop_SouthwestNHLCTank1_22'):
        filter_value = False
        # Pick events based on the HLC hits
        if inputCount in frame:
            number = frame[inputCount]
            if number >= requiredCounts:
                    filter_value = True
                    #print('IceTopFilterSelect True')
        # Store the filter result in the frame
        filter_value_fr = icetray.I3Bool(filter_value)
        frame[outputName] = filter_value_fr
        
    def CountIceTopHLCTanks(frame, inputPulses='IceTopHLCTankPulses',
                            stationlist=['33','34','43'],
                            bothTanks = False,
                            outputObject='IceTopHLCCount_SW'):
        required_things = [inputPulses,
                           'I3Geometry',
                           'I3DetectorStatus'
                           ]
        if any(frame.Has(item) is False for item in required_things):
            icetray.logging.log_fatal(inputPulses + ' missing inputs!')

        pulses = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, inputPulses)

        omgeo = frame['I3Geometry'].omgeo
        # Count HLC 
        n = 0
        countTanks = []
        countStations =[]
        for omkey, pulse in pulses:
            # Make sure this is an IceTop key...
            if not omgeo[omkey].omtype == dataclasses.I3OMGeo.IceTop:
                icetray.logging.log_fatal("CheckIceTopHLCTanks found non-IceTop launch")
                
            stationid = str(omkey.string)
            tankid    = str(omkey.string)
            if  stationid  in stationlist:
                if omkey.om < 63:
                    tankid += '_tankA'
                elif omkey.om > 62:
                    tankid += '_tankB'
                #Check if both tanks in a station should be hit
                if not bothTanks:
                    #Check if tank is already in tanklist if not add it and count the hit:
                    if tankid not in countTanks:
                        countTanks.append(tankid)
                        n += 1
                #If both tanks need to be hit to count the event
                elif bothTanks:
                    # Check if a firstthe tank is in the tanklist if not add it to the tanklist and add the station to the stationlist
                    if tankid not in countTanks and stationid not in countStations:
                        countTanks.append(tankid)
                        countStations.append(stationid)
                        n += 1
                    #If the tank is not in the tank list but the tanks' staion in the stationlist, add the tank to the tanklist and count the station
                    elif tankid not in countTanks and stationid in countStations:
                        countTanks.append(tankid)
                        n += 1
        # Store the filter result in the frame
        counter = icetray.I3Int(n)
        frame[outputObject] = counter
    # Counting HLC Hits
    tray.Add(CountIceTopHLCTanks, name + '_count',
             inputPulses = inputRecoPulses,
             stationlist = stationList,
             bothTanks = bothTanksHit,
             outputObject=outHLCCountname,
             If=which_split(split_name='IceTopSplit'))        

    # Adding IceTop Filter
    tray.Add(IceTopFilterSelect, name + '_do_filter',
             inputCount = outHLCCountname,
             requiredCounts = requiredCountsforFilter,
             outputName = outFiltername,
         If=which_split(split_name='IceTopSplit'))
    
