/**

 * @file FaintParticleTriggerAlgorithm.cxx
 * @version
 * @date 2023
 * @author tstuerwald
 */

#include <trigger-sim/algorithms/FaintParticleTriggerAlgorithm.h>
#include <trigger-sim/algorithms/FPTTimeWindow.h>
#include <boost/foreach.hpp>
#include <boost/assign/std/vector.hpp>
#include "trigger-sim/algorithms/TriggerHit.h"
#include "dataclasses/geometry/I3Geometry.h"
#include <dataclasses/I3Direction.h>
#include <icetray/I3Units.h>
using namespace boost::assign;
  double double_velocity_min_;
  double double_velocity_max_;
  unsigned int double_min_;
  unsigned int triple_min_; 
  double slcfraction_min_;
FaintParticleTriggerAlgorithm::FaintParticleTriggerAlgorithm(double time_window,double time_window_separation, double max_trigger_length, unsigned int hit_min,unsigned int hit_max,double double_velocity_min,double double_velocity_max, unsigned int double_min,unsigned int triple_min,double slcfraction_min,  I3GeometryConstPtr Geometry,int domSet, I3MapKeyVectorIntConstPtr customDomSets) : 
  TriggerService(domSet, customDomSets),  
  time_window_(time_window),
  time_window_separation_(time_window_separation),
  max_trigger_length_(max_trigger_length),
  hit_min_(hit_min),
  hit_max_(hit_max),
  double_velocity_min_(double_velocity_min),
  double_velocity_max_(double_velocity_max),
  double_min_(double_min),
  triple_min_(triple_min), 
  slcfraction_min_(slcfraction_min),
  geo_(Geometry)
 
{

  log_debug("FaintParticleTriggerAlgorithm configuration:");
  log_debug("  Time Window = %f", time_window_);
  log_debug("  Time window separation = %f", time_window_separation_);
  log_debug("  Maximum trigger length= %f", max_trigger_length_);
  log_debug("  Minimum hit threshold = %d", hit_min_);
  log_debug("  Maximum hit threshold = %d", hit_max_);
  log_debug("  Double velocity min = %f", double_velocity_min_);
  log_debug("  Double velocity max = %f", double_velocity_max_);
  log_debug("  Minimum Double threshold = %d",double_min_);
  log_debug("  Minimum Triple threshold= %d",triple_min);
  log_debug("  Minimum SLC fraction threshold= %f",slcfraction_min_);
   
}


