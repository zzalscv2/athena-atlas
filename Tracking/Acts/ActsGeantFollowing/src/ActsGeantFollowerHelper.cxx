/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ActsGeantFollowerHelper.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// StoreGate
#include "ActsGeantFollowerHelper.h"
#include "TTree.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/EventContext.h"

// CLHEP
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Geometry/Transform3D.h"
// Trk
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkSurfaces/PlaneSurface.h"
// Amg
#include "GeoPrimitives/GeoPrimitives.h"
//other
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "TrkExInterfaces/IExtrapolationEngine.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"

// constructor
ActsGeantFollowerHelper::ActsGeantFollowerHelper(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p),
  m_extrapolationEngine(""),
  m_actsExtrapolator(""),
  m_extrapolateDirectly(true),
  m_extrapolateIncrementally(true),
  m_parameterCache(nullptr),
  m_actsParameterCache(std::nullopt),
  m_tX0Cache(0.),
  m_tX0NonSensitiveCache(0.),
  m_tNonSensitiveCache(0.),
  m_tX0CacheActs(0.),
  m_tX0CacheATLAS(0.),
  m_validationTreeName("G4Follower_"+n),
  m_validationTreeDescription("Output of the G4Follower_"),
  m_validationTreeFolder("/val/G4Follower_"+n),
  m_validationTree(nullptr)
{
  // properties
  declareProperty("ExtrapolationEngine",            m_extrapolationEngine);
  declareProperty("ActsExtrapolator",               m_actsExtrapolator);
  declareProperty("ExtrapolateDirectly",            m_extrapolateDirectly);
  declareProperty("ExtrapolateIncrementally",       m_extrapolateIncrementally);
}

// destructor
ActsGeantFollowerHelper::~ActsGeantFollowerHelper()
{}

