//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARPHYSWAVETOOL_H
#define LARPHYSWAVETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "LArRawConditions/LArWFParams.h"
#include "LArRawConditions/LArCaliWave.h"
#include "LArRawConditions/LArPhysWave.h"
#include "LArRawConditions/LArWaveHelper.h"

static const InterfaceID IID_LArPhysWaveTool("LArPhysWaveTool", 1 , 0); 

class LArPhysWaveTool : public AthAlgTool
{
 public: 

  // Retrieve interface ID
  static const InterfaceID& interfaceID() { return IID_LArPhysWaveTool; }

  LArPhysWaveTool(const std::string& type, const std::string& name,
	 	  const IInterface* parent ) ;

  virtual ~LArPhysWaveTool(); 

  virtual StatusCode initialize(){return StatusCode::SUCCESS;}
  virtual StatusCode finalize(){return StatusCode::SUCCESS;}

  StatusCode makeLArPhysWave(const LArWFParams &, const LArCaliWave &,
  			     int region, int layer,
			     LArPhysWave & predLArPhysWave,
			     float & MphysMcali) const;


 private:

  bool m_injPointCorr , m_normalizeCali ,
       m_timeOriginShift , m_subtractBaseline;
  std::vector<bool> m_injPointCorrLayer, m_injPointUseTauR ;
  
  LArWave exp2Tri(const LArWave &,const unsigned N, const double dt, const LArWFParams& params) const ;
  LArWave physPred(LArCaliWave &);
  static double caliPhysCorr ( double t, const LArWFParams& params) ;
  static LArWave caliPhysCorr(const unsigned N, const double dt, const LArWFParams& params) ;
  LArWave injResp (const LArWave& w,unsigned N, double dt, const LArWFParams& params) const;
  LArWave stepResp (const LArCaliWave& gCali, const LArWFParams& params) const;
  LArWave step2Tri (const LArWave& w, unsigned N, double dt, const LArWFParams& params) const;
  static double stepPhysCorr ( double t, const double dT) ;
  static LArWave stepPhysCorr(unsigned N, double dt, const double dT) ;
  static LArWave stepCorr(unsigned N, double dt, const LArWFParams& params) ;
  static double stepCorr ( double t, const LArWFParams& params) ;
  static LArWave injCorr(unsigned N, double dt, const LArWFParams& params) ;
  static double injCorr ( double t, const LArWFParams& params) ;
  

};

#endif
