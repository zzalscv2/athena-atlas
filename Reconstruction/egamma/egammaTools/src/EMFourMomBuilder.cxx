/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

/********************************************************************

NAME:     EMFourMomBuilder.cxx
PACKAGE:  offline/Reconstruction/egamma/egammaTools

 ********************************************************************/

// INCLUDE HEADER FILES:
#include "EMFourMomBuilder.h"
#include "FourMom/ErrorMatrixEEtaPhiM.h"

// NEW xAOD INCLUDES.
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/PhotonContainer.h"

#include "EventPrimitives/EventPrimitives.h"
#include "FourMom/EigenP5Jacobiand0z0PhiThetaqOverP2d0z0PhiEtaP.h"
#include "GeoPrimitives/GeoPrimitives.h"

//  END OF HEADER FILES INCLUDE

namespace {
const float el_mass = 0.510998;
const float ph_mass = 0.0;

StatusCode
setFromCluster(xAOD::Egamma& eg)
{

  const xAOD::CaloCluster* cluster = eg.caloCluster();
  const float eta = cluster->eta();
  const float phi = cluster->phi();
  const float E = cluster->e();
  if (eg.type() == xAOD::Type::Electron) {
    const double pt =
      E > el_mass ? sqrt(E * E - el_mass * el_mass) / cosh(eta) : 0;
    eg.setP4(pt, eta, phi, el_mass);
  } else {
    eg.setP4(E / cosh(eta), eta, phi, ph_mass);
  }
  return StatusCode::SUCCESS;
}

StatusCode
setFromTrkCluster(xAOD::Electron& el, const eg_resolution& eg_resol)
{

  const xAOD::CaloCluster* cluster = el.caloCluster();
  const xAOD::TrackParticle* trackParticle = el.trackParticle();

  bool goodTrack = (xAOD::EgammaHelpers::numberOfSiHits(trackParticle) >= 4);
  const float E = cluster->e();
  const float eta = goodTrack ? trackParticle->eta() : cluster->eta();
  const float phi = goodTrack ? trackParticle->phi() : cluster->phi();

  const double pt =
    E > el_mass ? sqrt(E * E - el_mass * el_mass) / cosh(eta) : 0;
  el.setP4(pt, eta, phi, el_mass);

  // Electron with tracks all should  have a covariance matrix set
  AmgMatrix(4, 4) matrix;
  matrix.setZero();
  const float sigmaE_over_E = eg_resol.getResolution(el);
  matrix(0, 0) =
    (sigmaE_over_E * E * sigmaE_over_E * E) / (cosh(eta) * cosh(eta));

  // Direction can be filled only when a good track is there
  if (goodTrack) {
    xAOD::ParametersCovMatrix_t covmat =
      trackParticle->definingParametersCovMatrix();
    EigenP5Jacobiand0z0PhiThetaqOverP2d0z0PhiEtaP J(
      trackParticle->phi(), trackParticle->charge(), trackParticle->p4().P());
    AmgMatrix(5, 5) m;
    m.setZero();
    m = J * (covmat * J.transpose());
    // Rearrange the elements of the d0, z0, phi, eta, Pt
    // representation to make the (Et, eta, phi, M)
    matrix(1, 1) = m(3, 3);
    matrix(2, 2) = m(2, 2);
    matrix.fillSymmetric(0, 1, m(4, 3));
    matrix.fillSymmetric(0, 2, m(4, 2));
    matrix.fillSymmetric(1, 2, m(3, 2));
  }

  el.setCovMatrix(matrix.cast<float>());
  return StatusCode::SUCCESS;
}

StatusCode
setFromTrkCluster(xAOD::Photon& ph)
{
  const xAOD::CaloCluster* cluster = ph.caloCluster();
  float E = cluster->e();
  float eta = cluster->eta();
  float phi = cluster->phi();
  Amg::Vector3D momentumAtVertex = xAOD::EgammaHelpers::momentumAtVertex(&ph);
  if (momentumAtVertex.mag() > 1e-5) { // protection against p = 0
    eta = momentumAtVertex.eta();
    phi = momentumAtVertex.phi();
  }
  ph.setP4(E / cosh(eta), eta, phi, ph_mass);
  return StatusCode::SUCCESS;
}

}

/////////////////////////////////////////////////////////////////

EMFourMomBuilder::EMFourMomBuilder(const std::string& type,
                                   const std::string& name,
                                   const IInterface* parent)
  : egammaBaseTool(type, name, parent)
{
  // declare interface
  declareInterface<IEMFourMomBuilder>(this);
}

StatusCode
EMFourMomBuilder::initialize()
{
  ATH_MSG_DEBUG(" Initializing EMFourMomBuilder");
  m_eg_resol = std::make_unique<eg_resolution>(m_ResolutionConfiguration);
  return StatusCode::SUCCESS;
}

StatusCode
EMFourMomBuilder::execute([[maybe_unused]] const EventContext& ctx, xAOD::Egamma* eg) const
{
  if (!eg) {
    ATH_MSG_WARNING("Null pointer to egamma object ");
    return StatusCode::SUCCESS;
  }

  if (!eg->caloCluster()) {
    ATH_MSG_WARNING("Null pointer to cluster");
    return StatusCode::SUCCESS;
  }

  xAOD::Electron* electron = nullptr;
  xAOD::Photon* photon = nullptr;

  if (eg->type() == xAOD::Type::Electron) {
    electron = static_cast<xAOD::Electron*>(eg);
  } else {
    photon = static_cast<xAOD::Photon*>(eg);
  }

  /* One method deals with electron with tracks,
   * Another with double Si Conversion Photons
   * The last is for no track e.g forward electrons
   * or the rest of the photons
   */
  if (electron) {
    if (electron->trackParticle()) {
      return setFromTrkCluster(*electron,*m_eg_resol);
    }
  } else if (photon && xAOD::EgammaHelpers::conversionType(photon) ==
                         xAOD::EgammaParameters::doubleSi) {
    return setFromTrkCluster(*photon);
  }

  return setFromCluster(*eg);
}


