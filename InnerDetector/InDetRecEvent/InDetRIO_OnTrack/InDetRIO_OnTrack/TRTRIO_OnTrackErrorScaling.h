/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _TRTRIO_OnTrackErrorScaling_H_
#define _TRTRIO_OnTrackErrorScaling_H_

#include "TrkRIO_OnTrack/RIO_OnTrackErrorScaling.h"
class TRTRIO_OnTrackErrorScaling final : public RIO_OnTrackErrorScaling
{
public:
  static constexpr RIO_OnTrackErrorScaling::Type s_type = RIO_OnTrackErrorScaling::TRT;
  virtual CLID clid() const override final;

  Amg::MatrixX getScaledCovariance(Amg::MatrixX&& cov_input,
                                   bool is_endcap,
                                   double mu) const;

  enum ETRTErrorScalingRegions
  {
    kBarrel,
    kEndcap,
    kNParamTypes
  };

  static const char* const* paramNames() { return s_names; }
  virtual bool postProcess() override final;

  virtual Type type() const override final {return s_type;}
protected:
  static const char* const s_names[kNParamTypes];
};

CLASS_DEF(TRTRIO_OnTrackErrorScaling, 216884486, 1)
CONDCONT_DEF(TRTRIO_OnTrackErrorScaling, 112040354, RIO_OnTrackErrorScaling);
#endif
