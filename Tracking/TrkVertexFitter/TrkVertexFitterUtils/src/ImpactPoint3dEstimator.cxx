/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*********************************************************************
          ImpactPoint3dEstimator.cxx - Description in header file
*********************************************************************/

#include "ImpactPoint3dEstimator.h"
#include "TrkParameters/TrackParameters.h"
#include "VxVertex/VxTrackAtVertex.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "MagFieldElements/AtlasFieldCache.h"

// #define IMPACTPOINT3DESTIMATOR_DEBUG

//added for cuts in case of displaced vertex
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkSurfaces/PerigeeSurface.h"

#include <cmath>

namespace Trk
{

  ImpactPoint3dEstimator::ImpactPoint3dEstimator(const std::string& t, const std::string& n, const IInterface*  p) :
    base_class(t,n,p),
    m_extrapolator(""),
    m_maxiterations(20),
    m_precision(1e-10)//DeltaPhi
  {
    declareProperty("Extrapolator",m_extrapolator);
    declareProperty("MaxIterations",m_maxiterations);
    declareProperty("Precision",m_precision);
  }

  ImpactPoint3dEstimator::~ImpactPoint3dEstimator() = default;

  StatusCode ImpactPoint3dEstimator::initialize()
  {
    if (!m_extrapolator.empty())  {
      ATH_CHECK( m_extrapolator.retrieve() );
    } else {
      m_extrapolator.disable();
    }
    ATH_CHECK( m_fieldCacheCondObjInputKey.initialize() );

    ATH_MSG_DEBUG( "Initialize successful"  );
    return StatusCode::SUCCESS;
  }

  StatusCode ImpactPoint3dEstimator::finalize()
  {
    ATH_MSG_DEBUG( "Finalize successful"  );
    return StatusCode::SUCCESS;
  }


  template<typename T>
  std::unique_ptr<PlaneSurface>
  ImpactPoint3dEstimator::Estimate3dIPNoCurvature(const T* thePerigee,
                                                  const Amg::Vector3D* theVertex,
                                                  double& distance) const
  {
      const Amg::Vector3D  momentumUnit = thePerigee->momentum().unit();
      double pathLength  =  ( *theVertex  - thePerigee->position() ).dot( momentumUnit )
                                                                       / (  momentumUnit.dot( momentumUnit )) ;
       //first vector at 3d impact point


      Amg::Vector3D POCA   =  thePerigee->position()  + pathLength * momentumUnit;// Position of closest approach
      Amg::Vector3D DeltaR =  *theVertex  - POCA;
      distance=DeltaR.mag();
      DeltaR=DeltaR.unit();


      //correct DeltaR from small deviations from orthogonality to DeltaR -- DeltaR.dot(momentumUnit) should equal 0 if the above is correct

      Amg::Vector3D DeltaRcorrected=DeltaR-(DeltaR.dot(momentumUnit))*momentumUnit;

      if ((DeltaR-DeltaRcorrected).mag()>1e-4)
      {
        ATH_MSG_WARNING( " DeltaR and MomentumDir are not orthogonal " );
        ATH_MSG_DEBUG( std::setprecision(10) << " DeltaR-DeltaRcorrected: "  << (DeltaR-DeltaRcorrected).mag() );
      }

      Amg::Vector3D YDir=momentumUnit.cross(DeltaRcorrected);

      ATH_MSG_VERBOSE( "final minimal distance is: " << distance);
      ATH_MSG_DEBUG( "POCA in 3D is: " << POCA );

      //store the plane...
      ATH_MSG_VERBOSE( "plane to which to extrapolate X " << DeltaRcorrected << " Y " << YDir << " Z " << momentumUnit);

      Amg::Transform3D thePlane(DeltaRcorrected, YDir, momentumUnit, *theVertex);

#ifdef IMPACTPOINT3DESTIMATOR_DEBUG
      std::cout << "the translation is, directly from Transform3d: " << thePlane->getTranslation() << endmsg;
#endif

      return std::make_unique<PlaneSurface>(thePlane);

  }

  std::unique_ptr<PlaneSurface>
  ImpactPoint3dEstimator::Estimate3dIP(const NeutralParameters* neutralPerigee,
                                       const Amg::Vector3D* theVertex,
                                       double& distance) const
  {
    ATH_MSG_DEBUG("Neutral particle --  propagate like a straight line");
    return Estimate3dIPNoCurvature(neutralPerigee, theVertex, distance);
  }

