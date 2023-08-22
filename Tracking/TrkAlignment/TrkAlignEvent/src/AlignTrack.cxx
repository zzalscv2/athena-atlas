/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkTrack/Track.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"
#include "TrkMaterialOnTrack/EnergyLoss.h"

#include "TrkAlignEvent/AlignTrack.h"

namespace Trk {

  //________________________________________________________________________
  AlignTrack::AlignTrack(const Track& associatedTrack) 
    : Track(associatedTrack)
    , m_originalTrack(nullptr)
    , m_type(Unknown)
    , m_nAlignTSOSMeas(0)
    , m_alignTSOSCollection(nullptr)
    , m_localErrorMat(nullptr)
    , m_localErrorMatInv(nullptr)
    , m_derivativeMatrix(nullptr)
    , m_fullCovarianceMatrix(nullptr)
    , m_derivatives(nullptr)
    , m_derivativeErr(nullptr)
    , m_actualSecondDerivatives(nullptr)
    , m_residuals(nullptr)
    , m_weights(nullptr)
    , m_weightsFirstDeriv(nullptr)
    , m_chi2(0.)
    , m_chi2dof(0.)
    , m_trackAlignParamQuality(new double[6])
    , m_trackWithoutScattering()
  {
    for (int i=0;i<6;i++) m_trackAlignParamQuality[i]=0.;
    m_vtx=nullptr;
    m_refitD0=true;
    m_refitZ0=true;
    m_refitPhi=true;
    m_refitTheta=true;
    m_refitQovP=true;
  }

  //________________________________________________________________________
  AlignTrack::AlignTrack(const Track& associatedTrack, 
                         AlignTSOSCollection* alignTSOSCollection)
    : Track(associatedTrack)
    , m_originalTrack(nullptr)
    , m_type(Unknown)
    , m_nAlignTSOSMeas(0)
    , m_localErrorMat(nullptr)
    , m_localErrorMatInv(nullptr)
    , m_derivativeMatrix(nullptr)
    , m_fullCovarianceMatrix(nullptr)
    , m_derivatives(nullptr)
    , m_derivativeErr(nullptr)
    , m_actualSecondDerivatives(nullptr)
    , m_residuals(nullptr)
    , m_weights(nullptr)
    , m_weightsFirstDeriv(nullptr)
    , m_chi2(0.)
    , m_chi2dof(0.)
    , m_trackAlignParamQuality(new double[6])
    , m_trackWithoutScattering()
  {
    for (int i=0;i<6;i++) m_trackAlignParamQuality[i]=0.;
    setAlignTSOSCollection(alignTSOSCollection);
    m_vtx=nullptr;
    m_refitD0=true;
    m_refitZ0=true;
    m_refitPhi=true;
    m_refitTheta=true;
    m_refitQovP=true;
  }

