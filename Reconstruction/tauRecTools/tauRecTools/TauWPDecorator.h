/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUREC_TAUWPDECORATOR_H
#define TAUREC_TAUWPDECORATOR_H

#include "tauRecTools/TauRecToolBase.h"

#include "xAODTau/TauDefs.h"
#include "xAODEventInfo/EventInfo.h"
#include "AsgDataHandles/ReadHandleKey.h"

#include <map>

class TH2;

/**
 * @brief Implementation of tool to decorate flattened BDT score and working points
 * 
 *  Input comes from 2 ROOT files with lists of TH2s containing BDTScore distributions
 *  as a function of the dependent variables
 *
 * @author P.O. DeViveiros
 * @author W. Davey
 * @author L. Hauswald
 *                                                                              
 */

class TauWPDecorator : public TauRecToolBase {
  public:
    ASG_TOOL_CLASS2(TauWPDecorator, TauRecToolBase, ITauToolBase)
    TauWPDecorator(const std::string& name="TauWPDecorator");
    ~TauWPDecorator();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(xAOD::TauJet& pTau) const override;

  private:
    StatusCode retrieveHistos(int nProng);
    StatusCode storeLimits(int nProng);
    double transformScore(double score, double cutLow, double effLow, double cutHigh, double effHigh) const;

    bool m_electronMode;
    bool m_defineWPs;
    std::string m_scoreName;
    std::string m_scoreNameTrans;

    std::string m_file0p;
    std::string m_file1p;
    std::string m_file3p;
    
    std::vector<int> m_EDMWPs;
    std::vector<float> m_EDMWPEffs0p;
    std::vector<float> m_EDMWPEffs1p;
    std::vector<float> m_EDMWPEffs3p;

    std::vector<std::string> m_decorWPs;
    std::vector<float> m_decorWPEffs0p;
    std::vector<float> m_decorWPEffs1p;
    std::vector<float> m_decorWPEffs3p;
    
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this,"Key_eventInfo", "EventInfo", "EventInfo key"};
    
    typedef std::pair<double, std::unique_ptr<TH2> > m_pair_t;

    std::unique_ptr<std::vector<m_pair_t>> m_hists0p;
    std::unique_ptr<std::vector<m_pair_t>> m_hists1p;
    std::unique_ptr<std::vector<m_pair_t>> m_hists3p;
    
    std::map<int, double> m_xMin;
    std::map<int, double> m_yMin;
    std::map<int, double> m_xMax;
    std::map<int, double> m_yMax;
};

#endif
