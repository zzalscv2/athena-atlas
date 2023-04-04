//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "BasicGPUClusterInfoCalculatorImpl.h"


#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>

using namespace CaloRecGPU;

constexpr static int SeedCellPropertiesBlockSize = 512;

constexpr static int CalculateClusterInfoBlockSize = 320;
constexpr static int FinalizeClusterInfoBlockSize = 256;
constexpr static int ClearInvalidCellsBlockSize = 512;

#if defined(__CUDA_ARCH__) &&  __CUDA_ARCH__ > 350
  #if CUDART_VERSION >= 12000
    #define CAN_USE_TAIL_LAUNCH 1
  #else
    #define CAN_USE_TAIL_LAUNCH 0
  #endif
#elif defined(__CUDA_ARCH__)
  #error "CUDA compute capability at least 3.5 is needed so we can have dynamic parallelism!"
#endif

/**********************************************************************************/

__global__ static
void seedCellPropertiesKernel( Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                               Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temporaries,
                               const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                               const int cluster_number)
{
  const int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < cluster_number)
    {
      clusters_arr->clusterEnergy[i] = 0.f;
      clusters_arr->clusterEt[i] = 0.f;
      clusters_arr->clusterEta[i] = 0.f;
      clusters_arr->clusterPhi[i] = 0.f;
      const int seed_cell = clusters_arr->seedCellID[i];
      if (seed_cell >= 0)
        {
          temporaries->seedCellPhi[i] = geometry->phi[seed_cell];
        }
      else
        {
          temporaries->seedCellPhi[i] = 0.f;
        }
    }
}


__global__ static
void seedCellPropertiesDeferKernel( Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                    Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temporaries,
                                    const Helpers::CUDA_kernel_object<GeometryArr> geometry)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    {
      const int cluster_number = clusters_arr->number;

      const int i_dimBlock = SeedCellPropertiesBlockSize;
      const int i_dimGrid = Helpers::int_ceil_div(cluster_number, i_dimBlock);
      const dim3 dimBlock(i_dimBlock, 1, 1);
      const dim3 dimGrid(i_dimGrid, 1, 1);
#if CAN_USE_TAIL_LAUNCH
      seedCellPropertiesKernel <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(clusters_arr, temporaries, geometry, cluster_number);
#else
      seedCellPropertiesKernel <<< dimGrid, dimBlock>>>(clusters_arr, temporaries, geometry, cluster_number);
#endif
    }
}

void updateSeedCellProperties(CaloRecGPU::EventDataHolder & holder,
                              CaloRecGPU::Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temps,
                              const ConstantDataHolder & instance_data, const bool synchronize,
                              CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream != nullptr ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  seedCellPropertiesDeferKernel <<< 1, 1, 0, stream_to_use>>>(holder.m_clusters_dev, temps, instance_data.m_geometry_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}


/**********************************************************************************/

__device__  static inline
float regularize_angle(const float b, const float a)
//a. k. a. proxim in Athena code.
{
  const float diff = b - a;
  const float divi = (fabsf(diff) - Helpers::Constants::pi<float>) / (2 * Helpers::Constants::pi<float>);
  return b - ceilf(divi) * ((b > a + Helpers::Constants::pi<float>) - (b < a - Helpers::Constants::pi<float>)) * 2 * Helpers::Constants::pi<float>;
}

__global__ static
void calculateClusterInfoKernel( Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                 const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                 const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                 const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                 const Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temporaries)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      const ClusterTag tag = cell_state_arr->clusterTag[index];
      if (tag.is_part_of_cluster())
        //By this point they all have the terminals anyway, so...
        {
          if (tag.is_shared_between_clusters())
            {
              const int primary_cluster = tag.cluster_index();
              const int secondary_cluster = tag.secondary_cluster_index();

              const float secondary_weight = __int_as_float(tag.secondary_cluster_weight());
              const float weight = 1.0f - secondary_weight;

              const float energy = cell_info_arr->energy[index];
              const float abs_energy = fabsf(energy);
              const float phi_raw = geometry->phi[index];

              atomicAdd(&(clusters_arr->clusterEnergy[primary_cluster]), energy * weight);
              atomicAdd(&(clusters_arr->clusterEt[primary_cluster]), abs_energy * weight);
              atomicAdd(&(clusters_arr->clusterEta[primary_cluster]), abs_energy * geometry->eta[index] * weight);

              const float primary_phi_0 = temporaries->seedCellPhi[primary_cluster];
              const float primary_phi_real = regularize_angle(phi_raw, primary_phi_0);
              atomicAdd(&(clusters_arr->clusterPhi[primary_cluster]), primary_phi_real * abs_energy * weight);

              atomicAdd(&(clusters_arr->clusterEnergy[secondary_cluster]), energy * secondary_weight);
              atomicAdd(&(clusters_arr->clusterEt[secondary_cluster]), abs_energy * secondary_weight);
              atomicAdd(&(clusters_arr->clusterEta[secondary_cluster]), abs_energy * geometry->eta[index] * secondary_weight);

              const float secondary_phi_0 = temporaries->seedCellPhi[secondary_cluster];
              const float secondary_phi_real = regularize_angle(phi_raw, secondary_phi_0);
              atomicAdd(&(clusters_arr->clusterPhi[secondary_cluster]), secondary_phi_real * abs_energy * secondary_weight);
            }
          else
            {
              const int cluster_index = tag.cluster_index();
              const float energy = cell_info_arr->energy[index];
              const float abs_energy = fabsf(energy);
              const float phi_raw = geometry->phi[index];

              atomicAdd(&(clusters_arr->clusterEnergy[cluster_index]), energy);
              atomicAdd(&(clusters_arr->clusterEt[cluster_index]), abs_energy);
              atomicAdd(&(clusters_arr->clusterEta[cluster_index]), abs_energy * geometry->eta[index]);

              const float phi_0 = temporaries->seedCellPhi[cluster_index];
              const float phi_real = regularize_angle(phi_raw, phi_0);
              atomicAdd(&(clusters_arr->clusterPhi[cluster_index]), phi_real * abs_energy);
            }
        }
    }
}


