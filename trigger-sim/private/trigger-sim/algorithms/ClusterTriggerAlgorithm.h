/**
 * copyright  (C) 2006
 * the icecube collaboration
 * $Id: 
 *
 * @file ClusterTriggerAlgorithm.h
 * @version
 * @date
 * @author toale
 */

#ifndef CLUSTER_TRIGGER_ALGORITHM_H
#define CLUSTER_TRIGGER_ALGORITHM_H

#include "icetray/I3Logging.h"
#include "trigger-sim/algorithms/TriggerService.h"
#include "trigger-sim/algorithms/TriggerHit.h"

class ClusterTriggerAlgorithm : public TriggerService
{

 public:
  ClusterTriggerAlgorithm(double triggerWindow, unsigned int triggerThreshold,
			  unsigned int coherenceLength, 
                          int domSet, I3MapKeyVectorIntConstPtr customDomSets);
  ~ClusterTriggerAlgorithm() {};

  void Trigger();

  int GetHash(int string, unsigned int position);
  int GetString(int hash);
  unsigned int GetPosition(int hash);

 private:

  double triggerWindow_;
  unsigned int triggerThreshold_;
  unsigned int coherenceUp_;
  unsigned int coherenceDown_;

  TriggerHitList hitQueue_;

  bool PosWindow();

  SET_LOGGER("ClusterTriggerAlgorithm");
};

#endif
