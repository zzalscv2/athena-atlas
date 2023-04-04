//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "TopoAutomatonSplittingImpl.h"

#include "CaloIdentifier/LArNeighbours.h"
//It's just a struct.


#include "CLHEP/Units/SystemOfUnits.h"
//Probably will also work, given that it's just constexpr stuff.

#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>


using namespace CaloRecGPU;

void TASOptionsHolder::allocate()
{
  m_options.allocate();
  m_options_dev.allocate();
}

void TASOptionsHolder::sendToGPU(const bool clear_CPU)
{
  m_options_dev = m_options;
  if (clear_CPU)
    {
      m_options.clear();
    }
}

constexpr static int FillNeighboursFirstBlockSize = 128;
constexpr static int FillNeighboursSecondBlockSize = 128;

constexpr static int CountInferiorNeighsBlockSize = 256;
constexpr static int FindLocalMaximaBlockSize = 512;
constexpr static int FixClustersWithoutMaximaBlockSize = 512;

constexpr static int PrepareArrayForSecondaryMaximaBlockSize = 512;
constexpr static int ExcludeMaximaPropagationBlockSize = 256;
constexpr static int ResetAndCleanSecondariesBlockSize = 512;

constexpr static int PropagateSplitTagsBlockSize = 256;
constexpr static int HandleSplitIndexChangesBlockSize = 256;
constexpr static int HandleSplitTagChangesBlockSize = 256;

constexpr static int SumCellsBlockSize = 320;
constexpr static int CalculateCentroidsBlockSize = 512;
constexpr static int FinalizeWeightsBlockSize = 256;

//These numbers are not at all optimized,
//just going from rough similarity to TAC operations
//(which themselves are not that optimised
// since they were last tested on a previous version...)

#if defined(__CUDA_ARCH__) &&  __CUDA_ARCH__ > 350
  #if CUDART_VERSION >= 12000
    #define CAN_USE_TAIL_LAUNCH 1
  #else
    #define CAN_USE_TAIL_LAUNCH 0
  #endif
#elif defined(__CUDA_ARCH__)
  #error "CUDA compute capability at least 3.5 is needed so we can have dynamic parallelism!"
#endif

namespace TASHacks
//We will (ab)use the cluster info to hold the original cluster reference,
//shared cluster cells list and continue flag,
//as the cluster indices fit in an int (in fact, they're 16 bits)
//and the cluster properties are only overridden later.
//Some pointer trickery, but, given that CUDA allocations
//are, in their essence, casted from void * as in C,
//this should be safe, even more since int and float are both 32-bit.
{
  static __device__ int get_original_cluster_table_entry(const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                         const int index)
  {
    return __float_as_int(clusters_arr->clusterEnergy[index]);
  }

  static __device__ void set_original_cluster_table_entry(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                          const int index, const int new_value)
  {
    clusters_arr->clusterEnergy[index] = __int_as_float(new_value);
  }

  /*
  static __host__ __device__ int * get_original_cluster_table_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                                      const int index)
  {
    void * v_ptr = &(clusters_arr->clusterEnergy[index]);
    return (int *) v_ptr;
  }
  */


  static __device__ int get_continue_flag(const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    return __float_as_int(clusters_arr->clusterPhi[NMaxClusters - 1]);
  }

  static __device__ void set_continue_flag(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int value)
  {
    clusters_arr->clusterPhi[NMaxClusters - 1] = __int_as_float(value);
  }

  static __host__ __device__ int * get_continue_flag_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    void * v_ptr = &(clusters_arr->clusterPhi[NMaxClusters - 1]);
    return (int *) v_ptr;
  }


  //Extra pairs of neighbours used in checking for maxima
  //but not to actually grow the clusters.
  static __device__ int get_num_extra_neighs(const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    return __float_as_int(clusters_arr->clusterPhi[NMaxClusters - 2]);
  }

  /*
  static __device__ void set_num_extra_neighs(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int value)
  {
    clusters_arr->clusterPhi[NMaxClusters - 2] = __int_as_float(value);
  }
  */

  static __host__ __device__ int * get_num_extra_neighs_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    void * v_ptr = &(clusters_arr->clusterPhi[NMaxClusters - 2]);
    return (int *) v_ptr;
  }


  static __device__ int get_num_extra_reverse_neighs(const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    return __float_as_int(clusters_arr->clusterPhi[NMaxClusters - 3]);
  }

  /*
  static __device__ void set_num_extra_reverse_neighs(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int value)
  {
    clusters_arr->clusterPhi[NMaxClusters - 3] = __int_as_float(value);
  }
  */

  static __host__ __device__ int * get_num_extra_reverse_neighs_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    void * v_ptr = &(clusters_arr->clusterPhi[NMaxClusters - 3]);
    return (int *) v_ptr;
  }



  //We must do this this way because there are more cells than NMaxClusters,
  //so we spill over to the next cluster properties (Eta and the first ~third of Phi)
  static __host__ __device__ int * get_cell_to_cluster_table_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                                     const int index)
  {
    void * v_ptr = &(clusters_arr->clusterEt[0]);
    return ((int *) v_ptr) + index;
  }

  static __device__ int get_cell_to_cluster_table_entry(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                        const int index)
  {
    return *(get_cell_to_cluster_table_address(clusters_arr, index));
  }

  static __device__ void set_cell_to_cluster_table_entry(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                         const int index, const int new_value)
  {
    *(get_cell_to_cluster_table_address(clusters_arr, index)) = new_value;
  }

  static __device__ int get_reset_counter(const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    return __float_as_int(clusters_arr->clusterPhi[NMaxClusters - 4]);
  }

  static __device__ void set_reset_counter(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int value)
  {
    clusters_arr->clusterPhi[NMaxClusters - 4] = __int_as_float(value);
  }

  static __host__ __device__ int * get_reset_counter_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    void * v_ptr = &(clusters_arr->clusterPhi[NMaxClusters - 4]);
    return (int *) v_ptr;
  }

  //This next table is used in the secondary maxima invalidation
  //to be able to iterate in both directions simultaneously using
  //the main and secondary arrays, while storing the intermediate
  //cell assignments here in a reversible form...

  static __host__ __device__ int * get_secondary_restore_table_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                                       const int index)
  {
    void * v_ptr = &(clusters_arr->clusterEt[0]);
    return ((int *) v_ptr) + index;
  }

  static __device__ int get_secondary_restore_table_entry(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                          const int index)
  {
    return *(get_secondary_restore_table_address(clusters_arr, index));
  }

  static __device__ void set_secondary_restore_table_entry(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                                           const int index, const int new_value)
  {
    *(get_secondary_restore_table_address(clusters_arr, index)) = new_value;
  }

#if !CAN_USE_TAIL_LAUNCH

  static __device__ int get_stop_flag(const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    return __float_as_int(clusters_arr->clusterPhi[NMaxClusters - 5]);
  }

  static __device__ void set_stop_flag(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int value)
  {
    clusters_arr->clusterPhi[NMaxClusters - 5] = __int_as_float(value);
  }

  static __host__ __device__ int * get_stop_flag_address(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
  {
    void * v_ptr = &(clusters_arr->clusterPhi[NMaxClusters - 5]);
    return (int *) v_ptr;
  }
#endif

}