  std::unique_ptr<PlaneSurface>
  ImpactPoint3dEstimator::Estimate3dIP(const TrackParameters* trackPerigee,
                                       const Amg::Vector3D* theVertex,
                                       double& distance) const
  {
    SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCacheCondObjInputKey, Gaudi::Hive::currentContext()};
    const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};

    MagField::AtlasFieldCache fieldCache;
    fieldCondObj->getInitializedCache (fieldCache);

    double magnFieldVect[3];
    fieldCache.getField(trackPerigee->associatedSurface().center().data(),magnFieldVect);
    if(magnFieldVect[2] == 0 ){
      ATH_MSG_DEBUG("Magnetic field in the Z direction is 0 --  propagate like a straight line");
      return Estimate3dIPNoCurvature(trackPerigee, theVertex, distance);
    }
    
    const Trk::Perigee* thePerigee=dynamic_cast<const Trk::Perigee*>(trackPerigee);
    if (thePerigee==nullptr)
    {
      ATH_MSG_DEBUG(
        " ImpactPoint3dEstimator didn't get a Perigee* as ParametersBase*: "
        "cast not possible. Need to EXTRAPOLATE...");
      Trk::PerigeeSurface perigeeSurface(*theVertex);
      std::unique_ptr<const Trk::TrackParameters> tmp =
        m_extrapolator->extrapolateDirectly(
          Gaudi::Hive::currentContext(), *trackPerigee, perigeeSurface);
      if (tmp && tmp->associatedSurface().type() == Trk::SurfaceType::Perigee) {
        thePerigee = static_cast<const Trk::Perigee*>(tmp.release());
      }
      if (thePerigee == nullptr){
        return nullptr;
      }
    }

    ATH_MSG_VERBOSE( " Now running ImpactPoint3dEstimator::Estimate3dIP" );
    double phi0=thePerigee->parameters()[Trk::phi0];
    double dCosPhi0=-std::sin(phi0);
    double dSinPhi0=std::cos(phi0);
    double theta=thePerigee->parameters()[Trk::theta];
    double cotTheta=1./std::tan(thePerigee->parameters()[Trk::theta]);
    double d0=thePerigee->parameters()[Trk::d0];

    //I need the radius (magnetic field...)
    double Bz=magnFieldVect[2]*299.792;
    double Rt=std::sin(theta)/(Bz*thePerigee->parameters()[Trk::qOverP]);

    double x0=thePerigee->associatedSurface().center().x()+(d0-Rt)*dCosPhi0;
    double y0=thePerigee->associatedSurface().center().y()+(d0-Rt)*dSinPhi0;
    double z0=thePerigee->associatedSurface().center().z()+thePerigee->parameters()[Trk::z0]+Rt*phi0*cotTheta;

    if (thePerigee!=trackPerigee) {
      delete thePerigee;
      thePerigee=nullptr;
    }

    double xc=theVertex->x();
    double yc=theVertex->y();
    double zc=theVertex->z();

    double phiActual=phi0;
    double dCosPhiActual=-std::sin(phiActual);
    double dSinPhiActual=std::cos(phiActual);

    double secderivative=0.;
    double derivative=0.;

    int ncycle=0;
    bool isok=false;

    double deltaphi=0.;

#ifdef IMPACTPOINT3DESTIMATOR_DEBUG
    std::cout << std::setprecision(25) << "actual distance  before cycle is: " << std::hypot(x0-xc+Rt*dCosPhiActual, 
                                                                                  y0-yc+Rt*dSinPhiActual, 
                                                                                  z0-zc-Rt*cotTheta*phiActual) << std::endl;
