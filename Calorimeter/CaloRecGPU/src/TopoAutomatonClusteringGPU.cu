// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "TopoAutomatonClusteringGPU.h"


#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>

using namespace CaloRecGPU;

void TACTemporariesHolder::allocate()
{
  m_temporaries_dev.allocate();
}

void TACOptionsHolder::allocate()
{
  m_options.allocate();
  m_options_dev.allocate();
}

void TACOptionsHolder::sendToGPU(const bool clear_CPU)
{
  m_options_dev = m_options;
  if (clear_CPU)
    {
      m_options.clear();
    }
}

//constexpr static int DefaultBlockSize = 256;

constexpr static int SignalToNoiseBlockSize = 512;
constexpr static int CellPairsBlockSize = 64;
constexpr static int ClusterGrowingPropagationBlockSize = 64;
constexpr static int ClusterGrowingMergingBlockSize = 512;

/******************************************************************************
 * Kernel to compute the cells signal (Energy) to noise ratio, used for the
 * cells clustering step, to define the seed cells, the growing cells and the
 * border cells.
 * It also apply the energy thresholds (seed-4, grow-2, border-0)
 ******************************************************************************/


__device__ inline static tag_type calculateTag(const float SNR, const int seed_cell_index, const int address)
{
  return Tags::make_seed_tag(__float_as_int(SNR), seed_cell_index, address);
}


static __global__
void signalToNoiseKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                          Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                          Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                          Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                          const Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                          const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                          const Helpers::CUDA_kernel_object<TopoAutomatonOptions> opts)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;


  if (index < NCaloCells)
    {
      const float cellEnergy = cell_info_arr->energy[index];
      const int gain = cell_info_arr->gain[index];

      if (GainConversion::is_invalid_cell(gain))
        {
          cell_state_arr->clusterTag[index] = Tags::InvalidTag;
          temporaries->secondaryArray[index] = Tags::InvalidTag;
          return;
        }


      float sigNoiseRatio = 0.00001f;
      //It's what's done in the CPU implementation...
      if (GainConversion::is_normal_cell(gain) || GainConversion::is_invalid_seed_cell(gain))
        {
          const int corr_gain = GainConversion::recover_invalid_seed_cell_gain(gain);
          const float cellNoise = noise_arr->noise[corr_gain][index];
          if (isfinite(cellNoise) && cellNoise > 0.0f)
            {
              sigNoiseRatio = cellEnergy / cellNoise;
            }
        }

      const float absRatio = fabsf(sigNoiseRatio);

      if (sigNoiseRatio > opts->seed_threshold || (opts->abs_seed && absRatio > opts->seed_threshold)) // is Seed
        {
          if (GainConversion::is_invalid_seed_cell(gain))
            //If the cell is just ineligible to be a seed, it can still be a growing cell.
            {
              cell_state_arr->clusterTag[index] = Tags::GrowTag;
              temporaries->secondaryArray[index] = Tags::GrowTag;
            }
          else if ( !opts->uses_sampling(geometry->caloSample[index]) )
            //If the seed belongs to a sampling that's invalid for seeds...
            {
              cell_state_arr->clusterTag[index] = Tags::GrowTag;
              temporaries->secondaryArray[index] = Tags::GrowTag;
            }
          else
            {
              const int n = atomicAdd(&(clusters_arr->number), 1);

              const tag_type tag = calculateTag(absRatio, index, n);
              //Since seed_threshold will be positive,
              //no problem with using abs here always:
              //when actually using the absolute value,
              //it's what we want, when not, cells with
              //negative SNR will not be acceptable seeds.
              cell_state_arr->clusterTag[index] = tag;
              temporaries->secondaryArray[index] = tag;

              clusters_arr->seedCellID[n] = index;
              /*clusters_arr->clusterEnergy[n] = 0.f;
              clusters_arr->clusterEt[n] = 0.f;
              clusters_arr->clusterEta[n] = 0.f;
              clusters_arr->clusterPhi[n] = 0.f;*/
              //This will be zeroed out later by the property calculation tool.
              temporaries->mergeTable[n] = Tags::clear_counter(tag);
            }
        }
      else if (sigNoiseRatio > opts->grow_threshold || (opts->abs_grow && absRatio > opts->grow_threshold)) //is Grow
        {
          cell_state_arr->clusterTag[index] = Tags::GrowTag;
          temporaries->secondaryArray[index] = Tags::GrowTag;
        }
      else if (sigNoiseRatio > opts->terminal_threshold || (opts->abs_terminal && absRatio > opts->terminal_threshold)) //is Terminal
        {
          cell_state_arr->clusterTag[index] = Tags::TerminalTag;
          temporaries->secondaryArray[index] = Tags::TerminalTag;
        }
      else //is invalid for propagation
        {
          cell_state_arr->clusterTag[index] = Tags::InvalidTag;
          temporaries->secondaryArray[index] = Tags::InvalidTag;
        }
    }
}