/******************************************************************************************
 * Determine the same-cluster neighbours of the cells and fill the pairs list accordingly.
 ******************************************************************************************/


static __global__
void fillNeighboursFirstKernel( Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  if (index < NCaloCells)
    {
      const ClusterTag this_tag = cell_state_arr->clusterTag[index];

      if (this_tag.is_part_of_cluster())
        {
          const bool is_limited = geometry->neighbours.has_limited_neighbours(index, opts->limit_HECIW_and_FCal_neighs, opts->limit_PS_neighs);
          //The cells that have limited neighbours, for the split cluster growing part.
          //WARNING: the CPU version of the code does not limit PS neighbours ever, but we give additional freedom
          //         (even if it is disabled by default).

          int neighbours[NMaxNeighbours], good_neighbours[NMaxNeighbours];

          const unsigned int limited_flags = LArNeighbours::neighbourOption::nextInSamp;

          const unsigned int neighbour_flags = LArNeighbours::neighbourOption::nextSuperCalo;
          //We will add the rest of the maxima exclusion neighbours
          //in the second fill neighbours kernel, since they must be
          //iterated over separately for everything to work properly.


          const int num_limited = geometry->neighbours.get_neighbours_with_option(limited_flags & opts->neighbour_options, index, neighbours);

          const int num_no_secondary_max = geometry->neighbours.get_neighbours_with_option( neighbour_flags & opts->neighbour_options,
                                                                                            index, &(neighbours[num_limited])           ) + num_limited;
          //The limited also belong to the secondary maximum exclusion.

          const int num_others = geometry->neighbours.get_neighbours_with_option( ( ~(neighbour_flags | limited_flags) ) & opts->neighbour_options,
                                                                                  index, &(neighbours[num_no_secondary_max])                     );

          const int num_total_neighs = num_no_secondary_max + num_others;

          int num_good_neighs = 0;

          for (int i = 0; (i < num_limited || !is_limited) && i < num_total_neighs; ++i)
            {
              const int neigh_ID = neighbours[i];
              const ClusterTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];
              if (neigh_tag.is_part_of_cluster() && this_tag.cluster_index() == neigh_tag.cluster_index())
                {
                  good_neighbours[num_good_neighs] = neigh_ID;
                  ++num_good_neighs;
                }
            }
          if (num_good_neighs > 0)
            {
              const int n = atomicAdd(&(neighbour_pairs->number), num_good_neighs);
              for (int i = 0; i < num_good_neighs; ++i)
                {
                  neighbour_pairs->cellID[n + i] = good_neighbours[i];
                  neighbour_pairs->neighbourID[n + i] = index;
                }
            }

          if (num_no_secondary_max > 0)
            //Don't forget that secondary maxima invalidation
            //occurs regardless of cluster...
            {
              const int n = atomicAdd(&(neighbour_pairs->reverse_number), num_no_secondary_max);
              const int real_start = NMaxPairs - n - num_no_secondary_max;
              for (int i = 0; i < num_no_secondary_max; ++i)
                {
                  neighbour_pairs->cellID[real_start + i] = neighbours[i];
                  neighbour_pairs->neighbourID[real_start + i] = index;
                }
            }
          temporaries->get_extra_array<int>(index) = num_good_neighs;
        }
      else
        {
          int neighbours[NMaxNeighbours];

          const unsigned int neighbour_flags = LArNeighbours::neighbourOption::nextInSamp |
                                               LArNeighbours::neighbourOption::nextSuperCalo;

          const int num_neighs = geometry->neighbours.get_neighbours_with_option(neighbour_flags & opts->neighbour_options, index, neighbours);

          if (num_neighs > 0)
            //Don't forget that secondary maxima invalidation
            //occurs regardless of cluster...
            {
              const int n = atomicAdd(&(neighbour_pairs->reverse_number), num_neighs);
              const int real_start = NMaxPairs - n - num_neighs;
              for (int i = 0; i < num_neighs; ++i)
                {
                  neighbour_pairs->cellID[real_start + i] = neighbours[i];
                  neighbour_pairs->neighbourID[real_start + i] = index;
                }
            }

          temporaries->get_extra_array<int>(index) = -NMaxNeighbours;
        }
    }
}


static __global__
void fillNeighboursSecondKernel( Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                 Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                 const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                 const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                 const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                 const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts )
//Local maxima are checked even with the neighbourhood relations
//that end up otherwise being excluded in the cells that are limited
//to `nextInSample`. We add the relevant pairs here after the end of the normal pairs
//and store their numbers on the appropriate temporary.
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  if (index < NCaloCells)
    {
      const ClusterTag this_tag = cell_state_arr->clusterTag[index];

      int neighbours[NMaxNeighbours];

      if (this_tag.is_part_of_cluster())
        {
          if ( geometry->neighbours.has_limited_neighbours(index, opts->limit_HECIW_and_FCal_neighs, opts->limit_PS_neighs) )
            //WARNING: the CPU version of the code does not limit PS neighbours ever, but we give additional freedom
            //         (even if it is disabled by default).
            {
              int good_neighbours[NMaxNeighbours];
              const unsigned int limited_flags = LArNeighbours::neighbourOption::nextInSamp;

              const int num_total_neighs = geometry->neighbours.get_neighbours_with_option((~limited_flags) & opts->neighbour_options, index, neighbours);
              int num_good_neighs = 0;

              for (int i = 0; i < num_total_neighs; ++i)
                {
                  const int neigh_ID = neighbours[i];
                  const ClusterTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];
                  if (neigh_tag.is_part_of_cluster() && this_tag.cluster_index() == neigh_tag.cluster_index())
                    {
                      good_neighbours[num_good_neighs] = neigh_ID;
                      ++num_good_neighs;
                    }
                }

              if (num_good_neighs > 0)
                {
                  const int n = atomicAdd(TASHacks::get_num_extra_neighs_address(clusters_arr), num_good_neighs);
                  const int start = neighbour_pairs->number + n;
                  for (int i = 0; i < num_good_neighs; ++i)
                    {
                      neighbour_pairs->cellID[start + i] = good_neighbours[i];
                      neighbour_pairs->neighbourID[start + i] = index;
                    }
                }
              temporaries->get_extra_array<int>(index) += num_good_neighs;
            }

        }

      const unsigned int no_max_flags = LArNeighbours::neighbourOption::prevInSamp |
                                        LArNeighbours::neighbourOption::prevSuperCalo;
      const int num_neighs = geometry->neighbours.get_neighbours_with_option(no_max_flags & opts->neighbour_options, index, neighbours);

      if (num_neighs > 0)
        //Don't forget that secondary maxima invalidation
        //occurs regardless of cluster...
        {
          const int n = atomicAdd(TASHacks::get_num_extra_reverse_neighs_address(clusters_arr), num_neighs);
          const int real_start = NMaxPairs - n - num_neighs - neighbour_pairs->reverse_number;
          for (int i = 0; i < num_neighs; ++i)
            {
              neighbour_pairs->cellID[real_start + i] = neighbours[i];
              neighbour_pairs->neighbourID[real_start + i] = index;
            }
        }
    }
}