#endif

    do {

#ifdef IMPACTPOINT3DESTIMATOR_DEBUG
      ATH_MSG_VERBOSE( "Cycle number: " << ncycle << " old phi: " << phiActual  );
#endif
     

      derivative=(x0-xc)*(-Rt*dSinPhiActual)+(y0-yc)*Rt*dCosPhiActual+(z0-zc-Rt*phiActual*cotTheta)*(-Rt*cotTheta);
      secderivative=Rt*(-(x0-xc)*dCosPhiActual-(y0-yc)*dSinPhiActual+Rt*cotTheta*cotTheta);
#ifdef IMPACTPOINT3DESTIMATOR_DEBUG
      ATH_MSG_VERBOSE( "derivative is: " << derivative << " sec derivative is: " << secderivative  );
#endif

      deltaphi=-derivative/secderivative;

#ifdef IMPACTPOINT3DESTIMATOR_DEBUG
      std::cout << std::setprecision(25) << "deltaphi: " << deltaphi << std::endl;
#endif

      phiActual+=deltaphi;
      dCosPhiActual=-std::sin(phiActual);
      dSinPhiActual=std::cos(phiActual);

#ifdef IMPACTPOINT3DESTIMATOR_DEBUG
      ATH_MSG_VERBOSE( "derivative is: " << derivative << " sec derivative is: " << secderivative  );
      std::cout << std::setprecision(25) << std::hypot(x0-xc+Rt*dCosPhiActual, y0-yc+Rt*dSinPhiActual,
                                                      z0-zc-Rt*cotTheta*phiActual) << std::endl;
      ATH_MSG_VERBOSE( "actual distance is: " << std::hypot(x0-xc+Rt*dCosPhiActual,
                                                            y0-yc+Rt*dSinPhiActual,
                                                            z0-zc-Rt*cotTheta*phiActual));
#endif

      if (secderivative<0) throw error::ImpactPoint3dEstimatorProblem("Second derivative is negative");

      if (ncycle>m_maxiterations) throw error::ImpactPoint3dEstimatorProblem("Too many loops: could not find minimum distance to vertex");

      ncycle+=1;
      if (ncycle>m_maxiterations||std::abs(deltaphi)<m_precision) {
#ifdef IMPACTPOINT3DESTIMATOR_DEBUG
        ATH_MSG_VERBOSE( "found minimum at: " << phiActual  );
#endif
        isok=true;
      }

    } while (!isok);

    //now you have to construct the plane with PlaneSurface
    //first vector at 3d impact point
    Amg::Vector3D MomentumDir(std::cos(phiActual)*std::sin(theta),std::sin(phiActual)*std::sin(theta),std::cos(theta));
    Amg::Vector3D DeltaR(x0-xc+Rt*dCosPhiActual,y0-yc+Rt*dSinPhiActual,z0-zc-Rt*cotTheta*phiActual);
    distance=DeltaR.mag();
    if (distance==0.){
      ATH_MSG_WARNING("DeltaR is zero in ImpactPoint3dEstimator::Estimate3dIP, returning nullptr");
      return nullptr;
    }
    DeltaR=DeltaR.unit();


    //correct DeltaR from small deviations from orthogonality to DeltaR

    Amg::Vector3D DeltaRcorrected=DeltaR-(DeltaR.dot(MomentumDir))*MomentumDir;

    if ((DeltaR-DeltaRcorrected).mag()>1e-4)
    {
      ATH_MSG_WARNING( " DeltaR and MomentumDir are not orthogonal "  );
      ATH_MSG_DEBUG( std::setprecision(10) << " DeltaR-DeltaRcorrected: "  << (DeltaR-DeltaRcorrected).mag()  );
    }

    Amg::Vector3D YDir=MomentumDir.cross(DeltaRcorrected);

    //store the impact 3d point
    Amg::Vector3D vertex(x0+Rt*dCosPhiActual,y0+Rt*dSinPhiActual,z0-Rt*cotTheta*phiActual);

    ATH_MSG_VERBOSE( "final minimal distance is: " << std::hypot(x0-xc+Rt*dCosPhiActual,
                                                                 y0-yc+Rt*dSinPhiActual,
                                                                 z0-zc-Rt*cotTheta*phiActual));

    ATH_MSG_DEBUG( "POCA in 3D is: " << vertex  );


    //store the plane...
    ATH_MSG_VERBOSE( "plane to which to extrapolate X " << DeltaRcorrected << " Y " << YDir << " Z " << MomentumDir  );

    Amg::Transform3D thePlane(DeltaRcorrected, YDir, MomentumDir, *theVertex);

#ifdef IMPACTPOINT3DESTIMATOR_DEBUG
    std::cout << "the translation is, directly from Transform3d: " << thePlane.getTranslation() << endmsg;
