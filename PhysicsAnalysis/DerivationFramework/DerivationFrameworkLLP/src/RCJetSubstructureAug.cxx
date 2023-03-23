/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// RCJetSubstructureAug.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: G. Albouy (galbouy@lpsc.in2p3.fr)
// This tool computes substructure variables for ReClustered jets
// from LCTopo clusters ghost associated to RC jets 
// by constructing cluster jets 

#include "DerivationFrameworkLLP/RCJetSubstructureAug.h"
#include "xAODJet/JetContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include <vector>

#include "fastjet/ClusterSequence.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/contrib/Nsubjettiness.hh"
#include "JetSubStructureUtils/Qw.h"
#include "JetSubStructureUtils/Nsubjettiness.h"
#include "JetSubStructureUtils/KtSplittingScale.h"
#include "JetSubStructureUtils/EnergyCorrelator.h"

// Constructor
DerivationFramework::RCJetSubstructureAug::RCJetSubstructureAug(const std::string& t, const std::string& n, const IInterface* p) :
base_class(t, n, p) {}

// Destructor
DerivationFramework::RCJetSubstructureAug::~RCJetSubstructureAug() 
{}

// Athena initialize and finalize
StatusCode DerivationFramework::RCJetSubstructureAug::initialize()
{
    ATH_MSG_VERBOSE("initialize() ...");
    if (m_jetKey.key().empty()) {
        ATH_MSG_FATAL("No jet collection provided for augmentation.");
        return StatusCode::FAILURE;
    }
    ATH_CHECK( m_jetKey.initialize() );

    // Set up the text-parsing machinery for selectiong the jet directly according to user cuts
    if (!m_selectionString.empty()) {
       ATH_CHECK( initializeParser( m_selectionString ));
    }

    // Init moment struct
    m_moments = new moments_t(m_suffix);

    return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::RCJetSubstructureAug::finalize()
{
    ATH_MSG_VERBOSE("finalize() ...");
    delete m_moments;
    return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::RCJetSubstructureAug::addBranches() const
{
    const EventContext& ctx = Gaudi::Hive::currentContext();

    SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey,ctx);
    if (!jets.isValid()) {
        ATH_MSG_ERROR("No jet collection with name " << m_jetKey.key() << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    unsigned int nJets(jets->size());
    std::vector<const xAOD::Jet*> jetToCheck; jetToCheck.clear();
    
    // Execute the text parser if requested
    if (!m_selectionString.empty()) {
        std::vector<int> entries =  m_parser->evaluateAsVector();
        unsigned int nEntries = entries.size();
        // check the sizes are compatible
        if (nJets != nEntries ) {
        	ATH_MSG_ERROR("Sizes incompatible! Are you sure your selection string used jets??");
            return StatusCode::FAILURE;
        } else {
        	// identify which jets to keep for the thinning check
        	for (unsigned int i=0; i<nJets; ++i) if (entries[i]==1) jetToCheck.push_back((*jets)[i]);
        }
    } else {
        for (unsigned int i=0; i<nJets; ++i) jetToCheck.push_back((*jets)[i]);
    }
    
    std::vector<const xAOD::CaloCluster*> clusters;
    std::vector<fastjet::PseudoJet> constituents;

    for (const xAOD::Jet* jet : jetToCheck) {
        
        // Get list of ghost energy clusters
        clusters.clear();
        ATH_CHECK(jet->getAssociatedObjects(m_ghostName, clusters));

        // Construct list of constituent PseudoJets from clusters and compute timing information
        constituents.clear();
        double eTot = 0;
        double time = 0;
        for (auto cluster : clusters){
            constituents.push_back( fastjet::PseudoJet(cluster->p4()) );
            double eConstit = cluster->e()* cluster->e();
            time += cluster->time()* eConstit;
            eTot += eConstit;
        }

        if (eTot==0) { 
            m_moments->dec_timing(*jet) = 0;
        } else {
            m_moments->dec_timing(*jet) = time/eTot;
        }

        if (constituents.size()==0) {
            m_moments->dec_Qw(*jet) = -999;

            m_moments->dec_Tau1(*jet) = -999;
            m_moments->dec_Tau2(*jet) = -999;
            m_moments->dec_Tau3(*jet) = -999;
            m_moments->dec_Tau4(*jet) = -999;

            m_moments->dec_Tau21(*jet) = -999;
            m_moments->dec_Tau32(*jet) = -999;

            m_moments->dec_Split12(*jet) = -999;
            m_moments->dec_Split23(*jet) = -999;
            m_moments->dec_Split34(*jet) = -999;

            m_moments->dec_ECF1(*jet) = -999;
            m_moments->dec_ECF2(*jet) = -999;
            m_moments->dec_ECF3(*jet) = -999;
            m_moments->dec_ECF4(*jet) = -999;

            m_moments->dec_C2(*jet) = -999;
            m_moments->dec_D2(*jet) = -999;

            m_moments->dec_pT(*jet) = 0;
            m_moments->dec_m(*jet) = 0;
            m_moments->dec_NClusts(*jet) = 0;
            m_moments->dec_eta(*jet) = -999;
            m_moments->dec_phi(*jet) = -999;

		    return StatusCode::SUCCESS;
	    }

        // Run clustering
        auto jet_def = fastjet::JetDefinition(fastjet::antikt_algorithm, 1.5);
        fastjet::ClusterSequence cs(constituents, jet_def);
	    fastjet::PseudoJet LCTopo_jet = cs.inclusive_jets(0.0).front(); 

        // Save LCTopo jet infos
        m_moments->dec_pT(*jet) = LCTopo_jet.pt();
        m_moments->dec_m(*jet) = LCTopo_jet.m();
        m_moments->dec_NClusts(*jet) = constituents.size();
        m_moments->dec_eta(*jet) = LCTopo_jet.eta();
        m_moments->dec_phi(*jet) = LCTopo_jet.phi() - M_PI;

        // Qw
        JetSubStructureUtils::Qw qw;
        m_moments->dec_Qw(*jet) = qw.result(LCTopo_jet);
        
        // Nsubjetiness
        fastjet::contrib::WTA_KT_Axes wta_kt_axes;
        fastjet::contrib::NormalizedCutoffMeasure normalized_measure(1.0, 1.0, 1000000);
        JetSubStructureUtils::Nsubjettiness Tau1_wta(1, wta_kt_axes, normalized_measure);
        JetSubStructureUtils::Nsubjettiness Tau2_wta(2, wta_kt_axes, normalized_measure);
        JetSubStructureUtils::Nsubjettiness Tau3_wta(3, wta_kt_axes, normalized_measure);
        JetSubStructureUtils::Nsubjettiness Tau4_wta(4, wta_kt_axes, normalized_measure);
        float tau1_wta = Tau1_wta.result(LCTopo_jet);
        float tau2_wta = Tau2_wta.result(LCTopo_jet);
        float tau3_wta = Tau3_wta.result(LCTopo_jet);
        float tau4_wta = Tau4_wta.result(LCTopo_jet);
        float tau21_wta = -999;
        float tau32_wta = -999;

        if( tau1_wta > 1e-8 ) {
            tau21_wta = tau2_wta / tau1_wta;
        }
        if( tau2_wta > 1e-8 ) {
            tau32_wta = tau3_wta / tau2_wta;
        }

        m_moments->dec_Tau1(*jet) = tau1_wta;
        m_moments->dec_Tau2(*jet) = tau2_wta;
        m_moments->dec_Tau3(*jet) = tau3_wta;
        m_moments->dec_Tau4(*jet) = tau4_wta;
        m_moments->dec_Tau21(*jet) = tau21_wta;
        m_moments->dec_Tau32(*jet) = tau32_wta;

        // KtSplittingScale
        JetSubStructureUtils::KtSplittingScale Split12(1);
        JetSubStructureUtils::KtSplittingScale Split23(2);
        JetSubStructureUtils::KtSplittingScale Split34(3);
        float split12 = Split12.result(LCTopo_jet);
        float split23 = Split23.result(LCTopo_jet);
        float split34 = Split34.result(LCTopo_jet);

        m_moments->dec_Split12(*jet) = split12;
        m_moments->dec_Split23(*jet) = split23;
        m_moments->dec_Split34(*jet) = split34;

        // EnergyCorrelator
        JetSubStructureUtils::EnergyCorrelator ECF1(1, 1.0, JetSubStructureUtils::EnergyCorrelator::pt_R);
        JetSubStructureUtils::EnergyCorrelator ECF2(2, 1.0, JetSubStructureUtils::EnergyCorrelator::pt_R);
        JetSubStructureUtils::EnergyCorrelator ECF3(3, 1.0, JetSubStructureUtils::EnergyCorrelator::pt_R);
        JetSubStructureUtils::EnergyCorrelator ECF4(4, 1.0, JetSubStructureUtils::EnergyCorrelator::pt_R);
        float ecf1 = ECF1.result(LCTopo_jet);
        float ecf2 = ECF2.result(LCTopo_jet);
        float ecf3 = ECF3.result(LCTopo_jet);
        float ecf4 = ECF4.result(LCTopo_jet);
        float c2 = -999;
        float d2 = -999;

        if( ecf2 > 1e-8 ) {
            c2 = ecf3 * ecf1 / pow( ecf2, 2.0 );
            d2 = ecf3 * pow( ecf1, 3.0 ) / pow( ecf2, 3.0 );
        }

        m_moments->dec_ECF1(*jet) = ecf1;
        m_moments->dec_ECF2(*jet) = ecf2;
        m_moments->dec_ECF3(*jet) = ecf3;
        m_moments->dec_ECF4(*jet) = ecf4;
        m_moments->dec_C2(*jet) = c2;
        m_moments->dec_D2(*jet) = d2;

    }
    
    return StatusCode::SUCCESS;
}


