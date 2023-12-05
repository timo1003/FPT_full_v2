/**
 * copyright  (C) 2006
 * the icecube collaboration
 * $Id: 
 *
 * @file TriggerService.h
 * @version
 * @date
 * @author mlarson
 */

#ifndef TRIGGER_SERVICE_H
#define TRIGGER_SERVICE_H

#include "icetray/I3Logging.h"
#include "icetray/OMKey.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3Map.h"
#include "trigger-sim/utilities/DOMSetFunctions.h"
#include "trigger-sim/algorithms/TriggerHit.h"

class TriggerService
{
public: 
  TriggerService(int domSet, I3MapKeyVectorIntConstPtr customDomSets);
  virtual ~TriggerService() {};

  void FillHits(I3DOMLaunchSeriesMapConstPtr launches,
                I3RecoPulseSeriesMapConstPtr pulses,
                bool useSLC);
    
  virtual void Trigger() = 0;

  unsigned int GetNumberOfTriggers();
  TriggerHitVectorPtr GetNextTrigger();

 protected:
  void Extract(I3DOMLaunchSeriesMapConstPtr launches,
               bool useSLC);
  void Extract(I3RecoPulseSeriesMapConstPtr pulses,
               bool useSLC);

  int domSet_;
  I3MapKeyVectorIntConstPtr customDomSets_;

  TriggerHitVectorPtr hits_;
  TriggerHitVectorVector triggers_;
  unsigned int triggerCount_;
  unsigned int triggerIndex_;

  SET_LOGGER("TriggerService");
};

#endif