// Athena standard methods
// initialize
StatusCode ActsGeantFollowerHelper::initialize()
{
  m_treeData = std::make_unique<TreeData>();
  
  // if (m_extrapolator.retrieve().isFailure()){
  //   ATH_MSG_ERROR("Could not retrieve Extrapolator " << m_extrapolator << " . Abort.");
  //   return StatusCode::FAILURE;
  // }
  if (m_extrapolationEngine.retrieve().isFailure()){
    ATH_MSG_ERROR("Could not retrieve Extrapolator Engine " << m_extrapolationEngine << " . Abort.");
    return StatusCode::FAILURE;
  }
  
  if (m_actsExtrapolator.retrieve().isFailure()){
    ATH_MSG_ERROR("Could not retrieve ActsExtrapolator " << m_actsExtrapolator << " . Abort.");
    return StatusCode::FAILURE;
  }

  // create the new Tree
  m_validationTree = new TTree(m_validationTreeName.c_str(), m_validationTreeDescription.c_str());

  m_validationTree->Branch("InitX",        &m_treeData->m_t_x,       "initX/F");
  m_validationTree->Branch("InitY",        &m_treeData->m_t_y,       "initY/F");
  m_validationTree->Branch("InitZ",        &m_treeData->m_t_z,       "initZ/F");
  m_validationTree->Branch("InitTheta",    &m_treeData->m_t_theta,   "initTheta/F");
  m_validationTree->Branch("InitEta",      &m_treeData->m_t_eta,     "initEta/F");
  m_validationTree->Branch("InitPhi",      &m_treeData->m_t_phi,     "initPhi/F");
  m_validationTree->Branch("InitP",        &m_treeData->m_t_p,       "initP/F");
  m_validationTree->Branch("InitPdg",      &m_treeData->m_t_pdg,     "initPdg/I");
  m_validationTree->Branch("InitCharge",   &m_treeData->m_t_charge,  "initQ/F");

  m_validationTree->Branch("G4Steps",      &m_treeData->m_g4_steps, "g4steps/I");
  m_validationTree->Branch("G4StepPt",     m_treeData->m_g4_pt,     "g4stepPt[g4steps]/F");
  m_validationTree->Branch("G4StepEta",    m_treeData->m_g4_eta,    "g4stepEta[g4steps]/F");
  m_validationTree->Branch("G4StepTheta",  m_treeData->m_g4_theta,  "g4stepTheta[g4steps]/F");
  m_validationTree->Branch("G4StepPhi",    m_treeData->m_g4_phi,    "g4stepPhi[g4steps]/F");
  m_validationTree->Branch("G4StepX",      m_treeData->m_g4_x,      "g4stepX[g4steps]/F");
  m_validationTree->Branch("G4StepY",      m_treeData->m_g4_y,      "g4stepY[g4steps]/F");
  m_validationTree->Branch("G4StepZ",      m_treeData->m_g4_z,      "g4stepZ[g4steps]/F");
  m_validationTree->Branch("G4StepTX0",    m_treeData->m_g4_tX0,    "g4stepTX0[g4steps]/F");
  m_validationTree->Branch("G4AccumX0",    m_treeData->m_g4_accX0,  "g4stepAccTX0[g4steps]/F");
  m_validationTree->Branch("G4StepT",      m_treeData->m_g4_t,      "g4stepTX[g4steps]/F");
  m_validationTree->Branch("G4StepX0",     m_treeData->m_g4_X0,     "g4stepX0[g4steps]/F");

  m_validationTree->Branch("TrkStepStatus",m_treeData->m_trk_status, "trkstepStatus[g4steps]/I");
  m_validationTree->Branch("TrkStepPt",    m_treeData->m_trk_pt,     "trkstepPt[g4steps]/F");
  m_validationTree->Branch("TrkStepEta",   m_treeData->m_trk_eta,    "trkstepEta[g4steps]/F");
  m_validationTree->Branch("TrkStepTheta", m_treeData->m_trk_theta,  "trkstepTheta[g4steps]/F");
  m_validationTree->Branch("TrkStepPhi",   m_treeData->m_trk_phi,    "trkstepPhi[g4steps]/F");
  m_validationTree->Branch("TrkStepX",     m_treeData->m_trk_x,      "trkstepX[g4steps]/F");
  m_validationTree->Branch("TrkStepY",     m_treeData->m_trk_y,      "trkstepY[g4steps]/F");
  m_validationTree->Branch("TrkStepZ",     m_treeData->m_trk_z,      "trkstepZ[g4steps]/F");
  m_validationTree->Branch("TrkStepLocX",  m_treeData->m_trk_lx,     "trkstepLX[g4steps]/F");
  m_validationTree->Branch("TrkStepLocY",  m_treeData->m_trk_ly,     "trkstepLY[g4steps]/F");
  m_validationTree->Branch("TrkStepTX0",   m_treeData->m_trk_tX0,     "trkstepTX0[g4steps]/F");
  m_validationTree->Branch("TrkAccumX0",   m_treeData->m_trk_accX0,   "trkstepAccTX0[g4steps]/F");
  m_validationTree->Branch("TrkStepT",     m_treeData->m_trk_t,       "trkstepTX[g4steps]/F");
  m_validationTree->Branch("TrkStepX0",    m_treeData->m_trk_X0,      "trkstepX0[g4steps]/F");

  m_validationTree->Branch("ActsStepStatus",m_treeData->m_acts_status,  "actsstepStatus[g4steps]/I");
  m_validationTree->Branch("ActsVolumeId",  m_treeData->m_acts_volumeID,"actsvolumeid[g4steps]/I");
  m_validationTree->Branch("ActsStepPt",    m_treeData->m_acts_pt,      "actsstepPt[g4steps]/F");
  m_validationTree->Branch("ActsStepEta",   m_treeData->m_acts_eta,     "actsstepEta[g4steps]/F");
  m_validationTree->Branch("ActsStepTheta", m_treeData->m_acts_theta,   "actsstepTheta[g4steps]/F");
  m_validationTree->Branch("ActsStepPhi",   m_treeData->m_acts_phi,     "actsstepPhi[g4steps]/F");
  m_validationTree->Branch("ActsStepX",     m_treeData->m_acts_x,       "actsstepX[g4steps]/F");
  m_validationTree->Branch("ActsStepY",     m_treeData->m_acts_y,       "actsstepY[g4steps]/F");
  m_validationTree->Branch("ActsStepZ",     m_treeData->m_acts_z,       "actsstepZ[g4steps]/F");
  m_validationTree->Branch("ActsStepTX0",   m_treeData->m_acts_tX0,     "actsstepTX0[g4steps]/F");
  m_validationTree->Branch("ActsAccumX0",   m_treeData->m_acts_accX0,   "actsstepAccTX0[g4steps]/F");
  m_validationTree->Branch("ActsStepT",     m_treeData->m_acts_t,       "actsstepTX[g4steps]/F");
  m_validationTree->Branch("ActsStepX0",    m_treeData->m_acts_X0,      "actsstepX0[g4steps]/F");

  // now register the Tree
  ITHistSvc* tHistSvc = 0;
  if (service("THistSvc",tHistSvc).isFailure()){
    ATH_MSG_ERROR( "Could not find Hist Service -> Switching ValidationMode Off !" );
    delete m_validationTree; m_validationTree = 0;
  }
  if ((tHistSvc->regTree(m_validationTreeFolder, m_validationTree)).isFailure()) {
    ATH_MSG_ERROR( "Could not register the validation Tree -> Switching ValidationMode Off !" );
    delete m_validationTree; m_validationTree = 0;
  }

  ATH_MSG_INFO("initialize() successful" );
  return StatusCode::SUCCESS;
}

