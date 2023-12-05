#include "trigger-sim/algorithms/FPTTimeWindow.h"

/*
 Structure as in TimWindow.cxx but for a fixed time window separation. Time windows are moved in increments of the time_window_separation parameter instead of sliding them.
 */
FPTTimeWindow::FPTTimeWindow(unsigned int hit_min, unsigned int hit_max, double slcfraction_min, double time_window, double time_window_separation)
    : hit_min_(hit_min), hit_max_(hit_max), slcfraction_min_(slcfraction_min), time_window_(time_window), time_window_separation_(time_window_separation)
{
    FPTtimeWindowHits_ = TriggerHitListPtr(new TriggerHitList());
    FPTtriggerWindowHits_ = TriggerHitListPtr(new TriggerHitList());
}

std::pair<TriggerHitIterPairVectorPtr, std::vector<std::pair<double, double>>> FPTTimeWindow::FPTFixedTimeWindows(const TriggerHitVectorPtr& hits, double time_window_separation){
    TriggerHitIterPairVectorPtr const FPTtriggerWindows(new TriggerHitIterPairVector());
    //Keep track of the window boundaries
    std::vector<std::pair<double, double>> timeWindows;
    //Initialize the iterator
    TriggerHitVector::const_iterator current;
    double startTime = hits->begin()->time;
    //Loop over the event for which the startTime is incremented by time_window_separation
    while (startTime < hits->back().time)
    {
        //two counts to calculate the slc fraction
        unsigned int hlc_count = 0;
        unsigned int slc_count = 0;
        //Loop over all hits and count the SLC and HLC hits in the corresponding time window [startTime,startTime+timewindow_)
        for(current = hits->begin(); (current != hits->end()) && (current->time < startTime + time_window_); ++current){
            if (current->time>=startTime){
                if (current->lc)
                {
                    hlc_count++;
                }
                else
                {
                    slc_count++;
                }
            }
        }
        //First cut on the hit count
        if (hlc_count+slc_count >= hit_min_ && hlc_count+slc_count <= hit_max_)
        {
            double const slc_fraction = static_cast<double>(slc_count) / (slc_count + hlc_count);
            //Fourth cut on the SLC fraction
            if (slc_fraction > slcfraction_min_)
            {
                TriggerHitVector::const_iterator endHit;
                endHit=current;
                TriggerHitIterPair const FPTtriggerWindow(current - (hlc_count+slc_count), endHit);
                FPTtriggerWindows->push_back(FPTtriggerWindow);
                timeWindows.emplace_back(startTime, startTime + time_window_);
            }
        }

        startTime += time_window_separation;
    }

    return std::make_pair(FPTtriggerWindows, timeWindows);
}
