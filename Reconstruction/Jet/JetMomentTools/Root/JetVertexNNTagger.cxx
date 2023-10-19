/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "JetMomentTools/JetVertexNNTagger.h"

#include <fstream>
#include <regex>
#include <map>

#include "AsgDataHandles/ReadHandle.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODCore/ShallowCopy.h"
#include "PathResolver/PathResolver.h"

#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"

#include "lwtnn/generic/FastGraph.hh"
#include "lwtnn/parse_json.hh"
#include "lwtnn/Stack.hh"

using xAOD::JetContainer;

namespace JetPileupTag {

    JetVertexNNTagger::JetVertexNNTagger(const std::string& name)
    : asg::AsgTool(name) {}

    JetVertexNNTagger::~JetVertexNNTagger() {}

    StatusCode JetVertexNNTagger::initialize()
    {

        ATH_MSG_DEBUG("Initializing...");

        if(m_jetContainerName.empty()){
            ATH_MSG_ERROR("JetVertexTaggerTool needs to have its input jet container configured!");
            return StatusCode::FAILURE;
        }

        // Use the Path Resolver to find the jvt file and retrieve the likelihood histogram
        std::string configPath = PathResolverFindCalibFile(m_NNConfigDir+"/"+m_NNParamFileName);
        ATH_MSG_INFO("  Reading JVT NN file from:\n    " << m_NNParamFileName << "\n");
        ATH_MSG_INFO("                     resolved in  :\n    " << configPath << "\n\n");

        std::ifstream fconfig( configPath.c_str() );
        if ( !fconfig.is_open() ) {
        ATH_MSG_ERROR( "Error opening config file: " << m_NNParamFileName );
        ATH_MSG_ERROR( "Are you sure that the file exists at this path?" );
        return StatusCode::FAILURE;
        }

        ATH_MSG_INFO("\n Reading JVT likelihood histogram from:\n    " << configPath << "\n\n");
        lwt::GraphConfig cfg = lwt::parse_json_graph( fconfig );
        // FastGraph is initialised with the order in which inputs will be
        // provided, to avoid map lookup
        lwt::InputOrder order;
        lwt::order_t node_order;
        std::vector<std::string> inputs = {"Rpt","JVFCorr","ptbin","etabin"};
        // Single input block
        node_order.emplace_back(cfg.inputs[0].name,inputs);
        order.scalar = node_order;
        ATH_MSG_DEBUG( "Network NLayers: " << cfg.layers.size() );

        // Primarily using double to take advantage of build_vector later
        m_lwnn = std::make_unique<lwt::generic::FastGraph<double> >(cfg,order);

        std::string cutsPath = PathResolverFindCalibFile(m_NNConfigDir+"/"+m_NNCutFileName);
        ATH_MSG_INFO("  Reading JVT NN cut file from:\n    " << m_NNCutFileName << "\n");
        ATH_MSG_INFO("                     resolved in  :\n    " << cutsPath << "\n\n");
        std::ifstream fcuts( cutsPath.c_str() );
        if ( !fcuts.is_open() ) {
            ATH_MSG_ERROR( "Error opening cuts file: " << m_NNCutFileName );
            ATH_MSG_ERROR( "Are you sure that the file exists at this path?" );
            return StatusCode::FAILURE;
        }
        m_cutMap = NNJvtCutMap::fromJSON(fcuts);

        m_jvfCorrKey = m_jetContainerName + "." + m_jvfCorrKey.key();
        m_sumPtTrkKey = m_jetContainerName + "." + m_sumPtTrkKey.key();
        m_jvtKey = m_jetContainerName + "." + m_jvtKey.key();
        if (!m_rptKey.empty())
            m_rptKey = m_jetContainerName + "." + m_rptKey.key();
        if (!m_passJvtKey.empty())
            m_passJvtKey = m_jetContainerName + "." + m_passJvtKey.key();

        ATH_CHECK(m_vertexContainer_key.initialize());
#ifndef XAOD_STANDALONE
        if(m_suppressInputDeps) {
        // The user has promised that this will be produced by the same alg.
        // Tell the scheduler to ignore it to avoid circular dependencies.
            renounce(m_jvfCorrKey);
            renounce(m_sumPtTrkKey);
        }
        if(m_suppressOutputDeps) {
        // For analysis applications, may not enforce scheduling via data deps
            renounce(m_jvtKey);
            renounce(m_rptKey);
            renounce(m_passJvtKey);
        }
#endif
        ATH_CHECK(m_jvfCorrKey.initialize());
        ATH_CHECK(m_sumPtTrkKey.initialize());
        ATH_CHECK(m_jvtKey.initialize());
        ATH_CHECK(m_rptKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_passJvtKey.initialize(SG::AllowEmpty));

        return StatusCode::SUCCESS;
    }



