/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file MuonD3PDMaker/src/MuonPatternCombinationFillerTool.h
 * @author Daniel Ventura <ventura@cern.ch>
 * @date July 2012
 * @brief Block filler tool for MuonPatternCombinations, detail level 1
 */

#ifndef D3PDMAKER_MUONPATTERNCOMBINATIONFILLERTOOL_H
#define D3PDMAKER_MUONPATTERNCOMBINATIONFILLERTOOL_H 

#include "GaudiKernel/ToolHandle.h"
#include "D3PDMakerUtils/BlockFillerTool.h"
#include "MuonPattern/MuonPatternCombination.h"
#include "MuonRecToolInterfaces/IDetailedMuonPatternTruthBuilder.h"
#include "TrkTruthData/SubDetHitStatistics.h"
#include "AtlasDetDescr/AtlasDetectorID.h"

#include <vector>

namespace D3PD {


class MuonPatternCombinationFillerTool
  : public BlockFillerTool< Muon::MuonPatternCombination >
{
public:
  MuonPatternCombinationFillerTool (const std::string& type,
                        const std::string& name,
                        const IInterface* parent);

  virtual StatusCode book();
  virtual StatusCode initialize();

  virtual StatusCode fill(const Muon::MuonPatternCombination& pattern);
  virtual double deltaR(double eta1, double eta2, double phi1, double phi2);
  virtual SubDetHitStatistics::SubDetType findSubDetType(Identifier id);

private:
  float* m_pattern_gpos_eta = nullptr;
  float* m_pattern_gpos_phi = nullptr;
  float* m_pattern_gpos_r = nullptr;
  float* m_pattern_gpos_z = nullptr;
  float* m_pattern_gdir_x = nullptr;
  float* m_pattern_gdir_y = nullptr;
  float* m_pattern_gdir_z = nullptr;
  int* m_pattern_nMDT = nullptr;
  int* m_pattern_nRPC = nullptr;
  int* m_pattern_nTGC = nullptr;
  int* m_pattern_nCSC = nullptr;
  std::vector<std::vector<int> >* m_truth_barcode = nullptr;
  std::vector<int>* m_truth_nMDT = nullptr;
  std::vector<int>* m_truth_nRPC = nullptr;
  std::vector<int>* m_truth_nTGC = nullptr;
  std::vector<int>* m_truth_nCSC = nullptr;
  std::vector<int>* m_noise_nMDT = nullptr;
  std::vector<int>* m_noise_nRPC = nullptr;
  std::vector<int>* m_noise_nTGC = nullptr;
  std::vector<int>* m_noise_nCSC = nullptr;
  std::vector<int>* m_common_nMDT = nullptr;
  std::vector<int>* m_common_nRPC = nullptr;
  std::vector<int>* m_common_nTGC = nullptr;
  std::vector<int>* m_common_nCSC = nullptr;
  
  //int m_matchedTruth;
  std::string m_truthSGkey;
  std::string m_patternKey;

  const AtlasDetectorID *m_idHelper;
  ToolHandle<Trk::IDetailedMuonPatternTruthBuilder> m_truthTool;
};

} // namespace D3PD

#endif
