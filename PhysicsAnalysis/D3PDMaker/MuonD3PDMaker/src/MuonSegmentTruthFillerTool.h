/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file MuonD3PDMaker/src/MuonSegmentTruthFillerTool.h
 * @author Daniel Blackburn <ventura@cern.ch>
 * @date February 2013
 * @brief Block filler tool for MuonSegmentTruth
 */

#ifndef D3PDMAKER_MuonSegmentTruthFillerTool_H
#define D3PDMAKER_MuonSegmentTruthFillerTool_H 

#include "GaudiKernel/ToolHandle.h"
#include "D3PDMakerUtils/BlockFillerTool.h"
#include "MuonPattern/MuonPatternCombination.h"
#include "TrkTruthData/SubDetHitStatistics.h"
#include "AtlasDetDescr/AtlasDetectorID.h"

#include "MuonSegment/MuonSegment.h"
#include "MuonRecToolInterfaces/IDetailedMuonPatternTruthBuilder.h"

#include <vector>


namespace D3PD {


class MuonSegmentTruthFillerTool
  : public BlockFillerTool< Trk::Segment >
{
public:
  MuonSegmentTruthFillerTool (const std::string& type,
                              const std::string& name,
                              const IInterface* parent);

  virtual StatusCode book();
  virtual StatusCode initialize();

  virtual StatusCode fill(const Trk::Segment& segment);
  virtual double deltaR(double eta1, double eta2, double phi1, double phi2);
//  virtual SubDetHitStatistics::SubDetType findSubDetType(Identifier id);

private:
  std::vector<int>* m_truth_barcode = nullptr;
  float* m_res_x = nullptr;
  float* m_res_y = nullptr;
  float* m_dAngleYZ = nullptr;
  float* m_dAngleXZ = nullptr;
  int* m_truth_nSTGC = nullptr;
  int* m_truth_nMM = nullptr;
  int* m_truth_nMDT = nullptr;
  int* m_truth_nRPC = nullptr;
  int* m_truth_nTGC = nullptr;
  int* m_truth_nCSC = nullptr;
  int* m_noise_nSTGC = nullptr;
  int* m_noise_nMM = nullptr;
  int* m_noise_nMDT = nullptr;
  int* m_noise_nRPC = nullptr;
  int* m_noise_nTGC = nullptr;
  int* m_noise_nCSC = nullptr;
  int* m_common_nSTGC = nullptr;
  int* m_common_nMM = nullptr;
  int* m_common_nMDT = nullptr;
  int* m_common_nRPC = nullptr;
  int* m_common_nTGC = nullptr;
  int* m_common_nCSC = nullptr;
  
  //int m_matchedTruth;
  std::string m_truthSGkey;

  const AtlasDetectorID *m_idHelper;
  ToolHandle<Trk::IDetailedMuonPatternTruthBuilder> m_truthTool;
};

} // namespace D3PD

#endif
