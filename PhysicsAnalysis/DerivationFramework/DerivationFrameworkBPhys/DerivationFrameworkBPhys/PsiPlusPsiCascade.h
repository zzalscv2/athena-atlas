/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PSIPLUSPSICASCADE_H
#define PSIPLUSPSICASCADE_H
// Xin Chen <xin.chen@cern.ch>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "xAODTracking/VertexContainer.h"
#include <vector>
#include "xAODEventInfo/EventInfo.h"

namespace Trk {
    class IVertexFitter;
    class TrkVKalVrtFitter;
    class IVertexCascadeFitter;
    class VxCascadeInfo;
    class V0Tools;
    class ParticleDataTable;
}

namespace DerivationFramework {
    class CascadeTools;
}

namespace DerivationFramework {

  static const InterfaceID IID_PsiPlusPsiCascade("PsiPlusPsiCascade", 1, 0);

  class PsiPlusPsiCascade : virtual public AthAlgTool, public IAugmentationTool
  {
    typedef ElementLink<xAOD::VertexContainer> VertexLink;
  public:
    static const InterfaceID& interfaceID() { return IID_PsiPlusPsiCascade;}
    PsiPlusPsiCascade(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~PsiPlusPsiCascade() = default;
    virtual StatusCode initialize() override;
    StatusCode performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer_noConstr) const;
    virtual StatusCode addBranches() const override;

  private:
    std::string m_vertexPsi1ContainerKey;
    std::string m_vertexPsi2ContainerKey;
    std::vector<std::string> m_vertexPsi1HypoNames;
    std::vector<std::string> m_vertexPsi2HypoNames;
    std::vector<std::string> m_cascadeOutputsKeys;
    std::string m_VxPrimaryCandidateName;   //!< Name of primary vertex container

    double m_jpsi1MassLower;
    double m_jpsi1MassUpper;
    double m_jpsi2MassLower;
    double m_jpsi2MassUpper;
    double m_diTrack1MassLower;
    double m_diTrack1MassUpper;
    double m_diTrack2MassLower;
    double m_diTrack2MassUpper;
    double m_psi1MassLower;
    double m_psi1MassUpper;
    double m_psi2MassLower;
    double m_psi2MassUpper;
    double m_MassLower;
    double m_MassUpper;
    int    m_vtx1Daug_num;
    double m_vtx1Daug1MassHypo; // mass hypothesis of 1st daughter from vertex 1
    double m_vtx1Daug2MassHypo; // mass hypothesis of 2nd daughter from vertex 1
    double m_vtx1Daug3MassHypo; // mass hypothesis of 3rd daughter from vertex 1
    double m_vtx1Daug4MassHypo; // mass hypothesis of 4th daughter from vertex 1
    int    m_vtx2Daug_num;
    double m_vtx2Daug1MassHypo; // mass hypothesis of 1st daughter from vertex 2
    double m_vtx2Daug2MassHypo; // mass hypothesis of 2nd daughter from vertex 2
    double m_vtx2Daug3MassHypo; // mass hypothesis of 3rd daughter from vertex 2
    double m_vtx2Daug4MassHypo; // mass hypothesis of 4th daughter from vertex 2

    double m_massPsi1;
    double m_massPsi2;
    double m_massJpsi1;
    double m_massJpsi2;
    double m_massDiTrk1;
    double m_massDiTrk2;
    bool   m_constrPsi1;
    bool   m_constrPsi2;
    bool   m_constrJpsi1;
    bool   m_constrJpsi2;
    bool   m_constrDiTrk1;
    bool   m_constrDiTrk2;
    double m_chi2cut_Psi1;
    double m_chi2cut_Psi2;
    double m_chi2cut;
    bool   m_removeDuplicatePairs;
    unsigned int m_maxPsi1Candidates;
    unsigned int m_maxPsi2Candidates;

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};
    ToolHandle < Trk::TrkVKalVrtFitter >             m_iVertexFitter;
    ToolHandle < Analysis::PrimaryVertexRefitter >   m_pvRefitter;
    ToolHandle < Trk::V0Tools >                      m_V0Tools;
    ToolHandle < DerivationFramework::CascadeTools > m_CascadeTools;

    bool        m_refitPV;
    std::string m_refPVContainerName;
    std::string m_hypoName;
    int         m_PV_max;
    int         m_DoVertexType;
    size_t      m_PV_minNTracks;
  };
}

#endif
