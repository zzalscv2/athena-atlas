/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   TrackStateOnSurfaceCnv_p2.cxx
//
//-----------------------------------------------------------------------------

#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkParametersBase/ParametersT.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkEventTPCnv/TrkTrack/TrackStateOnSurfaceCnv_p2.h"

#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkMaterialOnTrack/MaterialEffectsBase.h"

//#include "TrkEventTPCnv/TrkParameters/TrackParametersCnv_p1.h"


void TrackStateOnSurfaceCnv_p2::
persToTrans( const Trk::TrackStateOnSurface_p2 *persObj, Trk::TrackStateOnSurface *transObj, MsgStream &log )
{
  ITPConverterFor<Trk::MeasurementBase>	*measureCnv = nullptr;
  const Trk::MeasurementBase* meas =  createTransFromPStore( &measureCnv, persObj->m_measurementOnTrack, log );

  ITPConverterFor<Trk::TrackParameters>	*paramsCnv = nullptr;
  const Trk::TrackParameters* trackParameters = dynamic_cast<const Trk::TrackParameters*>(createTransFromPStore( &paramsCnv, persObj->m_trackParameters, log ));

  std::unique_ptr<const Trk::FitQuality>  fitQ(createTransFromPStore( &m_fitQCnv, persObj->m_fitQualityOnSurface, log));
  auto fitQos = fitQ ? Trk::FitQualityOnSurface(*fitQ) : Trk::FitQualityOnSurface{};

  ITPConverterFor<Trk::MaterialEffectsBase> *matBaseCnv = nullptr;
  const Trk::MaterialEffectsBase* materialEffects = createTransFromPStore( &matBaseCnv, persObj->m_materialEffects, log );

  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> types;
  std::bitset<Trk::TrackStateOnSurface::NumberOfPersistencyHints> hints;
  Trk::TrackStateOnSurface::splitToBitsets(persObj->m_typeFlags, types, hints);
  *transObj = Trk::TrackStateOnSurface (fitQos,
                                        std::unique_ptr<const Trk::MeasurementBase> (meas),
                                        std::unique_ptr<const Trk::TrackParameters>(trackParameters),
                                        std::unique_ptr<const Trk::MaterialEffectsBase>(materialEffects),
                                        types);
  //Hints are atomic. Set once here if TSOS was slimmed
  //aka not 0. If 0 we want to allow setting them later on
  uint8_t hintsUInt = hints.to_ulong();
  if(hintsUInt!=0){
    transObj->setHints(hintsUInt);
  }

}


void TrackStateOnSurfaceCnv_p2::
transToPers( const Trk::TrackStateOnSurface *, Trk::TrackStateOnSurface_p2 *, MsgStream & )
{
  throw std::runtime_error("TrackStateOnSurfaceCnv_p2::transToPers is deprecated!");
}
