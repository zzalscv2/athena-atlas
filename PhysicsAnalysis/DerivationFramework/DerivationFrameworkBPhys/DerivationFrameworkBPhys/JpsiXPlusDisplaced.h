/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef JPSIXPLUSDISPLACED_H
#define JPSIXPLUSDISPLACED_H
// Xin Chen <xin.chen@cern.ch>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "xAODTracking/VertexContainer.h"
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include <vector>

namespace Trk {
    class IVertexFitter;
    class TrkV0VertexFitter;
    class TrkVKalVrtFitter;
    class IVertexCascadeFitter;
    class VxCascadeInfo;
    class V0Tools;
    class ParticleDataTable;
}

namespace DerivationFramework {
    class CascadeTools;
}
class IBeamCondSvc;

namespace DerivationFramework {

  static const InterfaceID IID_JpsiXPlusDisplaced("JpsiXPlusDisplaced", 1, 0);

  class JpsiXPlusDisplaced : virtual public AthAlgTool, public IAugmentationTool
  {
  public:
    static const InterfaceID& interfaceID() { return IID_JpsiXPlusDisplaced;}
    JpsiXPlusDisplaced(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~JpsiXPlusDisplaced() = default;
    virtual StatusCode initialize() override;
    StatusCode performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, xAOD::VertexContainer* v0VtxOutputContainer, xAOD::VertexContainer* disVtxOutputContainer) const;
    virtual StatusCode addBranches() const override;

  private:
    std::string m_vertexJXContainerKey;
    std::string m_vertexV0ContainerKey;
    std::string m_vertexDisVContainerKey;
    std::vector<std::string> m_vertexJXHypoNames;
    std::vector<std::string> m_vertexV0HypoNames;
    std::vector<std::string> m_cascadeOutputsKeys;
    bool m_refitV0;
    std::string m_v0VtxOutputsKey;
    std::string m_disVtxOutputsKey;
    std::string m_TrkParticleCollection;
    std::string m_VxPrimaryCandidateName;
    std::string m_refPVContainerName;
    std::string m_hypoName;

    double m_jxMassLower;
    double m_jxMassUpper;
    double m_jpsiMassLower;
    double m_jpsiMassUpper;
    double m_diTrackMassLower;
    double m_diTrackMassUpper;
    std::string m_V0Hypothesis;
    double m_V0MassLower;
    double m_V0MassUpper;
    double m_lxyV0_cut;
    double m_minMass_gamma;
    double m_chi2cut_gamma;
    double m_DisplacedMassLower;
    double m_DisplacedMassUpper;
    double m_lxyDisV_cut;
    double m_MassLower;
    double m_MassUpper;
    int    m_jxDaug_num;
    double m_jxDaug1MassHypo; // mass hypothesis of 1st daughter from vertex JX
    double m_jxDaug2MassHypo; // mass hypothesis of 2nd daughter from vertex JX
    double m_jxDaug3MassHypo; // mass hypothesis of 3rd daughter from vertex JX
    double m_jxDaug4MassHypo; // mass hypothesis of 4th daughter from vertex JX
    int    m_disVDaug_num;
    double m_disVDaug3MassHypo; // mass hypothesis of 3rd daughter from displaced vertex
    double m_massJX;
    double m_massJpsi;
    double m_massX;
    double m_massDisV;
    double m_massV0;
    bool   m_constrJX;
    bool   m_constrJpsi;
    bool   m_constrX;
    bool   m_constrDisV;
    bool   m_constrV0;
    double m_chi2cut_JX;
    double m_chi2cut_V0;
    double m_chi2cut_DisV;
    double m_chi2cut;
    bool   m_useTRT;
    double m_ptTRT;
    double m_d0_cut;
    unsigned int m_maxJXCandidates;
    unsigned int m_maxV0Candidates;
    unsigned int m_maxDisVCandidates;

    ServiceHandle<IBeamCondSvc>                      m_beamCondSvc;
    ToolHandle < Trk::TrkVKalVrtFitter >             m_iVertexFitter;
    ToolHandle < Trk::TrkV0VertexFitter >            m_iV0Fitter;
    ToolHandle < Trk::IVertexFitter >                m_iGammaFitter;
    ToolHandle < Analysis::PrimaryVertexRefitter >   m_pvRefitter;
    ToolHandle < Trk::V0Tools >                      m_V0Tools;
    ToolHandle < Reco::ITrackToVertex >              m_trackToVertexTool;
    ToolHandle < Trk::ITrackSelectorTool >           m_trkSelector;
    ToolHandle < DerivationFramework::CascadeTools > m_CascadeTools;

    bool        m_refitPV;
    int         m_PV_max;
    size_t      m_PV_minNTracks;
    int         m_DoVertexType;

    double mass_e;
    double mass_mu;
    double mass_pion;
    double mass_proton;
    double mass_Lambda;
    double mass_Ks;
    double mass_Xi;

    bool d0Pass(const xAOD::TrackParticle* track, const xAOD::Vertex* PV) const;
  };
}

#endif
