/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// class header include
#include "CrabKissingVertexPositioner.h"

// For the speed of light
#include "GaudiKernel/PhysicalConstants.h"

// CLHEP includes
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Transform3D.h"
#include <math.h>       /* erf */
// RandomNumber generator
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"

namespace Simulation
{

  /** Constructor */
  CrabKissingVertexPositioner::CrabKissingVertexPositioner( const std::string& t,
                                                            const std::string& n,
                                                            const IInterface* p )
    : base_class(t,n,p)
  {
    m_bunchShapeProp.declareUpdateHandler(&CrabKissingVertexPositioner::BunchShapeHandler, this);
  }

  void CrabKissingVertexPositioner::BunchShapeHandler(Gaudi::Details::PropertyBase&)
  {
    if(m_bunchShapeProp.value() == "GAUSS") m_bunchShape = BunchShape::GAUSS;
    else if(m_bunchShapeProp.value() == "FLAT") m_bunchShape = BunchShape::FLAT;
    else m_bunchShape=BunchShape::NSHAPES;
  }

  /** Athena algtool's Hooks */
  StatusCode CrabKissingVertexPositioner::initialize()
  {
    ATH_MSG_VERBOSE("Initializing ...");

    // prepare the RandonNumber generation
    ATH_CHECK(m_rndGenSvc.retrieve());
    ATH_CHECK(m_beamSpotKey.initialize());

    m_randomEngine = m_rndGenSvc->getEngine(this, m_randomEngineName);
    if (!m_randomEngine) {
      ATH_MSG_ERROR("Could not get random number engine from RandomNumberService. Abort.");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("BunchShape = " << m_bunchShapeProp.value());
    ATH_MSG_DEBUG("BunchLength = " << m_bunchLength);
    ATH_MSG_DEBUG("Epsilon (normalized emittance) = " << m_epsilon);
    ATH_MSG_DEBUG("BetaStar = " << m_betaStar);
    ATH_MSG_DEBUG("AlfaParallel (kissing angle) = " << m_alphaPar);
    ATH_MSG_DEBUG("AlfaX (crabbing angle) = " << m_alphaX);
    ATH_MSG_DEBUG("ThetaX (half crossing angle) = " << m_thetaX);

    // everything set up properly
    return StatusCode::SUCCESS;
  }


  /** Athena algtool's Hooks */
  StatusCode CrabKissingVertexPositioner::finalize()
  {
    ATH_MSG_VERBOSE("Finalizing ...");
    return StatusCode::SUCCESS;
  }

  // beamspotFunction for rectangular bunches from Table II in S.Fartoukh Phys.Rev.ST Accel.Beams 17 (2014) no.11, 111001
  // for  z smearing: angle1=psi, angle2=phi
  // for ct smearing: angle1=phi, angle2=psi
  double CrabKissingVertexPositioner::beamspotFunction(double displacement, double angle1, double angle2) const
  {
    if ( angle1<1e-10 ) angle1 = 1e-10; // to avoid divide_by_zero errors
    double temp(1.0-std::fabs(displacement)/m_bunchLength);
    return 1.0/M_2_SQRTPI * std::erf(angle1*temp)/angle1 *
           std::exp( -pow(angle2*displacement/m_bunchLength, 2) ) *
           heaviside(temp);
  }

  double CrabKissingVertexPositioner::getDisplacement(double bunchSize, double angle1, double angle2,
                                                      CLHEP::HepRandomEngine* randomEngine) const
  {
    size_t ntries(0);
    double yval(CLHEP::RandFlat::shoot(randomEngine, 0.0, 1.0));
    double displ(CLHEP::RandFlat::shoot(randomEngine, -bunchSize, bunchSize));
    while (this->beamspotFunction(displ, angle1, angle2)<yval) {
      if(ntries>1000000) return 0.0; //just so we don't sit in this loop forever
      yval = CLHEP::RandFlat::shoot(randomEngine, 0.0, 1.0);
      displ = CLHEP::RandFlat::shoot(randomEngine, -bunchSize, bunchSize);
      ++ntries;
    }
    return displ;
  }

  // computes the vertex displacement
  CLHEP::HepLorentzVector *CrabKissingVertexPositioner::generate(const EventContext& ctx) const
  {
    // Prepare the random engine
    m_randomEngine->setSeed( name(), ctx );
    CLHEP::HepRandomEngine* randomEngine(m_randomEngine->getEngine(ctx));
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
    // See jira issue ATLASSIM-497 for an explanation of why calling
    // shoot outside the CLHEP::HepLorentzVector constructor is
    // necessary/preferable.
    double vertexX = CLHEP::RandGaussZiggurat::shoot(randomEngine)*beamSpotHandle->beamSigma(0);
    double vertexY = CLHEP::RandGaussZiggurat::shoot(randomEngine)*beamSpotHandle->beamSigma(1);
    double piwinski_phi = std::fabs(m_thetaX - m_alphaX) * m_bunchLength/std::sqrt(m_epsilon * m_betaStar);
    double piwinski_psi = m_alphaPar * m_bunchLength / std::sqrt( m_epsilon * m_betaStar);
    double vertexZ = 0;
    double vertexT = 0;
    // Time should be set in units of distance, the following methods generate c*t
    if ( m_bunchShape == BunchShape::GAUSS) {
      double zWidth    = m_bunchLength / std::sqrt(2*(1+pow(piwinski_phi,2)));
      double timeWidth = m_bunchLength / std::sqrt(2*(1+pow(piwinski_psi,2)));
      vertexZ = CLHEP::RandGaussZiggurat::shoot(randomEngine, 0., zWidth);
      vertexT = CLHEP::RandGaussZiggurat::shoot(randomEngine, 0., timeWidth);
    }
    else if ( m_bunchShape == BunchShape::FLAT ) {
      vertexZ = getDisplacement(m_bunchLength, piwinski_psi, piwinski_phi, randomEngine);
      vertexT = getDisplacement(m_bunchLength, piwinski_phi, piwinski_psi, randomEngine);
    }
    else {
      ATH_MSG_WARNING("Invalid BunchShape ("<<m_bunchShapeProp.value()<<"). Vertex smearing disabled");
      return new CLHEP::HepLorentzVector(0.,0.,0.,0.);
    }
    ATH_MSG_VERBOSE("m_bunchLength = " << m_bunchLength <<
                    ", Zvertex = " << vertexZ << ", Tvertex = " << vertexT);

    // calculate the vertexSmearing
    CLHEP::HepLorentzVector *vertexSmearing =
      new CLHEP::HepLorentzVector( vertexX, vertexY, vertexZ, 0. );

    // (1) code from: Simulation/G4Atlas/G4AtlasUtilities/VertexPositioner.cxx
    const double tx = tan( beamSpotHandle->beamTilt(1) );
    const double ty = tan( beamSpotHandle->beamTilt(0) );

    const double sqrt_abc = std::sqrt(1. + tx*tx + ty*ty);
    const double sqrt_fgh = std::sqrt(1. + ty*ty);

    const double a = ty/sqrt_abc;
    const double b = tx/sqrt_abc;
    const double c = 1./sqrt_abc;

    HepGeom::Point3D<double> from1(0,0,1);
    HepGeom::Point3D<double> to1(a,b,c);

    const double f = 1./sqrt_fgh;
    const double g = 0.;
    const double h = -(ty)/sqrt_fgh;

    HepGeom::Point3D<double> from2(1,0,0);
    HepGeom::Point3D<double> to2(f,g,h);

    // first rotation, then translation
    HepGeom::Transform3D transform(
        HepGeom::Rotate3D(from1, from2, to1, to2).getRotation(),
        CLHEP::Hep3Vector( beamSpotHandle->beamPos().x(),
                           beamSpotHandle->beamPos().y(),
                           beamSpotHandle->beamPos().z() )
        );

    ATH_MSG_VERBOSE("BeamSpotSvc reported beam position as " << beamSpotHandle->beamPos());
    ATH_MSG_VERBOSE("Width is (" << beamSpotHandle->beamSigma(0) << ", " <<
                    beamSpotHandle->beamSigma(1) << ", " << m_bunchLength << ")");
    ATH_MSG_VERBOSE("Tilts are " << beamSpotHandle->beamTilt(0) << " and " <<
                    beamSpotHandle->beamTilt(1));
    ATH_MSG_VERBOSE("Vertex Position before transform: " << *vertexSmearing);

    // update with the tilt
    *vertexSmearing = transform * HepGeom::Point3D<double>(*vertexSmearing);
    vertexSmearing->setT(vertexT);

    // and return it
    return vertexSmearing;
  }

} // namespace Simulation
