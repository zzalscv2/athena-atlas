//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_BASEDEFINITIONS_H
#define CALORECGPU_BASEDEFINITIONS_H

namespace CaloRecGPU
{
  constexpr inline int NMaxNeighbours = 34;
  constexpr inline int NCaloCells = 187652;
  //Max index will be 0x0002 DD03
  constexpr inline int NMaxClusters = 0x10000U;
  //So it all fits in an int16_t
  //Would only start getting fishy if all the cells
  //were part of a cluster and all the clusters
  //had an average number of cells lower than 2.863...
  //(Or, in general, for N cells not part of a cluster,
  // an average number of cells per cluster lower than 2.863 - N/65536...)

  constexpr inline int NMaxPairs = NMaxNeighbours * NCaloCells;

  constexpr inline int NumGainStates = 4;
  //This is the number of different gain states a cell can have.
  //(For the noise calculation.)
  
  constexpr inline int NumSamplings = 28;
  //The number of samplings in the calorimeter.

}

#endif
