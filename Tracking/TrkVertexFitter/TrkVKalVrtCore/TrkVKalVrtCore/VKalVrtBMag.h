/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/* General magnetic field in any point access              */
/* If external magnetic field handler is provided as       */
/* either an object inherited from the baseMagFld class    */
/* or a function addrMagHandler - the vkalMagFld class     */
/* will use it, otherwise the vkalMagFld returns           */
/* the constant magnetic field.                            */
/* 							   */
/*  Thread-safe implementation				   */
/*---------------------------------------------------------*/
#ifndef TRKVKALVRTCORE_VKALVRTBMAG_H
#define TRKVKALVRTCORE_VKALVRTBMAG_H

#include "TrkVKalVrtCore/CommonPars.h"

namespace Trk {

class VKalVrtControlBase;

typedef void (*addrMagHandler)(double, double, double, double&, double&,
                               double&);

//
//  Base class for concrete magnetic field implementations (e.g. Athena tool) to
//  be called by vkalMagFld
//
class baseMagFld {
 public:
  baseMagFld() = default;
  virtual ~baseMagFld() = default;
  virtual void getMagFld(const double, const double, const double, double&,
                         double&, double&) = 0;
};

//
// Main magnetic field implememtation in VKalVrtCore package.
// Depending on VKalVrtControlBase it either calls external magnetic field
// or uses default fixed magnetic field.
//
class vkalMagFld {
 public:
  static void getMagFld(const double, const double, const double, double&,
                        double&, double&, VKalVrtControlBase*);
  static double getMagFld(const double xyz[3],
                          const VKalVrtControlBase* FitControl);
  /* Converstion for MeV and mm and Tesla*/
  inline static double getCnvCst() { return vkalMagCnvCst; }
};

}  // namespace Trk
#endif
