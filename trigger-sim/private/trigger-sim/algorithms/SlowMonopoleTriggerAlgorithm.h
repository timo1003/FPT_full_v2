#ifndef I3_SLOW_MONOPOLE_TRIGGER_H
#define I3_SLOW_MONOPOLE_TRIGGER_H

#include "dataclasses/physics/I3Trigger.h"
#include "dataclasses/physics/I3Particle.h"
#include <dataclasses/geometry/I3Geometry.h>
#include "trigger-sim/algorithms/TriggerHit.h"
#include "trigger-sim/algorithms/TriggerContainer.h"
#include "trigger-sim/algorithms/TriggerService.h"

/*
 * slow monopole trigger
 */

class SlowMonopoleTriggerAlgorithm : public TriggerService
{
public:
 
    SlowMonopoleTriggerAlgorithm(double t_proximity, double t_min, double t_max, 
                                 boost::optional<double> deltad, 
                                 boost::optional<double> alpha_min,
                                 boost::optional<bool> dc_algo,
                                 double relv, 
                                 int min_tuples,
                                 double max_event_length,
                                 I3GeometryConstPtr Geometry,
                                 int domSet, I3MapKeyVectorIntConstPtr customDomSets);
    ~SlowMonopoleTriggerAlgorithm() {};

    void Trigger();
    
private:    
    int GetTrigStatusSetting(const I3TriggerStatus&, const std::string&);
    void ConfigureFromDetStatus(const I3DetectorStatus& detstatus);

    void RunTrigger(TriggerHitVector *one_hit_list__, 
		    TriggerHitVector *two_hit_list__, 
		    double *muon_time_window__,
		    TriggerHit new_hit, 
		    const I3GeometryConstPtr &geo);
    bool HLCPairCheck(TriggerHit hit1, TriggerHit hit2);
    void CheckTriggerStatus(TriggerHitVector *two_hit_list__, const I3GeometryConstPtr &geo);
    void CheckTriple(TriggerHit hit1, TriggerHit hit2, TriggerHit hit3, 
		     const I3GeometryConstPtr &geo);
    double getDistance(TriggerHit hit1, TriggerHit hit2, const I3GeometryConstPtr &geo);

    //----------------------------------
    // Variables available at instantiation
    //----------------------------------
    double t_proximity_;
    double t_min_; // time difference between two LC pairs
    double t_max_;
    boost::optional<double> deltad_;
    boost::optional<double> alpha_min_;
    boost::optional<bool> dc_algo_;
    double relv_;
    int min_tuples_;
    double max_event_length_;
    I3GeometryConstPtr geo_;

    //----------------------------------
    // Variables calculated during instantiation
    //----------------------------------
    double cos_alpha_min_;

    //----------------------------------
    // Place to store triggers 
    //----------------------------------
    TriggerContainerVector trigger_container_vector;
    std::vector<I3Trigger> trigger_list;

    SET_LOGGER("SlowMonopoleTrigger");

};	// end of class 

#endif //I3_SLOWMP_TRIGGER_NEW_H
