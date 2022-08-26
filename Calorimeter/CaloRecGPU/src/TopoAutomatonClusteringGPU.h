//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef CALORECGPU_TOPOAUTOMATONCLUSTERINGGPU_H
#define CALORECGPU_TOPOAUTOMATONCLUSTERINGGPU_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"
#include "CaloRecGPU/Helpers.h"

struct TopoAutomatonTemporaries
{
  CaloRecGPU::tag_type secondaryArray[CaloRecGPU::NCaloCells];
  CaloRecGPU::tag_type mergeTable[CaloRecGPU::NMaxClusters];
  int continueFlag;
};

struct TopoAutomatonOptions
{
  float seed_threshold, grow_threshold, terminal_threshold;
  bool abs_seed, abs_grow, abs_terminal;
  bool use_two_gaussian;
  unsigned int validSamplingSeed;
  //This is used to pack the bits that tell us whether a sample can be used to have seeds or not.
  constexpr bool uses_sampling(const int sampling) const
  {
    return (validSamplingSeed >> sampling) & 1;
  }
};
//Just a bundled way to pass the options for the Topo-Automaton Clustering.

struct TACTemporariesHolder
{
  //Helpers::CPU_object<TopoAutomatonTemporaries> m_temporaries;

  CaloRecGPU::Helpers::CUDA_object<TopoAutomatonTemporaries> m_temporaries_dev;
  
  void allocate();
};

struct TACOptionsHolder
{
  CaloRecGPU::Helpers::CPU_object<TopoAutomatonOptions> m_options;

  CaloRecGPU::Helpers::CUDA_object<TopoAutomatonOptions> m_options_dev;

  void allocate();
  void sendToGPU(const bool clear_CPU = true);
};

void signalToNoise(EventDataHolder & holder, TACTemporariesHolder & temps,
                   const ConstantDataHolder & instance_data, const TACOptionsHolder & options, const bool synchronize = false);

void cellPairs(EventDataHolder & holder, TACTemporariesHolder & temps,
               const ConstantDataHolder & instance_data, const TACOptionsHolder & options, const bool synchronize = false);

void clusterGrowing(EventDataHolder & holder, TACTemporariesHolder & temps,
                    const ConstantDataHolder & instance_data, const TACOptionsHolder & options, const bool synchronize = false);

#endif //CALORECGPU_TOPOAUTOMATONCLUSTERINGGPU_H