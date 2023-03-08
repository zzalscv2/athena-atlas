/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _SCTRIO_OnTrackErrorScaling_H_
#define _SCTRIO_OnTrackErrorScaling_H_

#include "TrkRIO_OnTrack/RIO_OnTrackErrorScaling.h"
class SCTRIO_OnTrackErrorScaling final : public RIO_OnTrackErrorScaling
{
public:
  static constexpr RIO_OnTrackErrorScaling::Type s_type = RIO_OnTrackErrorScaling::SCT;
  virtual CLID clid() const override final;

  Amg::MatrixX getScaledCovariance(Amg::MatrixX&& cov_input,
                                   bool is_endcap,
                                   double sinLocalAngle) const;

  enum ESCTErrorScalingRegions
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

CLASS_DEF(SCTRIO_OnTrackErrorScaling, 14458362, 1)

#include "AthenaKernel/CondCont.h"
CONDCONT_DEF(SCTRIO_OnTrackErrorScaling, 6837094, RIO_OnTrackErrorScaling);

#endif
