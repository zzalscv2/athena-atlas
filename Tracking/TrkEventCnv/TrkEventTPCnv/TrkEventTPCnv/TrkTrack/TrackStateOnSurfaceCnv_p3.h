/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACK_STATE_ON_SURFACE_CNV_P3_H
#define TRACK_STATE_ON_SURFACE_CNV_P3_H

//-----------------------------------------------------------------------------
//
// file:   TrackStateOnSurfaceCnv_p3.h
//
//-----------------------------------------------------------------------------

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkTrack/MultiComponentStateOnSurface.h"
#include "TrkEventTPCnv/TrkTrack/TrackStateOnSurface_p3.h"

#include "TrkEventTPCnv/TrkEventPrimitives/FitQualityCnv_p1.h"
#include "TrkEventTPCnv/TrkMaterialOnTrack/MaterialEffectsBaseCnv_p1.h"
#include "TrkEventTPCnv/TrkParameters/TrackParametersCnv_p2.h"

class MsgStream;

class TrackStateOnSurfaceCnv_p3
   : public T_AthenaPoolTPCnvBase<Trk::TrackStateOnSurface, Trk::TrackStateOnSurface_p3>
{
public:
  TrackStateOnSurfaceCnv_p3() : m_fitQCnv(0), m_parametersCnv(0) {}
  
  virtual void persToTrans( const Trk::TrackStateOnSurface_p3 *persObj,
			    Trk::TrackStateOnSurface    *transObj,
			    MsgStream                     &log ) ;
  virtual void transToPers( const Trk::TrackStateOnSurface    *transObj,
			    Trk::TrackStateOnSurface_p3 *persObj,
			    MsgStream                      &log );

private:
  FitQualityCnv_p1                  *m_fitQCnv;
  // MaterialEffectsBaseCnv_p1         *m_matBaseCnv;
  TrackParametersCnv_p2             *m_parametersCnv;
};

// We need to handle MultiComponentStateOnSurface trans
// to TrackStateOnSurface_p3 pe
// We care just for the TSOS base part
class MultiComponentStateOnSurfaceCnv_p1
  : public T_AthenaPoolTPCnvBase<Trk::MultiComponentStateOnSurface,
                                 Trk::TrackStateOnSurface_p3>
{
public:
  MultiComponentStateOnSurfaceCnv_p1()
    : m_trackStateOnSurfaceCnv(nullptr)
  {
  }
  virtual void persToTrans(
    const Trk::TrackStateOnSurface_p3* persObj,
    Trk::MultiComponentStateOnSurface* transObj,
    MsgStream& log);
  virtual void transToPers(
    const Trk::MultiComponentStateOnSurface* transObj,
    Trk::TrackStateOnSurface_p3* persObj,
    MsgStream& log);

private:
  TrackStateOnSurfaceCnv_p3 *m_trackStateOnSurfaceCnv;
};
#endif // TRACK_STATE_ON_SURFACE_CNV_P2_H
