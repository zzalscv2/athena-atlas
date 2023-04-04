//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "TopoAutomatonClusteringImpl.h"


#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <cstddef>

using namespace CaloRecGPU;

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
constexpr static int CellPairsBlockSize = 256;
constexpr static int ClusterGrowingPropagationBlockSize = 256;
constexpr static int ClusterGrowingCopyAndCheckBlockSize = 512;
constexpr static int ClusterGrowingTerminalPropagationBlockSize = 256;
constexpr static int ClusterGrowingFinalizationBlockSize = 512;
constexpr static int ClusterGrowingSeedCellAssignmentBlockSize = 256;

#if defined(__CUDA_ARCH__) &&  __CUDA_ARCH__ > 350
  #if CUDART_VERSION >= 12000
    #define CAN_USE_TAIL_LAUNCH 1
  #else
    #define CAN_USE_TAIL_LAUNCH 0
  #endif
#elif defined(__CUDA_ARCH__)
  #error "CUDA compute capability at least 3.5 is needed so we can have dynamic parallelism!"
#endif

namespace TACHacks
//We will (ab)use the cluster info to hold
//the cell-to-cluster map and the continue flag,
//as the cluster properties aren't set here.
//Some pointer trickery, but, given that CUDA allocations
//are, in their essence, casted from void * as in C,
//this should be safe, even more since int and float are both 32-bit.
{
  //We must do this this way because there are more cells than NMaxClusters,
  //so we spill over to the next cluster properties (Phi and the first ~third of seedCellID)
  static __host__ __device__ int * get_cell_to_cluster_table_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                                     const int index)
  {
    //void * v_ptr = &(clusters_arr->clusterEt[0]);
    void * v_ptr = &(clusters_arr->clusterEta[4]);
    //The first 3 elements will possibly be taken over by
    //the seed array, and an offset of 4 feels more "natural"...
    return ((int *) v_ptr) + index;
  }

  static __device__ int get_cell_to_cluster_table_entry(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                        const int index)
  {
    return (*get_cell_to_cluster_table_address(clusters_arr, index));
  }

  static __device__ void set_cell_to_cluster_table_entry(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                         const int index, const int new_value)
  {
    (*get_cell_to_cluster_table_address(clusters_arr, index)) = new_value;
  }

  static __device__ int get_continue_flag(const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    return clusters_arr->seedCellID[NMaxClusters - 1];
  }

  static __device__ void set_continue_flag(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int value)
  {
    clusters_arr->seedCellID[NMaxClusters - 1] = value;
  }


  static __host__ __device__ int * get_continue_flag_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    return &(clusters_arr->seedCellID[NMaxClusters - 1]);
  }


  static __host__ __device__ unsigned long long int * get_seed_cell_table_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                                                  const int index)
  {
    void * base_ptr = (ClusterInfoArr *) clusters_arr;
    return ((unsigned long long int *) base_ptr ) + index + 1;
    //We have the numbers first...
    //Of course the ClusterInfoArr
    //will be 8-byte aligned,
    //thus any 8-byte offset from it will also be.
  }

  static __device__ unsigned long long int get_seed_cell_table_entry(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                                     const int index)
  {
    return (*get_seed_cell_table_address(clusters_arr, index));
  }

  static __device__ void set_seed_cell_table_entry(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                   const int index, const unsigned long long int new_value)
  {
    (*get_seed_cell_table_address(clusters_arr, index)) = new_value;
  }

#if !CAN_USE_TAIL_LAUNCH

  static __device__ int get_stop_flag(const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    return clusters_arr->seedCellID[NMaxClusters - 2];
  }

  static __device__ void set_stop_flag(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int value)
  {
    clusters_arr->seedCellID[NMaxClusters - 2] = value;
  }

  static __host__ __device__ int * get_stop_flag_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    return &(clusters_arr->seedCellID[NMaxClusters - 2]);
  }

#endif
}

