/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TGCDIGITJITTERDATA_H
#define TGCDIGITJITTERDATA_H

#include <GeoPrimitives/GeoPrimitives.h>
///
#include <GaudiKernel/SystemOfUnits.h>
#include <AthenaBaseComps/AthMessaging.h>
#include <AthenaKernel/BaseInfo.h>
#include <AthenaKernel/CLASS_DEF.h>
#include <AthenaKernel/CondCont.h>

#include <vector>

/**
 * 
*/
namespace CLHEP {
    class HepRandomEngine;
}

class TgcDigitJitterData : public AthMessaging {
  public:
     TgcDigitJitterData();

     ~TgcDigitJitterData() = default;
     
     double drawJitter(const Amg::Vector3D& localDir,
                       CLHEP::HepRandomEngine* rndmEngine) const;
      
     void cacheAngleInterval(const double minAngle, std::vector<double>&& timeProbs);
     
     StatusCode initialize();

  private:
    struct JitterBin {
       double minAngle{0.};
       double maxAngle{0.};
       bool operator<(const JitterBin& other) const{
         return maxAngle < other.minAngle;
       }
       JitterBin(const double angle, std::vector<double>&& probs):
          minAngle{angle}, maxAngle{angle}, timeProbs{std::move(probs)}{}
       std::vector<double> timeProbs{};
    };
    std::vector<JitterBin> m_bins{};

    double m_timeInterval{-1.};
    double m_angleInterval{-1.};

};
CLASS_DEF( TgcDigitJitterData , 244147337 , 1 );
CONDCONT_DEF( TgcDigitJitterData , 132172807 );
#endif 