/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class InDet::SiDetElementBoundaryLink_xk
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// Version 1.0 21/04/2004 I.Gavrilenko
///////////////////////////////////////////////////////////////////

#include "SiSPSeededTrackFinderData/SiDetElementBoundaryLink_xk.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "TrkSurfaces/AnnulusBounds.h" 
#include "TrkSurfaces/PlaneSurface.h"

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::SiDetElementBoundaryLink_xk::SiDetElementBoundaryLink_xk
( const InDetDD::SiDetectorElement*& Si, bool isITk)
{
  m_ITkGeometry = isITk;
  m_detelement = nullptr;
  m_dR = 0.;
  const Trk::PlaneSurface* pla = dynamic_cast<const Trk::PlaneSurface*>(& Si->surface());
  // Code below make sense if a surface exists
  if(!pla) return;
  m_detelement = Si;

  double x[4],y[4],z[4];
  double Ax[3] = {1., 0., 0.};
  double Ay[3] = {0., 1., 0.};

  // Check if we have annulus bounds as this changes a few calculations below
  const Trk::AnnulusBounds* annulusBounds = dynamic_cast<const Trk::AnnulusBounds*>(&Si->design().bounds());

  if (annulusBounds) {

    // Radial component of the sensor centre, from (m_R, 0., 0.)
    // This is used to correct the ITk strip endcap local position which have a local coordinate
    // system centred on the beam axis, due to their annulus shape.
    m_dR = (Si->design().sensorCenter())[0];

    // Getting the corners directly from the bounds, as for annulus bounds
    // the calculation below is not correct
    auto corners = annulusBounds->corners();

    x[0] = corners[0].first;
    y[0] = corners[0].second - m_dR;
    z[0] = 0.;

    x[1] = corners[1].first;
    y[1] = corners[1].second - m_dR;
    z[1] = 0.;

    x[2] = corners[2].first;
    y[2] = corners[2].second - m_dR;
    z[2] = 0.;

    x[3] = corners[3].first;
    y[3] = corners[3].second - m_dR;
    z[3] = 0.;

    // For annuli, don't need to re-evaluate the rotation matrix
    // as the corners are already provided in the correct frame

  } else {

    // Getting detector element length and widths from design class
    double Sl    = .5*Si->design().length  ();
    double Swmax = .5*Si->design().maxWidth();
    double Swmin = .5*Si->design().minWidth();

    // Getting phi and eta axis of the detector element
    // They provide the rotation of the local frame wrt the global frame
    Amg::Vector3D  AF = Si->phiAxis();
    Amg::Vector3D  AE = Si->etaAxis();

    // Module dimensions and rotation terms are used to evaluate the 4 corner points in
    // a reference frame centered in the center of the module and rotated as phi/eta axis
    x[0]     = AF.x()*Swmax+AE.x()*Sl;
    y[0]     = AF.y()*Swmax+AE.y()*Sl;
    z[0]     = AF.z()*Swmax+AE.z()*Sl;

    x[1]     = AF.x()*Swmin-AE.x()*Sl;
    y[1]     = AF.y()*Swmin-AE.y()*Sl;
    z[1]     = AF.z()*Swmin-AE.z()*Sl;

    x[2]     =-AF.x()*Swmin-AE.x()*Sl;
    y[2]     =-AF.y()*Swmin-AE.y()*Sl;
    z[2]     =-AF.z()*Swmin-AE.z()*Sl;

    x[3]     =-AF.x()*Swmax+AE.x()*Sl;
    y[3]     =-AF.y()*Swmax+AE.y()*Sl;
    z[3]     =-AF.z()*Swmax+AE.z()*Sl;

    const Amg::Transform3D&  T = pla->transform();
    Ax[0] = T(0,0);
    Ax[1] = T(1,0);
    Ax[2] = T(2,0);

    Ay[0] = T(0,1);
    Ay[1] = T(1,1);
    Ay[2] = T(2,1);
  }

  // Combine the 4 corners to measure 4 vector bounds.
  // Before starting, you transform the corner coordinates
  // to the local frame using the surface transform.
  // Then you evaluate the vector to the i-th bound where:
  // m_bound[i][0] and m_bound[i][1] are the unit vector components
  // along directions loc-x and loc-y representing the i-th bound
  // m_bound[i][2] is distance to the i-th bound along direction (m_bound[i][0], m_bound[i][1])

  // Corners are combined clockwise:
  // Representation for planar sensor with
  // rectangular (left) and trapezoidal (right) bounds:
  //
  //
  //         ^ loc-y                              ^ loc-y
  //         |                                    |
  //     p3  |  p0                      p3        |        p0
  //      ...|...                        .........|.........
  //      :  |  :                         .       |       .
  //      :  |  :                          .      |      .
  // --------------------> loc-x       -----------------------> loc-x
  //      :  |  :                            .    |    .
  //      :  |  :                             .   |   .
  //      ...|...                              ...|...
  //     p2  |  p1                            p2  |  p1
  //         |                                    |
  //
  // NB: Inner and outer radial bounds are assumed to be
  // straight lines for annulus surfaces.

  std::vector < std::pair<int, int> > combinations = { {0,1}, {1,2}, {2,3}, {3,0} };
  for(unsigned int bound = 0; bound < combinations.size(); bound++) {

    // Combining 4 corners (p0, p1), (p1, p2), (p2, p3), (p3, p0)
    int firstCornerIndex  = combinations.at(bound).first;
    int secondCornerIndex = combinations.at(bound).second;

    // Evaluation of 4 corners in local frame as shown above in the drawing
    double x1     =  x[firstCornerIndex]*Ax[0]+y[firstCornerIndex]*Ax[1]+z[firstCornerIndex]*Ax[2];
    double y1     =  x[firstCornerIndex]*Ay[0]+y[firstCornerIndex]*Ay[1]+z[firstCornerIndex]*Ay[2];
    double x2     =  x[secondCornerIndex]*Ax[0]+y[secondCornerIndex]*Ax[1]+z[secondCornerIndex]*Ax[2];
    double y2     =  x[secondCornerIndex]*Ay[0]+y[secondCornerIndex]*Ay[1]+z[secondCornerIndex]*Ay[2];

    // distance between p[firstCornerIndex] and p[secondCornerIndex]
    double d      = (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
    // Evaluating direction to bound:
    // x component of the distance to bound connecting p[firstCornerIndex] and p[secondCornerIndex]
    double ax     =-(y2-y1)*(y1*x2-x1*y2)/d;
    // y component of the distance to bound connecting p[firstCornerIndex] and p[secondCornerIndex]
    double ay     = (x2-x1)*(y1*x2-x1*y2)/d;

    // distance to the bound
    m_bound[bound][2] = sqrt(ax*ax+ay*ay);
    // unit vector components along loc-x and loc-y representing the direction to the bound
    m_bound[bound][0] = ax/m_bound[bound][2];
    m_bound[bound][1] = ay/m_bound[bound][2];
  }

}

///////////////////////////////////////////////////////////////////
// Detector element intersection test
///////////////////////////////////////////////////////////////////

int InDet::SiDetElementBoundaryLink_xk::intersect(const Trk::PatternTrackParameters& Tp,double& distance) const
{
  const AmgVector(5) & p = Tp.parameters();
  double x = p[0];
  double y = p[1]-m_dR;

  const AmgSymMatrix(5) & cov = *Tp.covariance();

  // Evaluating distance (in mm) between local parameters and origin of the local reference frame,
  // along 4 possible axis directions: positive local x, negative local y, negative local x, positive local y.
  // The evaluated distance behaves as follows:
  // - it is negative if the local parameter coordinate is inside the module along the given direction.
  // - it increases moving towards the boundary, and is 0 at the bound.
  // - it is positive if outside the module bounds.
  // The largest positive distance drives the intersection check.
  //
  // First evaluating distance along positive x axis (local frame)
  int direction = InDet::SiDetElementBoundaryLink_xk::AxisDirection::PositiveX;
  distance = m_bound[direction][0]*x+m_bound[direction][1]*y-m_bound[direction][2];

  // Then testing other directions (local frame)
  std::vector <int> otherDirections = { InDet::SiDetElementBoundaryLink_xk::AxisDirection::NegativeY,
                                        InDet::SiDetElementBoundaryLink_xk::AxisDirection::NegativeX,
                                        InDet::SiDetElementBoundaryLink_xk::AxisDirection::PositiveY };

  for (auto& testDirection : otherDirections) {
    double testDistance = m_bound[testDirection][0]*x+m_bound[testDirection][1]*y-m_bound[testDirection][2];
    if (testDistance>distance) {
      distance = testDistance;
      direction = testDirection;
    }
  }

  // If distance very big (>20 mm), the intersection is definitely outside the bounds
  // returning intersection outside detector element
  if(distance > 20. )
    return InDet::SiDetElementBoundaryLink_xk::IntersectionStatus::Outside;

  // Tolerance window from the bounds is evaluated accordingly to the
  // covariance and a scale factor=100
  double tolerance  = (m_bound[direction][0]*m_bound[direction][0]* cov(0, 0)+
                       m_bound[direction][1]*m_bound[direction][1]* cov(1, 1)+
                       m_bound[direction][0]*m_bound[direction][1]*(cov(0, 1)*2.))*100.;

  if(!m_ITkGeometry){

    // within the tolerance window, returning intersection not inside nor outside detector element
    if((distance*distance) <= tolerance)
      return InDet::SiDetElementBoundaryLink_xk::IntersectionStatus::NotInsideNorOutside;

    // if outside the tolerance window and distance is larger than 2 mm
    // returning intersection outside detector element
    if(distance >  2.)
      return InDet::SiDetElementBoundaryLink_xk::IntersectionStatus::Outside;

    // if inside bounds and far them by more than 2 mm
    if(distance < -2.) {
      // if not close to bond gap, returning intersection inside detector element
      if(!m_detelement->nearBondGap(Tp.localPosition(), 3.*sqrt(cov(1, 1))))
        return InDet::SiDetElementBoundaryLink_xk::IntersectionStatus::Inside;
    }
  }

  else{
    // within the tolerance window, returning 0: intersection not inside nor outside detector element
    // For ITk we use 3 mm range around the bounds + tolerance evaluated using the covariance matrix
    if(std::abs(distance) <=3. or (distance*distance) <= tolerance)
      return InDet::SiDetElementBoundaryLink_xk::IntersectionStatus::NotInsideNorOutside;
    // if outside the tolerance window and distance is positive (which means it is larger than 3 mm)
    // returning intersection outside detector element
    else if(distance >  0.)
      return InDet::SiDetElementBoundaryLink_xk::IntersectionStatus::Outside;
    // otherwise it is inside bounds
    else
      return InDet::SiDetElementBoundaryLink_xk::IntersectionStatus::Inside;
  }

  // in any other case
  // returning 0: intersection not inside nor outside detector element
  return InDet::SiDetElementBoundaryLink_xk::IntersectionStatus::NotInsideNorOutside;
}
