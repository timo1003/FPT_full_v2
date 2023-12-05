#include <trigger-sim/algorithms/TriggerService.h>
#include <trigger-sim/algorithms/ClusterTriggerAlgorithm.h>
#include <trigger-sim/algorithms/CylinderTriggerAlgorithm.h>
#include <trigger-sim/algorithms/SimpleMajorityTriggerAlgorithm.h>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>

namespace bp=boost::python;
void register_TriggerService(){
  bp::class_<TriggerService, boost::shared_ptr<TriggerService>, boost::noncopyable>("TriggerService", bp::no_init)
    .def("FillHits", &TriggerService::FillHits, bp::args("launches", "pulses"), bp::arg("useSLC")=false)
    .def("Trigger", &TriggerService::Trigger)
    .def("GetNumberOfTriggers", &TriggerService::GetNumberOfTriggers)
    .def("GetNextTrigger", &TriggerService::GetNextTrigger)
    ;
    
  // ClusterTrigger
  bp::class_<ClusterTriggerAlgorithm, boost::shared_ptr<ClusterTriggerAlgorithm>, bp::bases<TriggerService>, boost::noncopyable >
    ("ClusterTrigger", bp::init<double, unsigned int, unsigned int, unsigned int, I3MapKeyVectorIntConstPtr>
     (bp::args("triggerWindow", "triggerThreshold", "coherenceLength", "domSet", "customDomSets")));

  // CylinderTrigger
  bp::class_<CylinderTriggerAlgorithm, boost::shared_ptr<CylinderTriggerAlgorithm>, bp::bases<TriggerService>, boost::noncopyable >
    ("CylinderTrigger", bp::init<double, unsigned int, unsigned int, I3GeometryConstPtr, 
                                 double, double, unsigned int, I3MapKeyVectorIntConstPtr>
     (bp::args("triggerWindow", "triggerThreshold", "simpleMultiplicity", "geometry", "radius", "height", "domSet", "customDomSets")));

  // SMT
  bp::class_<SimpleMajorityTriggerAlgorithm, boost::shared_ptr<SimpleMajorityTriggerAlgorithm>, bp::bases<TriggerService>, boost::noncopyable >
    ("SimpleMajorityTrigger", bp::init<double, unsigned int, unsigned int, I3MapKeyVectorIntConstPtr>
     (bp::args("triggerWindow", "triggerThreshold", "domSet", "customDomSets")));
}
