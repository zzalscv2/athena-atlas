/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*     Common VKalVrtCore parameters                     	*/
/*								*/
/*  vkalInternalStepLimit - limit step in VtCFit to avoid 	*/
/*                          linearity loss in strongly    	*/
/*                          changeable magnetic field     	*/
/*                     Without re-extrapolation fitter can make */
/*			2 steps, so total shift is doubled      */
/*								*/
/*  vkalNTrkM    -  Maximal amount of tracks from old version 	*/
/*		    of fitter. Should disappear finally    	*/
/*								*/
/*  vkalMagCnvCst - conversion constant for field in Tesla and	*/
/*		    momentum in MeV and distance in mm		*/
/*--------------------------------------------------------------*/
#ifndef _TrkVKalVrtCore_CommonPars_H
#define _TrkVKalVrtCore_CommonPars_H

#define vkalNTrkM  200
#define vkalMagCnvCst 0.29979246
#define vkalInternalStepLimit   20.
#define vkalAllowedPtChange      3.
#define vkalShiftToTrigExtrapolation      20.
#define vkalMaxNMassCnst  8
//---Fortran style 2D array - first index changes first
#define ARR2D_FS(name,N,i,j) (name)[(j)*(N) + (i)]

#endif
