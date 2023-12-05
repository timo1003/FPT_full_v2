#ifndef FPT_TIME_WINDOW_H
#define FPT_TIME_WINDOW_H

#include <string>
#include "trigger-sim/algorithms/TriggerHit.h"
#include "icetray/I3Logging.h"

/**
 * @brief A simple timewindow class used by the triggers.
 */

class FPTTimeWindow
{
 public:

  /**
     * @param time_window The time window length.
     * @param time_window_separation The separation between consecutive time windows.
     * @param hit_min The minimum number of hits required for the first cut.
     * @param hit_max The maximum number of hits required for the first cut.
     * @param slcfraction_min The minimum slc fraction of the time window that is requried for the fourth cut.
     * @param customDomSets 
   */
  FPTTimeWindow(unsigned int hit_min, unsigned int hit_max, double slcfraction_min, double time_window, double time_window_separation);

  ~FPTTimeWindow() = default;


   //* Fixed time windows
 
  std::pair<TriggerHitIterPairVectorPtr, std::vector<std::pair<double, double>>> FPTFixedTimeWindows(const TriggerHitVectorPtr& hits,double separation);

 private:

  /**
   * Default constructor is private until we define defaults
   */
  FPTTimeWindow();

  unsigned int hit_min_;
  unsigned int hit_max_;
  double slcfraction_min_;
  double time_window_;
  double time_window_separation_;

  TriggerHitListPtr FPTtimeWindowHits_;
  TriggerHitListPtr FPTtriggerWindowHits_;

  SET_LOGGER("FPTTimeWindow");
};

#endif // FPT_TIME_WINDOW_H
