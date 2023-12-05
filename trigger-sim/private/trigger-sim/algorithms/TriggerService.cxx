/**
 * copyright  (C) 2022
 * the icecube collaboration
 * $Id: 
 *
 * @file TriggerService.cxx
 * @version
 * @date
 * @author mlarson
 */

#include "trigger-sim/algorithms/TriggerService.h"
#include <algorithm>
#include <boost/foreach.hpp>

TriggerService::TriggerService(int domSet, 
                               I3MapKeyVectorIntConstPtr customDomSets):
  domSet_(domSet), customDomSets_(customDomSets),
  hits_(new TriggerHitVector()), triggers_(TriggerHitVectorVector()), 
  triggerCount_(0), triggerIndex_(0)
{}

void TriggerService::FillHits(I3DOMLaunchSeriesMapConstPtr launches,
                              I3RecoPulseSeriesMapConstPtr pulses,
                              bool useSLC)
{
  // Reset the current hits 
  hits_->clear();

  // And then load the individuals
  if(launches->size()) Extract(launches,useSLC);
  if(pulses->size()) Extract(pulses,useSLC);

  std::sort(hits_->begin(), hits_->end());
  return;
}

void TriggerService::Extract(I3DOMLaunchSeriesMapConstPtr launches, bool useSLC)
{
  BOOST_FOREACH(auto mapItem, *launches){
    const OMKey& omKey = mapItem.first;
    
    BOOST_FOREACH(const I3DOMLaunch& launch, mapItem.second){
      if(!(launch.GetLCBit() || useSLC)) continue;
      
      if( !(domSet_ && DOMSetFunctions::InDOMSet(omKey, domSet_, customDomSets_)))
        continue;
      
      TriggerHitPtr hit(new TriggerHit);
      hit->pos = omKey.GetOM();
      hit->string = omKey.GetString();
      hit->time = launch.GetStartTime();
      hit->lc = launch.GetLCBit();
      hits_->push_back(*hit);
    }
  }
}
void TriggerService::Extract(I3RecoPulseSeriesMapConstPtr pulses, bool useSLC)
{
  BOOST_FOREACH(auto mapItem, *pulses){
    const OMKey& omKey = mapItem.first;

    BOOST_FOREACH(const I3RecoPulse& pulse, mapItem.second){
      if( !(pulse.GetFlags() & I3RecoPulse::LC)||useSLC) continue;
      
      if( !(domSet_ && DOMSetFunctions::InDOMSet(omKey, domSet_, customDomSets_)) )
          continue;
      
      TriggerHitPtr hit(new TriggerHit);
      hit->pos = omKey.GetOM();
      hit->string = omKey.GetString();
      hit->time = pulse.GetTime();
      hits_->push_back(*hit);
    }
  }
}


unsigned int TriggerService::GetNumberOfTriggers() {
  return triggerCount_;
}

TriggerHitVectorPtr TriggerService::GetNextTrigger() {
  TriggerHitVectorPtr hits;
  if (triggerCount_ > 0) {
    triggerCount_--;
    log_debug("Returning trigger window %d", triggerCount_);
    hits = TriggerHitVectorPtr(new TriggerHitVector(triggers_.at(triggerCount_)));
  }
  return hits;
}