void fillNeighbours(EventDataHolder & holder, Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temps,
                    const ConstantDataHolder & instance_data, const TASOptionsHolder & options, const bool synchronize,
                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream != nullptr ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  cudaMemsetAsync(&(holder.m_pairs_dev->number), 0, sizeof(holder.m_pairs_dev->number), stream_to_use);
  cudaMemsetAsync(&(holder.m_pairs_dev->reverse_number), 0, sizeof(holder.m_pairs_dev->reverse_number), stream_to_use);
  cudaMemsetAsync(TASHacks::get_num_extra_neighs_address(holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
  cudaMemsetAsync(TASHacks::get_num_extra_reverse_neighs_address(holder.m_clusters_dev), 0, sizeof(int), stream_to_use);

  const int i_dimBlock1 = FillNeighboursFirstBlockSize;
  const int i_dimGrid1 = Helpers::int_ceil_div(NCaloCells, i_dimBlock1);
  const dim3 dimBlock1(i_dimBlock1, 1, 1);
  const dim3 dimGrid1(i_dimGrid1, 1, 1);

  const int i_dimBlock2 = FillNeighboursSecondBlockSize;
  const int i_dimGrid2 = Helpers::int_ceil_div(NCaloCells, i_dimBlock2);
  const dim3 dimBlock2(i_dimBlock2, 1, 1);
  const dim3 dimGrid2(i_dimGrid2, 1, 1);

  fillNeighboursFirstKernel <<< dimGrid1, dimBlock1, 0, stream_to_use>>>(holder.m_pairs_dev, temps, holder.m_cell_state_dev,
                                                                         instance_data.m_geometry_dev, options.m_options_dev);

  fillNeighboursSecondKernel <<< dimGrid2, dimBlock2, 0, stream_to_use>>>(holder.m_pairs_dev, temps, holder.m_clusters_dev,
                                                                          holder.m_cell_state_dev, instance_data.m_geometry_dev, options.m_options_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}


/******************************************************************************************
 * Determine the local maxima and initialize the cell arrays appropriately.
 ******************************************************************************************/

static __global__
void countInferiorNeighsKernel( Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                const int num_normal_pairs,
                                const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  if (index < num_normal_pairs)
    {
      const int this_ID = neighbour_pairs->cellID[index];
      const int neigh_ID = neighbour_pairs->neighbourID[index];

      const int this_sampling = geometry->caloSample[this_ID];
      const int neigh_sampling = geometry->caloSample[neigh_ID];

      float this_energy = 0.f, neigh_energy = 0.f;

      if (!cell_info_arr->is_bad(*geometry, this_ID, opts->treat_L1_predicted_as_good) && opts->uses_sampling(this_sampling))
        {
          this_energy = cell_info_arr->energy[this_ID];
          if (opts->use_absolute_energy)
            {
              this_energy = fabsf(this_energy);
            }
          else if (this_energy <= 0.f)
            {
              this_energy = 0.f;
            }
        }

      if (!cell_info_arr->is_bad(*geometry, neigh_ID, opts->treat_L1_predicted_as_good) && opts->uses_sampling(neigh_sampling))
        {
          neigh_energy = cell_info_arr->energy[neigh_ID];
          if (opts->use_absolute_energy)
            {
              neigh_energy = fabsf(neigh_energy);
            }
          else if (neigh_energy <= 0.f)
            {
              neigh_energy = 0.f;
            }
        }

      bool is_max_neig = neigh_energy > this_energy;

      if (opts->uses_primary_sampling(neigh_sampling))
        {
          if (!opts->uses_primary_sampling(this_sampling) && opts->uses_secondary_sampling(this_sampling))
            {
              is_max_neig = true;
            }
        }

      if (!is_max_neig)
        {
          temporaries->get_extra_array<int>(neigh_ID) = -NCaloCells;
          //No need to count, just to invalidate!
        }
    }
}

static __global__
void countNeighsDeferrerKernel(Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                               const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                               const Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                               const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                               const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                               const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      const int full_pairs_number = neighbour_pairs->number + TASHacks::get_num_extra_neighs(clusters_arr);

      const int i_dimBlock1 = CountInferiorNeighsBlockSize;
      const int i_dimGrid1 = Helpers::int_ceil_div(full_pairs_number, i_dimBlock1);
      const dim3 dimBlock1(i_dimBlock1, 1, 1);
      const dim3 dimGrid1(i_dimGrid1, 1, 1);
#if CAN_USE_TAIL_LAUNCH
      countInferiorNeighsKernel <<< dimGrid1, dimBlock1, 0, cudaStreamTailLaunch>>>(temporaries, cell_info_arr, neighbour_pairs, full_pairs_number, geometry, opts);
#else
      countInferiorNeighsKernel <<< dimGrid1, dimBlock1>>>(temporaries, cell_info_arr, neighbour_pairs, full_pairs_number, geometry, opts);
#endif
    }
}

static __global__
void findLocalMaximaKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                            Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            const Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                            const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                            const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                            const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  if (index < NCaloCells)
    {
      const ClusterTag this_tag = cell_state_arr->clusterTag[index];

      if (this_tag.is_part_of_cluster())
        {
          const int this_sampling = geometry->caloSample[index];

          float cell_energy = 0.f;
          const float raw_cell_energy = cell_info_arr->energy[index];

          if (!cell_info_arr->is_bad(*geometry, index, opts->treat_L1_predicted_as_good) && opts->uses_sampling(this_sampling))
            {
              cell_energy = raw_cell_energy;
              if (opts->use_absolute_energy)
                {
                  cell_energy = fabsf(cell_energy);
                }
              else if (cell_energy <= 0.f)
                {
                  cell_energy = 0.f;
                }
            }

          const int num_neighs = temporaries->get_extra_array<int>(index);

          bool is_primary = false, is_maximum = false;

          if (num_neighs >= 0 && num_neighs >= opts->min_num_cells && cell_energy >= opts->min_maximum_energy)
            {
              if (opts->uses_primary_sampling(this_sampling))
                {
                  is_maximum = true;
                  is_primary = true;
                }
              else if (opts->uses_secondary_sampling(this_sampling))
                {
                  is_maximum = true;
                  is_primary = false;
                }
            }

          if (is_maximum)
            {
              const int original_cluster = this_tag.cluster_index();
              const int new_cluster = atomicAdd(&(clusters_arr->number), 1);


              const TASTag new_tag = TASTag::make_maximum_tag(new_cluster, __float_as_uint(raw_cell_energy), is_primary);

              cell_state_arr->clusterTag[index] = new_tag;

              clusters_arr->seedCellID[new_cluster] = index;

              clusters_arr->seedCellID[original_cluster] = -1;

              TASHacks::set_original_cluster_table_entry(clusters_arr, new_cluster, original_cluster);

            }
          else
            {
              const int this_cluster = this_tag.cluster_index();

              cell_state_arr->clusterTag[index] = TASTag::make_cluster_cell_tag(this_cluster);
            }
        }
      else
        {
          cell_state_arr->clusterTag[index] = TASTag::make_invalid_tag();
        }
    }
}

static __global__
void fixClustersWithoutMaximaKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                     Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                     Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;

  if (index < NCaloCells)
    {
      TASTag this_tag = cell_state_arr->clusterTag[index];

      if (this_tag.is_part_of_splitter_cluster())
        {
          temporaries->secondaryArray[index] = this_tag;
        }
      else if (this_tag.is_valid())
        //It'll be part of an original cluster, given how we've assigned the tags.
        {
          const int cluster_index = this_tag.index();

          if (clusters_arr->seedCellID[cluster_index] >= 0)
            //This means the cluster is not split.
            {
              TASTag new_tag = TASTag::make_original_cluster_tag(cluster_index);
              cell_state_arr->clusterTag[index] = new_tag;
              temporaries->secondaryArray[index] = new_tag;
              TASHacks::set_original_cluster_table_entry(clusters_arr, cluster_index, cluster_index);
            }
          else
            //This means the tag belongs to a cluster to be split
            {
              temporaries->secondaryArray[index] = this_tag;
            }
        }
      else
        {
          temporaries->secondaryArray[index] = TASTag::make_invalid_tag();
        }
    }

}