#endif
    return std::make_unique<PlaneSurface>(thePlane);
  }//end of estimate 3dIP method

  bool
  ImpactPoint3dEstimator::addIP3dAtaPlane(VxTrackAtVertex & vtxTrack,const Amg::Vector3D & vertex) const
  {
    if (vtxTrack.initialPerigee()) {
      const AtaPlane* myPlane=IP3dAtaPlane(vtxTrack,vertex);
      if (myPlane)
        {
          vtxTrack.setImpactPoint3dAtaPlane(myPlane);
          return true;
        }
    } else { //for neutrals
      const NeutralAtaPlane* myPlane=IP3dNeutralAtaPlane(vtxTrack.initialNeutralPerigee(),vertex);
      if (myPlane)      {
        ATH_MSG_VERBOSE ("Adding plane: " << myPlane->associatedSurface() );
        vtxTrack.setImpactPoint3dNeutralAtaPlane(myPlane);
        return true;
      }
    }
    return false;
  }


  const Trk::AtaPlane *
  ImpactPoint3dEstimator::IP3dAtaPlane(VxTrackAtVertex & vtxTrack,const Amg::Vector3D & vertex) const
  {
    if (!vtxTrack.initialPerigee() && vtxTrack.initialNeutralPerigee())
      ATH_MSG_WARNING( "Calling ImpactPoint3dEstimator::IP3dAtaPlane cannot return NeutralAtaPlane"  );
    std::unique_ptr<PlaneSurface> theSurfaceAtIP;
    try
    {
      double distance = 0;
      theSurfaceAtIP = Estimate3dIP(vtxTrack.initialPerigee(),&vertex,distance);
    }
    catch (error::ImpactPoint3dEstimatorProblem err)
    {
      ATH_MSG_WARNING( " ImpactPoint3dEstimator failed to find minimum distance between track and vertex seed: " << err.p  );
      return nullptr;
    }
    if(!theSurfaceAtIP){
      ATH_MSG_WARNING( " ImpactPoint3dEstimator failed to find minimum distance and returned 0 " );
      return nullptr;
    }
#ifdef ImpactPoint3dAtaPlaneFactory_DEBUG
    ATH_MSG_VERBOSE( "Original perigee was: " << *(vtxTrack.initialPerigee())  );
    ATH_MSG_VERBOSE( "The resulting surface is: " << *theSurfaceAtIP  );
#endif
   const auto* pTrackPar = m_extrapolator->extrapolate(
     Gaudi::Hive::currentContext(),
     *(vtxTrack.initialPerigee()),
     *theSurfaceAtIP).release();
   if (const Trk::AtaPlane* res = dynamic_cast<const Trk::AtaPlane *>(pTrackPar); res){
     return res;
   }
   ATH_MSG_WARNING("TrackParameters ptr returned from extrapolate could not be cast to Trk::AtaPlane* in IP3dAtaPlane(..)");
   return nullptr;
  }


  const Trk::NeutralAtaPlane *
  ImpactPoint3dEstimator::IP3dNeutralAtaPlane(const NeutralParameters * initNeutPerigee,const Amg::Vector3D & vertex) const
  {
    std::unique_ptr<PlaneSurface> theSurfaceAtIP;

    try
    {
        double distance = 0;
        theSurfaceAtIP = Estimate3dIP(initNeutPerigee,&vertex,distance);
    }
    catch (error::ImpactPoint3dEstimatorProblem err)
    {
      ATH_MSG_WARNING( " ImpactPoint3dEstimator failed to find minimum distance between track and vertex seed: " << err.p  );
      return nullptr;
    }
    if(!theSurfaceAtIP){
      ATH_MSG_WARNING( " ImpactPoint3dEstimator failed to find minimum distance and returned 0 " );
      return nullptr;
    } 
#ifdef ImpactPoint3dAtaPlaneFactory_DEBUG
    ATH_MSG_VERBOSE( "Original neutral perigee was: " << *initNeutPerigee  );
    ATH_MSG_VERBOSE( "The resulting surface is: " << *theSurfaceAtIP  );
#endif

    const Trk::NeutralAtaPlane* res = nullptr;
    std::unique_ptr<const Trk::NeutralParameters> tmp =  m_extrapolator->extrapolate(*initNeutPerigee,*theSurfaceAtIP);
    if(dynamic_cast<const Trk::NeutralAtaPlane*> (tmp.get())){
      res = static_cast<const Trk::NeutralAtaPlane*> (tmp.release());
    }
   return res;
  }


}