  //________________________________________________________________________
  AlignTrack::AlignTrack(const AlignTrack& atrack)
    : Track(atrack)
    , m_originalTrack(atrack.m_originalTrack)
    , m_type(atrack.m_type)
    , m_nAlignTSOSMeas(atrack.m_nAlignTSOSMeas)
    , m_localErrorMat(atrack.m_localErrorMat ?
        new Amg::SymMatrixX(*(atrack.m_localErrorMat)) : nullptr)
    , m_localErrorMatInv(atrack.m_localErrorMatInv ?
        new Amg::SymMatrixX(*(atrack.m_localErrorMatInv)) : nullptr)
    , m_derivativeMatrix(atrack.m_derivativeMatrix ?
        new Amg::MatrixX(*(atrack.m_derivativeMatrix)) : nullptr)
    , m_fullCovarianceMatrix(atrack.m_fullCovarianceMatrix ?
        new Amg::SymMatrixX(*(atrack.m_fullCovarianceMatrix)) : nullptr)
    , m_derivatives(atrack.m_derivatives ?
        new std::vector<AlignModuleDerivatives>(*(atrack.m_derivatives)) : nullptr)
    , m_derivativeErr(atrack.m_derivativeErr ?
        new std::vector<AlignModuleDerivatives>(*(atrack.m_derivativeErr)) : nullptr)
    , m_actualSecondDerivatives(atrack.m_actualSecondDerivatives ?
        new std::vector<std::pair<AlignModule*,std::vector<double> > > (*(atrack.m_actualSecondDerivatives)) : nullptr)
    , m_residuals(atrack.m_residuals ?
        new Amg::VectorX(*(atrack.m_residuals)) : nullptr)
    , m_weights(atrack.m_weights ?
        new Amg::SymMatrixX(*(atrack.m_weights)) : nullptr)
    , m_weightsFirstDeriv(atrack.m_weightsFirstDeriv ?
        new Amg::SymMatrixX(*(atrack.m_weightsFirstDeriv)) : nullptr)
    , m_chi2(atrack.m_chi2)
    , m_chi2dof(atrack.m_chi2dof)
    , m_trackAlignParamQuality(new double[6])
    , m_trackWithoutScattering()
  {
    if (atrack.m_trackWithoutScattering) {
      m_trackWithoutScattering.set(std::make_unique<Trk::Track>(*(atrack.m_trackWithoutScattering)));
    }

    for (int i=0;i<6;i++)
      m_trackAlignParamQuality[i] = atrack.m_trackAlignParamQuality[i];

    // copy AlignTSOSCollection
    if(atrack.m_alignTSOSCollection!=nullptr) {
      AlignTSOSCollection * aTSOScoll = new AlignTSOSCollection;
      aTSOScoll->reserve(atrack.m_alignTSOSCollection->size());
      AlignTSOSIt itAtsos_end = atrack.m_alignTSOSCollection->end();
      for(AlignTSOSIt itAtsos = atrack.m_alignTSOSCollection->begin(); itAtsos!=itAtsos_end; ++itAtsos) {
        assert(*itAtsos!=0); // check that is defined.
        AlignTSOS * atsos = new AlignTSOS(**itAtsos);
        aTSOScoll->push_back(atsos);
      }
      m_alignTSOSCollection=aTSOScoll;
    }
    else
      m_alignTSOSCollection=nullptr;

    findPerigee();
    m_vtx=nullptr;
    m_refitD0=true;
    m_refitZ0=true;
    m_refitPhi=true;
    m_refitTheta=true;
    m_refitQovP=true;
  }

  //________________________________________________________________________
  AlignTrack& AlignTrack::operator= (const AlignTrack& atrack)
  {
    if (this!=&atrack) {

      // assign Track content
      Track::operator=(atrack);

      // now fill the extra stuff
      m_originalTrack  = atrack.m_originalTrack;
      m_type           = atrack.m_type;
      m_nAlignTSOSMeas = atrack.m_nAlignTSOSMeas;
      m_chi2           = atrack.m_chi2;
      m_chi2dof        = atrack.m_chi2dof;
      m_vtx            = atrack.m_vtx;
      m_refitD0        = atrack.m_refitD0;
      m_refitZ0        = atrack.m_refitZ0;
      m_refitPhi       = atrack.m_refitPhi;
      m_refitTheta     = atrack.m_refitTheta;
      m_refitQovP      = atrack.m_refitQovP;

      for (int i=0;i<6;i++)
        m_trackAlignParamQuality[i] = atrack.m_trackAlignParamQuality[i];

      // copy AlignTSOSCollection
      delete m_alignTSOSCollection;
      if(atrack.m_alignTSOSCollection!=nullptr) {
        AlignTSOSCollection * aTSOScoll = new AlignTSOSCollection;
        aTSOScoll->reserve(atrack.m_alignTSOSCollection->size());
        AlignTSOSIt itAtsos_end = atrack.m_alignTSOSCollection->end();
        for(AlignTSOSIt itAtsos = atrack.m_alignTSOSCollection->begin(); itAtsos!=itAtsos_end; ++itAtsos) {
          assert(*itAtsos!=0); // check that is defined.
          AlignTSOS * atsos = new AlignTSOS(**itAtsos);
          aTSOScoll->push_back(atsos);
        }
        m_alignTSOSCollection=aTSOScoll;
      }
      else
        m_alignTSOSCollection=nullptr;

      findPerigee();

      // fill matrices, vectors, etc.
      delete m_derivatives;
      m_derivatives = atrack.m_derivatives ?
          new std::vector<AlignModuleDerivatives>(*(atrack.m_derivatives)) : nullptr;
      delete m_derivativeErr;
      m_derivativeErr = atrack.m_derivativeErr ?
          new std::vector<AlignModuleDerivatives>(*(atrack.m_derivativeErr)) : nullptr;
      delete m_actualSecondDerivatives;
      m_actualSecondDerivatives = atrack.m_actualSecondDerivatives ?
          new std::vector<std::pair<AlignModule*,std::vector<double> > > (*(atrack.m_actualSecondDerivatives)) : nullptr;
      delete m_localErrorMat;
      m_localErrorMat = atrack.m_localErrorMat ?
          new Amg::SymMatrixX(*(atrack.m_localErrorMat)) : nullptr;
      delete m_localErrorMatInv;
      m_localErrorMatInv = atrack.m_localErrorMatInv ?
          new Amg::SymMatrixX(*(atrack.m_localErrorMatInv)) : nullptr;
      delete m_derivativeMatrix;
      m_derivativeMatrix = atrack.m_derivativeMatrix ?
          new Amg::MatrixX(*(atrack.m_derivativeMatrix)) : nullptr;
      delete m_fullCovarianceMatrix;
      m_fullCovarianceMatrix = atrack.m_fullCovarianceMatrix ?
          new Amg::SymMatrixX(*(atrack.m_fullCovarianceMatrix)) : nullptr;
      delete m_residuals;
      m_residuals = atrack.m_residuals ?
          new Amg::VectorX(*(atrack.m_residuals)) : nullptr;
      delete m_weights;
      m_weights = atrack.m_weights ?
          new Amg::SymMatrixX(*(atrack.m_weights)) : nullptr;
      delete m_weightsFirstDeriv;
      m_weightsFirstDeriv = atrack.m_weightsFirstDeriv ?
          new Amg::SymMatrixX(*(atrack.m_weightsFirstDeriv)) : nullptr;
    }

    return *this;
  }