    float JetVertexNNTagger::evaluateJvt(float rpt, float jvfcorr, size_t ptbin, size_t etabin) const
    {

    lwt::VectorX<double> inputvals = lwt::build_vector({rpt,jvfcorr,static_cast<double>(ptbin),static_cast<double>(etabin)});
    std::vector<lwt::VectorX<double> > scalars{inputvals};
    lwt::VectorX<double> output = m_lwnn->compute(scalars);

    return output(0);

    }


    const xAOD::Vertex *JetVertexNNTagger::findHSVertex(const xAOD::VertexContainer& vertices) const
    {

        for ( const xAOD::Vertex* vertex : vertices ) {
            if(vertex->vertexType() == xAOD::VxType::PriVtx) {
                ATH_MSG_VERBOSE("JetVertexTaggerTool " << name() << " Found HS vertex at index: "<< vertex->index());
                return vertex;
            }
        }
        if (vertices.size()==1) {
            ATH_MSG_VERBOSE("JetVertexTaggerTool " << name() << " Found no HS vertex, return dummy");
            if (vertices.back()->vertexType() == xAOD::VxType::NoVtx)
                return vertices.back();
        }
        ATH_MSG_VERBOSE("No vertex found in container.");
        return nullptr;
    }

    StatusCode JetVertexNNTagger::decorate(const xAOD::JetContainer& jetCont) const
    {

        // Grab vertices for index bookkeeping
        SG::ReadHandle<xAOD::VertexContainer> vertexHandle = SG::makeHandle (m_vertexContainer_key);
        const xAOD::VertexContainer& vertices = *vertexHandle;
        ATH_MSG_DEBUG("Successfully retrieved VertexContainer: " << m_vertexContainer_key.key());

        const xAOD::Vertex *HSvertex = findHSVertex(vertices);
        if(!HSvertex) {
            ATH_MSG_WARNING("Invalid primary vertex found, will not continue decorating with JVT.");
            return StatusCode::FAILURE;
        }

        SG::ReadDecorHandle<xAOD::JetContainer, float> jvfCorrHandle(m_jvfCorrKey);
        SG::ReadDecorHandle<xAOD::JetContainer, std::vector<float> > sumPtTrkHandle(m_sumPtTrkKey);
        SG::WriteDecorHandle<xAOD::JetContainer, float> jvtHandle(m_jvtKey);
        SG::WriteDecorHandle<xAOD::JetContainer, float> rptHandle(m_rptKey);
        SG::WriteDecorHandle<xAOD::JetContainer, char>  passJvtHandle(m_passJvtKey);

        static constexpr float invalidJvt = -1;
        static constexpr float invalidRpt = 0;
        static constexpr char invalidPassJvt = true;


        for(const xAOD::Jet* jet : jetCont) {
            float jvt = invalidJvt;
            float rpt = invalidRpt;
            char passJvt = invalidPassJvt;
            if (HSvertex->vertexType() == xAOD::VxType::PriVtx) {
                // Calculate RpT and JVFCorr
                // Default JVFcorr to -1 when no tracks are associated.
                float jvfcorr = jvfCorrHandle(*jet);
                std::vector<float> sumpttrk = sumPtTrkHandle(*jet);
                rpt = sumpttrk[HSvertex->index() - vertices[0]->index()]/jet->pt();

                size_t ptbin, etabin;
                if (jet->pt() <= m_maxpt_for_cut && m_cutMap.edges(*jet, ptbin, etabin)) {
                    jvt = evaluateJvt(rpt, jvfcorr, ptbin, etabin);
                    float jvtCut = m_cutMap(ptbin, etabin);
                    passJvt = jvt > jvtCut;

                    ATH_MSG_VERBOSE("Jet with pt " << jet->pt() << ", eta " << jet->eta() );
                    ATH_MSG_VERBOSE("  --> ptbin " << ptbin << ", etabin " << etabin);
                    ATH_MSG_VERBOSE("  --> inputs: corrJVF " << jvfcorr << ", rpt " << rpt );
                    ATH_MSG_VERBOSE("JVT cut for ptbin " << ptbin << ", etabin " << etabin << " = " << jvtCut);
                    ATH_MSG_VERBOSE("Evaluated JVT = " << jvt << ", jet " << (passJvt ? "passes" :"fails") << " working point" );
                }
            }

            // Decorate jet
            jvtHandle(*jet) = jvt;
            if (!rptHandle.key().empty())
                rptHandle(*jet) = rpt;
            if (!passJvtHandle.key().empty())
                passJvtHandle(*jet) = passJvt;
        }

        return StatusCode::SUCCESS;
    }
}