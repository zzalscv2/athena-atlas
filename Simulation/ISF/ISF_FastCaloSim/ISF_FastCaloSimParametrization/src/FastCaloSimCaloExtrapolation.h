/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FastCaloSimCaloExtrapolation_H
#define FastCaloSimCaloExtrapolation_H

#include "CxxUtils/CachedPointer.h"

#include "ISF_FastCaloSimParametrization/IFastCaloSimGeometryHelper.h"
#include "ISF_FastCaloSimParametrization/IFastCaloSimCaloExtrapolation.h"
#include "ISF_FastCaloSimEvent/TFCSExtrapolationState.h"
#include "GeoPrimitives/GeoPrimitives.h"


#include "ISF_FastCaloSimParametrization/IFastCaloSimCaloTransportation.h"

class TFCSTruthState;
class G4FieldTrack;


struct CylinderIntersections
{
  Amg::Vector3D first;
  Amg::Vector3D second;
  unsigned int number;

};


class FastCaloSimCaloExtrapolation: public extends<AthAlgTool, IFastCaloSimCaloExtrapolation>
{

public:
  
  FastCaloSimCaloExtrapolation(const std::string& t, const std::string& n, const IInterface* p);
  ~FastCaloSimCaloExtrapolation() = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;

  enum SUBPOS { 
    SUBPOS_MID = TFCSExtrapolationState::SUBPOS_MID, //MID=middle of calo layer
    SUBPOS_ENT = TFCSExtrapolationState::SUBPOS_ENT, //ENT=entrance of calo layer
    SUBPOS_EXT = TFCSExtrapolationState::SUBPOS_EXT  //EXT=exit of calo layer
  }; 

  enum HITPOSITION{
    INSIDE,  //hit position is inside cylinder bounds
    OUTSIDE, //hit position is outside cylinder bounds
    ON       //hit position is on cylinder bounds
  };

  virtual void extrapolate(TFCSExtrapolationState& result,const TFCSTruthState* truth) const override final;

protected:

 const IFastCaloSimGeometryHelper* GetCaloGeometry() const {return &(*m_CaloGeometryHelper);};
  
  /*Main extrapolation methods*/

  ///Finds best extrapolation extPos from the caloSteps for a cylinder defined by radius cylR and half-length cylZ as well as corresponding momentum direction 
  bool extrapolateToCylinder(const std::vector<G4FieldTrack>& caloSteps, float cylR, float cylZ, Amg::Vector3D& extPos, Amg::Vector3D& momDir) const;
  ///Extrapolates to ID using three uniquely defined cylinder surfaces
  void extrapolateToID(TFCSExtrapolationState& result, const std::vector<G4FieldTrack>& caloSteps, const TFCSTruthState* truth) const;
  ///Extrapolates to all other layers of the calorimeter
  void extrapolateToLayers(TFCSExtrapolationState& result, const std::vector<G4FieldTrack>& caloSteps, const TFCSTruthState* truth) const;
  
  /*Extrapolator helper methods*/

