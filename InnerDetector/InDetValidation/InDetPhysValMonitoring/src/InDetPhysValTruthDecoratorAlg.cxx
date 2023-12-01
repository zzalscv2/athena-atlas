/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetPhysValTruthDecoratorAlg.cxx
 * @author shaun roe
 **/

#include "InDetPhysValTruthDecoratorAlg.h"
#include "safeDecorator.h"
#include <limits>

#include "xAODTruth/TruthVertex.h"
// #include "GeneratorUtils/PIDUtils.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "TrkParameters/TrackParameters.h" // Contains typedef to Trk::CurvilinearParameters
#include <cmath>

// ref:
// https://svnweb.cern.ch/trac/atlasoff/browser/Tracking/TrkEvent/TrkParametersBase/trunk/TrkParametersBase/CurvilinearParametersT.h





InDetPhysValTruthDecoratorAlg::InDetPhysValTruthDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

InDetPhysValTruthDecoratorAlg::~InDetPhysValTruthDecoratorAlg () {
  // nop
}

StatusCode
InDetPhysValTruthDecoratorAlg::initialize() {
  ATH_CHECK(m_extrapolator.retrieve());
  ATH_CHECK(m_beamSpotDecoKey.initialize());
  ATH_CHECK( m_truthPixelClusterName.initialize() );
  ATH_CHECK( m_truthSCTClusterName.initialize() );
  ATH_CHECK( m_truthSelectionTool.retrieve( EnableTool { not m_truthSelectionTool.name().empty() } ) );
  if (not m_truthSelectionTool.name().empty() ) {
    m_cutFlow = CutFlow(m_truthSelectionTool->nCuts() );
  }

  ATH_CHECK( m_truthParticleName.initialize());

  std::vector<std::string> decor_names(kNDecorators);
  decor_names[kDecorD0]="d0";
  decor_names[kDecorZ0]="z0";
  decor_names[kDecorPhi]="phi";
  decor_names[kDecorTheta]="theta";
  decor_names[kDecorZ0st]="z0st";
  decor_names[kDecorQOverP]="qOverP";
  decor_names[kDecorProdR]="prodR";
  decor_names[kDecorProdZ]="prodZ";
  decor_names[kDecorNSilHits]="nSilHits";

  IDPVM::createDecoratorKeysAndAccessor(*this, m_truthParticleName,m_prefix.value(),decor_names, m_decor);
  assert( m_decor.size() == kNDecorators);
  return StatusCode::SUCCESS;
}

StatusCode
InDetPhysValTruthDecoratorAlg::finalize() {
  if (not m_truthSelectionTool.name().empty()) {
    std::lock_guard<std::mutex> lock(m_mutex);
    ATH_MSG_DEBUG( "Truth selection cut flow : " << m_cutFlow.report(m_truthSelectionTool->names()) );
  }
  return StatusCode::SUCCESS;
}

