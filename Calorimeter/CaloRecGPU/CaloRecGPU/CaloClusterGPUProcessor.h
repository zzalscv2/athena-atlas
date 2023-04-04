//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOCLUSTERGPUPROCESSOR_H
#define CALORECGPU_CALOCLUSTERGPUPROCESSOR_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "CaloRecGPU/DataHolders.h"

/**
 * @class  CaloClusterGPUProcessor
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 27 May 2022
 * @brief Base class for GPU-accelerated cluster processing tools
 * to be called from @c CaloGPUHybridClusterProcessor.
 *
 * This class defines an @c execute method that takes as an argument
 * a @c EventContext, a @c CaloRecGPU::ConstantDataHolder and a @c CaloRecGPU::EventDataHolder.
 */

class CaloClusterGPUProcessor : virtual public IAlgTool
{
 public:

  /**
   * @brief Process the clusters on GPU.
   * @param ctx The event context.
   * @param constant_data Data held in GPU memory that is common to all events (cell noise and geometry).
   * @param event_data Data held in GPU memory that is specific to this event (including the description of the clusters themselves).
   * @param temporary_buffer A pointer to an \array in GPU memory that is at least as large as @p size_of_temporaries(),
   *        to hold temporary information for the algorithms. Given the way CUDA memory allocations work,
   *        casting this to a pointer (or a @p CaloRecGPU::Helpers::CUDA_kernel_object) to the intended type
   *        and then using it will be perfectly valid.
   */
  virtual StatusCode execute (const EventContext & ctx,
                              const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data,
                              void * temporary_buffer) const = 0;

  /**
    @brief The size (in bytes) of the temporary object(s) that the algorithm will use.
  */
  virtual size_t size_of_temporaries() const
  {
    return 0;
  };

  DeclareInterfaceID( CaloClusterGPUProcessor, 1, 0);

};


#endif //CALORECGPU_CALOCLUSTERGPUPROCESSOR_H
