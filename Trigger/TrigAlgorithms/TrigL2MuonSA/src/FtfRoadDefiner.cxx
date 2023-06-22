/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FtfRoadDefiner.h"

TrigL2MuonSA::FtfRoadDefiner::FtfRoadDefiner(const std::string& type,
                                             const std::string& name,
                                             const IInterface*  parent):
  AthAlgTool(type, name, parent)
{
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::FtfRoadDefiner::initialize()
{

  ATH_CHECK( m_extrapolator.retrieve() );

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::FtfRoadDefiner::defineRoad(const xAOD::TrackParticle* idtrack,
						    TrigL2MuonSA::MuonRoad&    muonRoad) const
{
  ATH_MSG_DEBUG("FtfRoadDefiner::defineRoad");

  const int N_SECTOR = 2; // 0: normal, 1:overlap

  double extFtfInnerEta=0.,  extFtfInnerZ=0.,  extFtfInnerR=0.,  extFtfInnerPhi=0.;
  double extFtfMiddleEta=0., extFtfMiddleZ=0., extFtfMiddleR=0., extFtfMiddlePhi=0.;
  double extFtfOuterEta=0.,  extFtfOuterZ=0.,  extFtfOuterR=0.,  extFtfOuterPhi=0.;

  double aw_ftf[3]={0.,0.,0.}; // slope of FTF Road for Inner/Middle/Outer
  double bw_ftf[3]={0.,0.,0.}; // intercept of FTF Road for Inner/Middle/Outer
  // Inner

  bool CylinderFirst = (std::abs(idtrack->eta()) < 1.05);

  double innerCylinderZ = 7500.;
  double middleCylinderZ = 14000.;
  double outerCylinderZ = 21500.;
  if(idtrack->eta() < 0) {
    innerCylinderZ = -innerCylinderZ;
    middleCylinderZ = -middleCylinderZ;
    outerCylinderZ = -outerCylinderZ;
  }

  auto extFtfInner = extTrack( CylinderFirst, idtrack, 4700., innerCylinderZ, muonRoad.ext_ftf_flag[0][0]);
  if( !extFtfInner ) {
    ATH_MSG_DEBUG("extrapolated track parameters on BarrelInner is null");
  } else {
    extFtfInnerEta = extFtfInner->eta();
    extFtfInnerPhi = extFtfInner->position().phi();
    extFtfInnerZ = extFtfInner->position().z();
    extFtfInnerR = std::hypot(extFtfInner->position().x(), extFtfInner->position().y());
    ATH_MSG_DEBUG("extFtfInnerEta: " << extFtfInnerEta << ", extFtfInnerPhi: " << extFtfInnerPhi << ", extFtfInnerZ: " << extFtfInnerZ << ", extFtfInnerR: " << extFtfInnerR);
    aw_ftf[0] = std::tan(2*std::atan(std::exp(-extFtfInnerEta)));
    bw_ftf[0] = extFtfInnerR - (aw_ftf[0])*extFtfInnerZ;
    muonRoad.r_ftf[0][0] = extFtfInnerR;
    muonRoad.z_ftf[0][0] = extFtfInnerZ;

    // Middle
    if(muonRoad.ext_ftf_flag[0][0]==0||
       muonRoad.ext_ftf_flag[0][0]==3)
      CylinderFirst = true;
    else
      CylinderFirst = false;

    auto extFtfMiddle = extTrack( CylinderFirst, *extFtfInner, 7300., middleCylinderZ, muonRoad.ext_ftf_flag[1][0]);
    if( !extFtfMiddle ) {
      ATH_MSG_DEBUG("extrapolated track parameters on BarrelMiddle is null");
    } else {
      extFtfMiddleEta = extFtfMiddle->eta();
      extFtfMiddlePhi = extFtfMiddle->position().phi();
      extFtfMiddleZ = extFtfMiddle->position().z();
      extFtfMiddleR = std::hypot(extFtfMiddle->position().x(), extFtfMiddle->position().y());
      ATH_MSG_DEBUG("extFtfMiddleEta: " << extFtfMiddleEta << ", extFtfMiddlePhi: " << extFtfMiddlePhi << ", extFtfMiddleZ: " << extFtfMiddleZ << ", extFtfMiddleR: " << extFtfMiddleR);
      aw_ftf[1] = std::tan(2*std::atan(std::exp(-extFtfMiddleEta)));
      bw_ftf[1] = extFtfMiddleR - (aw_ftf[1])*extFtfMiddleZ;
      muonRoad.r_ftf[1][0] = extFtfMiddleR;
      muonRoad.z_ftf[1][0] = extFtfMiddleZ;

      // Outer
      if(muonRoad.ext_ftf_flag[1][0]==0||
	 muonRoad.ext_ftf_flag[1][0]==3)
	CylinderFirst = true;
      else
	CylinderFirst = false;

      auto extFtfOuter = extTrack( CylinderFirst, *extFtfMiddle, 9800., outerCylinderZ, muonRoad.ext_ftf_flag[2][0]);
      if( !extFtfOuter ) {
	ATH_MSG_DEBUG("extrapolated track parameters on BarrelOuter is null");
      } else {
	extFtfOuterEta = extFtfOuter->eta();
	extFtfOuterPhi = extFtfOuter->position().phi();
	extFtfOuterZ = extFtfOuter->position().z();
	extFtfOuterR = std::hypot(extFtfOuter->position().x(), extFtfOuter->position().y());
	ATH_MSG_DEBUG("extFtfOuterEta: " << extFtfOuterEta << ", extFtfOuterPhi: " << extFtfOuterPhi << ", extFtfOuterZ: " << extFtfOuterZ << ", extFtfOuterR: " << extFtfOuterR);
	aw_ftf[2] = std::tan(2*std::atan(std::exp(-extFtfOuterEta)));
	bw_ftf[2] = extFtfOuterR - (aw_ftf[2])*extFtfOuterZ;
	muonRoad.r_ftf[2][0] = extFtfOuterR;
	muonRoad.z_ftf[2][0] = extFtfOuterZ;
      }
    }
  }

  muonRoad.extFtfMiddleEta = extFtfMiddleEta;
  muonRoad.extFtfMiddlePhi = extFtfMiddlePhi;

  for (int i_sector=0; i_sector<N_SECTOR; i_sector++) { // 0: normal sector, 1: overlap sector, which is used when a muon pass through boundary of MS sectors
    muonRoad.aw_ftf[0][i_sector]  = aw_ftf[0];
    muonRoad.bw_ftf[0][i_sector]  = bw_ftf[0];
    muonRoad.aw_ftf[1][i_sector]  = aw_ftf[1];
    muonRoad.bw_ftf[1][i_sector]  = bw_ftf[1];
    muonRoad.aw_ftf[2][i_sector]  = aw_ftf[2];
    muonRoad.bw_ftf[2][i_sector]  = bw_ftf[2];
    muonRoad.aw_ftf[3][i_sector]  = aw_ftf[0];
    muonRoad.bw_ftf[3][i_sector]  = bw_ftf[0];
    muonRoad.aw_ftf[4][i_sector]  = aw_ftf[1];
    muonRoad.bw_ftf[4][i_sector]  = bw_ftf[1];
    muonRoad.aw_ftf[5][i_sector]  = aw_ftf[2];
    muonRoad.bw_ftf[5][i_sector]  = bw_ftf[2];
    muonRoad.aw_ftf[6][i_sector]  = aw_ftf[0];
    muonRoad.bw_ftf[6][i_sector]  = bw_ftf[0];
    muonRoad.aw_ftf[7][i_sector]  = aw_ftf[0];
    muonRoad.bw_ftf[7][i_sector]  = bw_ftf[0];
    muonRoad.aw_ftf[8][i_sector]  = aw_ftf[0];
    muonRoad.bw_ftf[8][i_sector]  = bw_ftf[0];
    muonRoad.aw_ftf[9][i_sector]  = aw_ftf[1];//BME
    muonRoad.bw_ftf[9][i_sector]  = bw_ftf[1];
    muonRoad.aw_ftf[10][i_sector] = aw_ftf[1];//BMG
    muonRoad.bw_ftf[10][i_sector] = bw_ftf[1];

    muonRoad.eta_ftf[0][i_sector]  = extFtfInnerEta;
    muonRoad.phi_ftf[0][i_sector]  = extFtfInnerPhi;
    muonRoad.eta_ftf[1][i_sector]  = extFtfMiddleEta;
    muonRoad.phi_ftf[1][i_sector]  = extFtfMiddlePhi;
    muonRoad.eta_ftf[2][i_sector]  = extFtfOuterEta;
    muonRoad.phi_ftf[2][i_sector]  = extFtfOuterPhi;
    muonRoad.eta_ftf[3][i_sector]  = extFtfInnerEta;
    muonRoad.phi_ftf[3][i_sector]  = extFtfInnerPhi;
    muonRoad.eta_ftf[4][i_sector]  = extFtfMiddleEta;
    muonRoad.phi_ftf[4][i_sector]  = extFtfMiddlePhi;
    muonRoad.eta_ftf[5][i_sector]  = extFtfOuterEta;
    muonRoad.phi_ftf[5][i_sector]  = extFtfOuterPhi;
    muonRoad.eta_ftf[6][i_sector]  = extFtfInnerEta;
    muonRoad.phi_ftf[6][i_sector]  = extFtfInnerPhi;
    muonRoad.eta_ftf[7][i_sector]  = extFtfInnerEta;
    muonRoad.phi_ftf[7][i_sector]  = extFtfInnerPhi;
    muonRoad.eta_ftf[8][i_sector]  = extFtfInnerEta;
    muonRoad.phi_ftf[8][i_sector]  = extFtfInnerPhi;
    muonRoad.eta_ftf[9][i_sector]  = extFtfMiddleEta;//BME
    muonRoad.phi_ftf[9][i_sector]  = extFtfMiddlePhi;
    muonRoad.eta_ftf[10][i_sector] = extFtfMiddleEta;//BMG
    muonRoad.phi_ftf[10][i_sector] = extFtfMiddlePhi;
  }

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

// extrapolate a FTF track to MS in order to define FTF Road
std::unique_ptr<const Trk::TrackParameters> TrigL2MuonSA::FtfRoadDefiner::extTrack( const bool CylinderFirst, const xAOD::TrackParticle* trk, const double R, const double Z, int& extFlag ) const
{

  const EventContext& ctx = Gaudi::Hive::currentContext();
  const bool boundaryCheck = true;

  // Cylinder
  std::unique_ptr<const Trk::CylinderSurface> barrel = std::make_unique<const Trk::CylinderSurface>( R, Z );

  // Disk
  Amg::Transform3D matrix = Amg::Transform3D( Amg::Vector3D( 0.,0.,Z ) );
  std::unique_ptr<const Trk::DiscSurface> disc = std::make_unique<const Trk::DiscSurface>( matrix, 0, R );

  ATH_MSG_DEBUG("R: " << R << ", Z: " << Z);

  if(CylinderFirst) {
    std::unique_ptr<const Trk::TrackParameters> param1( m_extrapolator->extrapolate(ctx,
										    trk->perigeeParameters(),
										    *barrel,
										    Trk::anyDirection, boundaryCheck, Trk::muon) );
    if(param1){
      ATH_MSG_DEBUG("Cylinder -> eta: " << param1->eta() << ", phi: " << param1->position().phi() << ", Z: " << param1->position().z() << ", Rms: " << std::hypot(param1->position().x(), param1->position().y()));
      extFlag = 0;
      return param1;
    }
    // Cylinder failed, try Disk
    std::unique_ptr<const Trk::TrackParameters> param2( m_extrapolator->extrapolate(ctx,
										    trk->perigeeParameters(),
										    *disc,
										    Trk::anyDirection, boundaryCheck, Trk::muon) );
    if(param2){
      ATH_MSG_DEBUG("Disk     -> eta: " << param2->eta() << ", phi: " << param2->position().phi() << ", Z: " << param2->position().z() << ", Rms: " << std::hypot(param2->position().x(), param2->position().y()));
      extFlag = 1;
      return param2;
    }
  }
  else { // Endcap First
    std::unique_ptr<const Trk::TrackParameters> param2( m_extrapolator->extrapolate(ctx,
										    trk->perigeeParameters(),
										    *disc,
										    Trk::anyDirection, boundaryCheck, Trk::muon) );
    if(param2){
      ATH_MSG_DEBUG("Disk     -> eta: " << param2->eta() << ", phi: " << param2->position().phi() << ", Z: " << param2->position().z() << ", Rms: " << std::hypot(param2->position().x(), param2->position().y()));
      extFlag = 2;
      return param2;
    }
    // Disk failed, try Cylinder
    std::unique_ptr<const Trk::TrackParameters> param1( m_extrapolator->extrapolate(ctx,
										    trk->perigeeParameters(),
										    *barrel,
										    Trk::anyDirection, boundaryCheck, Trk::muon) );
    if(param1){
      ATH_MSG_DEBUG("Cylinder -> eta: " << param1->eta() << ", phi: " << param1->position().phi() << ", Z: " << param1->position().z() << ", Rms: " << std::hypot(param1->position().x(), param1->position().y()));
      extFlag = 3;
      return param1;
    }
  }

  extFlag = 4;

  return nullptr;
}

// extrapolate a FTF track to MS in order to define FTF Road
std::unique_ptr<const Trk::TrackParameters> TrigL2MuonSA::FtfRoadDefiner::extTrack( const bool CylinderFirst, const Trk::TrackParameters& param, const double R, const double Z, int& extFlag ) const
{

  const EventContext& ctx = Gaudi::Hive::currentContext();
  const bool boundaryCheck = true;

  // Cylinder
  std::unique_ptr<const Trk::CylinderSurface> barrel = std::make_unique<const Trk::CylinderSurface>( R, Z );

  // Disk
  Amg::Transform3D matrix = Amg::Transform3D( Amg::Vector3D( 0.,0.,Z ) );
  std::unique_ptr<const Trk::DiscSurface> disc = std::make_unique<const Trk::DiscSurface>( matrix, 0, R );

  ATH_MSG_DEBUG("R: " << R << ", Z: " << Z);

  if(CylinderFirst) {
    std::unique_ptr<const Trk::TrackParameters> param1( m_extrapolator->extrapolate(ctx,
										    param,
										    *barrel,
										    Trk::anyDirection, boundaryCheck, Trk::muon) );
    if(param1){
      ATH_MSG_DEBUG("Cylinder -> eta: " << param1->eta() << ", phi: " << param1->position().phi() << ", Z: " << param1->position().z() << ", Rms: " << std::hypot(param1->position().x(), param1->position().y()));
      extFlag = 0;
      return param1;
    }
    // Cylinder failed, try Disk
    std::unique_ptr<const Trk::TrackParameters> param2( m_extrapolator->extrapolate(ctx,
										    param,
										    *disc,
										    Trk::anyDirection, boundaryCheck, Trk::muon) );
    if(param2){
      ATH_MSG_DEBUG("Disk     -> eta: " << param2->eta() << ", phi: " << param2->position().phi() << ", Z: " << param2->position().z() << ", Rms: " << std::hypot(param2->position().x(), param2->position().y()));
      extFlag = 1;
      return param2;
    }
  }
  else { // Endcap First
    std::unique_ptr<const Trk::TrackParameters> param2( m_extrapolator->extrapolate(ctx,
										    param,
										    *disc,
										    Trk::anyDirection, boundaryCheck, Trk::muon) );
    if(param2){
      ATH_MSG_DEBUG("Disk     -> eta: " << param2->eta() << ", phi: " << param2->position().phi() << ", Z: " << param2->position().z() << ", Rms: " << std::hypot(param2->position().x(), param2->position().y()));
      extFlag = 2;
      return param2;
    }
    // Disk failed, try Cylinder
    std::unique_ptr<const Trk::TrackParameters> param1( m_extrapolator->extrapolate(ctx,
										    param,
										    *barrel,
										    Trk::anyDirection, boundaryCheck, Trk::muon) );
    if(param1){
      ATH_MSG_DEBUG("Cylinder -> eta: " << param1->eta() << ", phi: " << param1->position().phi() << ", Z: " << param1->position().z() << ", Rms: " << std::hypot(param1->position().x(), param1->position().y()));
      extFlag = 3;
      return param1;
    }
  }

  extFlag = 4;

  return nullptr;
}
