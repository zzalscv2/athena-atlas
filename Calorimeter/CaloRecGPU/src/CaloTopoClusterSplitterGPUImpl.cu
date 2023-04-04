//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloTopoClusterSplitterGPUImpl.h"


#include <cstring>
#include <cmath>
#include <iostream>
#include <stdio.h>

using namespace CaloRecGPU;

void GPUSplitterOptionsHolder::allocate()
{
  m_options.allocate();
  m_options_dev.allocate();
}

void GPUSplitterOptionsHolder::sendToGPU(const bool clear_CPU)
{
  m_options_dev = m_options;
  if (clear_CPU)
    {
      m_options.clear();
    }
}


#define check_if_secondary(cell_id, d_meta, d_fullclusters) ((d_fullcalogeometry)->caloSample[(cell_id)] >= (d_meta)->m_minSecondarySampling &&\
                                                             (d_fullcalogeometry)->caloSample[(cell_id)] <= (d_meta)->m_maxSecondarySampling &&\
                                                             ((d_meta)->m_useSecondarySampling & (1 << ((d_fullcalogeometry)->caloSample[(cell_id)] - (d_meta)->m_minSecondarySampling))))

constexpr static int DefaultBlockSize = 512;


/************************************************************************/


void preProcessingPreparation(EventDataHolder & holder, Helpers::CUDA_kernel_object<GPUSplitterTemporaries> temps,
                              const ConstantDataHolder & instance_data, const GPUSplitterOptionsHolder & options, const bool synchronize)
{
  CUDA_ERRCHECK(cudaMemsetAsync((void *) temps->max_cells, 0, NCaloCells * sizeof(temps->max_cells[0]), cudaStreamPerThread));
  CUDA_ERRCHECK(cudaMemsetAsync((void *) temps->secondary_max_cells, 0, NCaloCells * sizeof(temps->secondary_max_cells[0]), cudaStreamPerThread));
  CUDA_ERRCHECK(cudaMemsetAsync((void *) temps->splitter_seeds, 0, NMaxClusters * sizeof(temps->splitter_seeds[0]), cudaStreamPerThread));
  CUDA_ERRCHECK(cudaMemsetAsync((void *) temps->secondary_splitter_seeds, 0, NMaxClusters * sizeof(temps->secondary_splitter_seeds[0]), cudaStreamPerThread));

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));
    }
}



/***********************************************************************/

static __global__ void find_local_maximums(const Helpers::CUDA_kernel_object<CaloTopoClusterSplitterMetadata> d_meta,
                                           const Helpers::CUDA_kernel_object<GeometryArr> d_fullcalogeometry,
                                           const Helpers::CUDA_kernel_object<CellInfoArr> d_cellsfulldata,
                                           const Helpers::CUDA_kernel_object<CellStateArr> d_cellstate,
                                           Helpers::CUDA_kernel_object<GPUSplitterTemporaries> temporaries)
{
  unsigned cell_id = blockIdx.x * blockDim.x + threadIdx.x;
  int num_neighbours, n_id;
  int cell_tag;
  int i;
  int count_neigh = 0;
  bool max = true;
  float energy, n_energy, n_eta, n_phi;
  bool is_secondary = false, is_primary = true;
  int calo_sample, n_sample;

  if (cell_id > NCaloCells)
    {
      return;
    }

  num_neighbours = d_fullcalogeometry->neighbours.get_number_of_neighbours(cell_id);
  calo_sample = d_fullcalogeometry->caloSample[cell_id];
  energy = d_cellsfulldata->energy[cell_id];
  cell_tag = ClusterTag::cluster_index(d_cellstate->clusterTag[cell_id]);

  if (d_meta->m_absOpt)
    {
      energy = fabs(energy);
    }

  if (energy < d_meta->m_minEnergy)
    {
      return;
    }

  if (d_cellsfulldata->is_bad(*d_fullcalogeometry, cell_id, d_meta->m_treatL1PredictedCellsAsGood) && energy > 0)
    {

      /* check if cell can be used for local max */
      if (d_meta->uses_sampling(calo_sample))
        {
          is_primary = true;
        }
      else if (d_meta->uses_secondary_sampling(calo_sample))
        {
          is_secondary = true;
        }
    }

  for (i = 0; i < num_neighbours; i++)
    {
      n_id = d_fullcalogeometry->neighbours.get_neighbour(cell_id, i);
      //FUTURE TODO WARNING FIX ALERT ERROR PAY ATTENTION:
      //this is not taking into account limited neighbours!

      /* skip if both cells aren't in the same cluster */
      if (cell_tag != ClusterTag::cluster_index(d_cellstate->clusterTag[n_id]))
        {
          continue;
        }

      n_energy = d_meta->m_absOpt ? fabs(d_cellsfulldata->energy[n_id]) : d_cellsfulldata->energy[n_id];
      n_phi = d_fullcalogeometry->phi[n_id];
      n_eta = d_fullcalogeometry->eta[n_id];
      n_sample = d_fullcalogeometry->caloSample[n_id];

      if (energy > n_energy)
        {
          count_neigh++;
        }
      else
        {
          max = false;
        }

      if (n_energy >= d_meta->m_minEnergy &&
          is_secondary &&
          d_meta->uses_sampling(n_sample))
        {
          if (fabs(n_eta - d_fullcalogeometry->eta[cell_id]) < 0.0025)
            {
              double diff_phi = n_phi - d_fullcalogeometry->phi[cell_id];
              if (diff_phi > M_PI)
                {
                  diff_phi = diff_phi - 2 * M_PI;
                }
              else if (diff_phi < -M_PI)
                {
                  diff_phi = diff_phi + 2 * M_PI;
                }

              if (fabs(diff_phi) < 0.02)
                {
                  max = false;
                }
            }
        }
    }

  if (count_neigh < d_meta->m_nCells)
    {
      max = false;
    }

  if (max)
    {
      if (is_primary)
        {
          temporaries->max_cells[cell_id] = 1;
          atomicAdd(&temporaries->splitter_seeds[cell_tag], 1);
        }
      else if (is_secondary)
        {
          temporaries->secondary_max_cells[cell_id] = 1;
          atomicAdd(&temporaries->secondary_splitter_seeds[cell_tag], 1);
        }
    }
}


