/**
 * copyright  (C) 2010
 * the icecube collaboration
 * $Id: 
 *
 * @file CylinderTriggerAlgorithm.h
 * @version
 * @date
 * @author danninger
 */

#ifndef CYLINDER_TRIGGER_ALGORITHM_H
#define CYLINDER_TRIGGER_ALGORITHM_H

#include "icetray/I3Logging.h"
#include "trigger-sim/algorithms/TriggerService.h"
#include "trigger-sim/algorithms/TriggerHit.h"
#include <dataclasses/geometry/I3Geometry.h>

class CylinderTriggerAlgorithm : public TriggerService
{

 public:
  CylinderTriggerAlgorithm(double triggerWindow, unsigned int triggerThreshold, unsigned int simpleMultiplicity,
                           I3GeometryConstPtr Geometry, double Radius , double Height,
                           int domSet, I3MapKeyVectorIntConstPtr customDomSets);
  ~CylinderTriggerAlgorithm() {};

  void Trigger();

 private:

  double triggerWindow_;
  unsigned int triggerThreshold_;
  unsigned int simpleMultiplicity_;
  double Radius_; 
  double Height_; 
  I3GeometryConstPtr Geometry_;
  
  TriggerHitList hitQueue_;

  bool PosWindow();

  SET_LOGGER("CylinderTriggerAlgorithm");
};

#endif
