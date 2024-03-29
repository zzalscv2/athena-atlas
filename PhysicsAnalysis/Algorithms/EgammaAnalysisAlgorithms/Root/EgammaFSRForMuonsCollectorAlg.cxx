/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   EgammaFSRForMuonsCollectorAlg
//
//   Algorithm to collect photons and electrons which close in dR 
//   to muons as FSR candidates
///////////////////////////////////////////////////////////////////

#include "EgammaAnalysisAlgorithms/EgammaFSRForMuonsCollectorAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <xAODEgamma/Electron.h>

#include "xAODEgamma/ElectronxAODHelpers.h"

#include <SelectionHelpers/SelectionHelpers.h>

namespace CP
{
    EgammaFSRForMuonsCollectorAlg::EgammaFSRForMuonsCollectorAlg(const std::string &name, ISvcLocator *svcLoc)
        : EL::AnaAlgorithm(name, svcLoc)
    {
    }

    StatusCode EgammaFSRForMuonsCollectorAlg::initialize()
    {
        // Greet the user:
        ATH_MSG_INFO("Initialising");

        // WP ID selection
        if (m_selectionName.empty()) {
            ATH_MSG_ERROR("Empty string passed as WP selection flag!");
            return StatusCode::FAILURE;
        }

        ATH_CHECK(m_egammaContKey.initialize(m_systematicsList));
        ATH_CHECK(m_muonContKey.initialize(m_systematicsList));
        ANA_CHECK (m_systematicsList.initialize());
        m_wpDec = std::make_unique<SG::AuxElement::Decorator<uint32_t> > (m_selectionName.value());

        if (!m_vetoFSR) {
            ATH_MSG_INFO("Reading container " << m_egammaContKey.getNamePattern() << " for FSR search for muons from " <<  m_muonContKey.getNamePattern() << ". Those passing " << m_selectionName.value() << " are also accepted.");
        }
        else {
            ATH_MSG_INFO("Reading container " << m_egammaContKey.getNamePattern() << " for FSR search for muons from " <<  m_muonContKey.getNamePattern() << ". The electrons or photons matching FSR requirements with muons will be vetoed with " << m_selectionName.value());

        }

        // Return gracefully:
        return StatusCode::SUCCESS;
    }