/******************************************************************************
 * Kernel to compute the cells signal (Energy) to noise ratio, used for the
 * cells clustering step, to define the seed cells, the growing cells and the
 * border cells.
 * It also apply the energy thresholds (seed-4, grow-2, border-0)
 ******************************************************************************/

static __global__
void signalToNoiseKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                          Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                          Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                          const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                          const Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                          const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                          const Helpers::CUDA_kernel_object<TopoAutomatonOptions> opts)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;


  if (index < NCaloCells)
    {
      const int cell_sampling = geometry->caloSample[index];
      const float cellEnergy = cell_info_arr->energy[index];

      if (!cell_info_arr->is_valid(index) || !opts->uses_calorimeter_by_sampling(cell_sampling))
        {
          cell_state_arr->clusterTag[index] = TACTag::make_invalid_tag();
          temporaries->secondaryArray[index] = TACTag::make_invalid_tag();
          return;
        }

      float sigNoiseRatio = 0.00001f;
      //It's what's done in the CPU implementation...
      if (!cell_info_arr->is_bad(*geometry, index, opts->treat_L1_predicted_as_good))
        {
          const int gain = cell_info_arr->gain[index];

          float cellNoise = 0.f;
          if (opts->use_two_gaussian && geometry->is_tile(index))
            {
              //getTileEffSigma
            }
          else
            {
              cellNoise = noise_arr->noise[gain][index];
            }
          if (isfinite(cellNoise) && cellNoise > 0.0f)
            {
              sigNoiseRatio = cellEnergy / cellNoise;
            }
        }

      const float absRatio = fabsf(sigNoiseRatio);

      bool can_be_seed = (opts->abs_seed ? absRatio : sigNoiseRatio) > opts->seed_threshold;
      bool can_be_grow = (opts->abs_grow ? absRatio : sigNoiseRatio) > opts->grow_threshold;
      bool can_be_term = (opts->abs_terminal ? absRatio : sigNoiseRatio) > opts->terminal_threshold;

      if (can_be_seed && opts->use_time_cut && (!opts->keep_significant_cells || sigNoiseRatio <= opts->snr_threshold_for_keeping_cells))
        {
          if (!cell_info_arr->passes_time_cut(*geometry, index, opts->time_threshold))
            {
              can_be_seed = false;
              if (opts->completely_exclude_cut_seeds)
                {
                  can_be_grow = false;
                  can_be_term = false;
                }
            }
        }


      if (can_be_seed && opts->uses_seed_sampling(cell_sampling))
        {
          const int n = atomicAdd(&(clusters_arr->number), 1);

          const TACTag tag = TACTag::make_seed_tag(index, __float_as_int(absRatio), can_be_grow);
          //Since seed_threshold will be positive,
          //no problem with using abs here always:
          //when actually using the absolute value,
          //it's what we want, when not, cells with
          //negative SNR will not be acceptable seeds.
          //
          //As per the CPU algorithm,
          //if a cell does not pass the grow threshold
          //(which can happen if seeds are being evaluated
          // as absolute value while growing cells are not),
          //the clusters cannot be merged. Somehow.

          cell_state_arr->clusterTag[index] = tag;
          temporaries->secondaryArray[index] = tag;

          //TACHacks::set_merge_table_entry(clusters_arr, n, n);
          TACHacks::set_cell_to_cluster_table_entry(clusters_arr, index, n);

          unsigned long long int snr_and_cell = __float_as_uint(absRatio);

          snr_and_cell = (snr_and_cell << 32) | index;

          TACHacks::set_seed_cell_table_entry(clusters_arr, n, snr_and_cell);
        }
      else if (can_be_grow)
        {
          cell_state_arr->clusterTag[index] = TACTag::make_grow_tag();
          temporaries->secondaryArray[index] = TACTag::make_grow_tag();
        }
      else if (can_be_term)
        {
          cell_state_arr->clusterTag[index] = TACTag::make_terminal_tag();
          temporaries->secondaryArray[index] = TACTag::make_terminal_tag();
        }
      else //is invalid for propagation
        {
          cell_state_arr->clusterTag[index] = TACTag::make_invalid_tag();
          temporaries->secondaryArray[index] = TACTag::make_invalid_tag();
        }
    }
}

