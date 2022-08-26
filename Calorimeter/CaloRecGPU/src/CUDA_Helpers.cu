// Dear emacs, this is -*- c++ -*-
/*
// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloRecGPU/Helpers.h"

void * CaloRecGPU::CUDA_Helpers::allocate(const size_t num)
{
  void * ret;
  CUDA_ERRCHECK(cudaMalloc(&ret, num));
  return ret;
}

void CaloRecGPU::CUDA_Helpers::deallocate(void * address)
{
  CUDA_ERRCHECK(cudaFree(address));
}

void CaloRecGPU::CUDA_Helpers::GPU_to_CPU(void * dest, const void * const source, const size_t num)
{
  CUDA_ERRCHECK(cudaMemcpy(dest, source, num, cudaMemcpyDeviceToHost));
}

void CaloRecGPU::CUDA_Helpers::CPU_to_GPU(void * dest, const void * const source, const size_t num)
{
  CUDA_ERRCHECK(cudaMemcpy(dest, source, num, cudaMemcpyHostToDevice));
}

void CaloRecGPU::CUDA_Helpers::GPU_to_GPU(void * dest, const void * const source, const size_t num)
{
  CUDA_ERRCHECK(cudaMemcpy(dest, source, num, cudaMemcpyDeviceToDevice));
}