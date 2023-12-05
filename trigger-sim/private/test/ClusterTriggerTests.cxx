#include <I3Test.h>

#include "trigger-sim/algorithms/ClusterTriggerAlgorithm.h"
#include "trigger-sim/algorithms/TriggerHit.h"
#include <icetray/OMKey.h>
#include <dataclasses/physics/I3DOMLaunch.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <phys-services/I3GSLRandomService.h>

TEST_GROUP(ClusterTriggerTests);

void AddLaunch(I3DOMLaunchSeriesMapPtr target, double time, unsigned int om, unsigned int string){
  OMKey omk(string, om);
  I3DOMLaunch launch;
  launch.SetStartTime(time);
  launch.SetLCBit(true);
  if(target->find(omk) == target->end())
    (*target)[omk] = I3DOMLaunchSeries();
  (*target)[omk].push_back(launch);
  return;
}

void SimpleCTTest() {

  ClusterTriggerAlgorithm stringTrigger(1500, 3, 7, 2, I3MapKeyVectorIntConstPtr());
  I3DOMLaunchSeriesMapPtr hits(new I3DOMLaunchSeriesMap());

  AddLaunch(hits,1,1,1);
  AddLaunch(hits,2,2,1);
  AddLaunch(hits,3,3,1);
  AddLaunch(hits,2001,4,1);
  AddLaunch(hits,2002,5,1);
  AddLaunch(hits,2003,6,1);

  stringTrigger.FillHits(I3DOMLaunchSeriesMapConstPtr(hits),
                         I3RecoPulseSeriesMapConstPtr(new I3RecoPulseSeriesMap()),false);
  stringTrigger.Trigger();
  unsigned int nTrig = stringTrigger.GetNumberOfTriggers();
  ENSURE(nTrig == 2);

}

void TestPruningHits() {

  ClusterTriggerAlgorithm stringTrigger(1500, 3, 7, 2, I3MapKeyVectorIntConstPtr());
  I3DOMLaunchSeriesMapPtr hits(new I3DOMLaunchSeriesMap());

  AddLaunch(hits,1,1,1);
  AddLaunch(hits,2,2,2);
  AddLaunch(hits,3,3,2);
  AddLaunch(hits,4,1,1);
  AddLaunch(hits,5,2,1);
  AddLaunch(hits,6,3,1);
  AddLaunch(hits,7,4,3);

  stringTrigger.FillHits(I3DOMLaunchSeriesMapConstPtr(hits),
                         I3RecoPulseSeriesMapConstPtr(new I3RecoPulseSeriesMap()),false);
  stringTrigger.Trigger();
  unsigned int nTrig = stringTrigger.GetNumberOfTriggers();
  ENSURE(nTrig == 1);

}

void TestNoTrigger() {

  ClusterTriggerAlgorithm stringTrigger(1500, 3, 7, 2, I3MapKeyVectorIntConstPtr());
  I3DOMLaunchSeriesMapPtr hits(new I3DOMLaunchSeriesMap());

  AddLaunch(hits,1,1,1);
  AddLaunch(hits,2001,2,1);
  AddLaunch(hits,4001,3,1);
  AddLaunch(hits,5001,4,1);
  AddLaunch(hits,6001,5,1);
  AddLaunch(hits,8001,6,1);

  stringTrigger.FillHits(I3DOMLaunchSeriesMapConstPtr(hits),
                         I3RecoPulseSeriesMapConstPtr(new I3RecoPulseSeriesMap()),false);
  stringTrigger.Trigger();
  unsigned int nTrig = stringTrigger.GetNumberOfTriggers();
  ENSURE(nTrig == 0);

}

void TestHashing() {

  ClusterTriggerAlgorithm stringTrigger(1500, 3, 7, 2, I3MapKeyVectorIntConstPtr());

  for (int s = 1; s < 100; s++) {
    for (unsigned int p = 1; p <= 60; p++) {
      int hash = stringTrigger.GetHash(s, p);

      int string = stringTrigger.GetString(hash);
      unsigned int position = stringTrigger.GetPosition(hash);

      ENSURE( string == s );
      ENSURE( position == p );
    }
  }
}


void TriggerTest() {

  ClusterTriggerAlgorithm stringTrigger(2500, 5, 7, 2, I3MapKeyVectorIntConstPtr());

  I3GSLRandomService rand(99999); 

  I3DOMLaunchSeriesMapPtr hits(new I3DOMLaunchSeriesMap());
  for ( int i(0) ; i < 5; i++){
    double time( rand.Uniform(1,2500) );
    unsigned pos( rand.Uniform(27,34) );
    AddLaunch(hits, time, pos, 21);
  }

  stringTrigger.FillHits(I3DOMLaunchSeriesMapConstPtr(hits),
                         I3RecoPulseSeriesMapConstPtr(new I3RecoPulseSeriesMap()),false);
  stringTrigger.Trigger();
  unsigned int nTrig = stringTrigger.GetNumberOfTriggers();
  ENSURE(nTrig == 1);
}


TEST(simple_ct_test) {
  SimpleCTTest();
}

TEST(test_pruning_hits) {
  TestPruningHits();
}

TEST(test_no_trigger) {
  TestNoTrigger();
}

TEST(test_hashing) {
  TestHashing();
}


TEST(at_threshold){
  for( int i(0); i < 10000; i++)
    TriggerTest();
}
