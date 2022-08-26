/*
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloRecGPU/DataHolders.h"

void ConstantDataHolder::sendToGPU(const bool clear_CPU)
{
  m_cell_noise_dev = m_cell_noise;
  m_geometry_dev = m_geometry;
  if (clear_CPU)
    {
      m_cell_noise.clear();
      m_geometry.clear();
    }
}

void EventDataHolder::sendToGPU(const bool clear_CPU, const bool has_state, const bool has_clusters, const bool has_pairs)
{
  m_cell_info_dev = m_cell_info;
  if (has_state)
    {
      m_cell_state_dev = m_cell_state;
    }
  else
    {
      m_cell_state_dev.allocate();
    }
  if (has_clusters)
    {
      m_clusters_dev = m_clusters;
    }
  else
    {
      m_clusters_dev.allocate();
    }
  if (has_pairs)
    {
      m_pairs_dev = m_pairs;
    }
  else
    {
      m_pairs_dev.allocate();
    }

  if (!has_clusters)
    {
      cudaMemset(&(m_clusters_dev->number), 0, sizeof(m_clusters_dev->number));
    }
  if (!has_pairs)
    {
      cudaMemset(&(m_pairs_dev->number), 0, sizeof(m_pairs_dev->number));
      cudaMemset(&(m_pairs_dev->reverse_number), 0, sizeof(m_pairs_dev->reverse_number));
    }
  //We're not doing this through cudaMemsetAsync because it is reasonable to expect
  //the clusters to be fully sent before doing any more operations.

  if (clear_CPU)
    {
      m_cell_info.clear();
      m_cell_state.clear();
      m_pairs.clear();
    }
}

void EventDataHolder::returnToCPU(const bool clear_GPU, const bool return_clusters)
{
  m_cell_state = m_cell_state_dev;
  if (return_clusters)
    {
      m_clusters = m_clusters_dev;
    }
  if (clear_GPU)
    {
      m_cell_state_dev.clear();
      m_clusters_dev.clear();
      m_pairs_dev.clear();
      m_cell_info_dev.clear();
    }
}

void EventDataHolder::allocate(const bool also_GPU)
{
  m_cell_info.allocate();
  m_cell_state.allocate();
  m_pairs.allocate();
  m_clusters.allocate();

  if (also_GPU)
    {
      m_cell_info_dev.allocate();
      m_cell_state_dev.allocate();
      m_pairs_dev.allocate();
      m_clusters_dev.allocate();
    }
}