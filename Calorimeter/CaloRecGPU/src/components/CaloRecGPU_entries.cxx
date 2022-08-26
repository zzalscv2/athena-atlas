//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "../CaloGPUHybridClusterProcessor.h"
#include "../BasicConstantGPUDataExporter.h"
#include "../BasicEventDataGPUExporter.h"
#include "../BasicGPUToAthenaImporter.h"
#include "../CaloGPUOutput.h"
#include "../CaloCPUOutput.h"
#include "../TopoAutomatonClustering.h"
#include "../CaloCellsCounterCPU.h"
#include "../CaloCellsCounterGPU.h"
#include "../CaloTopoClusterSplitterGPU.h"
#include "../BasicGPUClusterInfoCalculator.h"
#include "../CaloClusterDeleter.h"

// Declare the "components".
DECLARE_COMPONENT( CaloGPUHybridClusterProcessor )
DECLARE_COMPONENT( BasicConstantGPUDataExporter )
DECLARE_COMPONENT( BasicEventDataGPUExporter )
DECLARE_COMPONENT( BasicGPUToAthenaImporter )
DECLARE_COMPONENT( CaloGPUOutput )
DECLARE_COMPONENT( CaloCPUOutput )
DECLARE_COMPONENT( TopoAutomatonClustering )
DECLARE_COMPONENT( CaloCellsCounterCPU )
DECLARE_COMPONENT( CaloCellsCounterGPU )
DECLARE_COMPONENT( CaloTopoClusterSplitterGPU )
DECLARE_COMPONENT( BasicGPUClusterInfoCalculator )
DECLARE_COMPONENT( CaloClusterDeleter )


