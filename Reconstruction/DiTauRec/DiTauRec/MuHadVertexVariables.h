/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_MUHADVERTEXVARIABLES_H
#define	DITAUREC_MUHADVERTEXVARIABLES_H

#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "tauRecTools/TauRecToolBase.h"
#include "GaudiKernel/ToolHandle.h"

// forwards
class TauEventData;
namespace Trk {
	class ITrackToVertexIPEstimator;
    class IVertexFitter;
    class IVertexSeedFinder;
    class IVxCandidateXAODVertex;
}

/**
 *  
 * @brief Modefied from /tauRecTools/TauVertexVariables Class for calculating vertex variables.
 * by Stan Lai, Felix Friedrich
 *
 * @author Lianyou SHAN
 */

class MuHadVertexVariables : public TauRecToolBase {
public:
    //-----------------------------------------------------------------
    // Constructor and destructor
    //-----------------------------------------------------------------
    MuHadVertexVariables(const std::string& name);
    ASG_TOOL_CLASS2(MuHadVertexVariables, TauRecToolBase, ITauToolBase);

    ~MuHadVertexVariables() override ;
    StatusCode initialize() override ;
    StatusCode execute(xAOD::TauJet&) override ;
    StatusCode eventInitialize() override ;

    //-------------------------------------------------------------
    //! determines the transverse flight path significance from
    //! the primary vertex and the secondary vertex of tau candidate
    //-------------------------------------------------------------
    double trFlightPathSig(const xAOD::TauJet& pTau, const xAOD::Vertex& secVertex);

private:

    static const float DEFAULT ;
    bool m_AODmode;
    ToolHandle< Trk::ITrackToVertexIPEstimator > m_trackToVertexIPEstimator;
    ToolHandle< Trk::IVertexFitter >     m_fitTool; // Pointer to the base class of the fit algtools
    ToolHandle< Trk::IVertexSeedFinder > m_SeedFinder;
    // necessary to convert VxCandidate to xAOD::Vertex in case old API is used :
    ToolHandle< Trk::IVxCandidateXAODVertex > m_xaodConverter; 

    xAOD::VertexContainer* m_pSecVtxContainer;
    xAOD::VertexAuxContainer* m_pSecVtxAuxContainer;
};

#endif	/* TAUREC_TAUVERTEXVARIABLES_H */

