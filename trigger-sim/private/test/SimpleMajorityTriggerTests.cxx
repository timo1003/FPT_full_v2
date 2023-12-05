
#include <I3Test.h>
#include <iostream>
#include <boost/foreach.hpp>
#include <trigger-sim/algorithms/SimpleMajorityTriggerAlgorithm.h>
#include <trigger-sim/utilities/DOMSetFunctions.h>
#include "phys-services/I3GSLRandomService.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "trigger-sim/algorithms/TriggerHit.h"
#include "icetray/I3Units.h"

TEST_GROUP(SimpleMajorityTriggerTests);

const unsigned int MIN_MULTIPLICITY(3);
const unsigned int MAX_MULTIPLICITY(20);
const unsigned int MIN_TIME_WINDOW(100);
const unsigned int MAX_TIME_WINDOW(10000);
const unsigned int TIME_STEP(500);
const unsigned int NTRIALS(10);

using namespace std;

void AddPulse(I3RecoPulseSeriesMapPtr target, double time, unsigned int om, unsigned int string){
  OMKey omk(string, om);
  I3RecoPulse pulse;
  pulse.SetTime(time);
  pulse.SetFlags(I3RecoPulse::PulseFlags::LC);
  if(target->find(omk) == target->end())
    (*target)[omk] = I3RecoPulseSeries();
  (*target)[omk].push_back(pulse);
  return;
}

void MultipleTest(unsigned int nTriggers) {

  cerr << endl;
  cerr << "MultipleTest (with nTriggers=" << nTriggers << ") details..." << endl;

  I3GSLRandomService rand(99999); 

  for(unsigned int timeWindow(MIN_TIME_WINDOW); 
      timeWindow < MAX_TIME_WINDOW ;  
      timeWindow += TIME_STEP){
    for(unsigned int multiplicity(MIN_MULTIPLICITY); multiplicity <  MAX_MULTIPLICITY; multiplicity++){

      SimpleMajorityTriggerAlgorithm smTrigger(timeWindow, multiplicity, 11, I3MapKeyVectorIntConstPtr());
      
      for(unsigned int trial(0); trial < NTRIALS; trial++){ 
        I3RecoPulseSeriesMapPtr hits(new I3RecoPulseSeriesMap());
        
	for(unsigned int n(0); n < nTriggers ; n++){
	  double windowShift(2*n * (timeWindow + 1*I3Units::nanosecond));
	  
	  for(unsigned int nHits(0); nHits < multiplicity; nHits++){
	    int pos(rand.Uniform(1,60));
	    int string(rand.Uniform(1,86));
	    double time(rand.Uniform(0,timeWindow));
            AddPulse(hits, time+windowShift, pos, string);
	  }
        }
        
        //SimpleMajorityTriggerAlgorithm::AddHits clears the hit vector
        //and the trigger count so, we only need one instantiation of the
        //trigger algoritmhm for each cycle.
        smTrigger.FillHits(I3DOMLaunchSeriesMapConstPtr(new I3DOMLaunchSeriesMap()),
                           I3RecoPulseSeriesMapConstPtr(hits),false);
        smTrigger.Trigger();
        unsigned int nTrig = smTrigger.GetNumberOfTriggers();
        log_debug("length of hits is %d, Ntrig is %d, should be %d", (int)hits->size(), nTrig, nTriggers);
        ENSURE(nTrig == nTriggers);
      }
    }
  }
}

