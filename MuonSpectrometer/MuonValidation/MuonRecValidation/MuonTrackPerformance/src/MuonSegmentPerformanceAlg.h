/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSEGMENTPERFORMANCEALG_H
#define MUONSEGMENTPERFORMANCEALG_H

#include <fstream>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODMuon/MuonSegmentContainer.h"

class MuonSegmentPerformanceAlg : public AthAlgorithm {
public:
    // Algorithm Constructor
    MuonSegmentPerformanceAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~MuonSegmentPerformanceAlg() = default;

    // Gaudi algorithm hooks
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

    unsigned int cardinality() const override final { return 1;}
private:
    bool retrieve(const SG::ReadHandleKey<xAOD::MuonSegmentContainer> &, const xAOD::MuonSegmentContainer *&ptr) const;
    std::string printRatio(const std::string& prefix, unsigned int begin, unsigned int end, const std::vector<int>& reco,
                           const std::vector<int>& truth) const;
    std::string printRatio(const std::string& prefix, unsigned int begin, unsigned int end, const std::vector<int>& reco) const;

    /** name of external file to write statistics */
    bool m_writeToFile;
    std::string m_fileName;

    /** output file*/
    std::ofstream m_fileOutput;

    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentLocation", "MuonSegments"};
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegmentKey{this, "TruthSegmentLocation", "MuonTruthSegments"};

    unsigned int m_nevents;
    std::vector<int> m_nhitCuts;
    std::vector<std::string> m_hitCutString;
    std::vector<std::vector<int> > m_ntruth;
    std::vector<std::vector<int> > m_nfound;
    std::vector<std::vector<int> > m_nfake;
};

#endif  // MUONPERFORMANCEALG
