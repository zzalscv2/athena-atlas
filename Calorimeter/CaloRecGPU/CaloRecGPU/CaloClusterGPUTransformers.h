/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_CALOCLUSTERGPUTRANSFORMERS_H
#define CALORECGPU_CALOCLUSTERGPUTRANSFORMERS_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloRecGPU/DataHolders.h"

/**
 * @class  ICaloClusterGPUConstantTransformer
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 27 May 2022
 * @brief Base class for tools that convert constant information
 * to the GPU-friendly format used in @c CaloGPUHybridClusterProcessor.
 *
 * This class defines a @c convert method that takes as an argument
 * a @c ConstantDataHolder.
 */

class ICaloClusterGPUConstantTransformer : virtual public IAlgTool
{
 public:

  /**
   * @brief Fill the @ConstantDataHolder with the relevant information.
   * @param constant_data Data held in GPU memory that is common to all events (cell noise and geometry).
   */
  virtual StatusCode convert (ConstantDataHolder & constant_data) const = 0;
  
  /**
   * @brief Fill the @ConstantDataHolder with the relevant information at the first event.
   * @param ctx The event context.
   * @param constant_data Data held in GPU memory that is common to all events (cell noise and geometry).
   *
   * Unless otherwise specified by the tool, this does exactly the same as @c convert(constant_data)
   */
  inline virtual StatusCode convert (const EventContext &, ConstantDataHolder & constant_data) const
  {
    return this->convert(constant_data);
  }

  DeclareInterfaceID( ICaloClusterGPUConstantTransformer, 1, 0);
};

/**
 * @class  ICaloClusterGPUInputTransformer
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 27 May 2022
 * @brief Base class for tools that convert event information from the Athena structures
 * to the GPU-friendly format used in @c CaloGPUHybridClusterProcessor.
 *
 * This class defines a @c convert method that takes as an argument
 * a @c EventContext, a @c ConstantDataHolder, a @xAOD::CaloClusterContainer and a @c EventDataHolder.
 */

class ICaloClusterGPUInputTransformer : virtual public IAlgTool
{
 public:

  /**
   * @brief Fill the @EventDataHolder with the relevant information.
   * @param ctx The event context.
   * @param constant_data Data held in GPU memory that is common to all events (cell noise and geometry).
   * @param cluster_collection The cluster collection, in the standard Athena structures.
   * @param event_data Data held in GPU memory that is specific to this event (including the description of the clusters themselves).
   */
  virtual StatusCode convert (const EventContext & ctx, const ConstantDataHolder & constant_data,
                              const xAOD::CaloClusterContainer * cluster_collection, EventDataHolder & event_data) const = 0;

  DeclareInterfaceID( ICaloClusterGPUInputTransformer, 1, 0);
};

/**
 * @class  ICaloClusterGPUOutputTransformer
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 27 May 2022
 * @brief Base class for tools that convert event information from the GPU-friendly format
 * used in @c CaloGPUHybridClusterProcessor to the Athena structures.
 *
 * This class defines a @c convert method that takes as an argument
 * a @c EventContext, a @c ConstantDataHolder, a @c EventDataHolder and a @xAOD::CaloClusterContainer.
 */

class ICaloClusterGPUOutputTransformer : virtual public IAlgTool
{
 public:

  /**
   * @brief Fill the @xAOD::CaloClusterContainer with the relevant information.
   * @param ctx The event context.
   * @param constant_data Data held in GPU memory that is common to all events (cell noise and geometry).
   * @param event_data Data held in GPU memory that is specific to this event (including the description of the clusters themselves).
   * @param cluster_collection The cluster collection, in the standard Athena structures.
   */
  virtual StatusCode convert (const EventContext & ctx, const ConstantDataHolder & constant_data,
                              EventDataHolder & event_data, xAOD::CaloClusterContainer * cluster_collection) const = 0;

  DeclareInterfaceID( ICaloClusterGPUOutputTransformer, 1, 0);
};

#endif //CALORECGPU_CALOCLUSTERGPUTRANSFORMERS_H
