#include "GaudiKernel/DeclareFactoryEntries.h"
#include "TrigT1CTMonitoring/BSMonitoring.h"
#include "TrigT1CTMonitoring/DeriveSimulationInputs.h"

using TrigT1CTMonitoring::BSMonitoring;
using TrigT1CTMonitoring::DeriveSimulationInputs;

DECLARE_TOOL_FACTORY( BSMonitoring )
DECLARE_ALGORITHM_FACTORY( DeriveSimulationInputs )


DECLARE_FACTORY_ENTRIES( TrigT1CTMonitoring ) {
  DECLARE_ALGORITHM( DeriveSimulationInputs )
  DECLARE_ALGTOOL( BSMonitoring )
}
