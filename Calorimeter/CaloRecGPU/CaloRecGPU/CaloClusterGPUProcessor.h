/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

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
 * a @c EventContext, a @c ConstantDataHolder and a @c EventDataHolder.
 */

class CaloClusterGPUProcessor : virtual public IAlgTool
{
 public:

  /**
   * @brief Process the clusters on GPU.
   * @param ctx The event context.
   * @param constant_data Data held in GPU memory that is common to all events (cell noise and geometry).
   * @param event_data Data held in GPU memory that is specific to this event (including the description of the clusters themselves).
   */
  virtual StatusCode execute (const EventContext & ctx, const ConstantDataHolder & constant_data, EventDataHolder & event_data) const = 0;
  
  DeclareInterfaceID( CaloClusterGPUProcessor, 1, 0);

};


#endif //CALORECGPU_CALOCLUSTERGPUPROCESSOR_H
