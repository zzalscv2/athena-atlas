/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiNoise_bt
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for noises
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 5/17/2006 T.Koffas
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiNoise_bt_H
#define SiNoise_bt_H

#include "TrkParameters/TrackParameters.h"


namespace InDet{

  class SiNoise_bt{
      
    public:
      const int&     model         () const {return m_model         ;}
      const double&  covarianceAzim() const {return m_covarianceAzim;}
      const double&  covariancePola() const {return m_covariancePola;}
      const double&  covarianceIMom() const {return m_covarianceIMom;}
      const double&  correctionIMom() const {return m_correctionIMom;}
      void reset();
      void production(int direction,int model,const Trk::TrackParameters& tp);

    private:
      int    m_model{}         ;
      double m_covarianceAzim{};
      double m_covariancePola{};
      double m_covarianceIMom{};
      double m_correctionIMom{1.0};

    };

} // end of name space

#endif // SiNoise_bt


