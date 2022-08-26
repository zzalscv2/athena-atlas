//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//


#ifndef CALORECGPU_BASICGPUCLUSTERINFOCALCULATOR_H
#define CALORECGPU_BASICGPUCLUSTERINFOCALCULATOR_H

#include "CxxUtils/checker_macros.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "BasicGPUClusterInfoCalculatorImpl.h"

/**
 * @class BasicGPUClusterInfoCalculator
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 11 August 2022
 * @brief Standard tool to calculate cluster info (energy, transverse energy, pseudo-rapidity and azimuthal angle).
 */


class BasicGPUClusterInfoCalculator:
  public AthAlgTool, virtual public CaloClusterGPUProcessor, public CaloGPUTimed
{
 public:

  BasicGPUClusterInfoCalculator(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode execute (const EventContext & ctx, const ConstantDataHolder & constant_data, EventDataHolder & event_data) const override;

  virtual StatusCode finalize() override;

  virtual ~BasicGPUClusterInfoCalculator();

 private:

  /**
   * @brief Number of events for which to pre-allocate space on GPU memory
   * (should ideally be set to the expected number of threads to be run with).
   *
   */
  Gaudi::Property<size_t> m_numPreAllocatedGPUData {this, "NumPreAllocatedDataHolders", 0, "Number of temporary data holders to pre-allocate on GPU memory"};

  /** @brief A way to reduce allocations over multiple threads by keeping a cache
  *   of previously allocated objects that get assigned to the threads as they need them.
  *   It's all thread-safe due to an internal mutex ensuring no objects get assigned to different threads.
  */
  mutable CaloRecGPU::Helpers::separate_thread_holder<BasicGPUClusterInfoCalculatorTemporariesHolder> m_temporariesHolder ATLAS_THREAD_SAFE;
};

#endif //CALORECGPU_TOPOAUTOMATONCLUSTERING_H