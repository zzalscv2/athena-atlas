/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// VKalVrtAtlas.h
//
#ifndef TRKVKALVRTFITTER_VKALVRTATLASMAGFIELD_H
#define TRKVKALVRTFITTER_VKALVRTATLASMAGFIELD_H

// Mag field service
#include "MagFieldElements/AtlasFieldCache.h"
#include "TrkVKalVrtCore/VKalVrtBMag.h"

namespace Trk {
//  ATLAS magnetic field access for TrkVKalVrtFitter
//-----------------------------------------------------
class VKalAtlasMagFld : public Trk::baseMagFld {
 public:
  VKalAtlasMagFld();
  ~VKalAtlasMagFld();
  virtual void getMagFld(const double, const double, const double, double &,
                         double &, double &) override;
  void setAtlasField(MagField::AtlasFieldCache *);
  void setAtlasField(const double);
  void setAtlasMagRefFrame(double, double, double);

 private:
  MagField::AtlasFieldCache *m_VKalAthenaField{};
  double m_FIXED_ATLAS_FIELD = 1.997;
  double m_magFrameX, m_magFrameY, m_magFrameZ;
};
}  // namespace Trk

#endif