//run the kernel
void signalToNoise(EventDataHolder & holder, Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temps,
                   const ConstantDataHolder & instance_data, const TACOptionsHolder & options, const bool synchronize,
                   CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream != nullptr ? * ((cudaStream_t *) stream) : cudaStreamPerThread);
  cudaMemsetAsync(&(holder.m_clusters_dev->number), 0, sizeof(holder.m_clusters_dev->number), stream_to_use);

  const int i_dimBlock = SignalToNoiseBlockSize;
  const int i_dimGrid = Helpers::int_ceil_div(NCaloCells, i_dimBlock);

  const dim3 dimBlock(i_dimBlock, 1, 1);
  const dim3 dimGrid(i_dimGrid, 1, 1);
  signalToNoiseKernel <<< dimGrid, dimBlock, 0, stream_to_use>>>(holder.m_cell_state_dev, holder.m_clusters_dev, temps,
                                                                 holder.m_cell_info_dev, instance_data.m_cell_noise_dev, instance_data.m_geometry_dev,
                                                                 options.m_options_dev);
  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}


/******************************************************************************
 * Kernel to generate the cell pairs for the growing algorithm.
 ******************************************************************************/


static __global__
void cellPairsKernel( Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                      const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                      const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                      const Helpers::CUDA_kernel_object<TopoAutomatonOptions> opts)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  if (index < NCaloCells)
    {
      const TACTag this_tag = cell_state_arr->clusterTag[index];

      if (this_tag.is_grow_or_seed())
        {
          int full_neighs[NMaxNeighbours], grow_neighs[NMaxNeighbours], term_neighs[NMaxNeighbours];

          int num_grow_neighs = 0, num_term_neighs = 0;

          const int num_neighs = geometry->neighbours.get_neighbours_with_option(opts->neighbour_options, index, full_neighs,
                                                                                 opts->limit_HECIW_and_FCal_neighs, opts->limit_PS_neighs);

          for (int i = 0; i < num_neighs; ++i)
            {
              const int neigh_ID = full_neighs[i];
              const TACTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];
              if (neigh_tag.is_grow_or_seed())
                {
                  grow_neighs[num_grow_neighs] = neigh_ID;
                  ++num_grow_neighs;
                }
              else if (neigh_tag.is_non_assigned_terminal())
                {
                  term_neighs[num_term_neighs] = neigh_ID;
                  ++num_term_neighs;
                }
            }

          if (num_grow_neighs > 0)
            {
              const int n = atomicAdd(&(neighbour_pairs->number), num_grow_neighs);
              for (int i = 0; i < num_grow_neighs; ++i)
                {
                  neighbour_pairs->cellID[n + i] = grow_neighs[i];
                  neighbour_pairs->neighbourID[n + i] = index;
                }
            }
          if (num_term_neighs > 0)
            {
              const int n = atomicAdd(&(neighbour_pairs->reverse_number), num_term_neighs);
              const int real_start = NMaxPairs - n - num_term_neighs;
              for (int i = 0; i < num_term_neighs; ++i)
                {
                  neighbour_pairs->cellID[real_start + i] = term_neighs[i];
                  neighbour_pairs->neighbourID[real_start + i] = index;
                }
            }
        }
    }
}