void findLocalMaxima(EventDataHolder & holder, Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temps,
                     const ConstantDataHolder & instance_data, const TASOptionsHolder & options, const bool synchronize,
                     CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream != nullptr ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  const int i_dimBlock1 = FindLocalMaximaBlockSize;
  const int i_dimGrid1 = Helpers::int_ceil_div(NCaloCells, i_dimBlock1);
  const dim3 dimBlock1(i_dimBlock1, 1, 1);
  const dim3 dimGrid1(i_dimGrid1, 1, 1);

  const int i_dimBlock2 = FixClustersWithoutMaximaBlockSize;
  const int i_dimGrid2 = Helpers::int_ceil_div(NCaloCells, i_dimBlock2);
  const dim3 dimBlock2(i_dimBlock2, 1, 1);
  const dim3 dimGrid2(i_dimGrid2, 1, 1);

  countNeighsDeferrerKernel <<< 1, 1, 0, stream_to_use>>>(temps, holder.m_cell_info_dev, holder.m_clusters_dev,
                                                          holder.m_pairs_dev, instance_data.m_geometry_dev, options.m_options_dev);


  findLocalMaximaKernel <<< dimGrid1, dimBlock1, 0, stream_to_use>>>(holder.m_cell_state_dev, holder.m_clusters_dev,
                                                                     temps, holder.m_cell_info_dev,
                                                                     instance_data.m_geometry_dev, options.m_options_dev);

  fixClustersWithoutMaximaKernel <<< dimGrid2, dimBlock2, 0, stream_to_use>>>(holder.m_cell_state_dev, temps, holder.m_clusters_dev);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}

/*****************************************************************************
 * Delete secondary maxima according to the criteria on the CPU version.
 ******************************************************************************/

namespace
{
  constexpr int primary_cluster_mark  = 0x40000000;
  constexpr int original_cluster_mark = 0x20000000;
  constexpr int part_of_cluster_mark  = 0x10000000;
  constexpr int invalid_cell_value    = ~( primary_cluster_mark  |
                                           original_cluster_mark |
                                           part_of_cluster_mark    );
}

__global__ static
void prepareArraysForSecondaryMaxima(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                     Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                     Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr                    )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      TASTag this_tag = cell_state_arr->clusterTag[index];

      if (this_tag.is_part_of_original_cluster())
        {
          TASHacks::set_secondary_restore_table_entry(clusters_arr, index, this_tag.index() | original_cluster_mark);
          this_tag = TASTag::make_invalid_tag();
          //We can propagate freely through original clusters!
        }
      else if (this_tag.is_part_of_splitter_cluster())
        {
          if (this_tag.is_secondary())
            {
              TASHacks::set_secondary_restore_table_entry(clusters_arr, index, this_tag.index());
              this_tag = this_tag.update_index(index);
              //To get the correct ordering.
            }
          else
            {
              TASHacks::set_secondary_restore_table_entry(clusters_arr, index, this_tag.index() | primary_cluster_mark);
              this_tag = TASTag::secondary_maxima_eliminator();
            }
        }
      else if (this_tag.is_non_assigned_part_of_split_cluster())
        {
          TASHacks::set_secondary_restore_table_entry(clusters_arr, index, this_tag.index() | part_of_cluster_mark);
          this_tag = TASTag::make_invalid_tag();
        }
      else
        {
          TASHacks::set_secondary_restore_table_entry(clusters_arr, index, invalid_cell_value);
          this_tag = TASTag::make_invalid_tag();
        }

      cell_state_arr->clusterTag[index] = this_tag;
      temporaries->secondaryArray[index] = this_tag;
    }
}

__global__ static
void checkForMaximaExclusionTermination( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                         Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                         Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                         const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs);

__global__ static
void propagateForMaximaExclusion( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                  Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                  Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                  const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                  const int pair_start,
                                  const int pair_switch,
                                  const int pair_number)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < pair_number)
    {
      const int this_ID = neighbour_pairs->cellID[pair_start + index];
      const int neigh_ID = neighbour_pairs->neighbourID[pair_start + index];

      tag_type * array = ( index < pair_switch ?
                           cell_state_arr->clusterTag :
                           temporaries->secondaryArray  );

      const TASTag this_tag = array[this_ID];

      if (this_tag.is_secondary_maxima_eliminator() || this_tag.is_secondary_maximum_seed())
        {
          if (atomicMax(&(array[neigh_ID]), this_tag) < this_tag)
            {
              TASHacks::set_continue_flag(clusters_arr, 1);
            }
        }
    }
#if CAN_USE_TAIL_LAUNCH
  else if (index == pair_number)
    {
      checkForMaximaExclusionTermination <<< 1, 1, 0, cudaStreamTailLaunch>>>(cell_state_arr, temporaries, clusters_arr, neighbour_pairs);
    }
#endif
}

__global__ static
void checkForMaximaExclusionTermination( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                         Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                         Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                         const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      if (TASHacks::get_continue_flag(clusters_arr))
        {
          TASHacks::set_continue_flag(clusters_arr, 0);
#if CAN_USE_TAIL_LAUNCH
          const int reverse_pairs_number = neighbour_pairs->reverse_number;
          const int reverse_pairs_start = NMaxPairs - reverse_pairs_number;

          const int extra_reverse_pairs_number = TASHacks::get_num_extra_reverse_neighs(clusters_arr);

          const int total_pairs_start = reverse_pairs_start - extra_reverse_pairs_number;

          const int total_pairs_number = reverse_pairs_number + extra_reverse_pairs_number;

          const int i_dimBlock1 = ExcludeMaximaPropagationBlockSize;
          const int i_dimGrid1 = Helpers::int_ceil_div(total_pairs_number + 1, i_dimBlock1);
          const dim3 dimBlock1(i_dimBlock1, 1, 1);
          const dim3 dimGrid1(i_dimGrid1, 1, 1);
          propagateForMaximaExclusion <<< dimGrid1, dimBlock1>>>(cell_state_arr, temporaries,
                                                                 clusters_arr, neighbour_pairs,
                                                                 total_pairs_start, extra_reverse_pairs_number,
                                                                 total_pairs_number);

#endif
        }
#if !CAN_USE_TAIL_LAUNCH
      else /*if (!TASHacks::get_continue_flag(clusters_arr))*/
        {
          TASHacks::set_stop_flag(clusters_arr, 1);
        }
#endif
    }
}

