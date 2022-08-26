/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_CALOGPUHYBRIDCLUSTERPROCESSOR_H
#define CALORECGPU_CALOGPUHYBRIDCLUSTERPROCESSOR_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "CxxUtils/checker_macros.h"

#include "CaloRec/CaloClusterCollectionProcessor.h"
#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "CaloRecGPU/DataHolders.h"

#include "xAODCaloEvent/CaloClusterContainer.h"

#include <string>
#include <mutex>
#include <atomic>

/**
 * @class CaloGPUHybridClusterProcessor
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 27 May 2022
 * @brief Algorithm to reconstruct CaloCluster objects with GPU acceleration,
 * providing interoperability for calling standard CPU algorithms before and after
 * the GPU processing part.
 *
 * This class is meant as a replacement for @c CaloClusterMaker in that
 * it creates a CaloClusterCollection and runs several tools over it.
 * The main addition is the fact that, besides CPU-based tools,
 * GPU-accelerated versions of the standard tools can be run,
 * with adequate memory sharing between them to minimize transfers
 * and (re-)conversions from and to the GPU-friendly data representation.  */

class CaloGPUHybridClusterProcessor : public AthReentrantAlgorithm, public CaloGPUTimed
{
 public:

  CaloGPUHybridClusterProcessor(const std::string & name, ISvcLocator * pSvcLocator);
  virtual ~CaloGPUHybridClusterProcessor() override;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext & ctx) const override;
  virtual StatusCode execute(const EventContext & ctx, xAOD::CaloClusterContainer * cluster_collection) const;
  //This second version is so we can 'hack' this into simply processing other containers
  //via AlgToToolHelperHack for debugging purposes...
  virtual StatusCode finalize() override;

 private:

  /**
   * @brief The tool that will convert the constant data from the CPU to the GPU.
   *
   */
  ToolHandle<ICaloClusterGPUConstantTransformer> m_transformConstantData;

  /**
   * @brief Tools to be applied to the clusters before being sent to the GPU for processing.
   *
   */
  ToolHandleArray<CaloClusterCollectionProcessor> m_preGPUoperations;


  /**
   * @brief The tool that will actually convert the data from the CPU to the GPU.
   *
   */
  ToolHandle<ICaloClusterGPUInputTransformer> m_transformForGPU;

  /**
   * @brief Tools to be applied to the clusters on the GPU.
   *
   */
  ToolHandleArray<CaloClusterGPUProcessor> m_GPUoperations;

  /**
   * @brief The tool that will convert the data from the GPU back to the CPU.
   *
   */
  ToolHandle<ICaloClusterGPUOutputTransformer> m_transformBackToCPU;

  /**
   * @brief Tools to be applied to the clusters after returning from the GPU.
   *
   */
  ToolHandleArray<CaloClusterCollectionProcessor> m_postGPUoperations;

  /**
   * @brief Number of events for which to pre-allocate space on GPU memory
   * (should ideally be set to the expected number of threads to be run with).
   *
   */
  Gaudi::Property<size_t> m_numPreAllocatedGPUData{this, "NumPreAllocatedDataHolders", 0, "Number of event data holders to pre-allocate on GPU memory"};

  /** @brief The name of the key in StoreGate for the output
      CaloClusterContainer */
  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_clusterOutput;

  /** @brief The name of the key in StoreGate for the output
      CaloClusterCellLinkContainer */
  SG::WriteHandleKey<CaloClusterCellLinkContainer> m_clusterCellLinkOutput;
  
  /** @brief If @p true, the constant data is only converted and
      sent to the GPU on the first event, in case not all the necessary
      information is available during the @p initialize phase.
      */
  Gaudi::Property<bool> m_deferConstantDataToFirstEvent {this, "DeferConstantDataPreparationToFirstEvent", true, "Convert and send event data on first event instead of during initialize (needed for exporting geometry and noise properly?)"};

  /** @brief A way to reduce allocations over multiple threads by keeping a cache
  *   of previously allocated objects that get assigned to the threads as they need them.
  *   It's all thread-safe due to an internal mutex ensuring no objects get assigned to different threads.
  */
  mutable CaloRecGPU::Helpers::separate_thread_holder<EventDataHolder> m_eventDataThreadedHolder ATLAS_THREAD_SAFE;

  /** @brief Constant data, common for all events and persisted throughout the run.
   *   
   *  Is @p mutable to deal with the cases where the data preparation is deferred to the first event.
   */

  mutable ConstantDataHolder m_constantData ATLAS_THREAD_SAFE;
  
  /** @brief A flag to signal that the constant data has been adequately sent to the GPU.
   *  This is required for everything to work properly in a multi-threaded context...
   */
  
  mutable std::atomic<bool> m_constantDataSent;
  
  
  /** @brief This mutex is locked when sending the constant data on the first event
    * to ensure thread safety. Otherwise, it's unused.
    */
  mutable std::mutex m_mutex;
  
};

#endif //CALORECGPU_CALOGPUHYBRIDCLUSTERPROCESSOR_H
