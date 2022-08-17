/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCluster_OnTrackBuilder.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

#include "CaloUtils/CaloLayerCalculator.h"

#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkSurfaces/Surface.h"

#include "TrkCaloCluster_OnTrack/CaloCluster_OnTrack.h"

#include "StoreGate/ReadHandle.h"

CaloCluster_OnTrackBuilder::CaloCluster_OnTrackBuilder(const std::string& t,
                                                       const std::string& n,
                                                       const IInterface* p)
  : AthAlgTool(t, n, p)
{
  declareInterface<ICaloCluster_OnTrackBuilder>(this);
  m_eg_resol = std::make_unique<eg_resolution>("run2_R21_v1");
}

CaloCluster_OnTrackBuilder::~CaloCluster_OnTrackBuilder() = default;

StatusCode
CaloCluster_OnTrackBuilder::initialize()
{
  ATH_CHECK(m_calosurf.retrieve());
  ATH_CHECK(m_caloMgrKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode
CaloCluster_OnTrackBuilder::finalize()
{
  return StatusCode::SUCCESS;
}

Trk::CaloCluster_OnTrack*
CaloCluster_OnTrackBuilder::buildClusterOnTrack(
  const EventContext& ctx,
  const xAOD::CaloCluster* cluster,
  int charge) const
{

  ATH_MSG_DEBUG("Building Trk::CaloCluster_OnTrack");

  if (!m_useClusterPhi && !m_useClusterEta && !m_useClusterEnergy) {
    ATH_MSG_WARNING("CaloCluster_OnTrackBuilder is configured incorrectly");
    return nullptr;
  }

  if (!cluster) {
    return nullptr;
  }

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{ m_caloMgrKey, ctx };
  const CaloDetDescrManager* caloDDMgr = *caloMgrHandle;
  std::unique_ptr<Trk::Surface> surface = getCaloSurface(cluster, caloDDMgr);

  if (!surface) {
    return nullptr;
  }

  Trk::LocalParameters lp =
    getClusterLocalParameters(cluster, surface.get(), charge);

  const Amg::MatrixX em = getClusterErrorMatrix(cluster, *surface, charge);

  Trk::CaloCluster_OnTrack* ccot =
    new Trk::CaloCluster_OnTrack(lp, em, *surface);

  if (ccot) {
    ATH_MSG_DEBUG("Successful build of Trk::CaloCluster_OnTrack");
  }

  return ccot;
}

std::unique_ptr<Trk::Surface>
CaloCluster_OnTrackBuilder::getCaloSurface(
  const xAOD::CaloCluster* cluster,
  const CaloDetDescrManager* caloDDMgr) const
{

  std::unique_ptr<Trk::Surface> destinationSurface = nullptr;

  // Determine if we want to extrapolate to the barrel or endcap.  If in the
  // crack choose the detector with largest amount of energy in the second
  // sampling layer
  if (xAOD::EgammaHelpers::isBarrel(cluster)) {
    destinationSurface.reset(m_calosurf->CreateUserSurface(
      CaloCell_ID::EMB2, 0., cluster->eta(), caloDDMgr));
  } else {
    destinationSurface.reset(m_calosurf->CreateUserSurface(
      CaloCell_ID::EME2, 0., cluster->eta(), caloDDMgr));
  }
  return destinationSurface;
}

Trk::LocalParameters
CaloCluster_OnTrackBuilder::getClusterLocalParameters(
  const xAOD::CaloCluster* cluster,
  const Trk::Surface* surf,
  int charge) const
{

  Amg::Vector3D surfRefPoint = surf->globalReferencePoint();
  double eta = cluster->eta();
  double phi = cluster->phi();
  double clusterQoverE = cluster->e() != 0 ? (double)charge / cluster->e() : 0;

  if (xAOD::EgammaHelpers::isBarrel(cluster)) {
    // Two corindate in a cyclinder are
    // Trk::locRPhi = 0 (ie phi)
    // Trk::locZ    = 1(ie z)
    double r = surfRefPoint.perp();
    std::vector<Trk::DefinedParameter> defPar;
    if (m_useClusterPhi) {
      Trk::DefinedParameter locRPhi(r * phi, Trk::locRPhi);
      defPar.push_back(locRPhi);
    }
    if (m_useClusterEta) {
      double theta = 2 * atan(exp(-eta)); //  -log(tan(theta/2));
      double tantheta = tan(theta);
      double z = tantheta == 0 ? 0. : r / tantheta;
      Trk::DefinedParameter locZ(z, Trk::locZ);
      defPar.push_back(locZ);
    }
    if (m_useClusterEnergy) {
      Trk::DefinedParameter qOverP(clusterQoverE, Trk::qOverP);
      defPar.push_back(qOverP);
    }
    return Trk::LocalParameters(defPar);
  }
  // Local paramters of a disk are
  // Trk::locR   = 0
  // Trk::locPhi = 1
  double z = surfRefPoint.z();
  std::vector<Trk::DefinedParameter> defPar;
  if (m_useClusterEta) {
    double theta = 2 * atan(exp(-eta)); //  -log(tan(theta/2));
    double tantheta = tan(theta);
    double r = z * tantheta;
    Trk::DefinedParameter locR(r, Trk::locR);
    defPar.push_back(locR);
  }
  if (m_useClusterPhi) {
    Trk::DefinedParameter locPhi(phi, Trk::locPhi);
    defPar.push_back(locPhi);
  }
  if (m_useClusterEnergy) {
    Trk::DefinedParameter qOverP(clusterQoverE, Trk::qOverP);
    defPar.push_back(qOverP);
  }

  return Trk::LocalParameters(defPar);
}

Amg::MatrixX
CaloCluster_OnTrackBuilder::getClusterErrorMatrix(
  const xAOD::CaloCluster* cluster,
  const Trk::Surface& surf,
  int) const
{
  int matrixSize = static_cast<int>(m_useClusterEta) +
                   static_cast<int>(m_useClusterPhi) +
                   static_cast<int>(m_useClusterEnergy);
  Amg::MatrixX covMatrix(matrixSize, matrixSize);
  covMatrix.setZero();

  //variance in phi ~ 1e-04
  //sigma ~ 0.01
  constexpr double phivariance = 1e-04;
  // simga ~ 20 mm large error as currently we do not want to rely
  // on the eta side of the cluster.
  constexpr double zvariance = 400;
  const double clusterE = cluster->e();
  const double sigmaP_over_P = m_eg_resol->getResolution(0, //electron
                                                         clusterE,
                                                         cluster->eta(),
                                                         2//90% quantile
                                                         );
  const double qOverP = 1./clusterE;
  const double qOverP_variance =(qOverP*qOverP)*(sigmaP_over_P*sigmaP_over_P);
  if (xAOD::EgammaHelpers::isBarrel(cluster)) {
    // The two coordinates for a cyclinder are
    // Trk::locRPhi = 0 (ie phi)
    // Trk::locZ    = 1(ie z)
    Amg::Vector3D surfRefPoint = surf.globalReferencePoint();
    double r = surfRefPoint.perp();
    double r2 = r*r;
    int indexCount(0);
    if (m_useClusterPhi) {
      covMatrix(indexCount, indexCount) = phivariance * r2;
      ++indexCount;
    }
    if (m_useClusterEta) {
      covMatrix(indexCount, indexCount) = zvariance;
      ++indexCount;
    }
    if (m_useClusterEnergy) {
      covMatrix(indexCount, indexCount) = qOverP_variance;
      ++indexCount;
    }
  } else {
    // Local paramters of a disk are
    // Trk::locR   = 0
    // Trk::locPhi = 1
    int indexCount(0);

    if (m_useClusterEta) {
      covMatrix(indexCount, indexCount) = zvariance;
      ++indexCount;
    }
    if (m_useClusterPhi) {
      covMatrix(indexCount, indexCount) = phivariance;
      ++indexCount;
    }
    if (m_useClusterEnergy) {
      covMatrix(indexCount, indexCount) = qOverP_variance;
      ++indexCount;
    }
  }

  return covMatrix;
}