__global__ static
void excludeSecondaryMaximaKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                   Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                   Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                   const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                   const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      const int reverse_pairs_number = neighbour_pairs->reverse_number;
      const int reverse_pairs_start = NMaxPairs - reverse_pairs_number;

      const int extra_reverse_pairs_number = TASHacks::get_num_extra_reverse_neighs(clusters_arr);

      const int total_pairs_start = reverse_pairs_start - extra_reverse_pairs_number;

      const int total_pairs_number = reverse_pairs_number + extra_reverse_pairs_number;

      const int i_dimBlock1 = ExcludeMaximaPropagationBlockSize;
      const int i_dimGrid1 = Helpers::int_ceil_div(total_pairs_number + 1, i_dimBlock1);
      const dim3 dimBlock1(i_dimBlock1, 1, 1);
      const dim3 dimGrid1(i_dimGrid1, 1, 1);

#if CAN_USE_TAIL_LAUNCH
      propagateForMaximaExclusion <<< dimGrid1, dimBlock1>>>(cell_state_arr, temporaries,
                                                             clusters_arr, neighbour_pairs,
                                                             total_pairs_start, extra_reverse_pairs_number,
                                                             total_pairs_number);
#else
      while (!TASHacks::get_stop_flag(clusters_arr))
        {
          propagateForMaximaExclusion <<< dimGrid1, dimBlock1>>>(cell_state_arr, temporaries,
                                                                 clusters_arr, neighbour_pairs,
                                                                 total_pairs_start, extra_reverse_pairs_number,
                                                                 total_pairs_number);
          checkForMaximaExclusionTermination <<< 1, 1>>> (cell_state_arr, temporaries,
                                                          clusters_arr, neighbour_pairs);
          //++counter;
        }

      //printf("COUNTS: %16d\n", counter);
#endif
    }
}



__global__ static
void resetAndCleanUpSecondaries(Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      const TASTag tag_one = cell_state_arr->clusterTag[index];
      const TASTag tag_two = temporaries->secondaryArray[index];
      const int prev_state = TASHacks::get_secondary_restore_table_entry(clusters_arr, index);
      const int prev_index = prev_state & 0xFFFF;
      const float  energy  = cell_info_arr->energy[index];

      TASTag new_tag = 0;

      if (prev_state == invalid_cell_value)
        {
          new_tag = TASTag::make_invalid_tag();
          TASHacks::set_cell_to_cluster_table_entry(clusters_arr, index, -1);
        }
      else if (prev_state == prev_index)
        //is a secondary cluster.
        {
          if (tag_one.index() != index || tag_two.index() != index)
            //The tag got replaced with that of a more energetic secondary maximum
            //or a primary maximum (being a secondary maxima eliminator,
            //which also has an index larger than any cell index...)
            {
              clusters_arr->seedCellID[prev_index] = -1;
              const int old_index = TASHacks::get_original_cluster_table_entry(clusters_arr, prev_index);
              new_tag = TASTag::make_cluster_cell_tag(old_index);
              TASHacks::set_cell_to_cluster_table_entry(clusters_arr, index, old_index | (old_index << 16));
              //This actually is the same as the restore table,
              //just different functions for semantic clarity...
            }
          else
            {
              //clusters_arr->seedCellID[prev_index] = index;
              new_tag = TASTag::make_maximum_tag(index, __float_as_uint(energy), true);
              //No further distinction between primaries and secondaries.
              TASHacks::set_cell_to_cluster_table_entry(clusters_arr, index, prev_index);
            }
        }
      else if (prev_state & original_cluster_mark)
        {
          new_tag = TASTag::make_original_cluster_tag(prev_index);
          TASHacks::set_cell_to_cluster_table_entry(clusters_arr, index, prev_index | (prev_index << 16));
        }
      else if (prev_state & part_of_cluster_mark)
        {
          new_tag = TASTag::make_cluster_cell_tag(prev_index);
          TASHacks::set_cell_to_cluster_table_entry(clusters_arr, index, prev_index | (prev_index << 16));
        }
      else if (prev_state & primary_cluster_mark)
        {
          new_tag = TASTag::make_maximum_tag(index, __float_as_uint(energy), true);
          TASHacks::set_cell_to_cluster_table_entry(clusters_arr, index, prev_index);
        }
      else
        {
          new_tag = TASTag::make_invalid_tag();
          TASHacks::set_cell_to_cluster_table_entry(clusters_arr, index, -1);
        }

      cell_state_arr->clusterTag[index] = new_tag;
      temporaries->secondaryArray[index] = new_tag;
    }
}

void excludeSecondaryMaxima(EventDataHolder & holder, Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temps,
                            const ConstantDataHolder & instance_data, const TASOptionsHolder & options, const bool synchronize,
                            CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream != nullptr ? * ((cudaStream_t *) stream) : cudaStreamPerThread);
  const int i_dimBlock1 = PrepareArrayForSecondaryMaximaBlockSize;
  const int i_dimGrid1 = Helpers::int_ceil_div(NCaloCells, i_dimBlock1);
  const dim3 dimBlock1(i_dimBlock1, 1, 1);
  const dim3 dimGrid1(i_dimGrid1, 1, 1);

  const int i_dimBlock2 = ResetAndCleanSecondariesBlockSize;
  const int i_dimGrid2 = Helpers::int_ceil_div(NCaloCells, i_dimBlock2);
  const dim3 dimBlock2(i_dimBlock2, 1, 1);
  const dim3 dimGrid2(i_dimGrid2, 1, 1);

  prepareArraysForSecondaryMaxima <<< dimGrid1, dimBlock1>>>(holder.m_cell_state_dev, temps, holder.m_clusters_dev);
  if (options.m_options->valid_sampling_secondary != 0)
    {
      cudaMemsetAsync(TASHacks::get_continue_flag_address(holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
#if !CAN_USE_TAIL_LAUNCH
      cudaMemsetAsync(TASHacks::get_stop_flag_address(holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
#endif
      excludeSecondaryMaximaKernel <<< 1, 1, 0, stream_to_use>>>(holder.m_cell_state_dev, holder.m_clusters_dev, temps, holder.m_pairs_dev, holder.m_cell_info_dev);
    }
  resetAndCleanUpSecondaries <<< dimGrid2, dimBlock2, 0, stream_to_use>>>(holder.m_cell_state_dev, temps, holder.m_clusters_dev, holder.m_cell_info_dev);
  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}


/******************************************************************************************
 * Propagate the new tags and create the final clusters.
 ******************************************************************************************/
__global__ static
void handleSplitterIndexChangesKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                       Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                       Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                       const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                       const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                       const bool share_cells                                                       );

__global__ static
void handleSplitterTagChangesKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                     Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                     Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                     const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                     const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                     const bool share_cells                                                        );

__global__ static
void checkForTagPropagationTermination( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                        Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                        Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                        const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                        const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                        const bool share_cells                                                       );

__global__ static
void propagateSplitterTagsKernel( const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                  Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                  Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                  const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                  const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                  const int pair_number, const bool share_cells                                 )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < pair_number)
    {
      const int this_ID = neighbour_pairs->cellID[index];
      const int neigh_ID = neighbour_pairs->neighbourID[index];

      const TASTag neigh_tag = cell_state_arr->clusterTag[neigh_ID];

      if (!neigh_tag.is_part_of_splitter_cluster())
        {
          return;
        }

      TASTag prop_tag = neigh_tag.propagate();

      if (neigh_tag.is_shared() && !neigh_tag.is_primary() && neigh_tag.counter() > 0x7FF)
        {
          prop_tag = prop_tag.update_counter(0x7FF);
          //Shared cells after the original ones
          //are not ordered by the propagation step
          //of the original shared cell.
          //Assuming less than 2^11 = 2048 propagation steps
          //before making a shared cell seems safe-ish?
        }

      const TASTag old_tag = cell_state_arr->clusterTag[this_ID];
      if (share_cells && !neigh_tag.is_shared() && old_tag.is_part_of_splitter_cluster() && !old_tag.is_shared() && !old_tag.is_primary())
        {
          const int old_count = old_tag.counter();
          const int new_count = prop_tag.counter();
          const int old_cell = old_tag.index();
          const int new_cell = prop_tag.index();
          if (old_count == new_count && old_cell != new_cell)
            //Note that, in the CPU implementation,
            //cells are only shared if they are in the to-grow list.
            {
              const int old_index = TASHacks::get_cell_to_cluster_table_entry(clusters_arr, old_cell);
              const int new_index = TASHacks::get_cell_to_cluster_table_entry(clusters_arr, new_cell);
              if (old_index != new_index)
                {
                  prop_tag = old_tag.prepare_for_sharing(prop_tag);
                  atomicMax(TASHacks::get_reset_counter_address(clusters_arr), old_count);
                }
            }
        }

      atomicMax(&(temporaries->secondaryArray[this_ID]), prop_tag);
    }
