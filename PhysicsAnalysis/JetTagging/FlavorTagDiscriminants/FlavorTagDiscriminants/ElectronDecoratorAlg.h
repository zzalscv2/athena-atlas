/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ELECTRON_DECORATOR_ALG_HH
#define ELECTRON_DECORATOR_ALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthContainers/AuxElement.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODEgamma/ElectronContainer.h"
#include "xAODTracking/VertexContainer.h"

namespace FlavorTagDiscriminants {

  class ElectronDecoratorAlg: public AthReentrantAlgorithm {
  public:
    ElectronDecoratorAlg(const std::string& name,
                          ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ) const override;

  private:

    // Input Containers
    SG::ReadHandleKey< xAOD::ElectronContainer > m_ElectronContainerKey {
      this, "electronContainer", "Electrons",
        "Key for the input electron collection"};
    SG::ReadHandleKey< xAOD::VertexContainer > m_VertexContainerKey {
      this, "vertexContainer", "PrimaryVertices",
        "Key for the input electron collection"};

    // Decorators for electrons

    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_electron_et {
      this, "et", "et", 
        "Transverse energy of the electron"};
    
    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_electron_deltaPOverP {
      this, "deltaPOverP", "deltaPOverP", 
        "Momentum lost by the electron track between the perigee and the last measurement point divided by the momentum at the perigee"};
    
    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_electron_isoPtOverPt {
      this, "ptVarCone30OverPt", "ptVarCone30OverPt", 
        "Ratio of isolated pt to pt"};
    
    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_electron_energyOverP {
      this, "energyOverP", "energyOverP", 
        "Ratio of cluster energy energy to track momentum"};
    
    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_electron_z0 {
      this, "z0AlongBeamspot", "z0AlongBeamspot", 
        "Electron z0 along beamspot"};

    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_electron_z0_significance {
      this, "z0AlongBeamspotSignificance", "z0AlongBeamspotSignificance", 
        "Electron z0 significance along beamspot"};
    
    SG::AuxElement::ConstAccessor<float> m_pt_varcone30{"ptvarcone30_TightTTVA_pt1000"};

    const xAOD::Vertex* primary(const xAOD::VertexContainer& vertices) const;
  };
}

#endif