StatusCode
InDetPhysValTruthDecoratorAlg::execute(const EventContext &ctx) const {
  SG::ReadHandle<xAOD::TruthParticleContainer> ptruth(m_truthParticleName, ctx);
  if ((not ptruth.isValid())) {
    return StatusCode::FAILURE;
  }

  std::vector< IDPVM::OptionalDecoration<xAOD::TruthParticleContainer,float> >
     float_decor( IDPVM::createDecoratorsIfNeeded(*ptruth, m_decor, ctx, msgLvl(MSG::DEBUG)) );

  ///truthbarcode-cluster maps to be pre-stored at event level
  std::map< int, float> barcodeSCTclustercount;
  std::map< int, float> barcodePIXclustercount;
  
  //Loop over the pixel and sct clusters to fill the truth barcode - cluster count maps
  SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> sctClusters(m_truthSCTClusterName, ctx); 
  SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> pixelClusters(m_truthPixelClusterName, ctx); 
  //only decorate the truth particles with truth silicon hits if both containers are available 
  if (sctClusters.isValid() && pixelClusters.isValid()) {
    for (const auto *const sct : *sctClusters) {
      const xAOD::TrackMeasurementValidation* sctCluster = sct;
      std::vector<int> truth_barcode;
      static const SG::AuxElement::ConstAccessor< std::vector<int> > barcodeAcc("truth_barcode");
      if (barcodeAcc.isAvailable(*sctCluster)) {
        truth_barcode = barcodeAcc(*sctCluster);
        std::map<int, float>::iterator it;
        for (auto barcode = truth_barcode.begin(); barcode != truth_barcode.end();  ++barcode) {
          auto result = barcodeSCTclustercount.emplace( std::pair<int, float>(*barcode, 0.0) ); 
          if (!result.second) ++(result.first->second); 
        }
      }
    } // Loop over SCT clusters
   
    for (const auto *const pix : *pixelClusters) {
      const xAOD::TrackMeasurementValidation* pixCluster = pix;
      std::vector<int> truth_barcode;
      static const SG::AuxElement::ConstAccessor< std::vector<int> > barcodeAcc("truth_barcode");
      if (barcodeAcc.isAvailable(*pixCluster)) {
        truth_barcode = barcodeAcc(*pixCluster);
        std::map<int, float>::iterator it;
        for (auto barcode = truth_barcode.begin(); barcode != truth_barcode.end();  ++barcode) {
          auto result = barcodePIXclustercount.emplace( std::pair<int, float>(*barcode, 0.0) ); 
          if (!result.second) ++(result.first->second); 
        }
      }
    } // Loop over PIX clusters
  }
  if (not float_decor.empty()) {
     SG::ReadDecorHandle<xAOD::EventInfo, float> beamPosX(m_beamSpotDecoKey[0], ctx);
     SG::ReadDecorHandle<xAOD::EventInfo, float> beamPosY(m_beamSpotDecoKey[1], ctx);
     SG::ReadDecorHandle<xAOD::EventInfo, float> beamPosZ(m_beamSpotDecoKey[2], ctx);
     Amg::Vector3D beamPos = Amg::Vector3D(beamPosX(0), beamPosY(0), beamPosZ(0));

     if ( m_truthSelectionTool.get() ) {
        CutFlow tmp_cut_flow(m_truthSelectionTool->nCuts());
        for (const xAOD::TruthParticle *truth_particle : *ptruth) {
           auto passed = m_truthSelectionTool->accept(truth_particle);
           tmp_cut_flow.update( passed.missingCuts() );
           if (not passed) continue;
           decorateTruth(*truth_particle, float_decor, beamPos, barcodePIXclustercount, barcodeSCTclustercount);
        }
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cutFlow.merge(std::move(tmp_cut_flow));
     }
     else {
        for (const xAOD::TruthParticle *truth_particle : *ptruth) {
           decorateTruth(*truth_particle, float_decor, beamPos, barcodePIXclustercount, barcodeSCTclustercount);
        }
     }
  }
  return StatusCode::SUCCESS;
}



bool
InDetPhysValTruthDecoratorAlg::decorateTruth(const xAOD::TruthParticle& particle,
                                             std::vector<IDPVM::OptionalDecoration<xAOD::TruthParticleContainer, float> > &float_decor,
                                             const Amg::Vector3D& beamPos, std::map<int, float> pixelMap, std::map<int, float> sctMap) const {
  ATH_MSG_VERBOSE("Decorate truth with d0 etc");
  if (particle.isNeutral()) {
    return false;
  }
  const EventContext& ctx = Gaudi::Hive::currentContext();
  const Amg::Vector3D momentum(particle.px(), particle.py(), particle.pz());
  const int pid(particle.pdgId());
  double charge = particle.charge();

  if (std::isnan(charge)) {
    ATH_MSG_DEBUG("charge not found on particle with pid " << pid);
    return false;
  }
   
  //Retrieve the cluster count from the pre-filled maps   
  std::map<int, float>::iterator it1, it2;
  it1 =pixelMap.find(particle.barcode());
  it2 =sctMap.find(particle.barcode());
  float nSiHits = 0;
  if (it1 !=pixelMap.end()) nSiHits += (*it1).second; 
  if (it2 !=sctMap.end()) nSiHits += (*it2).second; 
 
  IDPVM::decorateOrRejectQuietly(particle,float_decor[kDecorNSilHits], nSiHits);

  const xAOD::TruthVertex* ptruthVertex(nullptr);
  try{
    ptruthVertex = particle.prodVtx();
  } catch (const std::exception& e) {
    if (not m_errorEmitted) {
      ATH_MSG_WARNING("A non existent production vertex was requested in calculating the track parameters d0 etc");
    }
    m_errorEmitted = true;
    return false;
  }
  if (!ptruthVertex) {
    ATH_MSG_DEBUG("A production vertex pointer was retrieved, but it is NULL");
    return false;
  }
  const auto xPos = ptruthVertex->x();
  const auto yPos = ptruthVertex->y();
  const auto z_truth = ptruthVertex->z();
  const Amg::Vector3D position(xPos, yPos, z_truth);
  const float prodR_truth = std::sqrt(xPos * xPos + yPos * yPos);
  // delete ptruthVertex;ptruthVertex=0;
  const Trk::CurvilinearParameters cParameters(position, momentum, charge);

  Trk::PerigeeSurface persf(beamPos);

  std::unique_ptr<const Trk::TrackParameters> tP ( m_extrapolator->extrapolate(ctx,
                                                                               cParameters, 
                                                                               persf, Trk::anyDirection, false) );
  if (tP) {
    float d0_truth = tP->parameters()[Trk::d0];
    float theta_truth = tP->parameters()[Trk::theta];
    float z0_truth = tP->parameters()[Trk::z0];
    float phi_truth = tP->parameters()[Trk::phi];
    float qOverP_truth = tP->parameters()[Trk::qOverP]; // P or Pt ??
    float z0st_truth = z0_truth * std::sin(theta_truth);

    // 'safeDecorator' used to prevent a crash in case of adding something which pre-exists.
    // behaviour chosen is to reject quietly
    IDPVM::decorateOrRejectQuietly(particle,float_decor[kDecorD0],d0_truth);
    IDPVM::decorateOrRejectQuietly(particle,float_decor[kDecorZ0],z0_truth);
    IDPVM::decorateOrRejectQuietly(particle,float_decor[kDecorPhi],phi_truth);
    IDPVM::decorateOrRejectQuietly(particle,float_decor[kDecorTheta],theta_truth);
    IDPVM::decorateOrRejectQuietly(particle,float_decor[kDecorZ0st],z0st_truth);
    IDPVM::decorateOrRejectQuietly(particle,float_decor[kDecorQOverP],qOverP_truth);
    IDPVM::decorateOrRejectQuietly(particle,float_decor[kDecorProdR],prodR_truth);
    IDPVM::decorateOrRejectQuietly(particle,float_decor[kDecorProdZ],z_truth);

    return true;
  } else {
    ATH_MSG_DEBUG("The TrackParameters pointer for this TruthParticle is NULL");
    return false;
  }
}

