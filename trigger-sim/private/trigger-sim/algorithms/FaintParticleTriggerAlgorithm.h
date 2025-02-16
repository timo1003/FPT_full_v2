/**

 *
 * @file FaintParticleTriggerAlgorithm.h
 * @version
 * @date
 * @author toale
 */

#ifndef FAINT_PARTICLE_TRIGGER_ALGORITHM_H
#define FAINT_PARTICLE_TRIGGER_ALGORITHM_H

#include "icetray/I3Logging.h"
#include "trigger-sim/algorithms/TriggerHit.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "trigger-sim/algorithms/TriggerService.h"

/**
The FaintParticleTriggerAlgorithm looks for faint signatures of particles dominantly producing SLC hits and receives also SLC hits as an input. Four cuts are calculated for each time window. All cuts are described on https://wiki.icecube.wisc.edu/index.php/Faint_Particle_Trigger. Cuts 1 and 4 are calculated in the FPTTimeWindow.h class. 
The first cut is a hit count and the cuts are set to remove detector noise with the lower threshold and too bright signatures with the upper threshold. 
For the second cut velocity consistent hit pair combinations (Doubles) are calcuated to further reduce the detector noise contribution. 

For the third cut the directional consistency of Doubles is calculated, which is not expected for detector noise.For each Double the corresponding zenith and azimuth angles are calculated, binned and then the number of entries of the maximum bin for both histograms is taken and compared to a cut value.
For the last cut the amount of SLC hits in the time window over the amount of all hits is calculated. The amount of double triggered events (standard triggers and faint particle trigger) is reduced by selecting events with a high SLC fraction.    
 */

class FaintParticleTriggerAlgorithm : public TriggerService
{
/**
 * @param time_window The time window length.
 * @param time_window_separation The separation between consecutive time windows.
 * @param max_trigger_length If the max_trigger_length is exceeded the trigger is formed.
 * @param hit_min The minimum number of hits required for triggering.
 * @param hit_max The maximum number of hits required for triggering.
 * @param double_velocity_min The minimum velocity for hit pairs to count as a Double.
 * @param double_velocity_max The maximum velocity for hit pairs to count as a Double.
 * @param double_min The minimum number of Doubles required for triggering.
 * @param azimuth_histogram_min The minimum number of entries in the azimuth histogram. (DC version)
 * @param zenith_histogram_min The minimum number of entries in the zenith histogram.(DC version)
 * @param histogram_binning The binning value for the azimuth and zenith histograms.
 * @param slcfraction_min The minimum slc fraction of the time window that is requried for triggering.
 * @param Geometry Pointer to the I3Geometry
 * @param domSet The DOMSet
 * @param customDomSets 
 */

 public:
  FaintParticleTriggerAlgorithm(double time_window,double time_window_separation, double max_trigger_length,  unsigned int hit_min,unsigned int hit_max,double double_velocity_min,double double_velocity_max, unsigned int double_min,unsigned int azimuth_histogram_min,unsigned int zenith_histogram_min, double histogram_binning, double slcfraction_min, I3GeometryConstPtr Geometry,int domSet, I3MapKeyVectorIntConstPtr customDomSets);


  ~FaintParticleTriggerAlgorithm() {};

  void Trigger();


  std::vector<int> DoubleThreshold(TriggerHitVectorPtr timeWindowHits, I3GeometryConstPtr Geometry);
  double getDistance(TriggerHit hit1,TriggerHit hit2,I3GeometryConstPtr Geometry);
  std::vector<double> getDirection(TriggerHitVectorPtr timeWindowHits, std::vector<int> Double_Indices, I3GeometryConstPtr Geometry);
  std::vector<double> CalcHistogram(std::vector<double> Angles, int lower_bound, int upper_bound, int bin_size);
 private:

  double time_window_;
  double time_window_separation_;
  double max_trigger_length_;
  unsigned int hit_min_;
  unsigned int hit_max_;
  double double_velocity_min_;
  double double_velocity_max_;
  unsigned int double_min_;
  unsigned int azimuth_histogram_min_; 
  unsigned int zenith_histogram_min_;
  double histogram_binning_;
  double slcfraction_min_;
  I3GeometryConstPtr geo_;

  SET_LOGGER("FaintParticleTriggerAlgorithm");
};

#endif