//run the kernel
void cellPairs(EventDataHolder & holder, Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> /*temps*/,
               const ConstantDataHolder & instance_data, const TACOptionsHolder & options, const bool synchronize,
               CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream != nullptr ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  cudaMemsetAsync(&(holder.m_pairs_dev->number), 0, sizeof(holder.m_pairs_dev->number), stream_to_use);
  cudaMemsetAsync(&(holder.m_pairs_dev->reverse_number), 0, sizeof(holder.m_pairs_dev->reverse_number), stream_to_use);

  const int i_dimBlock = CellPairsBlockSize;
  const int i_dimGrid = Helpers::int_ceil_div(NCaloCells, i_dimBlock);

  const dim3 dimBlock(i_dimBlock, 1, 1);
  const dim3 dimGrid(i_dimGrid, 1, 1);

  cellPairsKernel <<< dimGrid, dimBlock, 0, stream_to_use>>>(holder.m_pairs_dev, holder.m_cell_state_dev, instance_data.m_geometry_dev, options.m_options_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}



/******************************************************************************
 * Series of kernels for the growing algorithm!
 ******************************************************************************/
__global__ static
void propagateNeighbours( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                          Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                          Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                          const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                          const int pair_number);

__global__ static
void propagateTerminals( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                         Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                         Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                         const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                         const int reverse_pair_number);

__global__ static
void copyTagsAndCheckTermination( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                  Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                                  Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                  const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs         );

__global__ static
void finalizeClusterAttribution(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                const Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                                Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr);

__global__ static
void assignSeedCells(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int clusters_number);

__global__ static
void propagateNeighbours( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                          Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                          Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                          const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                          const int pair_number)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < pair_number)
    {
      const int this_ID = neighbour_pairs->cellID[index];
      const int neigh_ID = neighbour_pairs->neighbourID[index];

      const TACTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];

      const TACTag prop_tag = neigh_tag.propagate();

      const TACTag this_old_tag = cell_state_arr->clusterTag[this_ID];

      if (this_old_tag.is_part_of_cluster() && neigh_tag.is_part_of_cluster() && this_old_tag.can_merge())
        {
          //If the cell was already part of a cluster,
          //we must merge the two of them.
          //Else, we keep growing.
          const int this_seed_idx = this_old_tag.index();
          const int neigh_seed_idx = neigh_tag.index();
          if (this_seed_idx != neigh_seed_idx)
            {
              const int this_cluster_index = TACHacks::get_cell_to_cluster_table_entry(clusters_arr, this_seed_idx);
              const int neigh_cluster_index = TACHacks::get_cell_to_cluster_table_entry(clusters_arr, neigh_seed_idx);
              if (this_cluster_index != neigh_cluster_index)
                {
                  TACHacks::set_continue_flag(clusters_arr, 1);
                  if (this_cluster_index > neigh_cluster_index)
                    {
                      atomicMax(TACHacks::get_cell_to_cluster_table_address(clusters_arr, neigh_seed_idx), this_cluster_index);
                      atomicMax( TACHacks::get_seed_cell_table_address(clusters_arr, this_cluster_index),
                                 TACHacks::get_seed_cell_table_entry(clusters_arr, neigh_cluster_index)    );
                    }
                  else /* if (neigh_cluster_index > this_cluster_index) */
                    {
                      atomicMax(TACHacks::get_cell_to_cluster_table_address(clusters_arr, this_seed_idx), neigh_cluster_index);
                      atomicMax( TACHacks::get_seed_cell_table_address(clusters_arr, neigh_cluster_index),
                                 TACHacks::get_seed_cell_table_entry(clusters_arr, this_cluster_index)    );
                    }
                }
            }
          if (prop_tag > this_old_tag)
            {
              atomicMax(&(temporaries->secondaryArray[this_ID]), prop_tag);
              TACHacks::set_continue_flag(clusters_arr, 1);
            }
        }
      else if (!this_old_tag.is_part_of_cluster() && neigh_tag.is_part_of_cluster())
        {
          TACHacks::set_continue_flag(clusters_arr, 1);
          atomicMax(&(temporaries->secondaryArray[this_ID]), prop_tag);
        }
    }
#if CAN_USE_TAIL_LAUNCH
  else if (index == pair_number)
    {
      const int i_dimBlock = ClusterGrowingCopyAndCheckBlockSize;
      const int i_dimGrid = Helpers::int_ceil_div(NCaloCells + 1, i_dimBlock);
      const dim3 dimBlock(i_dimBlock, 1, 1);
      const dim3 dimGrid(i_dimGrid, 1, 1);

      copyTagsAndCheckTermination <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(cell_state_arr, temporaries, clusters_arr, neighbour_pairs);

    }

#endif
}