//Add stragglers to the end of an event and make
//sure the time window isn't extended
void StragglerTest() {

  cerr << endl;
  cerr << "StragglerTest details..." << endl;

  I3GSLRandomService rand(99999); 

  for(unsigned int timeWindow(MIN_TIME_WINDOW); 
      timeWindow < MAX_TIME_WINDOW ;  
      timeWindow += TIME_STEP){

    for(unsigned int multiplicity(MIN_MULTIPLICITY); multiplicity <  MAX_MULTIPLICITY; multiplicity++){

      for(unsigned int trial(0); trial < NTRIALS; trial++){ 
        SimpleMajorityTriggerAlgorithm smTrigger(timeWindow, multiplicity, 11, I3MapKeyVectorIntConstPtr());
        I3RecoPulseSeriesMapPtr hits(new I3RecoPulseSeriesMap());
        
	for(unsigned int nHits(0); nHits < multiplicity-1; nHits++){
	  int pos(rand.Uniform(1,60));
	  int string(rand.Uniform(1,86));
	  double time(rand.Uniform(0,1*I3Units::nanosecond));
          AddPulse(hits, time, pos, string);
	}

	//these are the stragglers
	for(int n(0); n<100; n++){
	  int pos(rand.Uniform(1,60));
	  int string(rand.Uniform(1,86));

	  //just inside the time window
	  //the very first straggler contributes to the trigger
	  //the rest should not
	  double time((0.999 + 1.01*n)*timeWindow);
          AddPulse(hits, time, pos, string);
	}
	
	//SimpleMajorityTriggerAlgorithm::FillHits clears the hit vector
	//and the trigger count so, we only need one instantiation of the
	//trigger algoritmhm for each cycle.
        smTrigger.FillHits(I3DOMLaunchSeriesMapConstPtr(new I3DOMLaunchSeriesMap()),
                           I3RecoPulseSeriesMapConstPtr(hits),false);
        smTrigger.Trigger();

	unsigned int nTrig = smTrigger.GetNumberOfTriggers();
        log_debug("length of hits is %d, Ntrig is %d, should be 1", (int)hits->size(), nTrig);
	ENSURE(nTrig == 1);
        
	for (unsigned int i=0; i<nTrig; i++) {
	  TriggerHitVectorPtr triggerHits = smTrigger.GetNextTrigger();

	  // Ensure that the trigger has the correct number of hits
          ENSURE(triggerHits->size() == multiplicity);
	}
      }
    }
  }
}

//Add a cluster of THRESH-1 hits followed by a single "connector"
//hit just inside the time window, then another cluster, and so on.
//This should create one really long event.
void ConnectorTest(){ 

  cerr << endl;
  cerr << "ConnectorTest details..." << endl;

  I3GSLRandomService rand(99999); 

  const unsigned int NCLUSTERS(5);

  for(unsigned int timeWindow(MIN_TIME_WINDOW); 
      timeWindow < MAX_TIME_WINDOW ;  
      timeWindow += TIME_STEP){
    for(unsigned int multiplicity(MIN_MULTIPLICITY); multiplicity <  MAX_MULTIPLICITY; multiplicity++){
  
      for(unsigned int trial(0); trial < NTRIALS; trial++){ 
        SimpleMajorityTriggerAlgorithm smTrigger(timeWindow, multiplicity, 11, I3MapKeyVectorIntConstPtr());
        I3RecoPulseSeriesMapPtr hits(new I3RecoPulseSeriesMap());

	for(unsigned int n(0); n<NCLUSTERS; n++){
	  double clusterTime(2*n*0.99*timeWindow);
	  for(unsigned int nHits(0); nHits < multiplicity -1; nHits++){
	    int pos(rand.Uniform(1,60));
	    int string(rand.Uniform(1,86));
	    double time(rand.Uniform(0,1*I3Units::nanosecond) + clusterTime);

            AddPulse(hits, time, pos, string);
	  }

	  int pos(rand.Uniform(1,60));
	  int string(rand.Uniform(1,86));
	  double connectorTime((2*n+1)*0.99*timeWindow);	    	
          AddPulse(hits, connectorTime, pos, string);
	}
	
	//SimpleMajorityTriggerAlgorithm::AddHits clears the hit vector
	//and the trigger count so, we only need one instantiation of the
	//trigger algoritmhm for each cycle.
        smTrigger.FillHits(I3DOMLaunchSeriesMapConstPtr(new I3DOMLaunchSeriesMap()),
                           I3RecoPulseSeriesMapConstPtr(hits),false);
        smTrigger.Trigger();
	unsigned int nTrig = smTrigger.GetNumberOfTriggers();
        log_debug("length of hits is %d, Ntrig is %d, should be 1", (int)hits->size(), nTrig);
	ENSURE(nTrig == 1);
	
	TriggerHitVectorPtr timeHits = smTrigger.GetNextTrigger();
	double startTime(timeHits->front().time);
	double stopTime(timeHits->back().time);
	double triggerLength(stopTime - startTime);

	ENSURE( triggerLength > timeWindow);
	ENSURE( triggerLength < 2*NCLUSTERS*0.999*timeWindow);

      }
    }
  }
}


TEST(simple_test) {
  MultipleTest(1);
}

TEST(multiple_test) {
  for(int nTrigger(2); nTrigger < 10; nTrigger++)
    MultipleTest(nTrigger);
}

TEST(straggler_test){
  StragglerTest();
}

TEST(connector_test){
  ConnectorTest();
}

