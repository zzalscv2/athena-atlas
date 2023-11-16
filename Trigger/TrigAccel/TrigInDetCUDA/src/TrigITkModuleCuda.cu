/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <cuda.h>
#include <cuda_runtime.h>
#include <atomic>

#include "device_context.h"
#include "TrigITkModuleCuda.h"
#include "SeedMakingDataStructures_ITk.h"
#include "SeedMakingWorkCuda_ITk.h"

#include "TrigAccelEvent/TrigInDetAccelCodes.h"
#include "gpu_helpers.h"

#include <sstream>


TrigITkModuleCuda::TrigITkModuleCuda() : m_maxDevice(0), m_dumpTimeLine(false) {

  m_h_detmodel = 0;

  m_maxDevice = GPUHelpers::getNumberOfGPUs();

  for(unsigned int i=0;i<getProvidedAlgs().size();i++) {
    m_workItemCounters[i] = 0;
  }
  
  m_h_detmodel = (unsigned char*) malloc(sizeof(TrigAccel::ITk::DETECTOR_MODEL));

  m_timeLine.clear();

}

TrigITkModuleCuda::~TrigITkModuleCuda() {
  
  free(m_h_detmodel);
  m_h_detmodel = 0;
  if(m_dumpTimeLine) {
    if(m_timeLine.size() > 0) {
       tbb::tick_count t0 = m_timeLine[0].m_time;
       std::ostringstream fileName;
       fileName <<"timeLine_"<<getpid()<<".csv";
       std::ofstream tl(fileName.str());
       tl<<"workId,eventType,time"<<std::endl;
       tl<<m_timeLine[0].m_workId<<","<<m_timeLine[0].m_eventType<<",0"<<std::endl;
       for(unsigned int tIdx = 1;tIdx < m_timeLine.size();++tIdx) {
          tbb::tick_count t1 = m_timeLine[tIdx].m_time;
          auto duration = t1-t0;
          tl<<m_timeLine[tIdx].m_workId<<","<<m_timeLine[tIdx].m_eventType<<","<<1000*duration.seconds()<<std::endl;
       }
       tl.close();
      m_timeLine.clear();
    }
 }
}

bool TrigITkModuleCuda::configure() {

  if(m_maxDevice == 0) {
     std::cout<<"No CUDA devices found"<<std::endl;
     return false;
  }
  return true;
}

SeedMakingDeviceContext* TrigITkModuleCuda::createSeedMakingContext(int id) const {

  cudaSetDevice(id);
  checkError(11);
  SeedMakingDeviceContext* p = new SeedMakingDeviceContext;

  p->m_deviceId = id;

  //set stream

  cudaStreamCreate(&p->m_stream);
  checkError(12);
  //check device property

  cudaDeviceProp deviceProp;
  cudaGetDeviceProperties(&deviceProp, id);

  p->m_gpuParams.m_nSMX = deviceProp.multiProcessorCount;

  int ncores = GPUHelpers::getNumberOfCores(deviceProp.major, deviceProp.minor);
  
  p->m_gpuParams.m_nNUM_SMX_CORES = ncores;//_ConvertSMVer2Cores_local(deviceProp.major, deviceProp.minor);
  p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = NUM_TRIPLET_BLOCKS_ITk;
  if(deviceProp.maxThreadsPerBlock < p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS) 
    p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = deviceProp.maxThreadsPerBlock;

  //Allocate memory
  
  cudaMalloc((void **)&p->d_settings,    sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS));
  cudaMalloc((void **)&p->d_spacepoints, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE));
  cudaMalloc((void **)&p->d_detmodel,    sizeof(TrigAccel::ITk::DETECTOR_MODEL));
  checkError();
  cudaMalloc((void **)&p->d_outputseeds, sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));
  cudaMalloc((void **)&p->d_doubletstorage, sizeof(DOUBLET_STORAGE_ITk));
  cudaMalloc((void **)&p->d_doubletinfo, sizeof(DOUBLET_INFO_ITk));
  checkError(13);

  p->d_size = sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS) +  
              sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE) + sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE) + sizeof(DOUBLET_STORAGE_ITk) + sizeof(DOUBLET_INFO_ITk) + 
              sizeof(TrigAccel::ITk::DETECTOR_MODEL);
  
  cudaMallocHost((void **)&p->h_settings, sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS));
  cudaMallocHost((void **)&p->h_spacepoints, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE));
  cudaMallocHost((void **)&p->h_outputseeds, sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));

  p->h_size = sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS) + sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE) + sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE);
  
  checkError(14);
  return p;
}