  ///Finds Point of Closest Approach (PCA) on the cylinder defined by radius cylR and half-length cylZ of a line segment spanned by two hit positions to a cylinder 
  void findPCA(float cylR, float cylZ, Amg::Vector3D& hitPos1, Amg::Vector3D& hitPos2, Amg::Vector3D& PCA) const;
  ///Computes the distance between a point and the line segment spanned by hitPos1 and hitPos2
  static double getPointLineSegmentDistance(Amg::Vector3D& point, Amg::Vector3D& hitPos1, Amg::Vector3D& hitPos2) ;
  ///Finds PCA iteratively given two bounds A and B on a line segment, used for (rare) cases with no easy analytical solutions
  void getIterativePCA(float cylR, float cylZ, Amg::Vector3D& BoundA, Amg::Vector3D& BoundB, Amg::Vector3D& PCA) const;
  ///Returns true if point lies on the line segment spanned by hitPos1 and hitPos2, otherwise returns false
  static bool isOnSegment(Amg::Vector3D& point, Amg::Vector3D& hitPos1, Amg::Vector3D& hitPos2) ;
  ///Computes intersection between the (infinite) line spanned by pointA and pointB with the positive (negative) endcap of a cylinder, returns true if intersection is found
  static bool cylinderEndcapIntersection(float cylR, float cylZ, bool positiveEndcap, Amg::Vector3D& pointA, Amg::Vector3D& pointB, Amg::Vector3D& intersection) ;
  /*!Extrapolates position on cylinder by finding intersections of subsequent hit positions, intersection is considered if we detect a travel through the surface with
   the line segment or we find a forward intersection (in the travel direction of the particle) which lies on the line segment, returns false if no such postion is found*/
  bool extrapolateWithIntersection(const std::vector<G4FieldTrack>& caloSteps, float cylR, float cylZ, Amg::Vector3D& extPos, Amg::Vector3D& momDir) const;
  ///Extrapolates to the cylinder using the PCA to the polygon spanned by the individual line segments from the caloSteps
  bool extrapolateWithPCA(const std::vector<G4FieldTrack>& caloSteps, float cylR, float cylZ, Amg::Vector3D& extPos, Amg::Vector3D& momDir) const;
  ///Returns true if the line segment spanned by hitPos1 and hitPos2 crosses the cylinder surface, false otherwise
  static bool doesTravelThroughSurface(float cylR, float cylZ, Amg::Vector3D& hitPos1, Amg::Vector3D& hitPos2) ;
  ///Returns ID of more sensible intersection between line segment spanned by hitPos1 and hitPos2 and cylinder
  int whichIntersection(float cylR, float cylZ, Amg::Vector3D& hitPos1, Amg::Vector3D& hitPos2, Amg::Vector3D& intersectionA, Amg::Vector3D intersectionB) const;
  ///Analytically computes 2D intersections between circle of radius circR and (infinite) line spanned by pointA nad pointB
  int circleLineIntersection2D(float circR, Amg::Vector3D& pointA, Amg::Vector3D& pointB, Amg::Vector3D& intersectA, Amg::Vector3D& intersectB) const;
  ///Analytically computes the intersection between the (infinite) line defined by pointA and pointB and the cylinder cover (without endcaps)
  int cylinderLineIntersection(float cylR, float cylZ, Amg::Vector3D& pointA, Amg::Vector3D& pointB, Amg::Vector3D& intersectA, Amg::Vector3D& intersectB) const;
  ///Checks if position of hitPos is inside, outside or on the cylinder bounds
  static enum HITPOSITION whereOnCylinder(float cylR, float cylZ, Amg::Vector3D& hitPos) ;
  ///Projects position hitPos onto the cylinder surface and returns projected position
  static Amg::Vector3D projectOnCylinder(float cylR, float cylZ, Amg::Vector3D& hitPos) ;
  ///Analytically computes the intersection between the (infinite) line spanned by hitPos1 and hitPos2 with a cylinder
  CylinderIntersections getCylinderIntersections(float cylR, float cylZ, Amg::Vector3D& hitPos1, Amg::Vector3D& hitPos2) const;

  //helper methods for calo geometry
  void   minmaxeta(int sample, double eta, double& mineta, double& maxeta) const;
  bool   isCaloBarrel(int sample) const;
  double deta (int sample, double eta) const;
  double rzmid(int sample, double eta) const;
  double rzent(int sample, double eta) const;
  double rzext(int sample, double eta) const;
  double rmid (int sample, double eta) const;
  double rent (int sample, double eta) const;
  double rext (int sample, double eta) const;
  double zmid (int sample, double eta) const;
  double zent (int sample, double eta) const;
  double zext (int sample, double eta) const;
  double rpos (int sample, double eta, int subpos = CaloSubPos::SUBPOS_MID) const;
  double zpos (int sample, double eta, int subpos = CaloSubPos::SUBPOS_MID) const;
  double rzpos(int sample, double eta, int subpos = CaloSubPos::SUBPOS_MID) const;

  HepPDT::ParticleDataTable*     m_particleDataTable{nullptr};

  //uniquely defined ID-Calo surfaces
  FloatArrayProperty m_CaloBoundaryR{this, "CaloBoundaryR", {1148.0,120.0,41.0}};
  FloatArrayProperty m_CaloBoundaryZ{this, "CaloBoundaryZ", {3550.0,4587.0,4587.0}};

  // FastCaloSim particle transport with ATLAS tracking tools
  PublicToolHandle<IFastCaloSimCaloTransportation> m_CaloTransportation{this, "CaloTransportation", "FastCaloSimCaloTransportation"};
  // FastCaloSim geometry helper
  PublicToolHandle<IFastCaloSimGeometryHelper> m_CaloGeometryHelper{this, "CaloGeometryHelper", "FastCaloSimGeometryHelper"};

};

#endif // FastCaloSimCaloExtrapolation_H
