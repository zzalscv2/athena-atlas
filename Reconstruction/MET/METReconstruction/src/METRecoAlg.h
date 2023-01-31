/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// METRecoAlg.h

#ifndef METRecoAlg_H
#define METRecoAlg_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

class IMETRecoTool;

namespace met {
  class METRecoAlg : public AthReentrantAlgorithm { 

  public: 

    /// Constructor with parameters:
    METRecoAlg(const std::string& name, ISvcLocator* pSvcLocator);

    /// Destructor:
    ~METRecoAlg(); 

    /// Athena algorithm's Hooks
    virtual StatusCode  initialize() override;
    virtual StatusCode  execute(const EventContext& ctx) const override;
    virtual StatusCode  finalize() override;

  private: 

    /// Default constructor:
    METRecoAlg();

  private:

    /// Athena configured tools
    ToolHandleArray<IMETRecoTool> m_recotools;

  }; 

}

#endif
