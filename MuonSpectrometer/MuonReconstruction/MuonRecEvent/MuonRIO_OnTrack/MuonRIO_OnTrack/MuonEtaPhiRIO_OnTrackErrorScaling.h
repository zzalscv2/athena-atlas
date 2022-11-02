/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUONETAPHIRIO_OnTrackErrorScaling_H_
#define _MUONETAPHIRIO_OnTrackErrorScaling_H_

#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkRIO_OnTrack/RIO_OnTrackErrorScaling.h"

class MuonEtaPhiRIO_OnTrackErrorScaling final : public RIO_OnTrackErrorScaling
{
public:
  virtual CLID clid() const override final;

  Amg::MatrixX getScaledCovariance(const Amg::MatrixX& cov_input,
                                   const Trk::ParamDefs measuredCoord) const;

  enum EMuonEtaPhiErrorScalingRegions
  {
    kPhi,
    kEta,
    kNParamTypes
  };

  static const char* const* paramNames() { return s_names; }
  virtual bool postProcess() override final;

protected:
  static const char* const s_names[kNParamTypes];
};

CLASS_DEF(MuonEtaPhiRIO_OnTrackErrorScaling, 92816455, 1)
CONDCONT_DEF(MuonEtaPhiRIO_OnTrackErrorScaling,
             81544491,
             RIO_OnTrackErrorScaling);
#endif
