/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_MUHADELEVETO_H
#define DITAUREC_MUHADELEVETO_H

#include "tauRecTools/TauRecToolBase.h"
#include "GaudiKernel/ToolHandle.h"

namespace Trk {
  class IParticleCaloExtensionTool;
}

/**
 * @brief Calculate variables sensitive on electrons.
 * @brief modified from tauRecTools/TauElectronVetoVariables by Zofia Czyczula
 * 
 *  The variables are mainly used by the electron veto in the TauDiscriminant package.
 * 
 * @author Lianyou SHAN
 */

class MuHadElectronVetoVariables : public TauRecToolBase {
public:

    MuHadElectronVetoVariables(const std::string& name);
    ASG_TOOL_CLASS2(MuHadElectronVetoVariables, TauRecToolBase, ITauToolBase);

    ~MuHadElectronVetoVariables() override ;
    StatusCode execute(xAOD::TauJet& pTau) override ;
    StatusCode initialize() override ;

private :

    static const double DEFAULT;
    const float etamaxcut = 0.158;
    const float phimaxcut = 0.1;
    const float eta0cut = 0.075;
    const float eta1cut = 0.0475;
    const float eta2cut = 0.075;
    const float eta3cut = 1.5;
    const float phi0cut = 0.3;
    const float phi1cut = 0.3;
    const float phi2cut = 0.075;
    const float phi3cut = 0.075;
    const float etacase1 = 1.8;
    const float etagran1 = 0.00315;
    const float etagran2 = 0.00415;

    const float etahadcut = 0.2;
    const float phihadcut = 0.2;

    int FromSamplingToIndex( int sampling ) ;
    bool m_doCellCorrection; //   enable cell origin correction
    ToolHandle< Trk::IParticleCaloExtensionTool >  m_caloExtensionTool;
};

#endif
