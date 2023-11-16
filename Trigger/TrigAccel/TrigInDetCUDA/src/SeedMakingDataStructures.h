/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_SEEDMAKINGDATASTRUCTURES_H
#define TRIGINDETCUDA_SEEDMAKINGDATASTRUCTURES_H

static constexpr unsigned int MAX_MIDDLE_SP             = 300000;
static constexpr unsigned int MAX_DOUBLET               = 50000000;
static constexpr unsigned int NUM_MIDDLE_THREADS        = 32;
static constexpr unsigned int OUTER_THREADS_MULTIPLIER  = 4; // i.e thread block is 32 x 4*192/32
static constexpr unsigned int MAX_NUMBER_DOUBLETS       = 1500;
static constexpr unsigned int NUM_TRIPLET_BLOCKS        = 1024;
static constexpr unsigned int NUM_TRIPLET_THREADS       = 1024;
static constexpr unsigned int NUM_DOUBLET_THREADS       = 16;
static constexpr unsigned int MAX_TRIPLETS              = 300;
static constexpr unsigned int TRIPLET_BUFFER_DEPTH      = 3;

typedef struct doubletInfo {
public:
  int m_nInner[MAX_MIDDLE_SP];
  int m_nOuter[MAX_MIDDLE_SP];
  char m_good[MAX_MIDDLE_SP];
} DOUBLET_INFO;

typedef struct doubletStorage {
public:
  int m_nItems;
  int m_nI;
  int m_nO;
  int m_spmIdx[MAX_MIDDLE_SP];  
  int m_innerStart[MAX_MIDDLE_SP];
  int m_outerStart[MAX_MIDDLE_SP];
  int m_inner[MAX_DOUBLET];
  int m_outer[MAX_DOUBLET];
} DOUBLET_STORAGE;

#include "CommonStructures.h"


#endif
