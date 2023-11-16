/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_SEEDMAKINGWORKCUDA_ITK_H
#define TRIGINDETCUDA_SEEDMAKINGWORKCUDA_ITK_H

#include <vector>

#include "tbb/concurrent_vector.h"
#include "device_context.h"
#include "TrigAccelEvent/Work.h"
#include "TrigAccelEvent/WorkFactory.h"


class SeedMakingWorkCudaITk : public TrigAccel::Work{

public:
  SeedMakingWorkCudaITk(unsigned int, SeedMakingDeviceContext*, std::shared_ptr<TrigAccel::OffloadBuffer>, 
  tbb::concurrent_vector<WorkTimeStamp>*);
  ~SeedMakingWorkCudaITk();
  std::shared_ptr<TrigAccel::OffloadBuffer> getOutput();
  bool run();
  unsigned int getId() const {
    return m_workId;
  }
  
private:

  inline void checkError() const {
    cudaError_t error = cudaGetLastError();
    if(error != cudaSuccess) {
      printf("CUDA error: %s\n", cudaGetErrorString(error));
      exit(-1);
    }
  };
  
  unsigned int m_workId;
  SeedMakingDeviceContext* m_context;  
  std::shared_ptr<TrigAccel::OffloadBuffer> m_input, m_output;
  tbb::concurrent_vector<WorkTimeStamp>* m_timeLine;

  float m_CovMS, m_ptCoeff, m_minPt2, m_ptCoeff2, m_maxD0; 
};

class SeedMakingWorkCudaManagedITk : public TrigAccel::Work{

public:
  SeedMakingWorkCudaManagedITk(unsigned int, SeedMakingManagedDeviceContext*, std::shared_ptr<TrigAccel::OffloadBuffer>, 
  tbb::concurrent_vector<WorkTimeStamp>*);
  ~SeedMakingWorkCudaManagedITk();
  std::shared_ptr<TrigAccel::OffloadBuffer> getOutput();
  bool run();
  unsigned int getId() const {
    return m_workId;
  }
  
private:

  inline void checkError() const {
    cudaError_t error = cudaGetLastError();
    if(error != cudaSuccess) {
      printf("CUDA error: %s\n", cudaGetErrorString(error));
      exit(-1);
    }
  };
  
  unsigned int m_workId;
  SeedMakingManagedDeviceContext* m_context;  
  std::shared_ptr<TrigAccel::OffloadBuffer> m_input, m_output;
  tbb::concurrent_vector<WorkTimeStamp>* m_timeLine;

  float m_CovMS, m_ptCoeff, m_minPt2, m_ptCoeff2, m_maxD0; 
};

#endif