#if CAN_USE_TAIL_LAUNCH
  else if (index == pair_number)
    {
      const int i_dimBlock = HandleSplitIndexChangesBlockSize;
      const int i_dimGrid = Helpers::int_ceil_div(NCaloCells + 1, i_dimBlock);
      const dim3 dimBlock(i_dimBlock, 1, 1);
      const dim3 dimGrid(i_dimGrid, 1, 1);

      handleSplitterIndexChangesKernel <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(cell_state_arr, temporaries, clusters_arr, cell_info_arr, neighbour_pairs, share_cells);
    }
#endif
}

__global__ static
void handleSplitterIndexChangesKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                       Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                       Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                       const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                       const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                       const bool share_cells                                                             )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      TASTag old_tag = cell_state_arr->clusterTag[index];
      TASTag new_tag = temporaries->secondaryArray[index];

      if (!new_tag.is_part_of_splitter_cluster() || new_tag.counter() == TASTag::max_counter())
        {
          return;
        }

      const uint32_t new_cluster_index = TASHacks::get_cell_to_cluster_table_entry(clusters_arr, new_tag.index());
      const int desired_counter = TASHacks::get_reset_counter(clusters_arr);

      if ( new_tag.counter() < desired_counter || (old_tag.is_part_of_splitter_cluster() && old_tag.counter() < desired_counter) )
        {
          const int original_cluster_index = TASHacks::get_original_cluster_table_entry(clusters_arr, new_cluster_index & 0xFFFFU);
          new_tag = TASTag::make_cluster_cell_tag(original_cluster_index);
          TASHacks::set_cell_to_cluster_table_entry(clusters_arr, index, original_cluster_index | (original_cluster_index << 16));
          TASHacks::set_continue_flag(clusters_arr, 1);
          cell_state_arr->clusterTag[index] = new_tag;
          temporaries->secondaryArray[index] = new_tag;
        }
      else if (!new_tag.is_primary())
        {
          TASHacks::set_cell_to_cluster_table_entry(clusters_arr, index, new_cluster_index);
        }
    }
#if CAN_USE_TAIL_LAUNCH
  else if (index == NCaloCells)
    {
      const int i_dimBlock = HandleSplitTagChangesBlockSize;
      const int i_dimGrid = Helpers::int_ceil_div(NCaloCells + 1, i_dimBlock);
      const dim3 dimBlock(i_dimBlock, 1, 1);
      const dim3 dimGrid(i_dimGrid, 1, 1);
      handleSplitterTagChangesKernel <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(cell_state_arr, temporaries, clusters_arr, cell_info_arr, neighbour_pairs, share_cells);
    }
#endif
}

__global__ static
void handleSplitterTagChangesKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                     Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                     Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                     const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                     const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                     const bool share_cells                                                             )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      TASTag old_tag = cell_state_arr->clusterTag[index];
      TASTag new_tag = temporaries->secondaryArray[index];

      if (!new_tag.is_part_of_splitter_cluster() || old_tag == new_tag)
        {
          return;
        }

      const uint32_t old_cluster_index = TASHacks::get_cell_to_cluster_table_entry(clusters_arr, old_tag.index());
      const uint32_t new_cluster_index = TASHacks::get_cell_to_cluster_table_entry(clusters_arr, new_tag.index());

      if (old_cluster_index == new_cluster_index)
        {
          const float cell_energy = cell_info_arr->energy[index];
          new_tag = new_tag.update_cell(index, __float_as_uint(cell_energy));
          if (new_tag != old_tag)
            {
              TASHacks::set_continue_flag(clusters_arr, 1);
            }
          cell_state_arr->clusterTag[index] = new_tag;
          temporaries->secondaryArray[index] = new_tag;
          return;
        }

      if ( old_tag.is_part_of_splitter_cluster() && !old_tag.is_shared() &&
           new_tag.is_shared() && new_tag.is_primary()                        )
        {
          new_tag = new_tag.update_counter(old_tag.counter() + 1);
          const int min_index = min(new_cluster_index, old_cluster_index) & 0xFFFF;
          const int max_index = max(new_cluster_index, old_cluster_index) & 0xFFFF;
          TASHacks::set_cell_to_cluster_table_entry(clusters_arr, index, (max_index << 16) | min_index);
          const float cell_energy = cell_info_arr->energy[index];
          new_tag = new_tag.update_cell(index, __float_as_uint(cell_energy));
        }
      else
        {
          const float cell_energy = cell_info_arr->energy[index];
          new_tag = new_tag.update_cell(index, __float_as_uint(cell_energy));
        }

      TASHacks::set_continue_flag(clusters_arr, 1);
      cell_state_arr->clusterTag[index] = new_tag;
      temporaries->secondaryArray[index] = new_tag;
    }
#if CAN_USE_TAIL_LAUNCH
  else if (index == NCaloCells)
    {
      checkForTagPropagationTermination <<< 1, 1, 0, cudaStreamTailLaunch>>>(cell_state_arr, temporaries, clusters_arr, cell_info_arr, neighbour_pairs, share_cells);
    }
#endif
}