//run the kernel
void signalToNoise(EventDataHolder & holder, TACTemporariesHolder & temps,
                   const ConstantDataHolder & instance_data, const TACOptionsHolder & options, const bool synchronize)
{
  
  cudaMemsetAsync(&(holder.m_clusters_dev->number), 0, sizeof(holder.m_clusters_dev->number), cudaStreamPerThread);
  
  const int i_dimBlock = SignalToNoiseBlockSize;
  const int i_dimGrid = Helpers::int_ceil_div(NCaloCells, i_dimBlock);

  const dim3 dimBlock(i_dimBlock, 1, 1);
  const dim3 dimGrid(i_dimGrid, 1, 1);
  signalToNoiseKernel <<< dimGrid, dimBlock>>>(holder.m_cell_state_dev, holder.m_clusters_dev, temps.m_temporaries_dev,
                                               holder.m_cell_info_dev, instance_data.m_cell_noise_dev, instance_data.m_geometry_dev,
                                               options.m_options_dev);
  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));
    }
}


/******************************************************************************
 * Kernel to generate the cell pairs for the growing algorithm.
 ******************************************************************************/


static __global__
void cellPairsKernel( Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                      const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                      const Helpers::CUDA_kernel_object<GeometryArr> geometry)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;


  if (index < NCaloCells)
    {

      const tag_type this_tag = cell_state_arr->clusterTag[index];

      if (Tags::is_growing_or_seed(this_tag))
        {
          const int num_neighs = geometry->nNeighbours[index];

          int neighbourList[NMaxNeighbours];
          int num_bad_neighs = 0;
          for (int i = 0; i < num_neighs; ++i)
            {
              const int neigh_ID = geometry->neighbours[index][i];
              const tag_type neigh_tag = cell_state_arr->clusterTag[neigh_ID];
              if (Tags::is_valid(neigh_tag))
                {
                  neighbourList[i - num_bad_neighs] = neigh_ID;
                }
              else
                {
                  ++num_bad_neighs;
                }
            }

          const int real_neighs = num_neighs - num_bad_neighs;
          const int n = atomicAdd(&(neighbour_pairs->number), real_neighs);
          for (int i = 0; i < real_neighs; ++i)
            {
              neighbour_pairs->cellID[n + i] = index;
              neighbour_pairs->neighbourID[n + i] = neighbourList[i];
            }
        }
      else if (Tags::is_terminal(this_tag))
        {
          const int num_neighs = geometry->nNeighbours[index];

          int neighbourList[NMaxNeighbours];
          int num_bad_neighs = 0;
          for (int i = 0; i < num_neighs; ++i)
            {
              const int neigh_ID = geometry->neighbours[index][i];
              const tag_type neigh_tag = cell_state_arr->clusterTag[neigh_ID];
              if (Tags::is_growing_or_seed(neigh_tag))
                {
                  neighbourList[i - num_bad_neighs] = neigh_ID;
                }
              else
                {
                  ++num_bad_neighs;
                }
            }

          const int real_neighs = num_neighs - num_bad_neighs;
          const int n = atomicAdd(&(neighbour_pairs->reverse_number), real_neighs);
          const int real_start = NMaxPairs - n - real_neighs;
          for (int i = 0; i < real_neighs; ++i)
            {
              neighbour_pairs->cellID[real_start + i] = index;
              neighbour_pairs->neighbourID[real_start + i] = neighbourList[i];
            }
        }
    }
}

//run the kernel
void cellPairs(EventDataHolder & holder, TACTemporariesHolder & temps,
               const ConstantDataHolder & instance_data, const TACOptionsHolder & options, const bool synchronize)
{
  cudaMemsetAsync(&(holder.m_pairs_dev->number), 0, sizeof(holder.m_pairs_dev->number), cudaStreamPerThread);
  cudaMemsetAsync(&(holder.m_pairs_dev->reverse_number), 0, sizeof(holder.m_pairs_dev->reverse_number), cudaStreamPerThread);

  const int i_dimBlock = CellPairsBlockSize;
  const int i_dimGrid = Helpers::int_ceil_div(NCaloCells, i_dimBlock);

  const dim3 dimBlock(i_dimBlock, 1, 1);
  const dim3 dimGrid(i_dimGrid, 1, 1);

  cellPairsKernel <<< dimGrid, dimBlock>>>(holder.m_pairs_dev, holder.m_cell_state_dev, instance_data.m_geometry_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));
    }
}



/******************************************************************************
 * Series of kernels for the growing algorithm!
 ******************************************************************************/

