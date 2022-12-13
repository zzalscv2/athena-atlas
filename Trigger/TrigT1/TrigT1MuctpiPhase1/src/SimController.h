/*                                                                                                                      
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// This file is really -*- C++ -*-.
#ifndef TRIGT1MUCTPIPHASE1_SIMCONTROLLER_H
#define TRIGT1MUCTPIPHASE1_SIMCONTROLLER_H

#include <vector>
#include <list>
#include <string>

#include "Configuration.h"
#include "TrigT1MuctpiPhase1/L1TopoLUT.h"
#include "MuonSectorProcessor.h"
#include "TriggerProcessor.h"
#include "MUCTPIResults.h"

#include "TrigT1Interfaces/Lvl1MuCTPIInputPhase1.h"

namespace LVL1
{
  class MuCTPIL1Topo;
}

namespace LVL1MUCTPIPHASE1 {

  class SimController
  {
    
  public:
    typedef std::array<MuonSectorProcessor,
                       LVL1MUONIF::Lvl1MuCTPIInputPhase1::NumberOfMuonSubSystem> MuonSectorProcessors;

    SimController() = default;

    std::vector<std::string> configureTopo(const std::string& barrelFileName,
					   const std::string& ecfFileName,
					   const std::string& side0LUTFileName,
					   const std::string& side1LUTFileName);

    std::string processData(LVL1MUONIF::Lvl1MuCTPIInputPhase1* input, MUCTPIResults& results, int bcid) const;
    void setConfiguration( const Configuration& conf );

    TriggerProcessor& getTriggerProcessor() { return m_triggerProcessor; }
    MuonSectorProcessors& getMuonSectorProcessors() { return m_muonSectorProcessors; }

  private:

    bool m_doZeroSuppression{true};
    unsigned int m_threshold1Candidate{1};
    unsigned int m_threshold2Candidate{1};
    int m_suppressionMode{0};
    unsigned int m_maxCandPerPtvalue{64};
    unsigned int m_maxCandSendToRoib{14};
    unsigned int m_candBcidOffset{0};
    std::vector< std::vector< unsigned int > > m_ptSorterBuckets;

    L1TopoLUT m_l1topoLUT;

    TriggerProcessor m_triggerProcessor;
    MuonSectorProcessors m_muonSectorProcessors{
      MuonSectorProcessor(LVL1MUONIF::Lvl1MuCTPIInputPhase1::idSideA()),
      MuonSectorProcessor(LVL1MUONIF::Lvl1MuCTPIInputPhase1::idSideC())};
  };

}

#endif // TRIGT1MUCTPIPHASE1_SIMCONTROLLER_H