__global__ static
void finalizeClusterInfoKernel( Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int cluster_number,
                                const bool cut_in_absolute_ET, const float ET_threshold                             )
{
  const int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < cluster_number)
    {

      const float abs_energy = clusters_arr->clusterEt[i];

      if (abs_energy > 0)
        {
          const float tempeta = clusters_arr->clusterEta[i] / abs_energy;

          clusters_arr->clusterEta[i] = tempeta;

          const float temp_ET = clusters_arr->clusterEnergy[i] / coshf(abs(tempeta));

          clusters_arr->clusterEt[i] = temp_ET;

          clusters_arr->clusterPhi[i] = regularize_angle(clusters_arr->clusterPhi[i] / abs_energy, 0.f);

          if ( !(temp_ET > ET_threshold || (cut_in_absolute_ET && fabsf(temp_ET) > ET_threshold) ) )
            {
              clusters_arr->seedCellID[i] = -1;
            }
        }
      else
        {
          clusters_arr->seedCellID[i] = -1;
          //This is just a way to signal that this is an invalid cluster.
        }
    }
}

__global__ static
void finalizeClustersDeferKernel( Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                  const bool cut_in_absolute_ET, const float ET_threshold)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    {
      const int cluster_number = clusters_arr->number;

      const int i_dimBlock = FinalizeClusterInfoBlockSize;
      const int i_dimGrid = Helpers::int_ceil_div(cluster_number, i_dimBlock);
      const dim3 dimBlock(i_dimBlock, 1, 1);
      const dim3 dimGrid(i_dimGrid, 1, 1);
#if CAN_USE_TAIL_LAUNCH
      finalizeClusterInfoKernel <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(clusters_arr, cluster_number, cut_in_absolute_ET, ET_threshold);
#else
      finalizeClusterInfoKernel <<< dimGrid, dimBlock>>>(clusters_arr, cluster_number, cut_in_absolute_ET, ET_threshold);
#endif
    }
}


__global__ static
void clearInvalidCells(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                       const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      const ClusterTag tag = cell_state_arr->clusterTag[index];
      if (tag.is_part_of_cluster())
        //By this point they all have the terminals anyway, so...
        {
          if (tag.is_shared_between_clusters())
            {
              const int first_cluster = tag.cluster_index();
              const int second_cluster = tag.secondary_cluster_index();

              const int first_seed = clusters_arr->seedCellID[first_cluster];
              const int second_seed = clusters_arr->seedCellID[second_cluster];

              if (first_seed < 0 && second_seed < 0)
                {
                  cell_state_arr->clusterTag[index] = ClusterTag:: make_invalid_tag();
                }
              else if (first_seed < 0)
                {
                  cell_state_arr->clusterTag[index] = ClusterTag::make_tag(second_cluster);
                }
              else if (second_seed < 0)
                {
                  cell_state_arr->clusterTag[index] = ClusterTag::make_tag(first_cluster);
                }
              else /*if (first_seed >= 0 && second_seed >= 0)*/
                {
                  //Do nothing: the tag's already OK.
                }
            }
          else
            {
              if (clusters_arr->seedCellID[tag.cluster_index()] < 0)
                {
                  cell_state_arr->clusterTag[index] = ClusterTag:: make_invalid_tag();
                }
            }
        }
    }
}

void calculateClusterProperties(CaloRecGPU::EventDataHolder & holder,
                                CaloRecGPU::Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temps,
                                const ConstantDataHolder & instance_data, const bool synchronize,
                                const bool cut_in_absolute_ET, const float ET_threshold,
                                CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream != nullptr ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const int i_dimBlock1 = CalculateClusterInfoBlockSize;
  const int i_dimGrid1 = Helpers::int_ceil_div(NCaloCells, i_dimBlock1);
  const dim3 dimBlock1(i_dimBlock1, 1, 1);
  const dim3 dimGrid1(i_dimGrid1, 1, 1);

  const int i_dimBlock2 = ClearInvalidCellsBlockSize;
  const int i_dimGrid2 = Helpers::int_ceil_div(NCaloCells, i_dimBlock2);
  const dim3 dimBlock2(i_dimBlock2, 1, 1);
  const dim3 dimGrid2(i_dimGrid2, 1, 1);

  calculateClusterInfoKernel <<< dimGrid1, dimBlock1, 0, stream_to_use>>>(holder.m_clusters_dev, holder.m_cell_state_dev,
                                                                          holder.m_cell_info_dev, instance_data.m_geometry_dev, temps);

  finalizeClustersDeferKernel <<< 1, 1, 0, stream_to_use>>>(holder.m_clusters_dev, cut_in_absolute_ET, ET_threshold);

  clearInvalidCells <<< dimGrid2, dimBlock2, 0, stream_to_use>>>(holder.m_cell_state_dev, holder.m_clusters_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}