// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef TOPOAUTOMATONCLUSTEROPTIMIZER_DEFAULT_IMPLEMENTATION_H
#define TOPOAUTOMATONCLUSTEROPTIMIZER_DEFAULT_IMPLEMENTATION_H

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "OptimizerDataHolders.h"


#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>

using namespace CaloRecGPU;

namespace DefaultImplementation
{

  constexpr int DefaultBlockSize = 256;

  __device__ inline static tag_type calculateTag(const float SNR, const int seed_cell_index, const int address)
  {
    return Tags::make_seed_tag(__float_as_int(SNR), seed_cell_index, address);
  }


  static __global__
  void signalToNoiseKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                            Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                            Helpers::CUDA_kernel_object<TopoAutomatonTemporariesOld> temporaries,
                            Helpers::CUDA_kernel_object<CellInfoArr> info_arr,
                            const Helpers::CUDA_kernel_object<CellNoiseArr> noise_arr,
                            const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                            const Helpers::CUDA_kernel_object<TopoAutomatonOptions> opts)
  {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;


    if (index < NCaloCells)
      {
        const float cellEnergy = info_arr->energy[index];
        const int gain = info_arr->gain[index];

        if (GainConversion::is_invalid_cell(gain))
          {
            cell_state_arr->clusterTag[index] = Tags::InvalidTag;
            return;
          }

        float sigNoiseRatio = 0.00001f;
        //It's what's done in the CPU implementation...
        if (!GainConversion::is_bad_cell(gain))
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
              }
            else if ( !( opts->validSamplingSeed & (1 << geometry->caloSample[index]) ) )
              //If the seed belongs to a sampling that's invalid for seeds...
              {
                cell_state_arr->clusterTag[index] = Tags::GrowTag;
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

                clusters_arr->seedCellID[n] = index;
                temporaries->seedCellPhi[n] = geometry->phi[index];
                clusters_arr->clusterEnergy[n] = 0.f;
                clusters_arr->clusterEt[n] = 0.f;
                clusters_arr->clusterEta[n] = 0.f;
#if !TTAC_CALCULATE_PHI_BY_SIMPLE_AVERAGE
                clusters_arr->clusterSumSinPhi[n] = 0.f;
                clusters_arr->clusterSumCosPhi[n] = 0.f;
#else
                clusters_arr->clusterPhi[n] = 0.f;
#endif
                temporaries->mergeTable[n] = Tags::clear_counter(tag);
              }
          }
        else if (sigNoiseRatio > opts->grow_threshold || (opts->abs_grow && absRatio > opts->grow_threshold)) //is Grow
          {
            cell_state_arr->clusterTag[index] = Tags::GrowTag;
          }
        else if (sigNoiseRatio > opts->terminal_threshold || (opts->abs_terminal && absRatio > opts->terminal_threshold)) //is Terminal
          {
            cell_state_arr->clusterTag[index] = Tags::TerminalTag;
          }
        else //is invalid for propagation
          {
            cell_state_arr->clusterTag[index] = Tags::InvalidTag;
          }
      }
  }

  //run the kernel
  void signal_to_noise(ProcessingDataHolder & holder, const InstanceDataHolder & instance_data, const int blocksize = DefaultBlockSize)
  {

    const int i_dimBlock = blocksize;
    const int i_dimGrid = Helpers::int_ceil_div(NCaloCells, i_dimBlock);

    const dim3 dimBlock(i_dimBlock, 1, 1);
    const dim3 dimGrid(i_dimGrid, 1, 1);
    signalToNoiseKernel <<< dimGrid, dimBlock>>>(holder.m_cell_state_dev, holder.m_clusters_dev, holder.m_temporaries_dev,
        holder.m_cell_info_dev, instance_data.m_cell_noise_fixed_dev, instance_data.m_geometry_dev,
        instance_data.m_options_dev);

    CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));

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
      }
  }


  //run the kernel
  void cell_pairs(ProcessingDataHolder & holder, const InstanceDataHolder & instance_data, const int blocksize = DefaultBlockSize)
  {

    const int i_dimBlock = blocksize;
    const int i_dimGrid = Helpers::int_ceil_div(NCaloCells, i_dimBlock);

    const dim3 dimBlock(i_dimBlock, 1, 1);
    const dim3 dimGrid(i_dimGrid, 1, 1);

    cellPairsKernel <<< dimGrid, dimBlock>>>(holder.m_pairs_dev, holder.m_cell_state_dev, instance_data.m_geometry_dev);

    CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));

  }



  /******************************************************************************
   * Series of kernels for the growing algorithm!
   ******************************************************************************/

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

        const tag_type neigh_raw_tag = cell_state_arr->clusterTag[neigh_ID];

        const tag_type neigh_prop_tag = Tags::set_for_propagation(neigh_raw_tag);

        const tag_type this_old_raw_tag = atomicMax(&(cell_state_arr->clusterTag[this_ID]), neigh_prop_tag);

        if (Tags::is_part_of_cluster(this_old_raw_tag) && Tags::is_part_of_cluster(neigh_raw_tag))
          {
            //If the cell was already part of a cluster,
            //we must merge the two of them.
            //Else, we keep growing.
            const int this_address = Tags::get_index_from_tag(this_old_raw_tag);
            const int neigh_address = Tags::get_index_from_tag(neigh_raw_tag);
            const tag_type maximum_cluster = max(Tags::clear_counter(this_old_raw_tag), Tags::clear_counter(neigh_raw_tag));
            atomicMax(&(temporaries->mergeTable[this_address]), maximum_cluster);
            atomicMax(&(temporaries->mergeTable[this_address]), maximum_cluster);
          }
        else if (Tags::is_part_of_cluster(neigh_raw_tag))
          {
            temporaries->continueFlag = 1;
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
      }
  }

  __global__ static
  void propagateTerminals( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                           const int pair_number,
                           const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs)
  {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < pair_number)
      {
        const int this_ID = neighbour_pairs->cellID[index];
        const int neigh_ID = neighbour_pairs->neighbourID[index];

        const tag_type this_tag = cell_state_arr->clusterTag[this_ID];

        atomicMax(&(cell_state_arr->clusterTag[neigh_ID]), Tags::set_for_terminal_propagation(this_tag));
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
        cell_state_arr->clusterTag[index] = Tags::clear_counter(old_tag);
      }
  }


  __global__ static
  void clusterGrowingKernel( Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                             Helpers::CUDA_kernel_object<TopoAutomatonTemporariesOld> temporaries,
                             const Helpers::CUDA_kernel_object<PairsArr> neighbour_pairs,
                             const int blocksize1, const int blocksize2, const int blocksize3, const int blocksize4)
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
        const int i_dimGrid3 = Helpers::int_ceil_div(pairs_number, i_dimBlock3);
        const dim3 dimBlock3(i_dimBlock3, 1, 1);
        const dim3 dimGrid3(i_dimGrid3, 1, 1);

        const int i_dimBlock4 = blocksize4;
        const int i_dimGrid4 = Helpers::int_ceil_div(NCaloCells, i_dimBlock4);
        const dim3 dimBlock4(i_dimBlock4, 1, 1);
        const dim3 dimGrid4(i_dimGrid4, 1, 1);


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
        propagateTerminals <<< dimGrid3, dimBlock3>>>(cell_state_arr, pairs_number, neighbour_pairs);
        finalizeClusterAttribution <<< dimGrid4, dimBlock4>>>(cell_state_arr, temporaries);
      }
  }

  //run the kernel
  void cluster_growing(ProcessingDataHolder & holder, const InstanceDataHolder & instance_data,
                       const int blocksize1 = DefaultBlockSize, const int blocksize2 = DefaultBlockSize, const int blocksize3 = -1, const int blocksize4 = -1)
  {

    clusterGrowingKernel <<< 1, 1>>>(holder.m_cell_state_dev, holder.m_temporaries_dev, holder.m_pairs_dev,
                                     blocksize1, blocksize2, ( blocksize3 < 0 ? blocksize1 : blocksize3 ), ( blocksize4 < 0 ? blocksize2 : blocksize4));

    CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));

  }


  /******************************************************************************
   * Finalize cluster information.
   ******************************************************************************/

  __global__ static
  void calculateClusterInfoKernel( Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr,
                                   const Helpers::CUDA_kernel_object<CellStateArr> cell_state_arr,
                                   const Helpers::CUDA_kernel_object<CellInfoArr> info_arr,
                                   const Helpers::CUDA_kernel_object<GeometryArr> geometry,
                                   const Helpers::CUDA_kernel_object<TopoAutomatonTemporariesOld> temporaries)
  {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < NCaloCells)
      {
        const tag_type tag = cell_state_arr->clusterTag[index];
        if (Tags::is_part_of_cluster(tag))
          //By this point they all have the terminals anyway, so...
          {
            const int cluster_index = Tags::get_index_from_tag(tag);
            const float energy = info_arr->energy[index];
            const float abs_energy = fabsf(energy);
            const float phi_raw = geometry->phi[index];

            atomicAdd(&(clusters_arr->clusterEnergy[cluster_index]), energy);
            atomicAdd(&(clusters_arr->clusterEt[cluster_index]), abs_energy);
            atomicAdd(&(clusters_arr->clusterEta[cluster_index]), abs_energy * geometry->eta[index]);


#if !TTAC_CALCULATE_PHI_BY_SIMPLE_AVERAGE
            float sin_phi;
            float cos_phi;
            sincosf(phi_raw, &sin_phi, &cos_phi);
            //Calculates the sine and co-sine of phi
            atomicAdd(&(clusters_arr->clusterSumSinPhi[cluster_index]), sin_phi * abs_energy);
            atomicAdd(&(clusters_arr->clusterSumCosPhi[cluster_index]), cos_phi * abs_energy);
#else
            const float phi_0 = temporaries->seedCellPhi[cluster_index];
            const float phi_real = phi_raw +
                                   (phi_raw > phi_0 + Helpers::Constants::pi<float> ? -2 :
                                    phi_raw < phi_0 - Helpers::Constants::pi<float> ? 2 : 0) * Helpers::Constants::pi<float>;
            atomicAdd(&(clusters_arr->clusterPhi[cluster_index]), phi_real * abs_energy);
#endif

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

            clusters_arr->clusterEt[i] = abs_energy / coshf(tempeta);

#if !TTAC_CALCULATE_PHI_BY_SIMPLE_AVERAGE
            const float raw_phi = atan2f(clusters_arr->clusterSumSinPhi[i], clusters_arr->clusterSumCosPhi[i]);
            const float corr_phi = ( (isnan(raw_phi) || isinf(raw_phi)) ? -10 :
                                     raw_phi + (raw_phi < 0 ? 2 * Helpers::Constants::pi<float> : 0) );

            clusters_arr->clusterPhi[i] = corr_phi;
#else
            clusters_arr->clusterPhi[i] /= abs_energy;
#endif
          }
        else
          {
            clusters_arr->seedCellID[i] = -1;
            //This is just a way to signal that this is an invalid cluster.
          }
      }
  }

  __global__ static
  void finalizeClustersDeferKernel( Helpers::CUDA_kernel_object<ClusterInfoArr> clusters_arr, const int blocksize2 )
  {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index == 0)
      {
        const int cluster_number = clusters_arr->number;

        const int i_dimBlock = blocksize2;
        const int i_dimGrid = Helpers::int_ceil_div(cluster_number, i_dimBlock);
        const dim3 dimBlock(i_dimBlock, 1, 1);
        const dim3 dimGrid(i_dimGrid, 1, 1);

        finalizeClusterInfoKernel <<< dimGrid, dimBlock >>>(clusters_arr, cluster_number);

      }
  }

  void finalize_clusters(ProcessingDataHolder & holder, const InstanceDataHolder & instance_data, const int blocksize1 = DefaultBlockSize, const int blocksize2 = DefaultBlockSize)
  {

    const int i_dimBlock = blocksize1;
    const int i_dimGrid = Helpers::int_ceil_div(NCaloCells, i_dimBlock);
    const dim3 dimBlock(i_dimBlock, 1, 1);
    const dim3 dimGrid(i_dimGrid, 1, 1);

    calculateClusterInfoKernel <<< dimGrid, dimBlock>>>(holder.m_clusters_dev, holder.m_cell_state_dev, holder.m_cell_info_dev, instance_data.m_geometry_dev, holder.m_temporaries_dev);

    finalizeClustersDeferKernel <<< 1, 1>>>(holder.m_clusters_dev, blocksize2);

    CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));
  }
}

#endif