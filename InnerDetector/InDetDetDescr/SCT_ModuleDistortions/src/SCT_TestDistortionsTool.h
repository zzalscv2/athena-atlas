/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_TestDistortionsTool_h
#define SCT_TestDistortionsTool_h

#include "AthenaBaseComps/AthAlgorithm.h"

//package includes
#include "SCT_ModuleDistortions/ISCT_ModuleDistortionsTool.h"

//Gaudi
#include "GaudiKernel/ToolHandle.h"

//STL
#include <string>

//ROOT
#include "TH2F.h"
#include "TH3F.h"

class SCT_TestDistortionsTool:public AthAlgorithm {
 public:
  SCT_TestDistortionsTool(const std::string& name, ISvcLocator* pSvcLocator);
  ~SCT_TestDistortionsTool() = default;
  // Standard Gaudi functions
  virtual StatusCode initialize() override; //!< Gaudi initialiser
  virtual StatusCode execute() override;    //!< Gaudi executer
  virtual StatusCode finalize() override;   //!< Gaudi finaliser
    
 private:
  ToolHandle<ISCT_ModuleDistortionsTool> m_SCTDistoTool
    {this,"SCT_DistortionsTool","SCT_DistortionsTool",""};

  TH2F* m_ZvsX;
  TH2F* m_ZvsY;
  TH3F* m_XYZ;
  TH2F* m_outerXedge;
  TH2F* m_outerYedge;
  TH2F* m_outerX;
  TH2F* m_outerY;

};
#endif // SCT_TestDistortionsTool_h
