/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                SingleTrackConversionTool.cxx  -  Description
		-------------------
		begin   : 01-01-2008
		authors : Tatjana Lenz, Thomas Koffas
		email   : tatjana.lenz@cern.ch, Thomas.Koffas@cern.ch
		changes : Markus Elsing
***************************************************************************/
#include "InDetConversionFinderTools/SingleTrackConversionTool.h"
#include "TrkTrack/Track.h"
#include "TrkParameters/TrackParameters.h"
#include "AthLinks/ElementLink.h"
#include "TrkTrack/LinkToTrack.h"
#include "InDetConversionFinderTools/ConversionFinderUtils.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "VxVertex/VxTrackAtVertex.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkSurfaces/Surface.h"


#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexContainer.h"

#include "CLHEP/Geometry/Transform3D.h"
using HepGeom::Transform3D;

namespace InDet {

  // -------------------------------------------------------
  // athena interface definition
  // -------------------------------------------------------
  static const InterfaceID IID_ISingleTrackConversionTool("InDet::SingleTrackConversionTool", 1, 0);

  // -------------------------------------------------------
  // constructor
  // -------------------------------------------------------
  SingleTrackConversionTool::SingleTrackConversionTool(const std::string& type,
                                                       const std::string& name,
                                                       const IInterface* parent)
    : AthAlgTool(type, name, parent)
    , m_minInitR(70.)
    , m_minInitR_noBLay(120.)
    , m_singleThreshold(0.1)
    , m_maxBLhits(0)
  // m_maxPhiVtxTrk(0.2)
  {
    declareInterface<SingleTrackConversionTool>(this);
    declareProperty("MinInitialHitRadius"        , m_minInitR);
    declareProperty("MinInitialHitRadius_noBlay" , m_minInitR_noBLay);
    declareProperty("MinRatioOfHLhits"           , m_singleThreshold);
    //declareProperty("MaxPhiVtxTrk"               , m_maxPhiVtxTrk);
    declareProperty("MaxBLayerHits"              , m_maxBLhits);
    declareProperty("PIDonlyForXe"               , m_PIDonlyForXe = false,
      "Only check TRT PID if all hits are Xe hits");
  }

  // -------------------------------------------------------
  // destructor
  // -------------------------------------------------------
  SingleTrackConversionTool::~SingleTrackConversionTool() = default;

  // -------------------------------------------------------
  // not sure what this is about (Markus) ????
  // -------------------------------------------------------
  const InterfaceID& SingleTrackConversionTool::interfaceID() {
    return IID_ISingleTrackConversionTool;
  }

  // -------------------------------------------------------
  // init method
  // -------------------------------------------------------
  StatusCode SingleTrackConversionTool::initialize() {
    return StatusCode::SUCCESS;
  }

  // -------------------------------------------------------
  // finalize method
  // -------------------------------------------------------
  StatusCode SingleTrackConversionTool::finalize() {
    return StatusCode::SUCCESS;
  }


