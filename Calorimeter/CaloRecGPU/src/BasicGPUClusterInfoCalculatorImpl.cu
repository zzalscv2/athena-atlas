// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "BasicGPUClusterInfoCalculatorImpl.h"


#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>


using namespace CaloRecGPU;

void BasicGPUClusterInfoCalculatorTemporariesHolder::allocate()
{
  m_temporaries_dev.allocate();
}

constexpr static int SeedCellPropertiesBlockSize = 512;

constexpr static int CalculateClusterInfoBlockSize = 320;
constexpr static int FinalizeClusterInfoBlockSize = 256;

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
      temporaries->seedCellPhi[i] = geometry->phi[clusters_arr->seedCellID[i]];
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

      seedCellPropertiesKernel <<< dimGrid, dimBlock >>>(clusters_arr, temporaries, geometry, cluster_number);

    }
}

void updateSeedCellProperties(EventDataHolder & holder, BasicGPUClusterInfoCalculatorTemporariesHolder & temps,
                              const ConstantDataHolder & instance_data, const bool synchronize)
{
  seedCellPropertiesDeferKernel <<<1, 1>>>(holder.m_clusters_dev, temps.m_temporaries_dev, instance_data.m_geometry_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));
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
      const tag_type tag = cell_state_arr->clusterTag[index];
      if (Tags::is_part_of_cluster(tag))
        //By this point they all have the terminals anyway, so...
        {
          const int cluster_index = Tags::get_index_from_tag(tag);
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


__global__ static
void finalizeClusterInfoKernel( Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int cluster_number)
{
  const int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < cluster_number)
    {

      const float abs_energy = clusters_arr->clusterEt[i];

      if (abs_energy > 0)
        {
          const float tempeta = clusters_arr->clusterEta[i] / abs_energy;

          clusters_arr->clusterEta[i] = tempeta;

          clusters_arr->clusterEt[i] = clusters_arr->clusterEnergy[i] / coshf(abs(tempeta));

          clusters_arr->clusterPhi[i] = regularize_angle(clusters_arr->clusterPhi[i] / abs_energy, 0.f);
        }
      else
        {
          clusters_arr->seedCellID[i] = -1;
          //This is just a way to signal that this is an invalid cluster.
        }
    }
}

__global__ static
void finalizeClustersDeferKernel( Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    {
      const int cluster_number = clusters_arr->number;

      const int i_dimBlock = FinalizeClusterInfoBlockSize;
      const int i_dimGrid = Helpers::int_ceil_div(cluster_number, i_dimBlock);
      const dim3 dimBlock(i_dimBlock, 1, 1);
      const dim3 dimGrid(i_dimGrid, 1, 1);

      finalizeClusterInfoKernel <<< dimGrid, dimBlock >>>(clusters_arr, cluster_number);

    }
}

void calculateClusterProperties(EventDataHolder & holder, BasicGPUClusterInfoCalculatorTemporariesHolder & temps,
                                const ConstantDataHolder & instance_data, const bool synchronize)
{

  const int i_dimBlock = CalculateClusterInfoBlockSize;
  const int i_dimGrid = Helpers::int_ceil_div(NCaloCells, i_dimBlock);
  const dim3 dimBlock(i_dimBlock, 1, 1);
  const dim3 dimGrid(i_dimGrid, 1, 1);

  calculateClusterInfoKernel <<< dimGrid, dimBlock>>>(holder.m_clusters_dev, holder.m_cell_state_dev,
                                                      holder.m_cell_info_dev, instance_data.m_geometry_dev, temps.m_temporaries_dev);

  finalizeClustersDeferKernel <<< 1, 1>>>(holder.m_clusters_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));
    }
}