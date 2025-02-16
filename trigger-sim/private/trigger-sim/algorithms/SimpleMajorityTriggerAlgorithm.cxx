/**
 * copyright  (C) 2006
 * the icecube collaboration
 * $Id:
 *
 * @file SimpleMajorityTriggerAlgorithm.cxx
 * @version
 * @date
 * @author toale
 */

#include <trigger-sim/algorithms/SimpleMajorityTriggerAlgorithm.h>
#include <trigger-sim/algorithms/TimeWindow.h>
#include <boost/foreach.hpp>
#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

SimpleMajorityTriggerAlgorithm::SimpleMajorityTriggerAlgorithm(double triggerWindow, 
							       unsigned int triggerThreshold,
                                                               int domSet, 
                                                               I3MapKeyVectorIntConstPtr customDomSets): 
  TriggerService(domSet, customDomSets),
  triggerWindow_(triggerWindow),  triggerThreshold_(triggerThreshold)
{
  log_debug("SimpleMajorityTriggerAlgorithm configuration:");
  log_debug("  TriggerWindow = %f", triggerWindow_);
  log_debug("  TriggerThreshold = %d", triggerThreshold_);

}

void SimpleMajorityTriggerAlgorithm::Trigger()
{

  log_debug("Adding %zd hits to SimpleMajorityTigger", hits_->size());

  /*------------------------------------------------------------*
   * Check Trigger condition
   *------------------------------------------------------------*/
  TimeWindow timeWindow(triggerThreshold_, triggerWindow_);
  triggers_.clear();
  triggerCount_ = 0;
  triggerIndex_ = 0;

  // Get time windows
  TriggerHitIterPairVectorPtr timeWindows = timeWindow.SlidingTimeWindows(hits_);

  // If the vector is empty, there are no time windows for this string
  if (timeWindows->empty()) {
    log_debug("No valid time windows for this string");
    return;
  }
  log_debug("Found %zd triggered time windows", timeWindows->size());

  // Loop over the time windows and pull out the hits in each
  for (TriggerHitIterPairVector::const_iterator timeWindowIter = timeWindows->begin(); 
       timeWindowIter != timeWindows->end(); 
       timeWindowIter++) {

    // Create a vector of the hits in this time window
    TriggerHitVectorPtr timeHits(new TriggerHitVector);

    // Get the window boundaries
    TriggerHitVector::const_iterator firstHit = timeWindowIter->first;
    TriggerHitVector::const_iterator lastHit  = timeWindowIter->second;

    // Iterate over the hits and pull out the ones for this window
    for (TriggerHitVector::const_iterator hitIter = firstHit; 
	 hitIter != lastHit; 
	 hitIter++)
      timeHits->push_back(*hitIter);

    log_debug("Time window (%f, %f) has %zd hits", firstHit->time, (--lastHit)->time, timeHits->size());

    triggers_.push_back(*timeHits);
    triggerCount_++;
    log_debug("Trigger! Count = %d", triggerCount_);
  } 
}

