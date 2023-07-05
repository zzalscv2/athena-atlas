/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkBPhys/BPhysBGammaFinder.h"
#include "DerivationFrameworkBPhys/BPhysPVTools.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"


using VertexLink = ElementLink<xAOD::VertexContainer>;

namespace DerivationFramework {

BPhysBGammaFinder::BPhysBGammaFinder(const std::string& t, const std::string& n, const IInterface* p)
    : AthAlgTool(t,n,p),
      m_v0Tools("Trk::V0Tools"),
      m_vertexFitter("Trk::TrkVKalVrtFitter"),
      m_vertexEstimator("InDet::VertexPointEstimator"),
      m_inputTrackParticleContainerName("InDetTrackParticles"),
      m_inputLowPtTrackContainerName("LowPtRoITrackParticles"),
      m_conversionContainerName("BPhysConversionCandidates"),
      m_maxDistBetweenTracks(10.0),
      m_maxDeltaCotTheta(0.3),
      m_requireDeltaQ(true),
      m_maxDeltaQ(450.0),
      m_minRxy(10.0),
      m_Chi2Cut(20.0),
      m_maxGammaMass(110.0) {

  declareInterface<DerivationFramework::IAugmentationTool>(this);

  // Declare user-defined properties
  declareProperty("BVertexContainers", m_BVertexCollectionsToCheck);
  declareProperty("PassFlagsToCheck", m_passFlagsToCheck);
  declareProperty("V0Tools", m_v0Tools);
  declareProperty("VertexFitterTool", m_vertexFitter);
  declareProperty("VertexEstimator", m_vertexEstimator);
  declareProperty("InputTrackParticleContainerName", m_inputTrackParticleContainerName);
  declareProperty("InputLowPtTrackContainerName", m_inputLowPtTrackContainerName);
  declareProperty("ConversionContainerName", m_conversionContainerName);
  declareProperty("MaxDistBetweenTracks", m_maxDistBetweenTracks); // Maximum allowed distance of minimum approach
  declareProperty("MaxDeltaCotTheta", m_maxDeltaCotTheta); // Maximum allowed dCotTheta between tracks
  declareProperty("RequireDeltaQ", m_requireDeltaQ); // Only save a conversions if it's a chi_c,b candidate (must then pass "MaxDeltaM" requirement), if "False" all conversions in the event will be saved
  declareProperty("MaxDeltaQ", m_maxDeltaQ); // Maximum mass difference between di-muon+conversion and di-muon
  declareProperty("Chi2Cut", m_Chi2Cut);
  declareProperty("MinRxy", m_minRxy);
  declareProperty("MaxGammaMass", m_maxGammaMass);
}


StatusCode BPhysBGammaFinder::initialize() {

  ATH_MSG_DEBUG("in initialize()");

  ATH_CHECK( m_v0Tools.retrieve() );
  ATH_CHECK( m_vertexFitter.retrieve() );
  ATH_CHECK( m_vertexEstimator.retrieve() );
  return StatusCode::SUCCESS;
}


StatusCode BPhysBGammaFinder::finalize() {
  return StatusCode::SUCCESS;
}


StatusCode BPhysBGammaFinder::addBranches() const {

  int nTrackPairs_Selected = 0;
  int nConv_VertexFit = 0;
  int nConv_Selected = 0;

  std::vector<const xAOD::Vertex*> BVertices;
  BVertices.clear();
  std::vector<const xAOD::TrackParticle*> BVertexTracks;
  std::vector<const xAOD::TrackParticle*> theIDTracksAfterSelection;
  std::vector<char> IDTracksContainerNames;


  // Output conversion container
  std::unique_ptr<xAOD::VertexContainer> conversionContainer(new xAOD::VertexContainer());
  std::unique_ptr<xAOD::VertexAuxContainer> conversionAuxContainer(new xAOD::VertexAuxContainer());
  conversionContainer->setStore(conversionAuxContainer.get());

  // Retrieve track particles from StoreGate
  const xAOD::TrackParticleContainer* inputTrackParticles = nullptr;
  ATH_CHECK( evtStore()->retrieve(inputTrackParticles, m_inputTrackParticleContainerName));
  ATH_MSG_DEBUG( "Track particle container size " << inputTrackParticles->size() );

  // Low pT collection
  const xAOD::TrackParticleContainer* lowPtTrackParticles = nullptr;
  if(evtStore()->contains<xAOD::TrackParticleContainer>(m_inputLowPtTrackContainerName)){
    StatusCode sc = evtStore()->retrieve(lowPtTrackParticles, m_inputLowPtTrackContainerName);
    if (sc.isFailure()) {
      ATH_MSG_WARNING("No low pT collection with key " << m_inputLowPtTrackContainerName << " found in StoreGate.");
    }
    else {
      ATH_MSG_DEBUG("Low pT track particle container size " <<  lowPtTrackParticles->size());
    }
  }

  // Look for B candidate
  if (m_BVertexCollectionsToCheck.empty()) {
    ATH_MSG_FATAL( "No B vertex collections provided" );
    return StatusCode::FAILURE;
  }

  // Track Selection
  for (const xAOD::TrackParticle* trackParticle : *inputTrackParticles) {
    IDTracksContainerNames.push_back(0);
    theIDTracksAfterSelection.push_back(trackParticle);
  }
  if (lowPtTrackParticles != nullptr) {
    for (const xAOD::TrackParticle* lowPt_trackParticle: *lowPtTrackParticles) {
      IDTracksContainerNames.push_back(1);
      theIDTracksAfterSelection.push_back(lowPt_trackParticle);
    }
  }

  // Retrieve vertex containers
  for (const std::string& BVertexCollectionName : m_BVertexCollectionsToCheck) {
    ATH_MSG_DEBUG( "Using " << BVertexCollectionName << " as the source B vertex collection" );

    // retieve vertex
    const xAOD::VertexContainer* BVtxContainer = nullptr;
    CHECK( evtStore()->retrieve(BVtxContainer, BVertexCollectionName));
    ATH_MSG_DEBUG( "Vertex Container (" << BVertexCollectionName << ") contains " << BVtxContainer->size() << " vertices" );

    static SG::AuxElement::Decorator< std::vector< VertexLink > > BGammaLinks( "BGammaLinks" );
    static std::vector< VertexLink > vertexLinks;

    for (const xAOD::Vertex* vertex : *BVtxContainer) {
      BGammaLinks(*vertex) = vertexLinks;

      bool passedHypothesis = false;
      BVertexTracks.clear();

      for (const auto& flag : m_passFlagsToCheck) {
        bool pass = vertex->auxdata<Char_t>(flag);
        if (pass) passedHypothesis = true;
      }

      if (!passedHypothesis) continue;
      xAOD::BPhysHypoHelper Bc("Bc", vertex);

      // link to Bc+ vertex
      std::vector<const xAOD::Vertex*> precedingVertices(1, vertex);

      // Collect up B-vertex tracks
      for (auto trk : vertex->trackParticleLinks()) BVertexTracks.push_back(*trk);

      // Track1 Loop
      for (size_t tp1 = 0; tp1 < theIDTracksAfterSelection.size(); ++tp1) {
        const xAOD::TrackParticle* trackParticle1 = theIDTracksAfterSelection.at(tp1);

        auto itr1 = std::find(BVertexTracks.begin(), BVertexTracks.end(), trackParticle1);
        if (itr1 != BVertexTracks.end()) continue;

        const Trk::Perigee& trackPerigee1 = trackParticle1->perigeeParameters();

        // Track2 Loop
        for (size_t tp2 = tp1 + 1; tp2 < theIDTracksAfterSelection.size(); ++tp2) {
          const xAOD::TrackParticle* trackParticle2 = theIDTracksAfterSelection.at(tp2);
          if (trackParticle1 == trackParticle2) continue;

          auto itr2 = std::find(BVertexTracks.begin(), BVertexTracks.end(), trackParticle2);
          if (itr2 != BVertexTracks.end()) continue;

          const Trk::Perigee& trackPerigee2 = trackParticle2->perigeeParameters();

          // Track pair selection
          TLorentzVector e1, e2, m_gamma, BcStar;
          e1.SetPtEtaPhiM(trackParticle1->pt(), trackParticle1->eta(), trackParticle1->phi(), Trk::electron);
          e2.SetPtEtaPhiM(trackParticle2->pt(), trackParticle2->eta(), trackParticle2->phi(), Trk::electron);

          m_gamma = e1 + e2;
          if (m_gamma.M() > m_maxGammaMass) continue;

          TLorentzVector mu1 = Bc.refTrk(0, Trk::muon);
          TLorentzVector mu2 = Bc.refTrk(1, Trk::muon);
          TLorentzVector mu3 = Bc.refTrk(2, Trk::muon);

          BcStar = mu1 + mu2 + mu3 + e1 + e2;
          double Q = BcStar.M() - Bc.mass() - 2 * Trk::electron;
          if (Q > m_maxDeltaQ) continue;

          // Estimate starting point + cuts on compatiblity of tracks
          int sflag = 0;
          int errorcode = 0;
          Amg::Vector3D startingPoint = m_vertexEstimator->getCirclesIntersectionPoint(&trackPerigee1, &trackPerigee2, sflag, errorcode);
          if (errorcode != 0) startingPoint = Amg::Vector3D::Zero(3);

          nTrackPairs_Selected++;

          std::vector<float> RefTrackPx, RefTrackPy, RefTrackPz, RefTrackE;
          std::vector<float> OrigTrackPx, OrigTrackPy, OrigTrackPz, OrigTrackE;
          std::vector<char> IDTracksContainerName;

          std::vector<const xAOD::TrackParticle*> trackPair;
          trackPair.clear();
          trackPair.push_back(trackParticle1);
          trackPair.push_back(trackParticle2);

          // Do the vertex fit
          std::unique_ptr<xAOD::Vertex> convVertexCandidate( m_vertexFitter->fit(trackPair, startingPoint) );

          // Check for successful fit
          if (convVertexCandidate) {
            if (convVertexCandidate->chiSquared() / convVertexCandidate->numberDoF() > m_Chi2Cut) continue;

            xAOD::BPhysHelper Photon(convVertexCandidate.get());
            // set link to the parent Bc+ vertex
            Photon.setPrecedingVertices(precedingVertices, BVtxContainer);

            // Parameters at vertex
            convVertexCandidate->clearTracks();
            ElementLink<xAOD::TrackParticleContainer> newLink1;
            newLink1.setElement(trackParticle1);
            if (IDTracksContainerNames.at(tp1)==0) newLink1.setStorableObject(*inputTrackParticles);
            else newLink1.setStorableObject(*lowPtTrackParticles);
            ElementLink<xAOD::TrackParticleContainer> newLink2;
            newLink2.setElement(trackParticle2);
            if (IDTracksContainerNames.at(tp2)==0) newLink2.setStorableObject(*inputTrackParticles);
            else newLink2.setStorableObject(*lowPtTrackParticles);
            convVertexCandidate->addTrackAtVertex(newLink1);
            convVertexCandidate->addTrackAtVertex(newLink2);

            nConv_VertexFit++;

            std::vector<Amg::Vector3D> positionList;
            nConv_Selected++;

            //Get photon momentum 3-vector
            Amg::Vector3D momentum = m_v0Tools->V0Momentum(convVertexCandidate.get());

            TLorentzVector photon, electron1, electron2, ph;
            electron1.SetVectM( trackMomentum( convVertexCandidate.get(), 0 ), Trk::electron );
            electron2.SetVectM( trackMomentum( convVertexCandidate.get(), 1 ), Trk::electron );
            photon = electron1 + electron2;
            ph.SetXYZM(momentum.x(), momentum.y(), momentum.z(), 0.);

            // Use to keep track of which dimuon(s) gave a chi_c/b candidate
            std::vector<float> B_Px = vertex->auxdata<std::vector<float>>("RefTrackPx");
            std::vector<float> B_Py = vertex->auxdata<std::vector<float>>("RefTrackPy");
            std::vector<float> B_Pz = vertex->auxdata<std::vector<float>>("RefTrackPz");

            TLorentzVector muon1, muon2, muon3;
            muon1.SetXYZM(B_Px.at(0), B_Py.at(0), B_Pz.at(0), Trk::muon);
            muon2.SetXYZM(B_Px.at(1), B_Py.at(1), B_Pz.at(1), Trk::muon);
            muon3.SetXYZM(B_Px.at(2), B_Py.at(2), B_Pz.at(2), Trk::muon);

            TLorentzVector m_B = muon1 + muon2 + muon3;

            const double deltaQ = (m_B + photon).M() - Bc.mass() - 2 * Trk::electron;
            const double mass = photon.M();
            const double Rxy = m_v0Tools->rxy(convVertexCandidate.get());
            if (deltaQ > m_maxDeltaQ) continue;
            if (mass > m_maxGammaMass) continue;
            if (Rxy < m_minRxy) continue;

            RefTrackPx.push_back(trackMomentum(convVertexCandidate.get(), 0).Px());
            RefTrackPx.push_back(trackMomentum(convVertexCandidate.get(), 1).Px());

            RefTrackPy.push_back(trackMomentum(convVertexCandidate.get(), 0).Py());
            RefTrackPy.push_back(trackMomentum(convVertexCandidate.get(), 1).Py());

            RefTrackPz.push_back(trackMomentum(convVertexCandidate.get(), 0).Pz());
            RefTrackPz.push_back(trackMomentum(convVertexCandidate.get(), 1).Pz());

            for (size_t i = 0; i < B_Px.size(); i++) {
              RefTrackPx.push_back(B_Px.at(i));
              RefTrackPy.push_back(B_Py.at(i));
              RefTrackPz.push_back(B_Pz.at(i));
            }

            RefTrackE.push_back(electron1.E());
            RefTrackE.push_back(electron2.E());
            RefTrackE.push_back(muon1.E());
            RefTrackE.push_back(muon2.E());
            RefTrackE.push_back(muon3.E());

            OrigTrackPx.push_back(e1.Px());
            OrigTrackPx.push_back(e2.Px());

            OrigTrackPy.push_back(e1.Py());
            OrigTrackPy.push_back(e2.Py());

            OrigTrackPz.push_back(e1.Pz());
            OrigTrackPz.push_back(e2.Pz());

            OrigTrackE.push_back(e1.E());
            OrigTrackE.push_back(e2.E());
            IDTracksContainerName.push_back(IDTracksContainerNames.at(tp1));
            IDTracksContainerName.push_back(IDTracksContainerNames.at(tp2));

            ATH_MSG_DEBUG( "pt = " << photon.Pt() << " ph " << ph.Pt() << " mass " << photon.M() << " px size " << RefTrackPx.size() );
            ATH_MSG_DEBUG( "Candidate DeltaM = " << (m_B + photon).M() << " MeV DiMuon " << " ( Mass = " << m_B.M() << " MeV )");

            // Decorate selected conversions
            ATH_MSG_DEBUG( "Decorating conversion vertices" );

            convVertexCandidate->auxdata<float>("px") = momentum.x();
            convVertexCandidate->auxdata<float>("py") = momentum.y();
            convVertexCandidate->auxdata<float>("pz") = momentum.z();

            convVertexCandidate->auxdata<float>("deltaQ") = deltaQ;
            convVertexCandidate->auxdata<float>("gamma_mass") = mass;
            convVertexCandidate->auxdata<float>("Rxy") = Rxy;
            convVertexCandidate->auxdata<std::vector<float>>("RefTrackPx") = RefTrackPx;
            convVertexCandidate->auxdata<std::vector<float>>("RefTrackPy") = RefTrackPy;
            convVertexCandidate->auxdata<std::vector<float>>("RefTrackPz") = RefTrackPz;
            convVertexCandidate->auxdata<std::vector<float>>("RefTrackE") = RefTrackE;

            convVertexCandidate->auxdata<std::vector<float>>("OrigTrackPx") = OrigTrackPx;
            convVertexCandidate->auxdata<std::vector<float>>("OrigTrackPy") = OrigTrackPy;
            convVertexCandidate->auxdata<std::vector<float>>("OrigTrackPz") = OrigTrackPz;
            convVertexCandidate->auxdata<std::vector<float>>("OrigTrackE") = OrigTrackE;
            convVertexCandidate->auxdata<std::vector<char>>("IDTracksContainerName") = IDTracksContainerName;
            convVertexCandidate->auxdata<Char_t>("passed_Gamma") = true; // Used in event skimming

            conversionContainer->push_back( convVertexCandidate.release() );

            // add cross-link to the original Bc+ vertex
            VertexLink BGammaLink;
            BGammaLink.setElement(conversionContainer->back());
            BGammaLink.setStorableObject(*conversionContainer);
            BGammaLinks(*vertex).push_back(std::move(BGammaLink));
          }
          else {
            ATH_MSG_DEBUG( "Vertex Fit Failed" );
          }

        }  // end of Track2 Loop
      }  // end of Track1 Loop
    } // end of Bc loop
  } // end of vertex container loop

  // Write the results to StoreGate
  CHECK(evtStore()->record(conversionContainer.release(), m_conversionContainerName));
  CHECK(evtStore()->record(conversionAuxContainer.release(), m_conversionContainerName + "Aux."));

  return StatusCode::SUCCESS;
}


// trackMomentum: returns refitted track momentum
TVector3 BPhysBGammaFinder::trackMomentum(const xAOD::Vertex* vxCandidate, int trkIndex) const {

  double px = 0.;
  double py = 0.;
  double pz = 0.;
  if (vxCandidate) {
    const Trk::TrackParameters* aPerigee = vxCandidate->vxTrackAtVertex()[trkIndex].perigeeAtVertex();
    px = aPerigee->momentum()[Trk::px];
    py = aPerigee->momentum()[Trk::py];
    pz = aPerigee->momentum()[Trk::pz];
  }

  return TVector3(px,py,pz);
}

}  // end of namespace DerivationFramework