void FaintParticleTriggerAlgorithm::Trigger()
{
  //Necessary to assign beginHit and endHit in FPTTimeWindow
  if (hits_->size()>1){
  log_debug("Adding %zd hits to FaintParticleTigger", hits_->size());

  /*------------------------------------------------------------*
   * Check Trigger condition
   *------------------------------------------------------------*/
  
  FPTTimeWindow FPTtimeWindow(hit_min_,hit_max_,slcfraction_min_, time_window_,time_window_separation_);
  triggers_.clear();
  triggerCount_ = 0;
  triggerIndex_ = 0;

  TriggerHitIterPairVectorPtr timeWindows;
  //Keep track of the time window boundaries to avoid overlapping triggers
  std::vector<std::pair<double, double>> timeWindowRange;
  std::tie(timeWindows, timeWindowRange) = FPTtimeWindow.FPTFixedTimeWindows(hits_, time_window_separation_);
  double trigger_window_end;

  // If the vector is empty, there are no time windows for this string
  if (timeWindows->empty()) {
    log_debug("No valid time windows for this string");
    return;
  }
  log_debug("Found %zd triggered time windows", timeWindows->size());
  TriggerHitVectorPtr timeHits_current(new TriggerHitVector);
  //bool indicating that the last time window is analyzed and that there is an overlap with a previously triggered time window
  bool last_window = false;
  // Loop over the time windows and pull out the hits in each
  int timeWindowRange_ind = 0;
  for (TriggerHitIterPairVector::const_iterator timeWindowIter = timeWindows->begin(); 
       timeWindowIter != timeWindows->end(); 
       timeWindowIter++,timeWindowRange_ind++) {
    // Create a vector of the hits in this time window
    TriggerHitVectorPtr timeHits(new TriggerHitVector);
    // Get the window boundaries
    TriggerHitVector::const_iterator firstHit = timeWindowIter->first;
    TriggerHitVector::const_iterator lastHit  = timeWindowIter->second;
    
    // Iterate over the hits and pull out the ones for this window
    for (TriggerHitVector::const_iterator hitIter = firstHit; 
     hitIter != lastHit; 
     hitIter++){
      timeHits->push_back(*hitIter);
    }
    auto [startTime, endTime] = timeWindowRange[timeWindowRange_ind];
      
    //Check if previous window was above threshold
    if (timeHits_current->size()>0){
   // Only form the trigger if the current time window does not overlap with the trigger window (both defined by the time window boundaries and not the hit times), otherwise it might extend the trigger window.
         double overlap = trigger_window_end-startTime;
         if ((overlap < 0) ){             
             triggers_.push_back(*timeHits_current);
             triggerCount_++;
             log_debug("Trigger! Count = %d", triggerCount_);
             timeHits_current->clear();
            }
        //If it is the last time window and it overlaps with a previous triggered one , it can extend the trigger or the previous trigger needs to be formed
        else if (timeWindowIter == timeWindows->end() - 1){
            last_window = true;
        }
     }

    //Second cut: number of Doubles
    std::vector<int> Double_Indices = DoubleThreshold(timeHits, geo_);
    unsigned int number_doubles = Double_Indices.size()/2;
    if (number_doubles >= double_min_ ){
        
            // Third cut: Number of triple combinations
            unsigned int number_triples = TripleThreshold(timeHits,Double_Indices,geo_);
            if (number_triples > triple_min_ ){ 

                 //Check if previous window was above threshold
                 if (timeHits_current->size()>0){
                     // Set the trigger window end to the upper boundary of the current time window 
                     trigger_window_end = endTime;
                     for (auto it = timeHits->begin(); it != timeHits->end(); ++it) {
                        // Check if the current element already exists in timeHits_current
                        if (std::find(timeHits_current->begin(), timeHits_current->end(), *it) == timeHits_current->end()) {
                            // If it doesn't exist, insert it into timeHits_current
                            timeHits_current->push_back(*it);
                        }
                    }
                     if (timeHits_current->back().time> trigger_window_end){
                         log_debug("Hit in trigger window that was not yet analyzed.");}

                     //Check if the trigger length is exceeded at this point or if the last time window is reached
                     double trigger_length = timeHits_current->back().time - timeHits_current->front().time;
                     if (trigger_length > max_trigger_length_|| timeWindowIter == timeWindows->end() - 1){
                         //max trigger length exceeded -> form trigger
                         triggers_.push_back(*timeHits_current);
                         triggerCount_++;
                         log_debug("Trigger! Count = %d", triggerCount_);
                         timeHits_current->clear();
                          }                                              
                 }                             
                else {
                    
                    
                    //No previous trigger -> write hits to trigger window                  
                    for (auto it = timeHits->begin(); it != timeHits->end(); ++it) {
                            timeHits_current->push_back(*it);
                        }   
                    // Set the trigger window end to the upper boundary of the current time window 
                    trigger_window_end = endTime;
                    // Form a trigger if we are at the last time window.
                    if (timeWindowIter == timeWindows->end() - 1) {
                         triggers_.push_back(*timeHits_current);
                         triggerCount_++;
                         log_debug("Trigger! Count = %d", triggerCount_);       
                           }
                       }
                    
                }
                else {
                    //form the trigger of a previous window if the last time window is not triggered
                    if (last_window){
                        for (auto it = timeHits->begin(); it != timeHits->end(); ++it) {
                        // Check if the current element already exists in timeHits_current
                        if (std::find(timeHits_current->begin(), timeHits_current->end(), *it) == timeHits_current->end()) {
                            // If it doesn't exist, insert it into timeHits_current
                            timeHits_current->push_back(*it);
                            }
                        }
                        triggers_.push_back(*timeHits_current);
                        triggerCount_++;
                        log_debug("Trigger! Count = %d", triggerCount_);
                        timeHits_current->clear();

                        }
                    }
                }
        else {
            //form the trigger of a previous window if the last time window is not triggered
            if (last_window){
                for (auto it = timeHits->begin(); it != timeHits->end(); ++it) {
                        // Check if the current element already exists in timeHits_current
                        if (std::find(timeHits_current->begin(), timeHits_current->end(), *it) == timeHits_current->end()) {
                            // If it doesn't exist, insert it into timeHits_current
                            timeHits_current->push_back(*it);
                        }
                }
                triggers_.push_back(*timeHits_current);
                triggerCount_++;
                log_debug("Trigger! Count = %d", triggerCount_);
                timeHits_current->clear();
      }       
     }
   }
 }
}

