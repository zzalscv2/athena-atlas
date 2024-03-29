/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PANTAUALGS_TOOL_TAUCONSTITUENTSELECTOR
#define PANTAUALGS_TOOL_TAUCONSTITUENTSELECTOR

#include <map>
#include <vector>
#include <string>

//! ASG
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"

#include "PanTauAlgs/ITool_InformationStore.h"
#include "PanTauAlgs/ITool_TauConstituentSelector.h"
#include "PanTauAlgs/TauConstituent.h"

namespace PanTau {

  class Tool_TauConstituentSelector : public asg::AsgTool, virtual public PanTau::ITool_TauConstituentSelector  {

    ASG_TOOL_CLASS1(Tool_TauConstituentSelector, PanTau::ITool_TauConstituentSelector)

      public:
        
    Tool_TauConstituentSelector(const std::string &name);
    virtual ~Tool_TauConstituentSelector ();
        
    virtual StatusCode initialize();
        
    virtual StatusCode SelectTauConstituents(const std::vector<TauConstituent*>& inputList,
					     std::vector<TauConstituent*>& outputList) const;
        
        
  protected:
        
    virtual bool    passesSelection_NeutralConstituent(TauConstituent* tauConstituent) const;
    virtual bool    passesSelection_Pi0NeutConstituent(TauConstituent* tauConstituent) const;
    virtual bool    passesSelection_ChargedConstituent(TauConstituent* tauConstituent) const;
    virtual bool    passesSelection_OutNeutConstituent(TauConstituent* TauConstituent) const;
    virtual bool    passesSelection_OutChrgConstituent(TauConstituent* TauConstituent) const;
    virtual bool    passesSelection_NeutLowAConstituent(TauConstituent* TauConstituent) const;
    virtual bool    passesSelection_NeutLowBConstituent(TauConstituent* TauConstituent) const;
        
    virtual double  getEtCut(double eta, PanTau::TauConstituent::Type constituentType) const;
        
        
    //member variables 
    ToolHandle<PanTau::ITool_InformationStore> m_Tool_InformationStore;
    std::string  m_Tool_InformationStoreName;
        
    double                  m_MaxEta = 0.0;
    std::vector<double>     m_BinEdges_Eta;
    std::vector<double>     m_Selection_Neutral_EtaBinned_EtCut;
    std::vector<double>     m_Selection_Pi0Neut_EtaBinned_EtCut;
    std::vector<double>     m_Selection_Charged_EtaBinned_EtCut;
    std::vector<double>     m_Selection_OutNeut_EtaBinned_EtCut;
    std::vector<double>     m_Selection_OutChrg_EtaBinned_EtCut;
    std::vector<double>     m_Selection_NeutLowA_EtaBinned_EtCut;
    std::vector<double>     m_Selection_NeutLowB_EtaBinned_EtCut;

    bool m_init=false;

  public:
    inline bool isInitialized(){return m_init;}
        
  }; //end class ConstituentGetter


}//end namespace PanTau

#endif // PANTAUALGS_TOOL_TAUCONSTITUENTSELECTOR 