__global__ static
void propagateNeighbours( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                          Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                          const int pair_number,
                          const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < pair_number)
    {
      const int this_ID = neighbour_pairs->cellID[index];
      const int neigh_ID = neighbour_pairs->neighbourID[index];

      const tag_type neigh_raw_tag = cell_state_arr->clusterTag[neigh_ID];

      const tag_type neigh_prop_tag = Tags::set_for_propagation(neigh_raw_tag);

      const tag_type this_old_raw_tag = atomicMax(&(temporaries->secondaryArray[this_ID]), neigh_prop_tag);
      if (Tags::is_part_of_cluster(this_old_raw_tag) && Tags::is_part_of_cluster(neigh_raw_tag))
        {
          //If the cell was already part of a cluster,
          //we must merge the two of them.
          //Else, we keep growing.
          const int this_address = Tags::get_index_from_tag(this_old_raw_tag);
          const int neigh_address = Tags::get_index_from_tag(neigh_raw_tag);
          if (this_address != neigh_address)
            {
              const tag_type maximum_cluster = max(Tags::clear_counter(this_old_raw_tag), Tags::clear_counter(neigh_raw_tag));
              atomicMax(&(temporaries->mergeTable[this_address]), maximum_cluster);
              atomicMax(&(temporaries->mergeTable[neigh_address]), maximum_cluster);
              temporaries->continueFlag = 1;
            }
        }
      else if (Tags::is_part_of_cluster(neigh_raw_tag))
        {
          temporaries->continueFlag = 1;
        }
    }
}

__global__ static
void mergeClusters( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                    Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      const tag_type old_tag = temporaries->secondaryArray[index];
      if (Tags::is_part_of_cluster(old_tag))
        {
          const int address = Tags::get_index_from_tag(old_tag);
          const tag_type new_tag = temporaries->mergeTable[address];
          const tag_type final_tag = Tags::update_non_terminal_tag(old_tag, new_tag);
          cell_state_arr->clusterTag[index] = final_tag;
          temporaries->secondaryArray[index] = final_tag;
        }
    }
}

__global__ static
void propagateTerminals( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                         Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                         const int reverse_pair_number,
                         const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < reverse_pair_number)
    {
      const int start_index = NMaxPairs - reverse_pair_number;
      const int this_ID = neighbour_pairs->cellID[start_index + index];
      const int neigh_ID = neighbour_pairs->neighbourID[start_index + index];

      const tag_type neigh_raw_tag = cell_state_arr->clusterTag[neigh_ID];

      atomicMax(&(temporaries->secondaryArray[this_ID]), Tags::set_for_terminal_propagation(neigh_raw_tag));
    }
}

__global__ static
void finalizeClusterAttribution(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                const Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      const tag_type old_tag = temporaries->secondaryArray[index];
      cell_state_arr->clusterTag[index] = Tags::clear_counter(old_tag);
    }
}

__global__ static
void clusterGrowingKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                           Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                           const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      const int pairs_number = neighbour_pairs->number;
      const int reverse_pairs_number = neighbour_pairs->reverse_number;

      const int i_dimBlock1 = ClusterGrowingPropagationBlockSize;
      const int i_dimGrid1 = Helpers::int_ceil_div(pairs_number, i_dimBlock1);
      const dim3 dimBlock1(i_dimBlock1, 1, 1);
      const dim3 dimGrid1(i_dimGrid1, 1, 1);

      const int i_dimBlock2 = ClusterGrowingMergingBlockSize;
      const int i_dimGrid2 = Helpers::int_ceil_div(NCaloCells, i_dimBlock2);
      const dim3 dimBlock2(i_dimBlock2, 1, 1);
      const dim3 dimGrid2(i_dimGrid2, 1, 1);
      
      const int i_dimBlock3 = ClusterGrowingPropagationBlockSize;
      const int i_dimGrid3 = Helpers::int_ceil_div(reverse_pairs_number, i_dimBlock3);
      const dim3 dimBlock3(i_dimBlock3, 1, 1);
      const dim3 dimGrid3(i_dimGrid3, 1, 1);

      temporaries->continueFlag = 1;

      while (temporaries->continueFlag > 0)
        {
          temporaries->continueFlag = 0;
          propagateNeighbours <<< dimGrid1, dimBlock1>>>(cell_state_arr, temporaries, pairs_number, neighbour_pairs);
          mergeClusters <<< dimGrid2, dimBlock2>>>(cell_state_arr, temporaries);
          if (temporaries->continueFlag == 0)
            {
              cudaDeviceSynchronize();
            }
        }
      propagateTerminals <<< dimGrid3, dimBlock3>>>(cell_state_arr, temporaries, reverse_pairs_number, neighbour_pairs);
      finalizeClusterAttribution <<< dimGrid2, dimBlock2>>>(cell_state_arr, temporaries);
    }
}

//run the kernel
void clusterGrowing(EventDataHolder & holder, TACTemporariesHolder & temps,
                    const ConstantDataHolder & instance_data, const TACOptionsHolder & options, const bool synchronize)
{

  clusterGrowingKernel <<< 1, 1>>>(holder.m_cell_state_dev, temps.m_temporaries_dev, holder.m_pairs_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));
    }
}