void findLocalMaxima(EventDataHolder & holder, Helpers::CUDA_kernel_object<GPUSplitterTemporaries> temps,
                     const ConstantDataHolder & instance_data, const GPUSplitterOptionsHolder & options, const bool synchronize)
{
  const int block_size = DefaultBlockSize;
  const int num_blocks = Helpers::int_ceil_div(NCaloCells, block_size);

  dim3 bsize(block_size, 1, 1);
  dim3 gsize(num_blocks, 1, 1);

  find_local_maximums <<< gsize, bsize>>>(options.m_options_dev, instance_data.m_geometry_dev,
                                          holder.m_cell_info_dev, holder.m_cell_state_dev, temps);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));
    }
}


/***********************************************************************/

__global__ void splitter_tag_propagation(const Helpers::CUDA_kernel_object<GeometryArr> d_fullcalogeometry,
                                         const Helpers::CUDA_kernel_object<CellStateArr> d_cellstate,
                                         Helpers::CUDA_kernel_object<ClusterInfoArr> d_fullclusters,
                                         Helpers::CUDA_kernel_object<GPUSplitterTemporaries> temporaries)
{
  int tid = threadIdx.x;
  int cell_tag, n_cell_tag;
  int cell_id, n_cell_id;
  int old_num;
  /*int offset = d_fullclusters->number;*/
  int i, k, steps = 0;
  __shared__ int * aux, *first_q, *sec_q;


  __shared__ int current;
  __shared__ int Q_len, Q2_len;

  if (tid == 0)
    {
      current = 0;
      Q_len = 0;
      first_q = temporaries->queue1;
      sec_q = temporaries->queue2;

      //NSF: We need this so the new clusters can be properly processed...
      //(Obviously, since this is all on one block,
      // it's fine to zero it here...)
      d_fullclusters->number = 0;

    }

  __syncthreads();

  /* Iterate through all cells and:
   * - queue the local max and secondary maximums
   * - assign a new cluster tag = call_id + n_clusters
   * Compared to the CPU implementation, this doesn't start from the cells
   * with highest energy since we don't sort the array on GPU so we might
   * obtain slightly different results because of that.
   */
  for (cell_id = tid; cell_id < NCaloCells; cell_id += blockDim.x)
    {
      if (temporaries->max_cells[cell_id] || temporaries->secondary_max_cells[cell_id])
        {

          old_num = atomicAdd(&current, 1);
          temporaries->tags[cell_id] = old_num /*+ offset*/;

          old_num = atomicAdd(&Q_len, 1);
          temporaries->queue1[old_num] = cell_id;


          //NSF: We need this so the new clusters can be properly processed...
          const int old_size = atomicAdd(&(d_fullclusters->number), 1);
          d_fullclusters->seedCellID[old_size] = cell_id;

        }
      else
        {
          temporaries->tags[cell_id] = -1;
        }
    }

  __syncthreads();

  /* Start iterate through cells in queue built at previous step and:
   * - check what cell neighbour doesn't have a tag assigned in d_tags but
   *   has the same original cluster tag with current cell
   * - if the tag is -1 (unused yet), change its tag with current cell
   *   tag and add the neighbour cell in a secondary queue.
   * This algorithm will be repeated until the secondary queue is empty.
   */
  do
    {
      if (tid == 0)
        {
          Q2_len = 0;
        }

      __syncthreads();

      for (i = tid; i < Q_len; i += blockDim.x)
        {
          cell_id = first_q[i];
          cell_tag = ClusterTag::cluster_index(d_cellstate->clusterTag[cell_id]);
          int num_neighbours = d_fullcalogeometry->neighbours.get_number_of_neighbours(cell_id);
          //FUTURE TODO WARNING FIX ALERT ERROR PAY ATTENTION:
          //this is not taking into account limited neighbours!

          for (k = 0; k < num_neighbours; k++)
            {
              n_cell_id = d_fullcalogeometry->neighbours.get_neighbour(cell_id, k);
              n_cell_tag = ClusterTag::cluster_index(d_cellstate->clusterTag[n_cell_id]);

              if (cell_tag == n_cell_tag)
                {
                  old_num = atomicCAS(&temporaries->tags[n_cell_id], -1, temporaries->tags[cell_id]);
                  if (old_num == -1)
                    {
                      int j = atomicAdd(&Q2_len, 1);
                      sec_q[j] = n_cell_id;
                    }
                }
            }
        }

      __syncthreads();

      if (tid == 0)
        {
          steps ++;
          aux = sec_q;
          sec_q = first_q;
          first_q = aux;
          Q_len = Q2_len;
        }

      __syncthreads();
    }
  while (Q2_len > 0);
}