/** references:
   https://svnweb.cern.ch/trac/atlasoff/browser/Event/xAOD/xAODTruth/trunk/Root/TruthParticle_v1.cxx
   Typedefs for various TrackParameters:
   https://svnweb.cern.ch/trac/atlasoff/browser/Tracking/TrkEvent/TrkParameters/trunk/TrkParameters/TrackParameters.h
   TrackParameters is a typedef to:
   https://svnweb.cern.ch/trac/atlasoff/browser/Tracking/TrkEvent/TrkParametersBase/trunk/TrkParametersBase
   https://svnweb.cern.ch/trac/atlasoff/browser/Tracking/TrkEvent/TrkParametersBase/trunk/TrkParametersBase/CurvilinearParametersT.h
   https://svnweb.cern.ch/trac/atlasoff/browser/Event/xAOD/xAODBase/trunk/xAODBase/IParticle.h
 **/

/**references:
   // https://svnweb.cern.ch/trac/atlasoff/browser/Event/xAOD/xAODTracking/trunk/xAODTracking/versions/TrackParticle_v1.h
   // https://svnweb.cern.ch/trac/atlasoff/browser/Event/xAOD/xAODTruth/trunk/xAODTruth/versions/TruthParticle_v1.h
   //original code like this:

   float m_track_truth_eta  = generatedTrackPerigee->eta();
   int ieta = int(m_track_truth_eta/0.25 + 10.0);
    if (ieta < 0 || ieta>19){
      delete generatedTrackPerigee;
      continue;
    }
   m_hd0[ieta]->Fill(trkd0-m_track_truth_d0);
 **/

/** Email from Andi Salzburger to Shaun Roe **

   the correct way is actually to extrapolate it.
   So you would need to retrieve the extrapolator:
    TooHandle<IExtrapolator> m_extrapolator;

   -> retrieve it in your code and use the AtlasExtrapolator (which is fully configured, and can be imported in python
      as from TrkExTools.AtlasExtrapolator import AtlasExtrapolator).
   Then, in your code you need 3 lines:

    Trk::CurvilinearParameters cParameters(truthParticle->position(), truthParticle->momentum(), truthParticle->charge()
       ); // sorry for the little pseudo_code
    // for the moment express to (0,0,0), we may change this later
    const Trk::TrackParameters* tP = m_extrapolator->extrapolate(cParametrs,Trk::PerigeeSurface(), Trk::anyDirection,
       false);
    if (tP){
      double d0_truth = tP->parameters()[Trk::d0];
    }
   This will be super-transparent, because even if TrackingCP decides later on to change away from (0,0,0) you just need
      to change the Trk::PerigeeSurface() to the new one with different position.

 **/
