//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef CALORECGPU_BASICGPUCLUSTERINFOCALCULATORIMPL_H
#define CALORECGPU_BASICGPUCLUSTERINFOCALCULATORIMPL_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"

struct ClusterInfoCalculatorTemporaries
{
  float seedCellPhi[CaloRecGPU::NMaxClusters];
};

struct BasicGPUClusterInfoCalculatorTemporariesHolder
{
  //Helpers::CPU_object<TopoAutomatonTemporaries> m_temporaries;

  CaloRecGPU::Helpers::CUDA_object<ClusterInfoCalculatorTemporaries> m_temporaries_dev;

  void allocate();
};

void updateSeedCellProperties(EventDataHolder & holder, BasicGPUClusterInfoCalculatorTemporariesHolder & temps,
                              const ConstantDataHolder & instance_data, const bool synchronize = false);

void calculateClusterProperties(EventDataHolder & holder, BasicGPUClusterInfoCalculatorTemporariesHolder & temps,
                                const ConstantDataHolder & instance_data, const bool synchronize = false);

#endif //CALORECGPU_BASICGPUCLUSTERINFOCALCULATORIMPL_H