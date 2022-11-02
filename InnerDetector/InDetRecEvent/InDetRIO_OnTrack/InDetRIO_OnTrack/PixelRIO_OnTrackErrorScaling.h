/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _PixelRIO_OnTrackErrorScaling_H_
#define _PixelRIO_OnTrackErrorScaling_H_

#include "TrkRIO_OnTrack/RIO_OnTrackErrorScaling.h"
class PixelID;
class Identifier;

class PixelRIO_OnTrackErrorScaling final : public RIO_OnTrackErrorScaling
{
public:
  virtual CLID clid() const override final;

  Amg::MatrixX getScaledCovariance(const Amg::MatrixX& cov_input,
                                   const PixelID& pixel_id,
                                   const Identifier& id) const;

  enum EPixelErrorScalingRegions
  {
    kPixBarrelPhi,
    kPixBarrelEta,
    kPixEndcapPhi,
    kPixEndcapEta,
    kPixIBLPhi,
    kPixIBLEta,
    kNParamTypes
  };

  static const char* const* paramNames() { return s_names; }
  virtual bool postProcess() override final;

protected:
  static const char* const s_names[kNParamTypes];
};

CLASS_DEF(PixelRIO_OnTrackErrorScaling, 254709438, 1)

#include "AthenaKernel/CondCont.h"
CONDCONT_DEF(PixelRIO_OnTrackErrorScaling, 87055250, RIO_OnTrackErrorScaling);

#endif