  //________________________________________________________________________
  AlignTrack::~AlignTrack()
  {
    delete m_alignTSOSCollection;     m_alignTSOSCollection=nullptr;
    delete m_residuals;               m_residuals=nullptr;
    delete m_weights;                 m_weights=nullptr;
    delete m_weightsFirstDeriv;       m_weightsFirstDeriv=nullptr;

    delete m_derivatives;             m_derivatives=nullptr;
    delete m_derivativeErr;           m_derivativeErr=nullptr;
    delete m_actualSecondDerivatives; m_actualSecondDerivatives=nullptr;

    delete m_localErrorMat;           m_localErrorMat=nullptr;
    delete m_localErrorMatInv;        m_localErrorMatInv=nullptr;

    delete m_fullCovarianceMatrix;    m_fullCovarianceMatrix=nullptr;
    delete m_derivativeMatrix;        m_derivativeMatrix=nullptr;

    delete [] m_trackAlignParamQuality;
  }

  //________________________________________________________________________
  bool AlignTrack::isSLTrack(const Track* track) 
  {
    const Perigee* pp = track->perigeeParameters();
    if( pp ){
      
      
      const AmgSymMatrix(5)* covMatrix = pp->covariance();
      if( covMatrix ){
        // sum covariance terms of momentum, use it to determine whether fit was SL fit
        double momCov = 0.;
        for( int i=0;i<5;++i )
          momCov += fabs( (*covMatrix)(4,i));
        for( int i=0;i<4;++i )
          momCov += fabs( (*covMatrix)(i,4));
        if( momCov < 1e-10 )
          return true;
      }
    }
    return false;
  }

  //________________________________________________________________________
  bool AlignTrack::isSLTrack() const
  {
    return isSLTrack(this);
  }

  //________________________________________________________________________
  void AlignTrack::dumpTrackInfo(const Track& track, MsgStream& msg) 
  {
    for (const TrackStateOnSurface* tsos : *track.trackStateOnSurfaces())
      msg<<*tsos;
  }