std::vector<int> FaintParticleTriggerAlgorithm::DoubleThreshold(TriggerHitVectorPtr timeWindowHits,I3GeometryConstPtr Geometry)
{
 /*Calculate all hit pair combinations in the time window except for combinations of the same element and commutative combinations.
If the hit pair satisfies a velocity cut it is called a Double. The indices of the Doubles are returned
*/
    std::vector<int> Doubles;
    int ind_hit_1 = -1;
    
    for (TriggerHitVector::const_iterator hitIter1 = timeWindowHits->begin(); hitIter1 != timeWindowHits->end(); hitIter1++) {   
        ind_hit_1++;
        int ind_hit_2 = ind_hit_1;
            for (TriggerHitVector::const_iterator hitIter2 = hitIter1+1; hitIter2 != timeWindowHits->end(); hitIter2++) {
                ind_hit_2++;
                TriggerHit hit1 = *hitIter1;
                TriggerHit hit2 = *hitIter2;
                OMKey omkey1(hit1.string, hit1.pos);
                OMKey omkey2(hit2.string, hit2.pos);
                if (omkey1 != omkey2){

                    double distance = getDistance(*hitIter1, *hitIter2, geo_);
                    double time = abs(hit2.time - hit1.time);
                    //in km/s
                    double velocity = 1e6*distance/time;
                    if (velocity> double_velocity_min_ && velocity <double_velocity_max_){
                        Doubles.push_back(ind_hit_1);
                        Doubles.push_back(ind_hit_2);
                        }                    
                    }
                }                    
            }             
    return Doubles;
}



double FaintParticleTriggerAlgorithm::getDistance(TriggerHit hit1, TriggerHit hit2,I3GeometryConstPtr Geometry)
{
  //function from SLOP trigger to calculate the distance between two OMs

  I3OMGeoMap::const_iterator geo_iterator_1 = geo_->omgeo.find(OMKey(hit1.string, hit1.pos));
  I3OMGeoMap::const_iterator geo_iterator_2 = geo_->omgeo.find(OMKey(hit2.string, hit2.pos));
  double x1 = geo_iterator_1->second.position.GetX();
  double y1 = geo_iterator_1->second.position.GetY();
  double z1 = geo_iterator_1->second.position.GetZ();
  double x2 = geo_iterator_2->second.position.GetX();
  double y2 = geo_iterator_2->second.position.GetY();
  double z2 = geo_iterator_2->second.position.GetZ(); 
  double diff = sqrt( pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2) );
  return diff;
}


unsigned int FaintParticleTriggerAlgorithm::TripleThreshold(TriggerHitVectorPtr timeWindowHits,std::vector<int> Double_Indices,I3GeometryConstPtr Geometry)
{
    unsigned int triple_combinations =0;
    int loop_end = Double_Indices.size();
    for (int j = 0; j < loop_end-2; j+= 2) {
            for (int k = j + 2; k <= loop_end-2; k += 2) {
                //Check for two doubles that share the middle hit in time (0,1) (1,2) -> (0,1,2)
                int hit1_ind = Double_Indices[j+1];
                int hit2_ind =Double_Indices[k];
                if ( hit1_ind == hit2_ind )
                {
                    TriggerHit hit1 = (*timeWindowHits)[Double_Indices[j]];
                    TriggerHit hit2 = (*timeWindowHits)[Double_Indices[k+1]]; 
                    OMKey omkey1(hit1.string, hit1.pos);
                    OMKey omkey2(hit2.string, hit2.pos);
                    if (omkey1 != omkey2){
                        // As the doublets are velocity consistent only the third component (0-2) is checked                        
                        double distance = getDistance(hit1, hit2, geo_);
                        double time = abs(hit2.time - hit1.time);
                        //in km/s
                        double velocity = 1e6*distance/time;
                        if (velocity> double_velocity_min_ && velocity <double_velocity_max_){
                            triple_combinations++;
                            //stop calculating if threshold is exceeded (No other cut uses the triple information)
                            if (triple_combinations>triple_min_) {
                                return triple_combinations;
                                
                                }                            
                        }                     
                    }
                }
            }
     } 
    return triple_combinations;
}