void propagateTags(EventDataHolder & holder, Helpers::CUDA_kernel_object<GPUSplitterTemporaries> temps,
                   const ConstantDataHolder & instance_data, const GPUSplitterOptionsHolder & options, const bool synchronize)
{
  splitter_tag_propagation <<< 1, 2 * DefaultBlockSize>>>(instance_data.m_geometry_dev,
                                                          holder.m_cell_state_dev,
                                                          holder.m_clusters_dev,
                                                          temps);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));
    }

}

/***********************************************************************/


static __global__ void refill_clusters(const Helpers::CUDA_kernel_object<GeometryArr> d_fullcalogeometry,
                                       Helpers::CUDA_kernel_object<ClusterInfoArr> d_fullclusters,
                                       Helpers::CUDA_kernel_object<CellStateArr> d_cellstate,
                                       Helpers::CUDA_kernel_object<GPUSplitterTemporaries> temporaries)
{
  unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

  if (i >= NCaloCells)
    {
      return;
    }

  /* reset all clusters to 0 */
  /*
  if (i < NMaxClusters) {
      d_fullclusters->clusterSize[i] = 0;
      d_fullclusters->clusterEnergy[i] = 0;
      d_fullclusters->clusterEt[i] = 0;
      d_fullclusters->clusterEta[i] = 0;
  }
  */

  int tag = temporaries->tags[i];

  /* set the seed tags accordingly to what we found in splitter algorithm */
  if (tag >= 0)
    {
      d_cellstate->clusterTag[i] = ClusterTag::make_tag(tag);
    }
  else
    {
      tag = -1;
      d_cellstate->clusterTag[i] = ClusterTag::make_invalid_tag();
    }

  //NSF: As there's no synchronization, there might be trouble between the zeroing and the calculation...

  /*
  if (tag >= 0) {
      float energy = d_fullclusters->cellSN2[i];
      atomicAdd(&d_fullclusters->clusterEnergy[tag], energy);
      atomicAdd(&d_fullclusters->clusterEt[tag], abs(energy));
      atomicAdd(&d_fullclusters->clusterEta[tag], d_fullcalogeometry->eta[i] * abs(energy));

      // TODO: why?
      float phi0 = d_fullcalogeometry->phi[i];
      float phi = phi0;
      if (phi > phi0 + M_PI)
          phi = phi - 2 * M_PI;
      if (phi < phi0 - M_PI)
          phi = phi + 2 * M_PI;
      atomicAdd(&d_fullclusters->clusterPhi[tag], phi * abs(energy));
  }
  */
}

void refillClusters(EventDataHolder & holder, Helpers::CUDA_kernel_object<GPUSplitterTemporaries> temps,
                    const ConstantDataHolder & instance_data, const GPUSplitterOptionsHolder & options, const bool synchronize)
{
  const int block_size = DefaultBlockSize;
  const int num_blocks = Helpers::int_ceil_div(NCaloCells, block_size);

  dim3 bsize(block_size, 1, 1);
  dim3 gsize(num_blocks, 1, 1);

  refill_clusters <<< gsize, bsize>>>(instance_data.m_geometry_dev, holder.m_clusters_dev,
                                      holder.m_cell_state_dev, temps);

  if (synchronize)
    {
      CUDA_ERRCHECK(cudaPeekAtLastError());
      CUDA_ERRCHECK(cudaStreamSynchronize(cudaStreamPerThread));
    }
}