SeedMakingManagedDeviceContext* TrigITkModuleCuda::createManagedSeedMakingContext(int id) const {

  cudaSetDevice(id);
  checkError(11);
  SeedMakingManagedDeviceContext* p = new SeedMakingManagedDeviceContext;

  p->m_deviceId = id;

  //set stream

  cudaStreamCreate(&p->m_stream);
  checkError(12);
  //check device property

  cudaDeviceProp deviceProp;
  cudaGetDeviceProperties(&deviceProp, id);

  p->m_gpuParams.m_nSMX = deviceProp.multiProcessorCount;

  int ncores = GPUHelpers::getNumberOfCores(deviceProp.major, deviceProp.minor);

  p->m_gpuParams.m_nNUM_SMX_CORES = ncores;//_ConvertSMVer2Cores_local(deviceProp.major, deviceProp.minor);
  p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = NUM_TRIPLET_BLOCKS_ITk;
  if(deviceProp.maxThreadsPerBlock < p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS) 
    p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = deviceProp.maxThreadsPerBlock;

  //Allocate memory
  
  cudaMallocManaged((void **)&p->m_settings,    sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS));
  cudaMallocManaged((void **)&p->m_spacepoints, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE));
  cudaMallocManaged((void **)&p->m_outputseeds, sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));

  cudaMalloc((void **)&p->d_detmodel,    sizeof(TrigAccel::ITk::DETECTOR_MODEL));
  checkError();
  cudaMalloc((void **)&p->d_doubletstorage, sizeof(DOUBLET_STORAGE_ITk));
  cudaMalloc((void **)&p->d_doubletinfo, sizeof(DOUBLET_INFO_ITk));
  checkError(13);

  p->d_size = sizeof(DOUBLET_STORAGE_ITk) + sizeof(DOUBLET_INFO_ITk) + sizeof(TrigAccel::ITk::DETECTOR_MODEL);
  
  p->h_size = 0;
  
  p->m_size = sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS) + sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE) + sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE);

  checkError(14);
  return p;
}

TrigAccel::Work* TrigITkModuleCuda::createWork(int workType, std::shared_ptr<TrigAccel::OffloadBuffer> data){
  
  if(workType == TrigAccel::InDetJobControlCode::SIL_LAYERS_EXPORT){

    memcpy(m_h_detmodel, (unsigned char*)data->get(), sizeof(TrigAccel::ITk::DETECTOR_MODEL));

    return 0;
  }

  if(workType == TrigAccel::InDetJobControlCode::FIND_SEEDS){
 
    int deviceId = 0;//always using device 0 for the time being

    //TO-DO: to support mult-GPU load balancing get deviceId from a tbb_concurrent_queue

    SeedMakingDeviceContext* ctx = createSeedMakingContext(deviceId);

    cudaMemcpy(ctx->d_detmodel, m_h_detmodel, sizeof(TrigAccel::ITk::DETECTOR_MODEL), cudaMemcpyHostToDevice);
    checkError(21);
    TrigAccel::ITk::SEED_MAKING_JOB *pArray = reinterpret_cast<TrigAccel::ITk::SEED_MAKING_JOB*>(data->get());
    
    //1. copy settings to the context host array

    TrigAccel::ITk::SEED_FINDER_SETTINGS* p_settings = reinterpret_cast<TrigAccel::ITk::SEED_FINDER_SETTINGS*>(ctx->h_settings);
    memcpy(p_settings, &pArray->m_settings, sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS));

    //2. copy spacepoints to the context host array

    TrigAccel::ITk::SPACEPOINT_STORAGE* p_spacePoints = reinterpret_cast<TrigAccel::ITk::SPACEPOINT_STORAGE*>(ctx->h_spacepoints);
    memcpy(p_spacePoints, &pArray->m_data, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE));
    
    unsigned int workNum = m_workItemCounters[0]++;//seed making uses counter #0
    
    unsigned int workId = workNum*100;
    
    SeedMakingWorkCudaITk* w = new SeedMakingWorkCudaITk(workId, ctx, data, &m_timeLine);
    
    return w;
  }

  if(workType == TrigAccel::InDetJobControlCode::MAKE_SEEDS){
 
    int deviceId = 0;//always using device 0 for the time being

    //TO-DO: to support mult-GPU load balancing get deviceId from a tbb_concurrent_queue

    SeedMakingManagedDeviceContext* ctx = createManagedSeedMakingContext(deviceId);

    cudaMemcpy(ctx->d_detmodel, m_h_detmodel, sizeof(TrigAccel::ITk::DETECTOR_MODEL), cudaMemcpyHostToDevice);//TO-DO: try CoW here
    checkError(21);
    TrigAccel::ITk::SEED_MAKING_JOB *pArray = reinterpret_cast<TrigAccel::ITk::SEED_MAKING_JOB*>(data->get());
    
    //1. copy settings to the context host array

    TrigAccel::ITk::SEED_FINDER_SETTINGS* p_settings = reinterpret_cast<TrigAccel::ITk::SEED_FINDER_SETTINGS*>(ctx->m_settings);
    memcpy(p_settings, &pArray->m_settings, sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS));

    //2. copy spacepoints to the context host array

    TrigAccel::ITk::SPACEPOINT_STORAGE* p_spacePoints = reinterpret_cast<TrigAccel::ITk::SPACEPOINT_STORAGE*>(ctx->m_spacepoints);
    memcpy(p_spacePoints, &pArray->m_data, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE));
    
    unsigned int workNum = m_workItemCounters[0]++;//seed making uses counter #0
    
    unsigned int workId = workNum*100;
    
    SeedMakingWorkCudaManagedITk* w = new SeedMakingWorkCudaManagedITk(workId, ctx, data, &m_timeLine);
    
    return w;
  }

  return 0;
}

const std::vector<int> TrigITkModuleCuda::getProvidedAlgs(){
  std::vector<int> v{
      TrigAccel::InDetJobControlCode::SIL_LAYERS_EXPORT,
      TrigAccel::InDetJobControlCode::MAKE_SEEDS,//the default
      TrigAccel::InDetJobControlCode::FIND_SEEDS //the alternative
  };
  return v;
}