  //________________________________________________________________________
  void AlignTrack::dumpLessTrackInfo(const Track& track, MsgStream& msg) 
  {
    int ntsos(0);
    for (const TrackStateOnSurface* tsos : *track.trackStateOnSurfaces()) {
      msg<<"ntsos "<<ntsos<<":"<<", type "<<tsos->dumpType();
      //msg << " perigee center of this TSOS: "<< tsos->trackParameters()->associatedSurface()->center() << endmsg;

      if (tsos->type(TrackStateOnSurface::Perigee)) 
        msg << ", Perigee"<<endmsg;

      else if (tsos->type(TrackStateOnSurface::Outlier))
        msg << ", Outlier"<<endmsg;

      else if ( !tsos->type(TrackStateOnSurface::Scatterer) &&
                !tsos->type(TrackStateOnSurface::InertMaterial)) {
          msg << "," << ( tsos->fitQualityOnSurface() )<<endmsg;
      }

      else {

        const MaterialEffectsBase*     meb = tsos->materialEffectsOnTrack();
        const MaterialEffectsOnTrack* meot = dynamic_cast<const MaterialEffectsOnTrack*>(meb);

        if (meot) {
          msg<<", meot";
          if (meot->scatteringAngles()) 
            msg<<", have angles"<<endmsg;
          else
            msg<<", no angles"<<endmsg;
        }
        else if (tsos->type(TrackStateOnSurface::InertMaterial))
          msg<<", InertMaterial"<<endmsg;
        else
          msg << ", hmm... no material effects on track!"<<endmsg;
      }
      ++ntsos;
    }
  }

  //________________________________________________________________________
  void AlignTrack::dump(MsgStream& msg) 
  {
    msg<<"dumping AlignTrack"<<endmsg;
    double chi2=0.;
    int imeas(1);
    if (m_alignTSOSCollection) {
      int natsos = m_alignTSOSCollection->size();
      for (int iatsos=0;iatsos<natsos;iatsos++) {
        const AlignTSOS* atsos=(*m_alignTSOSCollection)[iatsos];
        std::vector<Residual>::const_iterator itRes     = atsos->firstResidual();
        std::vector<Residual>::const_iterator itRes_end = atsos->lastResidual();
        for ( ; itRes != itRes_end; ++itRes,++imeas) {
          double resNorm = itRes->residualNorm();
          chi2 += resNorm*resNorm;
          //msg<<"resNorm="<<resNorm
          //   <<", errorMat("<<imeas<<")="<<(*m_localErrorMat)(imeas,imeas)
          //   <<endmsg;
        }
        msg<<"iatsos "<<iatsos<<", chi2="<<chi2<<*atsos<<endmsg;
      }
    }
    msg<<"total chi2: "<<m_chi2<<endmsg;
  }

  //________________________________________________________________________
  void AlignTrack::setAlignTSOSCollection(AlignTSOSCollection* atsosColl)
  {
    m_alignTSOSCollection=atsosColl;

    // calculate m_nAlignTSOSMeas
    int natsos = m_alignTSOSCollection->size();
    for (int iatsos=0;iatsos<natsos;iatsos++)
      m_nAlignTSOSMeas += (*m_alignTSOSCollection)[iatsos]->nResDim();

    // calculate V and Vinv    
    m_localErrorMat    = new Amg::SymMatrixX(m_nAlignTSOSMeas,m_nAlignTSOSMeas); 
    m_localErrorMat->setZero();
    m_localErrorMatInv = new Amg::SymMatrixX(m_nAlignTSOSMeas,m_nAlignTSOSMeas);
    m_localErrorMatInv->setZero();
 
    int matrixindex(0);
    AlignTSOSCollection::const_iterator itAtsos     = firstAtsos();
    AlignTSOSCollection::const_iterator itAtsos_end = lastAtsos();
    for ( ; itAtsos != itAtsos_end; ++itAtsos) {
      const AlignTSOS * atsos = *itAtsos;
      std::vector<Residual>::const_iterator itRes     = atsos->firstResidual();
      std::vector<Residual>::const_iterator itRes_end = atsos->lastResidual();
      for ( ; itRes != itRes_end; ++itRes) {
        (*m_localErrorMat)(matrixindex,matrixindex) = itRes->errSq();
        (*m_localErrorMatInv)(matrixindex,matrixindex) = 1./itRes->errSq();
        matrixindex++;
      }
    }
  }

  //________________________________________________________________________
  MsgStream& operator<< (MsgStream& sl, const AlignTrack::AlignTrackType type)
  {
    switch(type) {
      case AlignTrack::Unknown:
        return sl<<"Unknown             ";
      case AlignTrack::Original:
        return sl<<"Original            ";
      case AlignTrack::NormalRefitted:
        return sl<<"NormalRefitted      ";
      case AlignTrack::BeamspotConstrained:
        return sl<<"BeamspotConstrained ";
      case AlignTrack::VertexConstrained:
        return sl<<"VertexConstrained   ";
      default:
        return sl<<"UNDEFINED           ";
    }
  }

