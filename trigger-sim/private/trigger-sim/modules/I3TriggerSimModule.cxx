 /**
 * class: I3GlobalTriggerSim
 *
 * Version $Id: $
 *
 * (c) 2022 IceCube Collaboration
 * @file I3GlobalTriggerSim.cxx
 * @date $Date: $
 * @author mlarson
 **/
#include <boost/foreach.hpp>
#include <trigger-sim/modules/I3TriggerSimModule.h>
#include <trigger-sim/algorithms/ClusterTriggerAlgorithm.h>
#include <trigger-sim/algorithms/CylinderTriggerAlgorithm.h>
#include <trigger-sim/algorithms/SimpleMajorityTriggerAlgorithm.h>
#include <trigger-sim/algorithms/SlowMonopoleTriggerAlgorithm.h>
#include <trigger-sim/algorithms/FaintParticleTriggerAlgorithm.h>


typedef TriggerKey::TypeID TypeID;
typedef TriggerKey::SourceID SourceID;

I3TriggerSimModule::I3TriggerSimModule(const I3Context& ctx) 
  : I3Module(ctx),
    iniceLaunches_("InIceRawData"),
    inicePulses_("InIceRawPulses"),
    icetopLaunches_("IceTopRawData"),
    outputName_("I3Triggers"),
    domsetsName_("DOMSets")
{
  AddParameter("InIceLaunches", 
               "The name of the I3DOMLaunchSeriesMap to use as a default for in-ice data"
               " when no name is included in the trigger configuration. Defaults to"
               " InIceRawData to reproduce Gen1 simulation.",
               iniceLaunches_);
  AddParameter("InIcePulses", 
               "The name of the I3RecoPulseSeriesMap to use as a default for in-ice data"
               " when no name is included in the trigger configuration. Defaults to"
               " nothing to reproduce Gen1 simulation.",
               inicePulses_);
  AddParameter("IceTopLaunches", 
               "The name of the I3DOMLaunchSeriesMap to use as a default for IceTop data"
               " when no name is included in the trigger configuration. Defaults to"
               " IceTopRawData to reproduce Gen1 simulation.",
               icetopLaunches_);
  AddParameter("OutputName", 
               "The name of the output I3TriggerHierarchy to produce. This will not yet"
               " contain a global trigger, but can be used to add additional triggers by"
               " hand if desired. If this already exists, new triggers will be appended.",
               outputName_);
  AddParameter("DOMSets",
               "The name of the DOMSets object in the frame. If this does not exist, you will"
               " need to run the InjectDefaultDOMSets function from trigger-sim before running"
               " this module. Defaults to DOMSets.",
               domsetsName_);
   AddOutBox("OutBox");
}

void I3TriggerSimModule::Configure()
{
   log_debug("Configuring the I3TriggerSimModule");
   
   GetParameter("InIceLaunches", iniceLaunches_);
   GetParameter("InIcePulses", inicePulses_);
   GetParameter("IceTopLaunches", icetopLaunches_);
   GetParameter("OutputName", outputName_);
   GetParameter("DOMSets", domsetsName_);
}


void I3TriggerSimModule::Geometry(I3FramePtr frame)
{
   log_debug("Entering I3TriggerSimModule::Geometry()");

   if(!frame->Has("I3Geometry"))
     log_fatal("No I3Geometry found in the G-frame");
   
   geometry_ = frame->Get<I3GeometryConstPtr>("I3Geometry");
   PushFrame(frame);
}

void I3TriggerSimModule::DetectorStatus(I3FramePtr frame)
{
   log_debug("Entering I3TriggerSimModule::DetectorStatus()");
   
   //---------------------------
   // Grab the domsets that we'll need for triggering
   //---------------------------
   if(!frame->Has(domsetsName_))
     log_fatal("No DOMSets with name %s in D-frame."
               " You may need to run trigger_sim.InjectDefaultDomSets",
               domsetsName_.c_str());
   domsets_ = frame->Get<I3MapKeyVectorIntConstPtr>(domsetsName_);

   //---------------------------
   // Need to copy the trigger configurations from the
   // D-frame to loop over later
   //---------------------------
   if(!frame->Has("I3DetectorStatus"))
     log_fatal("No I3DetectorStatus found in D-frame");
   I3DetectorStatusConstPtr detStat = frame->Get<I3DetectorStatusConstPtr>("I3DetectorStatus");
   triggerConfigurations_ = I3TriggerStatusMap(detStat->triggerStatus);
   PushFrame(frame);
}


