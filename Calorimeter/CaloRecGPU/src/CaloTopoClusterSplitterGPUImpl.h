//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef CALORECGPU_CALOTOPOCLUSTERSPLITTERGPU_CUDA_H
#define CALORECGPU_CALOTOPOCLUSTERSPLITTERGPU_CUDA_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"

struct GPUSplitterTemporaries
{
  int max_cells[CaloRecGPU::NCaloCells];
  int secondary_max_cells[CaloRecGPU::NCaloCells];
  int queue1[CaloRecGPU::NCaloCells];
  int queue2[CaloRecGPU::NCaloCells];
  int tags[CaloRecGPU::NCaloCells];

  int splitter_seeds[CaloRecGPU::NMaxClusters];
  int secondary_splitter_seeds[CaloRecGPU::NMaxClusters];
  
};

struct CaloTopoClusterSplitterMetadata
{
  //int m_minSampling;
  //int m_maxSampling;
  //int m_minSecondarySampling;
  //int m_maxSecondarySampling;

  /* We can have up to 64 sample values stored in this bitfield.
   * At this moment, we have enough space to accomodate all the
   * samples but it might need to be "enlarged" in the future
   *
   * This is a bitfield and each bit on position pos tells if
   * the sample with id = pos can be used.
   */
  unsigned long long m_useSampling;
  unsigned long long m_useSecondarySampling;

  //unsigned int m_hashMin;
  //unsigned int m_hashMax;

  int m_nCells;
  int m_nOption;
  float m_minEnergy;
  float m_emShowerScale;
  bool m_shareBorderCells;
  bool m_absOpt;
  
  
  constexpr bool uses_sampling(const int sampling) const
  {
    return (m_useSampling >> sampling) & 1;
  }
  constexpr bool uses_secondary_sampling(const int sampling) const
  {
    return (m_useSecondarySampling >> sampling) & 1;
  }
};

struct GPUSplitterTemporariesHolder
{
  //Helpers::CPU_object<TopoAutomatonTemporaries> m_temporaries;

  CaloRecGPU::Helpers::CUDA_object<GPUSplitterTemporaries> m_temporaries_dev;

  void allocate();
};

struct GPUSplitterOptionsHolder
{
  CaloRecGPU::Helpers::CPU_object<CaloTopoClusterSplitterMetadata> m_options;

  CaloRecGPU::Helpers::CUDA_object<CaloTopoClusterSplitterMetadata> m_options_dev;

  void allocate();
  void sendToGPU(const bool clear_CPU = true);
};

void preProcessingPreparation(EventDataHolder & holder, GPUSplitterTemporariesHolder & temps,
                              const ConstantDataHolder & instance_data, const GPUSplitterOptionsHolder & options, const bool synchronize = false);

void findLocalMaxima(EventDataHolder & holder, GPUSplitterTemporariesHolder & temps,
                     const ConstantDataHolder & instance_data, const GPUSplitterOptionsHolder & options, const bool synchronize = false);

void propagateTags(EventDataHolder & holder, GPUSplitterTemporariesHolder & temps,
                   const ConstantDataHolder & instance_data, const GPUSplitterOptionsHolder & options, const bool synchronize = false);

void refillClusters(EventDataHolder & holder, GPUSplitterTemporariesHolder & temps,
                    const ConstantDataHolder & instance_data, const GPUSplitterOptionsHolder & options, const bool synchronize = false);

#endif //CALORECGPU_CALOTOPOCLUSTERSPLITTERGPU_CUDA_H