  //________________________________________________________________________
  std::ostream& operator<< (std::ostream& sl, const AlignTrack::AlignTrackType type)
  {
    switch(type) {
      case AlignTrack::Unknown:
        return sl<<"Unknown             ";
      case AlignTrack::Original:
        return sl<<"Original            ";
      case AlignTrack::NormalRefitted:
        return sl<<"NormalRefitted      ";
      case AlignTrack::BeamspotConstrained:
        return sl<<"BeamspotConstrained ";
      case AlignTrack::VertexConstrained:
        return sl<<"VertexConstrained   ";
      default:
        return sl<<"UNDEFINED           ";
    }
  }

  //________________________________________________________________________
  std::string dumpAlignTrackType(const AlignTrack::AlignTrackType type)
  {
    switch(type) {
      case AlignTrack::Unknown:              return "Unknown";
      case AlignTrack::Original:             return "Original";
      case AlignTrack::NormalRefitted:       return "NormalRefitted";
      case AlignTrack::BeamspotConstrained:  return "BeamspotConstrained";
      case AlignTrack::VertexConstrained:    return "VertexConstrained";
      default:                               return "UNDEFINED";
    }
  }

  //________________________________________________________________________
  const Trk::Track* AlignTrack::trackWithoutScattering() const
  {
    if (not m_trackWithoutScattering) {

      const DataVector<const Trk::TrackStateOnSurface>* states = this->trackStateOnSurfaces();
      if (!states) return nullptr;
      
      // loop over TSOSs
      DataVector<const Trk::TrackStateOnSurface>::const_iterator tsit = states->begin();
      DataVector<const Trk::TrackStateOnSurface>::const_iterator tsit_end = states->end();
      
      // This is the list of new TSOS 
      auto newTrackStateOnSurfaces = std::make_unique<DataVector<const Trk::TrackStateOnSurface>>();
      newTrackStateOnSurfaces->reserve( states->size() );
      
      for (; tsit!=tsit_end ; ++tsit) {
        auto newMeas  = (*tsit)->measurementOnTrack() ? (*tsit)->measurementOnTrack()->uniqueClone() : nullptr;
        auto newPars  = (*tsit)->trackParameters() ? (*tsit)->trackParameters()->uniqueClone() : nullptr;
        auto newFitQoS= (*tsit)->fitQualityOnSurface();
        auto meb      = (*tsit)->materialEffectsOnTrack() ? (*tsit)->materialEffectsOnTrack()->uniqueClone() : nullptr;
  
        if (meb) {
          //meot is just used as observer, not owner, so can safely duplicate the pointer
          const auto *meot=dynamic_cast<const Trk::MaterialEffectsOnTrack*>(meb.get());
          if (meot) {
            double tinX0=meot->thicknessInX0();
            std::unique_ptr<Trk::EnergyLoss> eLoss =
              meot->energyLoss()
                ? std::unique_ptr<Trk::EnergyLoss>(meot->energyLoss()->clone())
                : nullptr;
            const Trk::Surface& surf = meot->associatedSurface();
            std::bitset<MaterialEffectsBase::NumberOfMaterialEffectsTypes> typeMaterial;
            if (eLoss) typeMaterial.set(MaterialEffectsBase::EnergyLossEffects);
            Trk::MaterialEffectsOnTrack* newmeot=
                new Trk::MaterialEffectsOnTrack(tinX0,std::nullopt,std::move(eLoss),surf,typeMaterial);
            meb.reset(newmeot);
          }
        }
  
        std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> typePattern;
        for (int i=0;i<(int)Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes;i++) {
          if ((*tsit)->type(Trk::TrackStateOnSurface::TrackStateOnSurfaceType(i)))
            typePattern.set(i);
        }
        const Trk::TrackStateOnSurface* newTsos =
          new Trk::TrackStateOnSurface(newFitQoS,
                                       std::move(newMeas),
                                       std::move(newPars),
                                       std::move(meb),
                                       typePattern);
        newTrackStateOnSurfaces->push_back(newTsos);
      }
      
      m_trackWithoutScattering.set(std::make_unique<Trk::Track>( this->info(), std::move(newTrackStateOnSurfaces), 
                                                                 this->fitQuality() ? 
                                                                 this->fitQuality()->uniqueClone() : nullptr ));
    }
    return m_trackWithoutScattering.get();
  }
  
} // end namespace
