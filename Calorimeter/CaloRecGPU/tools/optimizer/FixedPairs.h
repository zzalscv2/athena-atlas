// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef TOPOAUTOMATONCLUSTEROPTIMIZER_FIXEDPAIR_IMPLEMENTATION_H
#define TOPOAUTOMATONCLUSTEROPTIMIZER_FIXEDPAIR_IMPLEMENTATION_H

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "OptimizerDataHolders.h"
#include "ExtraDefs.h"


#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>

using namespace CaloRecGPU;

namespace FixedPairs
{

  constexpr int DefaultBlockSize = 256;
  __global__ static
  void propagateNeighbours( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                            Helpers::CUDA_kernel_object<TopoAutomatonTemporariesOld> temporaries,
                            const int pair_number,
                            const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs)
  {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < pair_number)
      {
        const int this_ID = neighbour_pairs->cellID[index];
        const int neigh_ID = neighbour_pairs->neighbourID[index];

        const tag_type this_raw_tag = cell_state_arr->clusterTag[this_ID];
        const tag_type neigh_raw_tag = cell_state_arr->clusterTag[neigh_ID];
        
        if (Tags::is_invalid(this_raw_tag) || Tags::is_invalid(neigh_raw_tag))
          {
            return;
          }
        
        if (Tags::is_growing_or_seed(this_raw_tag) && Tags::is_growing_or_seed(neigh_raw_tag))
          {            
            const tag_type neigh_prop_tag = Tags::set_for_propagation(neigh_raw_tag);
            const tag_type this_prop_tag = Tags::set_for_propagation(this_raw_tag);

            atomicMax(&(cell_state_arr->clusterTag[this_ID]), neigh_prop_tag);
            atomicMax(&(cell_state_arr->clusterTag[neigh_ID]), this_prop_tag);
            if (Tags::is_part_of_cluster(this_raw_tag) && Tags::is_part_of_cluster(neigh_raw_tag))
              {
                tag_type maximum_cluster = max(Tags::clear_counter(this_raw_tag), Tags::clear_counter(neigh_raw_tag));

                const int this_address = Tags::get_index_from_tag(this_raw_tag);
                const int neigh_address = Tags::get_index_from_tag(neigh_raw_tag);

                atomicMax(&(temporaries->mergeTable[this_address]), maximum_cluster);
                atomicMax(&(temporaries->mergeTable[neigh_address]), maximum_cluster);
              }
            else if (Tags::is_part_of_cluster(this_raw_tag) || Tags::is_part_of_cluster(neigh_raw_tag))
              {
                temporaries->continueFlag = 1;
              }
          }
        else if (Tags::is_part_of_cluster(this_raw_tag) /* && Tags::is_assignable_terminal(neigh_raw_tag)*/)
          {
            atomicMax(&(cell_state_arr->clusterTag[neigh_ID]), ExtraTags::set_for_terminal_propagation2(this_raw_tag));
          }
        else if (Tags::is_part_of_cluster(neigh_raw_tag) /* && Tags::is_assignable_terminal(neigh_raw_tag)*/)
          {
            atomicMax(&(cell_state_arr->clusterTag[this_ID]), ExtraTags::set_for_terminal_propagation2(neigh_raw_tag));
          }
        else
          {
            //Between terminals, nothing.
          }
      }
  }


  __global__ static
  void mergeClusters( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                      const Helpers::CUDA_kernel_object<TopoAutomatonTemporariesOld> temporaries)
  {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < NCaloCells)
      {
        const tag_type old_tag = cell_state_arr->clusterTag[index];
        if (Tags::is_part_of_cluster(old_tag))
          {
            const int address = Tags::get_index_from_tag(old_tag);
            const tag_type new_tag = temporaries->mergeTable[address];
            cell_state_arr->clusterTag[index] = Tags::update_non_terminal_tag(old_tag, new_tag);
          }
        else if (Tags::is_assigned_terminal(old_tag))
          {
            const int address = Tags::get_index_from_tag(old_tag);
            const tag_type new_tag = temporaries->mergeTable[address];
            cell_state_arr->clusterTag[index] = ExtraTags::update_terminal_tag2(old_tag, new_tag);
          }
      }
  }

  __global__ static
  void finalizeClusterAttribution( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                   const Helpers::CUDA_kernel_object<TopoAutomatonTemporariesOld> temporaries)
  {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < NCaloCells)
      {
        const tag_type old_tag = cell_state_arr->clusterTag[index];
        if (Tags::is_assigned_terminal(old_tag))
          {
            cell_state_arr->clusterTag[index] = Tags::terminal_to_seed_tag(old_tag);
          }
        else /*if (Tags::is_part_of_cluster(old_tag))*/
          {
            cell_state_arr->clusterTag[index] = Tags::clear_counter(old_tag);
          }
      }
  }

  __global__ static
  void clusterGrowingKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                             Helpers::CUDA_kernel_object<TopoAutomatonTemporariesOld> temporaries,
                             const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                             const int blocksize1, const int blocksize2, const int blocksize3)
  {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index == 0)
      //Will be called with just 1 thread, but...
      {
        const int pairs_number = neighbour_pairs->number;

        const int i_dimBlock1 = blocksize1;
        const int i_dimGrid1 = Helpers::int_ceil_div(pairs_number, i_dimBlock1);
        const dim3 dimBlock1(i_dimBlock1, 1, 1);
        const dim3 dimGrid1(i_dimGrid1, 1, 1);

        const int i_dimBlock2 = blocksize2;
        const int i_dimGrid2 = Helpers::int_ceil_div(NCaloCells, i_dimBlock2);
        const dim3 dimBlock2(i_dimBlock2, 1, 1);
        const dim3 dimGrid2(i_dimGrid2, 1, 1);

        const int i_dimBlock3 = blocksize3;
        const int i_dimGrid3 = Helpers::int_ceil_div(NCaloCells, i_dimBlock3);
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
        finalizeClusterAttribution <<< dimGrid3, dimBlock3>>>(cell_state_arr, temporaries);
      }
  }

  //run the kernel
  void cluster_growing(ProcessingDataHolder & holder, const InstanceDataHolder & instance_data,
                       const int blocksize1 = DefaultBlockSize, const int blocksize2 = DefaultBlockSize, const int blocksize3 = -1)
  {

    clusterGrowingKernel <<< 1, 1>>>(holder.m_cell_state_dev, holder.m_temporaries_dev, instance_data.m_fixed_pairs_dev,
                                     blocksize1, blocksize2, ( blocksize3 < 0 ? blocksize1 : blocksize3 ));

    CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));

  }

}

#endif