__global__ static
void copyTagsAndCheckTermination( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                  Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                                  Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                  const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs  )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      cell_state_arr->clusterTag[index] = temporaries->secondaryArray[index];
    }
  else if (index == NCaloCells)
    {
#if CAN_USE_TAIL_LAUNCH
      if (TACHacks::get_continue_flag(clusters_arr))
        {
          const int pairs_number = neighbour_pairs->number;

          const int i_dimBlock = ClusterGrowingPropagationBlockSize;
          const int i_dimGrid = Helpers::int_ceil_div(pairs_number + 1, i_dimBlock);
          const dim3 dimBlock(i_dimBlock, 1, 1);
          const dim3 dimGrid(i_dimGrid, 1, 1);

          TACHacks::set_continue_flag(clusters_arr, 0);

          propagateNeighbours <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(cell_state_arr, temporaries, clusters_arr, neighbour_pairs, pairs_number);
        }
      else
        {

          const int reverse_pairs_number = neighbour_pairs->reverse_number;
          const int i_dimBlock = ClusterGrowingTerminalPropagationBlockSize;
          const int i_dimGrid = Helpers::int_ceil_div(reverse_pairs_number + 1, i_dimBlock);
          const dim3 dimBlock(i_dimBlock, 1, 1);
          const dim3 dimGrid(i_dimGrid, 1, 1);
          propagateTerminals <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(cell_state_arr, temporaries, clusters_arr, neighbour_pairs, reverse_pairs_number);

        }
#else
      if (!TACHacks::get_continue_flag(clusters_arr))
        {
          TACHacks::set_stop_flag(clusters_arr, 1);
        }
      else
        {
          TACHacks::set_continue_flag(clusters_arr, 0);
        }
#endif
    }

}

__global__ static
void propagateTerminals( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                         Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                         Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                         const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                         const int reverse_pair_number)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < reverse_pair_number)
    {
      const int start_index = NMaxPairs - reverse_pair_number;
      const int this_ID = neighbour_pairs->cellID[start_index + index];
      const int neigh_ID = neighbour_pairs->neighbourID[start_index + index];

      const TACTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];

      atomicMax(&(temporaries->secondaryArray[this_ID]), neigh_tag.propagate());
    }
#if CAN_USE_TAIL_LAUNCH
  else if (index == reverse_pair_number)
    {
      const int i_dimBlock = ClusterGrowingFinalizationBlockSize;
      const int i_dimGrid = Helpers::int_ceil_div(NCaloCells + 1, i_dimBlock);
      const dim3 dimBlock(i_dimBlock, 1, 1);
      const dim3 dimGrid(i_dimGrid, 1, 1);

      finalizeClusterAttribution <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(cell_state_arr, temporaries, clusters_arr);
    }
#endif
}

__global__ static
void finalizeClusterAttribution(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                const Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                                Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      const TACTag old_tag = temporaries->secondaryArray[index];

      if (old_tag.is_part_of_cluster())
        {
          cell_state_arr->clusterTag[index] = ClusterTag::make_tag(TACHacks::get_cell_to_cluster_table_entry(clusters_arr, old_tag.index()));
        }
      else
        {
          cell_state_arr->clusterTag[index] = ClusterTag::make_invalid_tag();
        }
    }
#if CAN_USE_TAIL_LAUNCH
  else if (index == NCaloCells)
    {
      const int num_clusters = clusters_arr->number;
      const int i_dimBlock = ClusterGrowingSeedCellAssignmentBlockSize;
      const int i_dimGrid = Helpers::int_ceil_div(clusters_arr->number, i_dimBlock);
      const dim3 dimBlock(i_dimBlock, 1, 1);
      const dim3 dimGrid(i_dimGrid, 1, 1);

      assignSeedCells <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(clusters_arr, num_clusters);
    }
#endif
}

__global__ static
void assignSeedCells(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int clusters_number)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < clusters_number)
    {
      const unsigned long long int SNR_and_cell = TACHacks::get_seed_cell_table_entry(clusters_arr, index);
      const int cell = SNR_and_cell & 0xFFFFFU;
      clusters_arr->seedCellID[index] = cell;
      //This was built in such a way there's no overlap between the parts we access...
    }
}