StatusCode ActsGeantFollowerHelper::finalize()
{
  return StatusCode::SUCCESS;
}

void ActsGeantFollowerHelper::beginEvent()
{
  m_treeData->m_t_x        = 0.;
  m_treeData->m_t_y        = 0.;
  m_treeData->m_t_z        = 0.;
  m_treeData->m_t_theta    = 0.;
  m_treeData->m_t_eta      = 0.;
  m_treeData->m_t_phi      = 0.;
  m_treeData->m_t_p        = 0.;
  m_treeData->m_t_charge   = 0.;
  m_treeData->m_t_pdg      = 0;
  m_treeData->m_g4_steps   = 0;
  m_tX0Cache   = 0.;
  m_tX0NonSensitiveCache = 0.;
  m_tNonSensitiveCache = 0.;
  m_tX0CacheActs   = 0.;
  m_tX0CacheATLAS   = 0.;
}

void ActsGeantFollowerHelper::trackParticle(const G4ThreeVector& pos,
                                             const G4ThreeVector& mom,
                                             int pdg, double charge,
                                             float t, float X0, bool isSensitive)
{
  // const EventContext ctx;
  const EventContext &ctx = Gaudi::Hive::currentContext();
  const ActsGeometryContext &gctx = m_actsExtrapolator->trackingGeometryTool()->getGeometryContext(ctx);
  auto trackingGeometry = m_actsExtrapolator->trackingGeometryTool()->trackingGeometry();
  // construct the initial parameters
  Amg::Vector3D npos(pos.x(),pos.y(),pos.z());
  Amg::Vector3D nmom(mom.x(),mom.y(),mom.z());

  // Use the G4 pdgId as the particle hypothesis
  Trk::ParticleHypothesis particleHypo = m_pdgToParticleHypothesis.convert(m_treeData->m_t_pdg, m_treeData->m_t_charge);

  if(m_treeData->m_g4_steps == 0 && m_tNonSensitiveCache == 0){
    ATH_MSG_INFO("Initial step ... preparing event cache.");
    m_treeData->m_t_x        = pos.x();
    m_treeData->m_t_y        = pos.y();
    m_treeData->m_t_z        = pos.z();
    m_treeData->m_t_theta    = mom.theta();
    m_treeData->m_t_eta      = mom.eta();
    m_treeData->m_t_phi      = mom.phi();
    m_treeData->m_t_p        = mom.mag();
    m_treeData->m_t_charge   = charge;
    m_treeData->m_t_pdg      = pdg;
    m_treeData->m_g4_steps   = -1;
    m_tX0Cache     = 0.;
    m_tX0CacheActs = 0.;
    m_parameterCache = new Trk::CurvilinearParameters(npos, nmom, charge);

    std::shared_ptr<Acts::PerigeeSurface> surface =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(
        npos);


    float mass = Trk::ParticleMasses::mass[particleHypo] * Acts::UnitConstants::MeV;

    Acts::Vector4 actsStart(pos.x(),pos.y(),pos.z(),0);
    Acts::Vector3 dir = nmom.normalized();
    Acts::ParticleHypothesis hypothesis{Acts::makeAbsolutePdgParticle(static_cast<Acts::PdgParticle>(pdg)), 
                                        mass, 
                                        Acts::AnyCharge{static_cast<float>(charge)}};
    m_actsParameterCache = Acts::GenericBoundTrackParameters<Acts::ParticleHypothesis>::create(
        surface, gctx.context(), actsStart, dir, charge/(mom.mag()/1000), std::nullopt, hypothesis)
      .value();
  }

  // Store material in cache
  float tX0 = X0 > 10e-5 ? t/X0 : 0.;
  m_tX0NonSensitiveCache += tX0;
  m_tNonSensitiveCache += t;
  if (!isSensitive)
  {
    return;
  }

  // jumping over inital step
  m_treeData->m_g4_steps = (m_treeData->m_g4_steps == -1) ? 0 : m_treeData->m_g4_steps;

  if (!m_parameterCache){
    ATH_MSG_WARNING("No Parameters available. Bailing out.");
    return;
  }

  if ( m_treeData->m_g4_steps >= MAXPROBES) {
    ATH_MSG_WARNING("Maximum number of " << MAXPROBES << " reached, step is ignored.");
    return;
  }
  // parameters of the G4 step point
  Trk::CurvilinearParameters* g4Parameters = new Trk::CurvilinearParameters(npos, nmom, m_treeData->m_t_charge);
  // destination surface
  const Trk::PlaneSurface& destinationSurface = g4Parameters->associatedSurface();
  // extrapolate to the destination surface
  Trk::ExtrapolationCell<Trk::TrackParameters> ecc(*m_parameterCache);
  ecc.setParticleHypothesis((Trk::ParticleHypothesis) particleHypo);
  ecc.addConfigurationMode(Trk::ExtrapolationMode::StopAtBoundary);
  ecc.addConfigurationMode(Trk::ExtrapolationMode::CollectMaterial);
  // call the extrapolation engine
  auto eCodeSteps = m_extrapolationEngine->extrapolate(ecc, &destinationSurface);
  Trk::TrackParameters *trkParameters = ecc.endParameters;
  float X0ATLAS = ecc.materialX0;

  if(eCodeSteps.code != 2 ){
    ATH_MSG_ERROR("Error in the Extrapolator Engine, skip the current step");  
    return;
  }

  // create a Acts::Surface that correspond to the Trk::Surface
  auto destinationSurfaceActs = Acts::Surface::makeShared<Acts::PlaneSurface>(destinationSurface.center(), destinationSurface.normal());
  std::optional<Acts::BoundTrackParameters> actsParameters = m_actsExtrapolator->propagate(ctx, 
											   *m_actsParameterCache, 
											   *destinationSurfaceActs, 
											   Acts::Direction::Forward,
											   std::numeric_limits<double>::max());

  float X0Acts = m_actsExtrapolator->propagationSteps(ctx,
                                                       *m_actsParameterCache, 
                                                       *destinationSurfaceActs,
                                                       Acts::Direction::Forward,
                                                       std::numeric_limits<double>::max()).second.materialInX0;
                                                       
  if(not actsParameters.has_value()){
    ATH_MSG_ERROR("Error in the Acts extrapolation, skip the current step");  
    return;
  }
  int volID = trackingGeometry->lowestTrackingVolume(gctx.context(), actsParameters->position(gctx.context()))->geometryId().volume();

  // fill the geant information and the trk information
  m_treeData->m_g4_pt[m_treeData->m_g4_steps]      =  mom.mag()/std::cosh(mom.eta());
  m_treeData->m_g4_eta[m_treeData->m_g4_steps]     =  mom.eta();
  m_treeData->m_g4_theta[m_treeData->m_g4_steps]   =  mom.theta();
  m_treeData->m_g4_phi[m_treeData->m_g4_steps]     =  mom.phi();
  m_treeData->m_g4_x[m_treeData->m_g4_steps]       =  pos.x();
  m_treeData->m_g4_y[m_treeData->m_g4_steps]       =  pos.y();
  m_treeData->m_g4_z[m_treeData->m_g4_steps]       =  pos.z();
  
  m_tX0Cache                                       += m_tX0NonSensitiveCache;
  m_treeData->m_g4_tX0[m_treeData->m_g4_steps]     = m_tX0NonSensitiveCache;
  m_treeData->m_g4_accX0[m_treeData->m_g4_steps]   = m_tX0Cache;
  m_treeData->m_g4_t[m_treeData->m_g4_steps]       = m_tNonSensitiveCache;
  m_treeData->m_g4_X0[m_treeData->m_g4_steps]      = m_tNonSensitiveCache/m_tX0NonSensitiveCache;

  m_treeData->m_trk_status[m_treeData->m_g4_steps] = trkParameters ? 1 : 0;
  m_treeData->m_trk_pt[m_treeData->m_g4_steps]      = trkParameters ? trkParameters->pT()      : 0.;
  m_treeData->m_trk_eta[m_treeData->m_g4_steps]    = trkParameters ? trkParameters->momentum().eta()      : 0.;
  m_treeData->m_trk_theta[m_treeData->m_g4_steps]  = trkParameters ? trkParameters->momentum().theta()    : 0.;
  m_treeData->m_trk_phi[m_treeData->m_g4_steps]    = trkParameters ? trkParameters->momentum().phi()      : 0.;
  m_treeData->m_trk_x[m_treeData->m_g4_steps]      = trkParameters ? trkParameters->position().x()        : 0.;
  m_treeData->m_trk_y[m_treeData->m_g4_steps]      = trkParameters ? trkParameters->position().y()        : 0.;
  m_treeData->m_trk_z[m_treeData->m_g4_steps]      = trkParameters ? trkParameters->position().z()        : 0.;
  m_treeData->m_trk_lx[m_treeData->m_g4_steps]     = trkParameters ? trkParameters->parameters()[Trk::locX] : 0.;
  m_treeData->m_trk_ly[m_treeData->m_g4_steps]     = trkParameters ? trkParameters->parameters()[Trk::locY] : 0.;
  // Incremental extrapolation, the extrapolation correspond to one step
  if(m_extrapolateIncrementally || m_treeData->m_g4_steps == 0){
    float tATLAS = (trkParameters->position() - m_parameterCache->position()).norm();
    m_tX0CacheATLAS                                  += X0ATLAS;
    m_treeData->m_trk_tX0[m_treeData->m_g4_steps]     = X0ATLAS;
    m_treeData->m_trk_accX0[m_treeData->m_g4_steps]   = m_tX0CacheATLAS;
    m_treeData->m_trk_t[m_treeData->m_g4_steps]       = tATLAS;
    m_treeData->m_trk_X0[m_treeData->m_g4_steps]      = tATLAS/X0ATLAS;
  }
  // Extrapolation perform from the start, step varaible need to be computed by comparing to the last extrapolation.
  else{
    Amg::Vector3D previousPos(m_treeData->m_trk_x[m_treeData->m_g4_steps-1],
                              m_treeData->m_trk_y[m_treeData->m_g4_steps-1],
                              m_treeData->m_trk_z[m_treeData->m_g4_steps-1]);
    float tATLAS = (trkParameters->position() - previousPos).norm();
    m_treeData->m_trk_tX0[m_treeData->m_g4_steps]     = X0ATLAS - m_treeData->m_trk_accX0[m_treeData->m_g4_steps-1]   ;
    m_treeData->m_trk_accX0[m_treeData->m_g4_steps]   = X0ATLAS;
    m_treeData->m_trk_t[m_treeData->m_g4_steps]       = tATLAS;
    m_treeData->m_trk_X0[m_treeData->m_g4_steps]      = tATLAS/m_treeData->m_trk_tX0[m_treeData->m_g4_steps];
  }

  m_treeData->m_acts_status[m_treeData->m_g4_steps] = actsParameters ? 1 : 0;
  m_treeData->m_acts_volumeID[m_treeData->m_g4_steps] = actsParameters ? volID : 0;
  m_treeData->m_acts_pt[m_treeData->m_g4_steps]      = actsParameters ? actsParameters->transverseMomentum()*1000     : 0.;
  m_treeData->m_acts_eta[m_treeData->m_g4_steps]    = actsParameters ? actsParameters->momentum().eta()     : 0.;
  m_treeData->m_acts_theta[m_treeData->m_g4_steps]  = actsParameters ? actsParameters->momentum().theta()   : 0.;
  m_treeData->m_acts_phi[m_treeData->m_g4_steps]    = actsParameters ? actsParameters->momentum().phi()     : 0.;
  m_treeData->m_acts_x[m_treeData->m_g4_steps]      = actsParameters ? actsParameters->position(gctx.context()).x()   : 0.;
  m_treeData->m_acts_y[m_treeData->m_g4_steps]      = actsParameters ? actsParameters->position(gctx.context()).y()   : 0.;
  m_treeData->m_acts_z[m_treeData->m_g4_steps]      = actsParameters ? actsParameters->position(gctx.context()).z()   : 0.;
  // Incremental extrapolation, the extrapolation correspond to one step
  if(m_extrapolateIncrementally || m_treeData->m_g4_steps == 0){
    float tActs = (actsParameters->position(gctx.context()) - m_actsParameterCache->position(gctx.context())).norm();
    m_tX0CacheActs                                    += X0Acts;
    m_treeData->m_acts_tX0[m_treeData->m_g4_steps]     = X0Acts;
    m_treeData->m_acts_accX0[m_treeData->m_g4_steps]   = m_tX0CacheActs;
    m_treeData->m_acts_t[m_treeData->m_g4_steps]       = tActs;
    m_treeData->m_acts_X0[m_treeData->m_g4_steps]      = tActs/X0Acts;
  }
  // Extrapolation perform from the start, step varaible need to be computed by comparing to the last extrapolation.
  else{
    Acts::Vector3 previousPos(m_treeData->m_acts_x[m_treeData->m_g4_steps-1],
                               m_treeData->m_acts_y[m_treeData->m_g4_steps-1],
                               m_treeData->m_acts_z[m_treeData->m_g4_steps-1]);
    float tActs = (actsParameters->position(gctx.context()) - previousPos).norm();
    m_treeData->m_acts_tX0[m_treeData->m_g4_steps]     = X0Acts - m_treeData->m_acts_accX0[m_treeData->m_g4_steps-1]   ;
    m_treeData->m_acts_accX0[m_treeData->m_g4_steps]   = X0Acts;
    m_treeData->m_acts_t[m_treeData->m_g4_steps]       = tActs;
    m_treeData->m_acts_X0[m_treeData->m_g4_steps]      = tActs/m_treeData->m_acts_tX0[m_treeData->m_g4_steps];
  }

  // update the parameters if needed/configured
  if (m_extrapolateIncrementally && trkParameters && actsParameters) {
    delete m_parameterCache;
    m_actsParameterCache.reset();
    m_parameterCache = trkParameters;
    m_actsParameterCache = actsParameters;
  }
  // delete cache and increment
  delete g4Parameters;
  destinationSurfaceActs.reset();
  m_tX0NonSensitiveCache = 0.;
  m_tNonSensitiveCache = 0.;
  ++m_treeData->m_g4_steps;
}

void ActsGeantFollowerHelper::endEvent()
{
  if (m_tX0Cache != 0)
  {
    // fill the validation tree
    m_validationTree->Fill();
    delete m_parameterCache;
    m_actsParameterCache.reset();
  }
}
