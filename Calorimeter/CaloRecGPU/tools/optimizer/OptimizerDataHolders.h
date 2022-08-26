// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
//
#ifndef TOPOAUTOMATONCLUSTEROPTIMIZER_DATAHOLDERS_H
#define TOPOAUTOMATONCLUSTEROPTIMIZER_DATAHOLDERS_H

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "../../src/TopoAutomatonClusteringGPU.h"


#include <fstream>

using namespace CaloRecGPU;

struct TopoAutomatonTemporariesOld
{
  tag_type secondaryArray[NCaloCells];
  tag_type mergeTable[NMaxClusters];
  float seedCellPhi[NMaxClusters];
  int continueFlag;
};


class InstanceDataHolder
{
  public:


    Helpers::CPU_object<GeometryArr> m_geometry;

    Helpers::CPU_object<CellNoiseArr> m_cell_noise_fixed;

    Helpers::CPU_object<TopoAutomatonOptions> m_options;

    Helpers::CPU_object<PairsArr> m_fixed_pairs;
    
    Helpers::CUDA_object<GeometryArr> m_geometry_dev;

    Helpers::CUDA_object<CellNoiseArr> m_cell_noise_fixed_dev;

    Helpers::CUDA_object<TopoAutomatonOptions> m_options_dev;
    
    Helpers::CUDA_object<PairsArr> m_fixed_pairs_dev;

    bool m_GPU_info_ready = false;


    void send(const bool clear_CPU = false)
    {
      m_options_dev = m_options;
      m_geometry_dev = m_geometry;
      m_cell_noise_fixed_dev = m_cell_noise_fixed;
      m_fixed_pairs_dev = m_fixed_pairs;
      m_GPU_info_ready = true;
      if (clear_CPU)
        {
          m_options.clear();
          m_geometry.clear();
          m_cell_noise_fixed.clear();
        }
    }

    bool prepare(Helpers::CPU_object<GeometryArr> && geom, Helpers::CPU_object<CellNoiseArr> && noise)
    {
      m_geometry = geom;
      m_cell_noise_fixed = noise;

      for (int cell = 0; cell < NCaloCells; ++cell)
        {
          const int n_neighs = m_geometry->nNeighbours[cell];

          for (int i = 0; i < n_neighs; ++i)
            {
              const int neigh = m_geometry->neighbours[cell][i];

              const int neigh_neighs = m_geometry->nNeighbours[neigh];

              bool symmetry = false;

              for (int j = 0; j < neigh_neighs; ++j)
                {
                  if (m_geometry->neighbours[neigh][j] == cell)
                    {
                      symmetry = true;
                      break;
                    }
                }
              if (symmetry == false)
                {
                  m_geometry->neighbours[neigh][neigh_neighs] = cell;
                  m_geometry->nNeighbours[neigh] += 1;
                }
            }
        }
      m_options.allocate();
      (*m_options) = TopoAutomatonOptions { 4.0f, 2.0f, 0.0f, true, true, true, false, 0x7FFFFFFF};
      
      m_fixed_pairs.allocate();
      
      for (int i = 0; i < NCaloCells; ++i)
      {
        int neighs[NMaxNeighbours];
        int num_valid_neighs = 0;
        for (int j = 0; j < m_geometry->nNeighbours[i]; ++j)
        {
          const int this_neigh_idx = m_geometry->neighbours[i][j];
          if (this_neigh_idx >= i)
          {
            neighs[num_valid_neighs] = this_neigh_idx;
            ++num_valid_neighs;
          }
        }
        const int old_number = m_fixed_pairs->number;
        
        m_fixed_pairs->number += num_valid_neighs;
        for (int j = 0; j < num_valid_neighs; ++j)
        {
          m_fixed_pairs->cellID[old_number + j] = i;
          m_fixed_pairs->neighbourID[old_number + j] = neighs[j];
        }
        
      }
      
      return false;
    }

};

class ProcessingDataHolder
{
  public:

    Helpers::CPU_object<CellInfoArr> m_cell_info;
    Helpers::CPU_object<CellStateArr> m_cell_state;
    Helpers::CPU_object<PairsArr> m_pairs, m_half_pairs;
    Helpers::CPU_object<ClusterInfoArr> m_clusters;

    Helpers::CUDA_object<CellInfoArr> m_cell_info_dev;
    Helpers::CUDA_object<CellStateArr> m_cell_state_dev;
    Helpers::CUDA_object<PairsArr> m_pairs_dev, m_half_pairs_dev;
    Helpers::CUDA_object<ClusterInfoArr> m_clusters_dev;


    //Temporaries for the method.
    Helpers::CPU_object<TopoAutomatonTemporariesOld> m_temporaries;

    Helpers::CUDA_object<TopoAutomatonTemporariesOld> m_temporaries_dev;

    bool prepare(Helpers::CPU_object<CellInfoArr> && cell_info)
    {
      m_cell_info = cell_info;

      m_cell_state.allocate();
      m_clusters.allocate();
      m_clusters->number = 0;
      m_pairs.allocate();
      m_pairs->number = 0;
      m_temporaries.allocate();
      m_temporaries_dev.allocate();
      //Yes, unneeded for processing from scratch,
      //but we'll save intermediate results.

      return false;
    }

};

#endif //TOPOAUTOMATONCLUSTEROPTIMIZER_DATAHOLDERS_H