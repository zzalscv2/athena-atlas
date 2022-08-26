/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_DATAHOLDERS_H
#define CALORECGPU_DATAHOLDERS_H

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"

class ConstantDataHolder
{
 public:

  void sendToGPU(const bool clear_CPU = true);

  CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr> m_geometry;

  CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr> m_cell_noise;

  CaloRecGPU::Helpers::CUDA_object<CaloRecGPU::GeometryArr> m_geometry_dev;

  CaloRecGPU::Helpers::CUDA_object<CaloRecGPU::CellNoiseArr> m_cell_noise_dev;

};

class EventDataHolder
{
 public:

  void sendToGPU(const bool clear_CPU = false,
                 const bool has_state = false,
                 const bool has_clusters = false,
                 const bool has_pairs = false);
  void returnToCPU(const bool clear_GPU = false,
                   const bool return_clusters = true);

  void allocate(const bool also_GPU = true);

  CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> m_cell_info;
  CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> m_cell_state;
  CaloRecGPU::Helpers::CPU_object<CaloRecGPU::PairsArr> m_pairs;
  CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> m_clusters;

  CaloRecGPU::Helpers::CUDA_object<CaloRecGPU::CellInfoArr> m_cell_info_dev;
  CaloRecGPU::Helpers::CUDA_object<CaloRecGPU::CellStateArr> m_cell_state_dev;
  CaloRecGPU::Helpers::CUDA_object<CaloRecGPU::PairsArr> m_pairs_dev;
  CaloRecGPU::Helpers::CUDA_object<CaloRecGPU::ClusterInfoArr> m_clusters_dev;

};


#endif //CALORECGPU_DATAHOLDERS_H