__global__ static
void checkForTagPropagationTermination( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                        Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                        Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                        const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                        const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                        const bool share_cells                                                             )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      TASHacks::set_reset_counter(clusters_arr, 0);
      if (TASHacks::get_continue_flag(clusters_arr))
        {
          TASHacks::set_continue_flag(clusters_arr, 0);
#if CAN_USE_TAIL_LAUNCH
          const int pairs_number = neighbour_pairs->number;

          const int i_dimBlock = PropagateSplitTagsBlockSize;
          const int i_dimGrid = Helpers::int_ceil_div(pairs_number + 1, i_dimBlock);
          const dim3 dimBlock(i_dimBlock, 1, 1);
          const dim3 dimGrid(i_dimGrid, 1, 1);

          propagateSplitterTagsKernel <<< dimGrid, dimBlock, 0, cudaStreamTailLaunch>>>(cell_state_arr, temporaries, clusters_arr,
                                                                                        cell_info_arr, neighbour_pairs, pairs_number, share_cells);
#endif
        }
#if !CAN_USE_TAIL_LAUNCH
      else /*if (!TASHacks::get_continue_flag(clusters_arr))*/
        {
          TASHacks::set_stop_flag(clusters_arr, 1);
        }
#endif
    }
}


__global__ static
void splitClusterTagPropagationKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                       Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                                       Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                       const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                                       const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                                       const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      const int pairs_number = neighbour_pairs->number;

      const int i_dimBlock1 = PropagateSplitTagsBlockSize;
      const int i_dimGrid1 = Helpers::int_ceil_div(pairs_number + 1, i_dimBlock1);
      const dim3 dimBlock1(i_dimBlock1, 1, 1);
      const dim3 dimGrid1(i_dimGrid1, 1, 1);
#if CAN_USE_TAIL_LAUNCH

#else
      const int i_dimBlock2 = HandleSplitIndexChangesBlockSize;
      const int i_dimGrid2 = Helpers::int_ceil_div(NCaloCells, i_dimBlock2);
      const dim3 dimBlock2(i_dimBlock2, 1, 1);
      const dim3 dimGrid2(i_dimGrid2, 1, 1);

      const int i_dimBlock3 = HandleSplitTagChangesBlockSize;
      const int i_dimGrid3 = Helpers::int_ceil_div(NCaloCells, i_dimBlock3);
      const dim3 dimBlock3(i_dimBlock3, 1, 1);
      const dim3 dimGrid3(i_dimGrid3, 1, 1);

      //int counter = 0;

      const bool share_cells = opts->share_border_cells;

      while (!TASHacks::get_stop_flag(clusters_arr))
        {
          propagateSplitterTagsKernel <<< dimGrid1, dimBlock1>>>(cell_state_arr, temporaries, clusters_arr, cell_info_arr,
                                                                 neighbour_pairs, pairs_number, share_cells);
          handleSplitterIndexChangesKernel <<< dimGrid2, dimBlock2>>>(cell_state_arr, temporaries, clusters_arr, cell_info_arr, neighbour_pairs, share_cells);
          handleSplitterTagChangesKernel <<< dimGrid3, dimBlock3>>>(cell_state_arr, temporaries, clusters_arr, cell_info_arr, neighbour_pairs, share_cells);
          checkForTagPropagationTermination <<< 1, 1>>>(cell_state_arr, temporaries, clusters_arr, cell_info_arr, neighbour_pairs, share_cells);
          //++counter;
        }
      //printf("COUNTS: %16d\n", counter);
#endif
    }
}

//run the kernel
void splitClusterGrowing(EventDataHolder & holder, Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temps,
                         const ConstantDataHolder & instance_data, const TASOptionsHolder & options, const bool synchronize,
                         CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream != nullptr ? * ((cudaStream_t *) stream) : cudaStreamPerThread);
  cudaMemsetAsync(TASHacks::get_continue_flag_address(holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
  cudaMemsetAsync(TASHacks::get_reset_counter_address(holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
#if !CAN_USE_TAIL_LAUNCH
  cudaMemsetAsync(TASHacks::get_stop_flag_address(holder.m_clusters_dev), 0, sizeof(int), stream_to_use);
#endif
  splitClusterTagPropagationKernel <<< 1, 1, 0, stream_to_use>>>(holder.m_cell_state_dev, temps, holder.m_clusters_dev,
                                                                 holder.m_cell_info_dev, holder.m_pairs_dev, options.m_options_dev);
  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}

/******************************************************************************************
 * Calculate the cell weights (only if indeed using shared_cells).
 ******************************************************************************************/

namespace
{
  constexpr int clusterprop_abs_E = 0;
  constexpr int clusterprop_E = 1;
  constexpr int clusterprop_x = 2;
  constexpr int clusterprop_y = 3;
  constexpr int clusterprop_z = 4;
}


__global__ static
void sumCellsForCentroid( Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                          Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                          const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                          const Helpers::CUDA_kernel_object<CellInfoArr> cell_info_arr,
                          const Helpers::CUDA_kernel_object<GeometryArr> geometry)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      const TASTag tag = cell_state_arr->clusterTag[index];
      if (tag.is_part_of_splitter_cluster() && !tag.is_shared())
        {
          const int cluster = TASHacks::get_cell_to_cluster_table_entry(clusters_arr, tag.index()) & 0xFFFF;

          const float energy = cell_info_arr->energy[index];
          const float abs_energy = fabsf(energy);
          const float x = geometry->x[index];
          const float y = geometry->y[index];
          const float z = geometry->z[index];

          atomicAdd( &( temporaries->get_cluster_property_aux_array<clusterprop_abs_E>(cluster) ), abs_energy    );
          atomicAdd( &( temporaries->get_cluster_property_aux_array<clusterprop_E>(cluster) ), energy        );

          atomicAdd( &( temporaries->get_cluster_property_aux_array<clusterprop_x>(cluster) ), x * abs_energy );
          atomicAdd( &( temporaries->get_cluster_property_aux_array<clusterprop_y>(cluster) ), y * abs_energy );
          atomicAdd( &( temporaries->get_cluster_property_aux_array<clusterprop_z>(cluster) ), z * abs_energy );

        }
    }
}

__global__ static
void calculateCentroids(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                        Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                        const int cluster_number)
{
  const int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < cluster_number)
    {

      const float abs_energy = temporaries->get_cluster_property_aux_array<clusterprop_abs_E>(i);

      if (abs_energy > 0)
        {
          temporaries->get_cluster_property_aux_array<clusterprop_x>(i) /= abs_energy; // x

          temporaries->get_cluster_property_aux_array<clusterprop_y>(i) /= abs_energy; // y

          temporaries->get_cluster_property_aux_array<clusterprop_z>(i) /= abs_energy; // z
        }
    }
}

__global__ static
void calculateCentroidsDeferKernel(Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                   Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries)
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index == 0)
    //Will be called with just 1 thread, but...
    {
      const int i_dimBlock1 = CalculateCentroidsBlockSize;
      const int i_dimGrid1 = Helpers::int_ceil_div(NCaloCells, i_dimBlock1);
      const dim3 dimBlock1(i_dimBlock1, 1, 1);
      const dim3 dimGrid1(i_dimGrid1, 1, 1);
#if CAN_USE_TAIL_LAUNCH
      calculateCentroids <<< dimGrid1, dimBlock1, 0, cudaStreamTailLaunch>>>(clusters_arr, temporaries, clusters_arr->number);
#else
      calculateCentroids <<< dimGrid1, dimBlock1>>>(clusters_arr, temporaries, clusters_arr->number);
#endif
    }
}


