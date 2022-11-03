/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   TrackStateOnSurfaceCnv_p1.cxx
//
//-----------------------------------------------------------------------------

#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkEventTPCnv/TrkTrack/TrackStateOnSurfaceCnv_p1.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkParameters/TrackParameters.h"


void TrackStateOnSurfaceCnv_p1::
persToTrans( const Trk::TrackStateOnSurface_p1 *persObj, Trk::TrackStateOnSurface *transObj, MsgStream &log )
{
  ITPConverterFor<Trk::MeasurementBase>	*measureCnv = nullptr;
  const Trk::MeasurementBase* meas =  createTransFromPStore( &measureCnv, persObj->m_measurementOnTrack, log );

  ITPConverterFor<Trk::TrackParameters>	*paramsCnv = nullptr;
  const Trk::TrackParameters* trackParameters = dynamic_cast<const Trk::TrackParameters*>(createTransFromPStore( &paramsCnv, persObj->m_trackParameters, log ));

  std::unique_ptr<const Trk::FitQuality>  fitQ(createTransFromPStore( &m_fitQCnv, persObj->m_fitQualityOnSurface, log));
  auto fitQos = fitQ ? std::make_unique<Trk::FitQualityOnSurface>(*fitQ) : nullptr;

  const Trk::MaterialEffectsBase* materialEffects = nullptr;
  if (! persObj->m_scatteringAngle.isNull() ) {
    materialEffects =
      createTransFromPStore( &m_scatCnv, persObj->m_scatteringAngle, log );
  }

   if (!persObj->m_materialEffectsOnTrack.isNull()) {
     materialEffects =
       createTransFromPStore( &m_bremCnv, persObj->m_materialEffectsOnTrack,log);
   }

  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> types;
  std::bitset<Trk::TrackStateOnSurface::NumberOfPersistencyHints> hints;
  Trk::TrackStateOnSurface::splitToBitsets(persObj->m_typeFlags, types, hints);
  *transObj = Trk::TrackStateOnSurface (std::unique_ptr<const Trk::MeasurementBase>(meas),
                                        std::unique_ptr<const Trk::TrackParameters>(trackParameters),
                                        std::move(fitQos),
                                        std::unique_ptr<const Trk::MaterialEffectsBase>(materialEffects),
                                        types,
                                        hints);
}


void TrackStateOnSurfaceCnv_p1::
transToPers( const Trk::TrackStateOnSurface *, Trk::TrackStateOnSurface_p1 *, MsgStream & ) 
{
  throw std::runtime_error("TrackStateOnSurfaceCnv_p1::transToPers is deprecated!");   
}