void I3TriggerSimModule::DAQ(I3FramePtr frame)
{
  log_debug("Entering I3TriggerSimModule::DAQ()");

  //---------------------------
  // Are we initialized?
  //---------------------------
  if(!geometry_)
    log_fatal("No geometry found. Did you include a GCD file?");
  if(triggerConfigurations_.size() == 0)
    log_fatal("No trigger configurations found. Did you include a GCD file?");

  //---------------------------
  // Create the output hierarchy
  //---------------------------
  I3TriggerHierarchyPtr hierarchy;
  if (frame->Has(outputName_)){
    hierarchy = I3TriggerHierarchyPtr(new I3TriggerHierarchy(frame->Get<I3TriggerHierarchy>(outputName_)));
    frame->Delete(outputName_);
  }
  else 
    hierarchy = I3TriggerHierarchyPtr(new I3TriggerHierarchy());

  //---------------------------
  // Read the various sets of launches and pulses
  //---------------------------
  I3DOMLaunchSeriesMapConstPtr inice_launches;
  if(frame->Has(iniceLaunches_))
    inice_launches = frame->Get<I3DOMLaunchSeriesMapConstPtr>(iniceLaunches_);
  else
    inice_launches = I3DOMLaunchSeriesMapConstPtr(new I3DOMLaunchSeriesMap());
  
  I3DOMLaunchSeriesMapConstPtr icetop_launches;
  if(frame->Has(icetopLaunches_))
    icetop_launches = frame->Get<I3DOMLaunchSeriesMapConstPtr>(icetopLaunches_);
  else
    icetop_launches = I3DOMLaunchSeriesMapConstPtr(new I3DOMLaunchSeriesMap());
  
  I3RecoPulseSeriesMapConstPtr inice_pulses;
  if(frame->Has(inicePulses_))
    inice_pulses = frame->Get<I3RecoPulseSeriesMapConstPtr>(inicePulses_);
  else
    inice_pulses = I3RecoPulseSeriesMapConstPtr(new I3RecoPulseSeriesMap());
    
  //---------------------------
  // Start triggering.
  // Loop over all of the trigger configurations
  // in order to run the right algorithm.
  //---------------------------
  BOOST_FOREACH(auto triggerPair, triggerConfigurations_){
    TriggerKey trigger_key = triggerPair.first;
    I3TriggerStatus trigger_config = triggerPair.second;
    
    log_trace_stream("Running over trigger key " << trigger_key << std::endl;);
    
    //****************************
    // Get the domset now so that we can handle a
    // special case: old GCD files sometimes
    // didn't include a domset for every trigger...
    //****************************
    double window;
    unsigned int threshold, domset;
    if (trigger_config.settings_.count("domSet")==0)
      domset = 11;
    else
      trigger_config.GetTriggerConfigValue("domSet", domset);

    //*****************************
    // Set up the right algorithm first.
    //*****************************
    std::unique_ptr<TriggerService> service;
    switch (trigger_key.GetType()){
      case TypeID::SIMPLE_MULTIPLICITY: 
      {
        trigger_config.GetTriggerConfigValue("timeWindow", window);
        trigger_config.GetTriggerConfigValue("threshold", threshold);
        service = std::make_unique<SimpleMajorityTriggerAlgorithm>(window,
                                                                   threshold,
                                                                   domset,
                                                                   domsets_);
        break;
      }
      case TypeID::STRING:
      {
        double coherence;
        trigger_config.GetTriggerConfigValue("timeWindow", window);
        trigger_config.GetTriggerConfigValue("multiplicity", threshold);
        trigger_config.GetTriggerConfigValue("coherenceLength", coherence);
        service = std::make_unique<ClusterTriggerAlgorithm>(window,
                                                            threshold,
                                                            coherence,
                                                            domset,
                                                            domsets_);
        break;
      }
      case TypeID::VOLUME:
      {
        double radius, height;
        unsigned int multiplicity, simple_multiplicity;
        trigger_config.GetTriggerConfigValue("timeWindow", window);
        trigger_config.GetTriggerConfigValue("multiplicity", multiplicity);
        trigger_config.GetTriggerConfigValue("simpleMultiplicity", simple_multiplicity);
        trigger_config.GetTriggerConfigValue("radius", radius);
        trigger_config.GetTriggerConfigValue("height", height);
        service = std::make_unique<CylinderTriggerAlgorithm>(window, 
                                                            multiplicity,
                                                            simple_multiplicity,
                                                            geometry_,
                                                            radius, 
                                                            height,
                                                            domset, 
                                                            domsets_);
        break;
      }
      case TypeID::SLOW_PARTICLE:
      {
        int min_tuples;
        double t_proximity, t_min, t_max, relv, max_event_length;
        boost::optional<double> deltad;
        boost::optional<double> alpha_min;
        boost::optional<bool> dc_algo;
        trigger_config.GetTriggerConfigValue("t_proximity", t_proximity);
        trigger_config.GetTriggerConfigValue("t_min", t_min);
        trigger_config.GetTriggerConfigValue("t_max", t_max);
        trigger_config.GetTriggerConfigValue("rel_v", relv);
        trigger_config.GetTriggerConfigValue("min_n_tuples", min_tuples);
        trigger_config.GetTriggerConfigValue("max_event_length", max_event_length);
        trigger_config.GetTriggerConfigValue("alpha_min", alpha_min);//these are optional
        trigger_config.GetTriggerConfigValue("delta_d", deltad);//these are optional
        trigger_config.GetTriggerConfigValue("dc_algo", dc_algo);//these are optional
        service = std::make_unique<SlowMonopoleTriggerAlgorithm>(t_proximity,
                                                                 t_min,
                                                                 t_max,
                                                                 deltad,
                                                                 alpha_min,
                                                                 dc_algo,
                                                                 relv,
                                                                 min_tuples,
                                                                 max_event_length,
                                                                 geometry_,
                                                                 domset,
                                                                 domsets_);
        break;
      }
             


            
      case TypeID::FAINT_PARTICLE:
      {
        int hit_min, hit_max, double_min,azimuth_histogram_min,zenith_histogram_min;
        double time_window, time_window_separation, max_trigger_length, double_velocity_min, double_velocity_max, histogram_binning, slcfraction_min;
        trigger_config.GetTriggerConfigValue("time_window", time_window);
        trigger_config.GetTriggerConfigValue("time_window_separation", time_window_separation);
        trigger_config.GetTriggerConfigValue("max_trigger_length", max_trigger_length);
        trigger_config.GetTriggerConfigValue("hit_min", hit_min);
        trigger_config.GetTriggerConfigValue("hit_max", hit_max);
        trigger_config.GetTriggerConfigValue("double_velocity_min", double_velocity_min);
        trigger_config.GetTriggerConfigValue("double_velocity_max", double_velocity_max);
        trigger_config.GetTriggerConfigValue("double_min", double_min);
        trigger_config.GetTriggerConfigValue("azimuth_histogram_min", azimuth_histogram_min);
        trigger_config.GetTriggerConfigValue("zenith_histogram_min", zenith_histogram_min);
        trigger_config.GetTriggerConfigValue("histogram_binning", histogram_binning);
        trigger_config.GetTriggerConfigValue("slcfraction_min", slcfraction_min);
        service = std::make_unique<FaintParticleTriggerAlgorithm>(time_window,
                                                                 time_window_separation,
                                                                 max_trigger_length,
                                                                 hit_min,
                                                                 hit_max,
                                                                 double_velocity_min,
                                                                 double_velocity_max,
                                                                 double_min,
                                                                 azimuth_histogram_min,
                                                                 zenith_histogram_min,
                                                                 histogram_binning,
                                                                 slcfraction_min,                                                       
                                                                 geometry_,
                                                                 domset,
                                                                 domsets_);
        break;
      }
    default:
      log_debug_stream("Unknown trigger type " << trigger_key.GetType() << ". Ignoring...");
      continue;
    }

    //*****************************
    // Add the hits and trigger.
    //*****************************
    switch (trigger_key.GetSource()){
      case SourceID::IN_ICE:
            if (trigger_key.GetType()==TypeID::SIMPLE_MULTIPLICITY || trigger_key.GetType()==TypeID::VOLUME||trigger_key.GetType()==TypeID::STRING ||trigger_key.GetType()==TypeID::SLOW_PARTICLE)  {
                service->FillHits(inice_launches, I3RecoPulseSeriesMapConstPtr(new I3RecoPulseSeriesMap()),false);
                break;
            }
            if (trigger_key.GetType()==TypeID::FAINT_PARTICLE){
                service->FillHits(inice_launches,I3RecoPulseSeriesMapConstPtr(new I3RecoPulseSeriesMap()),true);
                break;

            }
      case SourceID::ICE_TOP:
        service->FillHits(icetop_launches, I3RecoPulseSeriesMapConstPtr(new I3RecoPulseSeriesMap()),false);
        break;
      case SourceID::IN_ICE_PULSES:
        service->FillHits(I3DOMLaunchSeriesMapConstPtr(new I3DOMLaunchSeriesMap()), inice_pulses,false);
        break;
      default:
        log_fatal_stream("Triggering for SourceID " << trigger_key.GetSource() << " is not implemented.");
        continue;
    }
    
    //*****************************
    // Run this trigger
    //*****************************
    service->Trigger();
    
    //*****************************
    // Push the results to the hierarchy
    //*****************************
    unsigned int num_triggers = service->GetNumberOfTriggers();
    log_debug_stream("Found " << num_triggers << " triggers for key " << trigger_key <<std::endl);

    for(unsigned int i=0; i<num_triggers; ++i){
      TriggerHitVectorPtr trigger_hits = service->GetNextTrigger();
      // Trigger time windows are defined by the hits in the time window.
      double start_time = trigger_hits->front().time;
      double stop_time = trigger_hits->back().time;
      
      // Create the trigger
      I3Trigger trigger;
      trigger.SetTriggerKey(trigger_key);
      trigger.SetTriggerFired(true);
      trigger.SetTriggerTime(start_time);
      trigger.SetTriggerLength(stop_time - start_time);
      hierarchy->insert(hierarchy->begin(), trigger);
    }
  }
  
  //---------------------------
  // Write the hierarchy back out.
  //---------------------------
  frame->Put(outputName_, hierarchy);
  PushFrame(frame);
}


void I3TriggerSimModule::Finish()
{
   log_debug("Entering I3TriggerSimModule::Finish()");
}

I3_MODULE(I3TriggerSimModule);