__global__ static
void assignFinalCells( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                       Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                       const Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temporaries,
                       const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                       const Helpers::CUDA_kernel_object<TopoAutomatonSplittingOptions> opts )
{
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < NCaloCells)
    {
      const TASTag tag = cell_state_arr->clusterTag[index];
      if (tag.is_part_of_splitter_cluster())
        {
          if (opts->share_border_cells && tag.is_shared())
            {
              const uint32_t shared_clusters_packed = TASHacks::get_cell_to_cluster_table_entry(clusters_arr, tag.index());
              const int cluster_1 = shared_clusters_packed & 0xFFFFU;
              const int cluster_2 = (shared_clusters_packed >> 16) & 0xFFFFU;

              const float cell_x = geometry->x[index];
              const float cell_y = geometry->y[index];
              const float cell_z = geometry->z[index];


              const float delta_x_1 = cell_x - temporaries->get_cluster_property_aux_array<clusterprop_x>(cluster_1);
              const float delta_x_2 = cell_x - temporaries->get_cluster_property_aux_array<clusterprop_x>(cluster_2);

              const float delta_y_1 = cell_y - temporaries->get_cluster_property_aux_array<clusterprop_y>(cluster_1);
              const float delta_y_2 = cell_y - temporaries->get_cluster_property_aux_array<clusterprop_y>(cluster_2);

              const float delta_z_1 = cell_z - temporaries->get_cluster_property_aux_array<clusterprop_z>(cluster_1);
              const float delta_z_2 = cell_z - temporaries->get_cluster_property_aux_array<clusterprop_z>(cluster_2);


              const float d_1 = sqrtf(delta_x_1 * delta_x_1 + delta_y_1 * delta_y_1 + delta_z_1 * delta_z_1);

              const float d_2 = sqrtf(delta_x_2 * delta_x_2 + delta_y_2 * delta_y_2 + delta_z_2 * delta_z_2);

              float r_exp = (d_1 - d_2) / opts->EM_shower_scale;

              if (r_exp > 10)
                {
                  r_exp = 10;
                }
              else if (r_exp < -10)
                {
                  r_exp = -10;
                }

              const float r = expf(r_exp);
              const float r_reverse = expf(-r_exp);

              float E_1 = temporaries->get_cluster_property_aux_array<clusterprop_E>(cluster_1);

              float E_2 = temporaries->get_cluster_property_aux_array<clusterprop_E>(cluster_2);

              if (opts->use_absolute_energy)
                {
                  E_1 = fabsf(E_1);
                  E_2 = fabsf(E_2);
                }

              if (E_1 <= 0)
                {
                  E_1 = 1.0f * CLHEP::MeV;
                }
              if (E_2 <= 0)
                {
                  E_2 = 1.0f * CLHEP::MeV;
                }

              float weight = E_1 / (E_1 + E_2 * r);
              float rev_weight = E_2 / (E_2 + E_1 * r_reverse);

              if (__float_as_uint(weight) == 0)
                {
                  weight == __uint_as_float(1);
                }

              if (__float_as_uint(rev_weight) == 0)
                {
                  rev_weight == __uint_as_float(1);
                }

              //This is just so that shared clusters
              //always show up as shared clusters.
              //A denormal weight is... negligible for physics.

              if (weight > 0.5f)
                {
                  cell_state_arr->clusterTag[index] = ClusterTag::make_tag(cluster_1, __float_as_uint(rev_weight), cluster_2);
                }
              else if (weight == 0.5f)
                {
                  const int max_cluster = cluster_1 > cluster_2 ? cluster_1 : cluster_2;
                  const int min_cluster = cluster_1 > cluster_2 ? cluster_2 : cluster_1;
                  cell_state_arr->clusterTag[index] = ClusterTag::make_tag(max_cluster, __float_as_uint(weight), min_cluster);
                }
              else /*if (weight < 0.5f)*/
                {
                  cell_state_arr->clusterTag[index] = ClusterTag::make_tag(cluster_2, __float_as_uint(weight), cluster_1);
                }
            }
          else
            {
              const int this_cluster = TASHacks::get_cell_to_cluster_table_entry(clusters_arr, tag.index()) & 0xFFFF;
              cell_state_arr->clusterTag[index] = ClusterTag::make_tag(this_cluster);
            }
        }
      else if (tag.is_non_assigned_part_of_split_cluster())
        {
          const int this_cluster = tag.index();
          cell_state_arr->clusterTag[index] = ClusterTag::make_tag(this_cluster);
          //Cells that are part of a pre-splitter cluster get added to the "same"?

          atomicMax(&(clusters_arr->seedCellID[this_cluster]), index);
          //Not the seed cell, but just a consistent way of marking this cluster as still valid...
        }
      else if (tag.is_part_of_original_cluster())
        {
          const int this_cluster = tag.index();
          cell_state_arr->clusterTag[index] = ClusterTag::make_tag(this_cluster);
        }
      else
        {
          cell_state_arr->clusterTag[index] = ClusterTag::make_invalid_tag();
        }
    }
}

//run the kernel
void cellWeightingAndFinalization(EventDataHolder & holder, Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temps,
                                  const ConstantDataHolder & instance_data, const TASOptionsHolder & options, const bool synchronize,
                                  CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream)
{
  const cudaStream_t & stream_to_use = (stream != nullptr ? * ((cudaStream_t *) stream) : cudaStreamPerThread);

  if (options.m_options->share_border_cells)
    {
      cudaMemsetAsync(temps->secondaryArray, 0, sizeof(tag_type) * NCaloCells, cudaStreamPerThread);
      const int i_dimBlock1 = SumCellsBlockSize;
      const int i_dimGrid1 = Helpers::int_ceil_div(NCaloCells, i_dimBlock1);
      const dim3 dimBlock1(i_dimBlock1, 1, 1);
      const dim3 dimGrid1(i_dimGrid1, 1, 1);

      sumCellsForCentroid <<< dimGrid1, dimBlock1, 0, stream_to_use>>>( holder.m_clusters_dev, temps,
                                                                        holder.m_cell_state_dev, holder.m_cell_info_dev,
                                                                        instance_data.m_geometry_dev                       );

      calculateCentroidsDeferKernel <<< 1, 1, 0, stream_to_use>>>(holder.m_clusters_dev, temps);

    }

  const int i_dimBlock2 = FinalizeWeightsBlockSize;
  const int i_dimGrid2 = Helpers::int_ceil_div(NCaloCells, i_dimBlock2);
  const dim3 dimBlock2(i_dimBlock2, 1, 1);
  const dim3 dimGrid2(i_dimGrid2, 1, 1);

  assignFinalCells <<< dimGrid2, dimBlock2, 0, stream_to_use>>>( holder.m_cell_state_dev, holder.m_clusters_dev, temps,
                                                                 instance_data.m_geometry_dev, options.m_options_dev     );

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(stream_to_use));
    }
}
