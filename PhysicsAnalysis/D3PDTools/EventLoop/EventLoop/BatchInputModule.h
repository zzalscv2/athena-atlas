/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef EVENT_LOOP__BATCH_INPUT_MODULE_H
#define EVENT_LOOP__BATCH_INPUT_MODULE_H

#include <EventLoop/Module.h>

namespace EL
{
  struct BatchSample;
  struct BatchSegment;

  namespace Detail
  {
    /// @brief the @ref IInputModule implementation for the batch driver

    class BatchInputModule final : public Module
    {
      /// Public Members
      /// ==============

    public:

      BatchSample *sample = nullptr;
      BatchSegment *segment = nullptr;



      /// Inherited Members
      /// =================

    public:

      StatusCode processInputs (ModuleData& data, IInputModuleActions& actions) override;
    };
  }
}

#endif
