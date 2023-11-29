// This is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PROMPT_PRIMARYVERTEXREFITTER_H
#define PROMPT_PRIMARYVERTEXREFITTER_H

/**********************************************************************************
 * @Package: LeptonTaggers
 * @Class  : PrimaryVertexReFitter
 * @Author : Fudong He
 * @Author : Rustem Ospanov
 * @Author : Kees Benkendorfer
 *
 * @Brief  :
 *
 *  Decorate leptons with secondary vertex algorithem output
 *
 **********************************************************************************/

// Local
#include "VertexFittingTool.h"

// Athena
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// xAOD
#include "xAODTracking/VertexContainer.h"

// ROOT
#include "TStopwatch.h"

namespace Prompt
{
    class PrimaryVertexReFitter: public AthAlgorithm
    {
    public:

        PrimaryVertexReFitter(const std::string& name, ISvcLocator* pSvcLocator);

        virtual StatusCode initialize() override;
        virtual StatusCode execute() override;
        virtual StatusCode finalize() override;

    private:

        bool decorateLepWithReFitPrimaryVertex(const FittingInput &input,
            const xAOD::TrackParticle* tracklep,
            const xAOD::IParticle *lep,
            const std::vector<const xAOD::TrackParticle*> &tracks,
            xAOD::VertexContainer &refitVtxContainer);


    private:

        typedef SG::AuxElement::Decorator<float> decoratorFloat_t;
        typedef SG::AuxElement::Decorator<ElementLink<xAOD::VertexContainer> > decoratorElemVtx_t;

    private:

        //
        // Tools and services:
        //
        ToolHandle<Prompt::VertexFittingTool> m_vertexFitterTool {
            this, "VertexFittingTool", "Prompt::VertexFittingTool/VertexFittingTool"
        };

        //
        // Properties:
        //
        Gaudi::Property<bool> m_printTime {this, "PrintTime", false};

        Gaudi::Property<std::string> m_distToRefittedPriVtxName {
            this, "DistToRefittedPriVtxName", "default"
        };
        Gaudi::Property<std::string> m_normDistToRefittedPriVtxName {
            this, "NormDistToRefittedPriVtxName", "default"
        };
        Gaudi::Property<std::string> m_lepVtxLinkName {
            this, "RefittedVtxLinkName", "default"
        };
        Gaudi::Property<std::string> m_lepRefittedVtxWithoutLeptonLinkName {
            this, "RefittedVtxWithoutLeptonLinkName", "default"
        };

        TStopwatch m_timerAll;
        TStopwatch m_timerExec;

        // Read/write handles
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inDetTracksKey{
            this, "InDetTrackParticlesKey", "InDetTrackParticles"
        };

        SG::ReadHandleKey<xAOD::IParticleContainer> m_leptonContainerKey {
            this,
            "LeptonContainerName",
            "lepContainerNameDefault", "Name of lepton container"
        };
        SG::ReadHandleKey<xAOD::VertexContainer> m_primaryVertexContainerKey {
            this, "PriVertexContainerName", "PrimaryVertices",
            "Name of primary vertex container"
        };
        SG::WriteHandleKey<xAOD::VertexContainer> m_reFitPrimaryVertexKey {
            this, "ReFitPriVtxName", "default"
        };

        //
        // Decorators:
        //
        std::unique_ptr<decoratorFloat_t> m_distToRefittedPriVtx;
        std::unique_ptr<decoratorFloat_t> m_normdistToRefittedPriVtx;
        std::unique_ptr<decoratorElemVtx_t> m_lepRefittedRMVtxLinkDec;
    };
}

#endif // PROMPT_PRIMARYVERTEXREFITTER_H
