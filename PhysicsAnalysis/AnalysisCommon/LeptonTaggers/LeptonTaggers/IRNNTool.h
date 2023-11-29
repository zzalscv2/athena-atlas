// This is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATH_IRNNTOOL_H
#define ATH_IRNNTOOL_H

/**********************************************************************************
 * @Package: PhysicsAnpProd
 * @Class  : IRNNTool
 * @Author : Rustem Ospanov
 *
 * @Brief  :
 *
 * Interface for tool to compute RNN output
 *
 **********************************************************************************/

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// C/C++
#include <set>

namespace Prompt
{
  // Forward declarations
  class VarHolder;

  // Main body

  static const InterfaceID IID_IRNNTool ( "Prompt::IRNNTool", 1, 0 );

  class IRNNTool : public virtual IAlgTool
  {
    /*
      Interface of the RNN tool, which will take the input of VarHolder vector and predict RNN score for different categories.
    */
  public:

    static const InterfaceID& interfaceID() { return IID_IRNNTool; }

    virtual StatusCode initialize() override = 0;

    virtual std::map<std::string, double> computeRNNOutput(
      const std::vector<Prompt::VarHolder> &tracks
    ) = 0;

    virtual std::set<std::string> getOutputLabels() const = 0;
   };
}

#endif