  // -------------------------------------------------------
  // building single track conversions from a particle
  // -------------------------------------------------------
  xAOD::Vertex*
  SingleTrackConversionTool::buildSingleTrackParticleConversion(
    const xAOD::TrackParticle* track,
    xAOD::VertexContainer* container) const
  {
    /// Create a RecVertex at the first measurement of the track.

    unsigned int index(0);
    if (!track->indexOfParameterAtPosition(index, xAOD::FirstMeasurement)) {
      ATH_MSG_WARNING("TrackParticle has no first measurement");
      return nullptr;
    }

    const Trk::CurvilinearParameters trkPar =
      track->curvilinearParameters(index);

    const Amg::Vector3D& gp = trkPar.position();
    const AmgSymMatrix(5) em = *(trkPar.covariance());

    // ME: this is nuts, those values are 0, 0
    //    double chi2 = track->fitQuality()->chiSquared();
    //    int Ndf     = track->fitQuality()->numberDoF();

    /// Need to compute a global position covariance matrix as J.C.JT
    const Amg::Transform3D& T = trkPar.associatedSurface().transform();
    AmgSymMatrix(3) nCovVtx;

    // Should use eigen to do all of this

    // ME: use the surface to find out what we do, do not hardcode the geoemtry
    if (Trk::SurfaceType::Plane == trkPar.associatedSurface().type()) {

      /// The local position parameters covariance matrix C (2x2)
      double p11 = em(Trk::locX, Trk::locX);
      double p12 = em(Trk::locX, Trk::locY);
      double p21 = em(Trk::locY, Trk::locX);
      double p22 = em(Trk::locY, Trk::locY);

      /// The Jacobian matrix J (3x2)
      double Ax[3] = { T(0, 0), T(1, 0), T(2, 0) };
      double Ay[3] = { T(0, 1), T(1, 1), T(2, 1) };
      double a11 = Ax[0];
      double a12 = Ay[0];
      double a21 = Ax[1];
      double a22 = Ay[1];
      double a31 = Ax[2];
      double a32 = Ay[2];

      /// The A = J.C (3x2)
      double A11 = a11 * p11 + a12 * p21;
      double A12 = a11 * p12 + a12 * p22;
      double A21 = a21 * p11 + a22 * p21;
      double A22 = a21 * p12 + a22 * p22;
      double A31 = a31 * p11 + a32 * p21;
      double A32 = a31 * p12 + a32 * p22;

      /// The A.JT = J.C.JT (3x3)
      double P11 = a11 * A11 + A12 * a12;
      double P12 = A11 * a21 + A12 * a22;
      double P13 = A11 * a31 + A12 * a32;
      double P21 = A21 * a11 + A22 * a12;
      double P22 = A21 * a21 + A22 * a22;
      double P23 = A21 * a31 + A22 * a32;
      double P31 = A31 * a11 + A32 * a12;
      double P32 = A31 * a21 + A32 * a22;
      double P33 = A31 * a31 + A32 * a32;

      /// Construct the new covariance matrix (3x3)
      nCovVtx(0, 0) = P11;
      nCovVtx(0, 1) = P12;
      nCovVtx(0, 2) = P13;
      nCovVtx(1, 0) = P21;
      nCovVtx(1, 1) = P22;
      nCovVtx(1, 2) = P23;
      nCovVtx(2, 0) = P31;
      nCovVtx(2, 1) = P32;
      nCovVtx(2, 2) = P33;

    } else if (Trk::SurfaceType::Line == trkPar.associatedSurface().type()) {

      /// The local position parameters covariance matrix C (2x2)
      double p11 = em(Trk::locR, Trk::locR);
      double p12 = em(Trk::locR, Trk::locZ);
      double p21 = em(Trk::locZ, Trk::locR);
      double p22 = em(Trk::locZ, Trk::locZ);

      ///The straight line surface (wire) global directions
      double A[3] = {T(0,2),T(1,2),T(2,2)};

      ///The particle global direction
      double Px = trkPar.momentum().x();
      double Py = trkPar.momentum().y();
      double Pz = trkPar.momentum().z();

      ///The Jacobian matrix J (3x2)
      double Bx = A[1]*Pz-A[2]*Py;
      double By = A[2]*Px-A[0]*Pz;
      double Bz = A[0]*Py-A[1]*Px;
      double Bn = 1./sqrt(Bx*Bx+By*By+Bz*Bz); Bx*=Bn; By*=Bn; Bz*=Bn;
      double a11 = Bx; double a12 = A[0];
      double a21 = By; double a22 = A[1];
      double a31 = Bz; double a32 = A[2];

      ///The A = J.C (3x2)
      double A11 = a11*p11 + a12*p21; double A12 = a11*p12 + a12*p22;
      double A21 = a21*p11 + a22*p21; double A22 = a21*p12 + a22*p22;
      double A31 = a31*p11 + a32*p21; double A32 = a31*p12 + a32*p22;

      ///The A.JT = J.C.JT (3x3)
      double P11 = a11*A11 + A12*a12; double P12 = A11*a21 + A12*a22; double P13 = A11*a31 + A12*a32;
      double P21 = A21*a11 + A22*a12; double P22 = A21*a21 + A22*a22; double P23 = A21*a31 + A22*a32;
      double P31 = A31*a11 + A32*a12; double P32 = A31*a21 + A32*a22; double P33 = A31*a31 + A32*a32;

      ///Construct the new covariance matrix (3x3)
      nCovVtx(0,0) = P11; nCovVtx(0,1) = P12; nCovVtx(0,2) = P13;
      nCovVtx(1,0) = P21; nCovVtx(1,1) = P22; nCovVtx(1,2) = P23;
      nCovVtx(2,0) = P31; nCovVtx(2,1) = P32; nCovVtx(2,2) = P33;
    }

    // now construct the vertex from the global position, cov. put NdF and chi2 to zero (Markus)


    xAOD::Vertex* vertex = new xAOD::Vertex();
    container->push_back( vertex );

    vertex->setPosition(gp);
    vertex->setCovariancePosition(nCovVtx);
    vertex->setVertexType(xAOD::VxType::ConvVtx);
    vertex->setFitQuality( 0, 0);

    return vertex;
  }