__global__ static
void clusterGrowingKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                           Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temporaries,
                           Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                           const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      const int pairs_number = neighbour_pairs->number;

      const int i_dimBlock1 = ClusterGrowingPropagationBlockSize;
      const int i_dimGrid1 = Helpers::int_ceil_div(pairs_number + 1, i_dimBlock1);
      const dim3 dimBlock1(i_dimBlock1, 1, 1);
      const dim3 dimGrid1(i_dimGrid1, 1, 1);

#if CAN_USE_TAIL_LAUNCH
      propagateNeighbours <<< dimGrid1, dimBlock1, 0, cudaStreamTailLaunch>>>(cell_state_arr, temporaries, clusters_arr, neighbour_pairs, pairs_number);
#else

      const int i_dimBlock2 = ClusterGrowingCopyAndCheckBlockSize;
      const int i_dimGrid2 = Helpers::int_ceil_div(NCaloCells + 1, i_dimBlock2);
      const dim3 dimBlock2(i_dimBlock2, 1, 1);
      const dim3 dimGrid2(i_dimGrid2, 1, 1);

      const int reverse_pairs_number = neighbour_pairs->reverse_number;

      const int i_dimBlock3 = ClusterGrowingTerminalPropagationBlockSize;
      const int i_dimGrid3 = Helpers::int_ceil_div(reverse_pairs_number, i_dimBlock3);
      const dim3 dimBlock3(i_dimBlock3, 1, 1);
      const dim3 dimGrid3(i_dimGrid3, 1, 1);

      const int i_dimBlock4 = ClusterGrowingFinalizationBlockSize;
      const int i_dimGrid4 = Helpers::int_ceil_div(NCaloCells, i_dimBlock4);
      const dim3 dimBlock4(i_dimBlock4, 1, 1);
      const dim3 dimGrid4(i_dimGrid4, 1, 1);

      const int i_dimBlock5 = ClusterGrowingSeedCellAssignmentBlockSize;
      const int i_dimGrid5 = Helpers::int_ceil_div(clusters_arr->number, i_dimBlock5);
      const dim3 dimBlock5(i_dimBlock5, 1, 1);
      const dim3 dimGrid5(i_dimGrid5, 1, 1);

      //int counter = 0;

      while (!TACHacks::get_stop_flag(clusters_arr))
        {
          propagateNeighbours <<< dimGrid1, dimBlock1>>>(cell_state_arr, temporaries, clusters_arr, neighbour_pairs, pairs_number);
          copyTagsAndCheckTermination <<< dimGrid2, dimBlock2>>>(cell_state_arr, temporaries, clusters_arr, neighbour_pairs);

          //++counter;

        }

      //printf("COUNTS: %16d\n", counter);

      propagateTerminals <<< dimGrid3, dimBlock3>>>(cell_state_arr, temporaries, clusters_arr, neighbour_pairs, reverse_pairs_number);
      finalizeClusterAttribution <<< dimGrid4, dimBlock4>>>(cell_state_arr, temporaries, clusters_arr);
      assignSeedCells <<< dimGrid5, dimBlock5>>>(clusters_arr, clusters_arr->number);
#endif
    }
}

//run the kernel
void clusterGrowing(EventDataHolder & holder, Helpers::CUDA_kernel_object<TopoAutomatonTemporaries> temps,
                    const ConstantDataHolder & instance_data, const TACOptionsHolder & options, const bool synchronize,
                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{

  const cudaStream_t & stream_to_use = (stream != nullptr ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  cudaMemsetAsync(TACHacks::get_continue_flag_address(holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
#if !CAN_USE_TAIL_LAUNCH
  cudaMemsetAsync(TACHacks::get_stop_flag_address(holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
#endif
  clusterGrowingKernel <<< 1, 1, 0, stream_to_use>>>(holder.m_cell_state_dev, temps, holder.m_clusters_dev, holder.m_pairs_dev);


  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}