    StatusCode EgammaFSRForMuonsCollectorAlg::execute()
    {

        // const EventContext &ctx = Gaudi::Hive::currentContext();

        auto selDec   = std::make_unique<SG::AuxElement::Decorator<uint32_t> > ("selectEta");
        auto oqDec    = std::make_unique<SG::AuxElement::Decorator<uint32_t> > ("goodOQ");
        auto cleanDec = std::make_unique<SG::AuxElement::Decorator<uint32_t> > ("isClean");

        // Loop over systematics
        for (const auto& sys : m_systematicsList.systematicsVector()) {


            // Retrieve electrons or photons
            const xAOD::IParticleContainer* egammaCont = nullptr;
            ANA_CHECK (m_egammaContKey.retrieve (egammaCont, sys));

            // Retrieve muons 
            const xAOD::MuonContainer* muonCont = nullptr;
            ANA_CHECK (m_muonContKey.retrieve (muonCont, sys));

            // Loop over each electron or photon. If already passing WP selection, decorate passWPorFSR as true
            // If m_vetoFSR is set, then reverse logic is used - require electron or photon to pass WP selection, 
            //   and then veto is if if passes the FSR selection
            for ( auto eg : *egammaCont ) {
                if (!m_vetoFSR) {
                    // Standard logic - If passes std WP, accept
                    if ((*m_wpDec)(*eg) == selectionAccept()) {
                        ATH_MSG_DEBUG("Eg passed WP - pt, eta: " << eg->type() << ", " << eg->pt()/1000. << ", " << eg->eta() << ", " << (*m_wpDec)(*eg)  );
                        continue; // ok, skip to next el/ph
                    }
                    else ATH_MSG_DEBUG("Eg failed WP: " <<  m_selectionName.value() << ", " << (*m_wpDec)(*eg) );
                }
                else {
                    // Inverted logic - If passes std WP, continue with the FSR search
                    if ((*m_wpDec)(*eg) == selectionAccept()) {
                        ATH_MSG_DEBUG("Veto FSR: Eg passed WP - pt, eta: " << eg->type() << ", " << eg->pt()/1000. << ", " << eg->eta() << ", " << (*m_wpDec)(*eg)  );

                    }
                    else {
                        ATH_MSG_DEBUG("Veto FSR: Eg failed WP: " <<  m_selectionName.value() << ", " << (*m_wpDec)(*eg) << " skipping." );
                        continue; // ok, skip to next el/ph
                    }
                }

                const xAOD::Electron* el = (eg->type() == xAODType::Electron) ? dynamic_cast<const xAOD::Electron*>(eg) : 0;

                ATH_MSG_DEBUG( "Incoming eg: pt, eta, phi " << eg->pt()/1000. << ", " << eg->eta() << ", " << eg->phi() << ", is electron " << (el != 0));

                // Loop over muons and check dR
                for ( auto mu : *muonCont ) {

                    float dR = xAOD::P4Helpers::deltaR(*eg, *mu);

                    ATH_MSG_DEBUG( "dR with mu: pt, eta, phi " << dR << ", " << mu->pt()/1000. << ", " << mu->eta() << ", " << mu->phi() );

                    if (dR < m_dRMax) {

                        // if electron (not photon) check track matching 
                        bool elmutrackmatchOK = true; // default true for photons
                        if (el) {
                            const xAOD::TrackParticle* electron_track = el->trackParticle();
                            const xAOD::TrackParticle* elOrig_track   = xAOD::EgammaHelpers::getOriginalTrackParticle(el);
                            const xAOD::TrackParticle* muon_track     = mu->primaryTrackParticle();

                            elmutrackmatchOK = 
                            ( (std::abs(electron_track->theta()- muon_track->theta()) < 0.01) &&
                            (xAOD::P4Helpers::deltaPhi(electron_track->phi(),  muon_track->phi())   < 0.01) );
                            ATH_MSG_DEBUG( "dtheta trk " << std::abs(electron_track->theta()- muon_track->theta()) << ", dphi trk " 
                            << xAOD::P4Helpers::deltaPhi(electron_track->phi(),  muon_track->phi()));
                            if (elOrig_track) ATH_MSG_DEBUG( "origTrk: dtheta trk " << std::abs(elOrig_track->theta()- muon_track->theta()) << ", dphi trk " 
                            << xAOD::P4Helpers::deltaPhi(elOrig_track->phi(),  muon_track->phi()));
                            if (elmutrackmatchOK) ATH_MSG_DEBUG( "track match OK");
                            else                  ATH_MSG_DEBUG( "track match NOT OK");
                        }
                        if (elmutrackmatchOK) {
                            (*m_wpDec)(*eg) = (m_vetoFSR) ? selectionReject() : selectionAccept();
                            ATH_MSG_DEBUG( "dR OK - wp " << (*m_wpDec)(*eg) );

                            if (selDec->isAvailable(*eg))   ATH_MSG_DEBUG( "selectEta: " << (*selDec)(*eg) );
                            if (oqDec->isAvailable(*eg))    ATH_MSG_DEBUG( "goodOQ:    " << (*oqDec)(*eg) );
                            if (cleanDec->isAvailable(*eg)) ATH_MSG_DEBUG( "isClean:   " << (*cleanDec)(*eg) );

                            break;
                        }
                    }
                    ATH_MSG_DEBUG( "dR not OK");
                }
            }
        }

        ATH_MSG_DEBUG("Done !");

        return StatusCode::SUCCESS;
    }

}
