/**
 * copyright  (C) 2022
 * the icecube collaboration
 * $Id: 
 *
 * @file I3TriggerSim.h
 * @version
 * @date
 * @author mlarson
 */

#ifndef I3TRIGGERSIMMODULE_H
#define I3TRIGGERSIMMODULE_H

#include <icetray/I3Context.h>
#include <icetray/I3Frame.h>
#include <icetray/I3Module.h>
#include <icetray/I3ConditionalModule.h>

#include <dataclasses/I3Map.h>
#include <dataclasses/physics/I3DOMLaunch.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/status/I3DetectorStatus.h>

#include <dataclasses/TriggerKey.h>
#include <dataclasses/physics/I3Trigger.h>
#include <dataclasses/physics/I3TriggerHierarchy.h>
#include <dataclasses/status/I3TriggerStatus.h>

#include <trigger-sim/algorithms/TriggerHit.h>
#include <trigger-sim/algorithms/TriggerService.h>

class I3TriggerSimModule : public I3Module
{
 public:
  I3TriggerSimModule(const I3Context& context);
  ~I3TriggerSimModule() {};

  void Configure();
  void Geometry(I3FramePtr frame);
  void DetectorStatus(I3FramePtr frame);
  void DAQ(I3FramePtr frame);
  void Finish();

 private:
  std::string iniceLaunches_;
  std::string inicePulses_;
  std::string icetopLaunches_;
  std::string outputName_;
  std::string domsetsName_;

  I3MapKeyVectorIntConstPtr domsets_;

  // Grab these in the Geometry() and DetectorStatus() functions
  std::map<TriggerKey, I3TriggerStatus> triggerConfigurations_;
  I3GeometryConstPtr geometry_;

  SET_LOGGER("I3TriggerSimModule");
};

#endif
