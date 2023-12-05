# module to filter events with DM Ice triggers

from icecube import icetray

@icetray.traysegment
def DMIceTrigFilter(tray, name, If = lambda f: True):

  from icecube.filterscripts import filter_globals

  icetray.load("filterscripts", False)

  tray.AddModule("I3FilterModule<I3BoolFilter>", name+"DMIceTrigFilter",
	DecisionName=filter_globals.DMIceFilter,
	DiscardEvents=False,
	Boolkey=filter_globals.dmicesmttriggered,
	TriggerEvalList=[filter_globals.dmicesmttriggered],
	If=If
	)
