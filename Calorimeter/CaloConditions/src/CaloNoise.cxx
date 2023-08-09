/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloConditions/CaloNoise.h"
#include "boost/multi_array.hpp"
#include "TMath.h"
#include <cmath>

CaloNoise::CaloNoise(const size_t nLArCells,
                     const size_t nLArGains,
                     const size_t nTileCells,
                     const size_t nTileGains,
                     const CaloCell_Base_ID* caloCellId,
                     const NOISETYPE noisetype)
  : m_caloCellId(caloCellId)
  , m_noiseType(noisetype)
{

  boost::multi_array_types::extent_gen lar_extent_gen;
  m_larNoise.resize(lar_extent_gen[nLArGains][nLArCells]);
  boost::multi_array_types::extent_gen tile_extent_gen;
  m_tileNoise.resize(tile_extent_gen[nTileGains][nTileCells]);
  
  IdentifierHash h1,h2;
  m_caloCellId->calo_cell_hash_range(CaloCell_ID::TILE, h1,h2);
  m_tileHashOffset=h1;
}

void CaloNoise::setTileBlob(const CaloCondBlobFlt* flt, const float lumi) {
  m_tileBlob=flt;
  m_lumi=lumi;
}

CaloNoise::~CaloNoise() {
  if ( m_tileBlob) delete m_tileBlob;
}



float CaloNoise::calcSig(const IdentifierHash subHash, const int dbGain, const float e) const {

  const double sigma1 = m_tileBlob->getData(subHash,dbGain,2);
  const double sigma2 = m_tileBlob->getData(subHash,dbGain,3);
  const double ratio  = m_tileBlob->getData(subHash,dbGain,4);
 

  if((sigma1 == 0. && sigma2 == 0.) || e == 0.) return 0.;
  if(sigma1 == 0.) return e/sigma2;
  if((ratio  == 0.) || sigma2 == 0.) return e/sigma1;
  const double x1 = e/sigma1;
  const double x2 = e/sigma2;

  constexpr std::array<float,2> valid_range{0.9,7.5};
  const float wide_gauss_sigma = std::min(fabs(x1),fabs(x2));
  if(wide_gauss_sigma > valid_range[1]) return wide_gauss_sigma;

  const float narrow_gauss_sigma= std::max(fabs(x1),fabs(x2));
  if(narrow_gauss_sigma < valid_range[0]) return narrow_gauss_sigma;
  

  const double y1= TMath::Erf(M_SQRT1_2*x1);
  const double y2= TMath::Erf(M_SQRT1_2*x2);

  const double z = ( y1*sigma1 + ratio*y2*sigma2 )/( sigma1 + ratio*sigma2);

  //return the C.L. probability (with sign!)
  //  return z;
 
  // if instead you want to return the sigma-equivalent C.L.
  // (with sign!) use the following line
  return M_SQRT2*TMath::ErfInverse(z);
}


float CaloNoise::getTileEffSigma(const IdentifierHash subHash, const int gain, const float e) const {
  // Tell clang to optimize assuming that FP exceptions can trap.
  // Otherwise, it can vectorize the division, which can lead to
  // spurious division-by-zero traps from unused vector lanes.
#ifdef __clang__
# pragma float_control(except, on)
#endif

  const unsigned int dbGain = CaloCondUtils::getDbCaloGain(gain);
  if (!m_tileBlob) {
    //No data (pilup-noise only): return cached noise
    return m_tileNoise[dbGain][subHash];
  }

  const float sigma=calcSig(subHash,dbGain,e);
  const float a= (sigma != 0.) ? fabs(e/sigma) : 0.0;

  if (m_noiseType==CaloNoise::ELEC) {
    return a;
  }

  //Case: Total Noise
  const float b= m_tileBlob->getData(subHash,dbGain,1);
  const int objver = m_tileBlob->getObjVersion();
  float x=0;
  if(objver==1){
    //=== Total noise parameterized as
    //=== Sigma**2 = a**2 + b**2 * Lumi
    x = std::sqrt( a*a + b*b*m_lumi );
  }
  else if (objver==2) {
    //== parameterization for pedestal = a + b*Lumi
    x = a+b*m_lumi;
  }
  else{
    throw CaloCond::VersionConflict("CaloNoise::get2dEffSigma ",objver);
  }
  return x;
}
