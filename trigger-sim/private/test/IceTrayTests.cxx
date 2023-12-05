#include <I3Test.h>

#include "icetray/I3Tray.h"
#include "icetray/test/ConstructorTest.h"

#include "trigger-sim/modules/I3TriggerSimModule.h"
#include "trigger-sim/modules/I3GlobalTriggerSim.h"
#include "trigger-sim/modules/I3Pruner.h"

TEST_GROUP(IceCubeTests);

#define TEST_CC(MODULE)                \
  TEST(clean_construction_##MODULE){   \
    clean_constructor_test<MODULE>();} \

TEST_CC(I3TriggerSimModule);
TEST_CC(I3GlobalTriggerSim);
TEST_CC(I3Pruner);
