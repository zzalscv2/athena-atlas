/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef FPGATrackSim_DUMPOUTPUTSTATALG_H
#define FPGATrackSim_DUMPOUTPUTSTATALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "FPGATrackSimInput/IFPGATrackSimEventOutputHeaderTool.h"


class TH2F;

class FPGATrackSimDumpOutputStatAlg : public AthAlgorithm {
public:
  FPGATrackSimDumpOutputStatAlg (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~FPGATrackSimDumpOutputStatAlg () {};
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;  
  StatusCode BookHistograms();


private:
  ToolHandle<IFPGATrackSimEventOutputHeaderTool>    m_readOutputTool  { this, "InputTool",  "FPGATrackSimOutputHeaderTool/ReadOutputHeaderTool", "Input Tool" };
  ToolHandle<IFPGATrackSimEventOutputHeaderTool>    m_writeOutputTool { this, "OutputTool", "FPGATrackSimOutputHeaderTool/WriteOutputHeaderTool", "Output Tool" };
  
  // histograms
  //TH2F*   m_hits_r_vs_z = nullptr;
};

#endif // FPGATrackSim_DUMPOUTPUTSTATALG_H