  // -------------------------------------------------------
  // preselection cuts on track particles
  // -------------------------------------------------------
  bool SingleTrackConversionTool::selectSingleTrackParticleConversion(const xAOD::TrackParticle* track) const{

     //Position of first hit in track particle

    int index(-1);
    for(unsigned int i(0); i< track->numberOfParameters() ; ++i ){
      if( xAOD::FirstMeasurement == track->parameterPosition(i) ){
        index = i;
        break;
      }
    }
    if(index ==-1){
      ATH_MSG_WARNING("Track Particle does not contain first Measurement track parameters");
      return false;
    }

    const Trk::CurvilinearParameters trk_meas = track->curvilinearParameters(index);

    uint8_t dummy;

    uint8_t expectedHitInBLayer(0);
    if( track->summaryValue(dummy,xAOD::expectInnermostPixelLayerHit) )
       expectedHitInBLayer = dummy;

    float Rfirst = trk_meas.position().perp();
    if (expectedHitInBLayer)
    {
      // ME: cut on minInitR if blayer is ok
      if (Rfirst < m_minInitR)
      {
        ATH_MSG_DEBUG("BLayer hit expected. Radius of first hit (" <<
                      Rfirst << ") below minimum: " << m_minInitR);
        return false;
      }
    }
    else
    {
      // ME: cut on minInitR_NBLay if blayer is off
      if(Rfirst < m_minInitR_noBLay)
      {
        ATH_MSG_DEBUG("No BLayer hit expected. Radius of first hit (" <<
                      Rfirst << ") below minimum: " << m_minInitR_noBLay);
        return false;
      }
    }


    uint8_t nTrtHits(0);
    if( track->summaryValue(dummy, xAOD::numberOfTRTHits))
      nTrtHits = dummy;

    uint8_t nTrtOutliers(0);
    if(track->summaryValue(dummy, xAOD::numberOfTRTOutliers))
      nTrtOutliers = dummy;

    uint8_t ntrt = nTrtHits + nTrtOutliers;

    uint8_t nTrtXenonHits(0);
    if( track->summaryValue(dummy, xAOD::numberOfTRTXenonHits) )
       nTrtXenonHits = dummy;



    if(ntrt > 0 && (!m_PIDonlyForXe || nTrtXenonHits==ntrt) ) {
      // only check TRT PID if m_PIDonlyForXe is false or all TRT hits are Xenon hits
      float prob = 1.0;
      if( !track->summaryValue(prob,xAOD::eProbabilityHT) )
      {
        ATH_MSG_WARNING("Could not retrieve TR probability");
        return false;
      }
      if (prob < m_singleThreshold)
      {
        ATH_MSG_DEBUG("Probability (" << prob << ") below threshold: "
                      << m_singleThreshold);
        return false;
      }
    }

    uint8_t nBLHits(0);
    if( track->summaryValue(dummy,xAOD::numberOfInnermostPixelLayerHits))
      nBLHits += dummy;
    if( track->summaryValue(dummy, xAOD::numberOfInnermostPixelLayerOutliers))
      nBLHits += dummy;
    if(nBLHits > m_maxBLhits)
    {
      ATH_MSG_DEBUG("BLayer hits (" << nBLHits << ") above maximum: " << m_maxBLhits);
      return false;
    }

    return true;
  }
}
