/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDETTRT_HTCALCULATOR
#define INDETTRT_HTCALCULATOR

///////////////////////////////////////////////////////////////////
// HTcalculater.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

/****************************************************************************************\

  This class is instantiated in TRTHTCondAlg and put on CondStore

  Original creator: Simon Heisterkamp (simon.heisterkamp@cern.ch)
  Author: Troels Petersen (petersen@nbi.dk)
          Peter Hansen (phansen@nbi.dk)

\****************************************************************************************/
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include "AthenaPoolUtilities/CondAttrListVec.h"
#include "GaudiKernel/StatusCode.h"
#include "TRT_ConditionsData/StorePIDinfo.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"

class HTcalculator {
 public:
  HTcalculator() = default;
  virtual ~HTcalculator() = default;

  void checkInitialization() {
    // no-op
  }
  static float Limit(float prob);
  // set constants to hard coded defaults
  void setDefaultCalibrationConstants();

  StatusCode ReadVectorDB(const CondAttrListVec* channel_values);

  float getProbHT(float pTrk, Trk::ParticleHypothesis hypothesis, int TrtPart,
                  int GasType, int StrawLayer, float ZR, float rTrkAnode,
                  float Occupancy, bool hasTrackPars) const;

  float pHTvsPGOG(int TrtPart, int GasType, float p, float mass,
                  float occ) const;

 private:
  static constexpr int N_GAS = 3;
  static constexpr int N_DET = 3;
  static constexpr int N_PAR2 = 10;
  StorePIDinfo
      m_par_pHTvsPGOG_new[N_GAS][N_DET];  // New approach (useOccupancy = true)

  // Store in a compact way all the corrections
  StorePIDinfo m_CpHT_B_Zee_SL_new[N_GAS][N_DET];
  StorePIDinfo m_CpHT_B_Zmm_SL_new[N_GAS][N_DET];

  StorePIDinfo m_CpHT_B_Zee_ZR_new[N_GAS][N_DET];
  StorePIDinfo m_CpHT_B_Zmm_ZR_new[N_GAS][N_DET];

  StorePIDinfo m_CpHT_B_Zee_TW_new[N_GAS][N_DET];
  StorePIDinfo m_CpHT_B_Zmm_TW_new[N_GAS][N_DET];

  StorePIDinfo m_CpHT_B_Zee_OR_new[N_GAS][N_DET];
  StorePIDinfo m_CpHT_B_Zmm_OR_new[N_GAS][N_DET];

  static constexpr int SIZE_OF_HEADER = sizeof(float) * 4;
  static constexpr int SIZE_OF_BLOB = sizeof(float) * ((N_PAR2 * N_DET));
};

CLASS_DEF(HTcalculator, 241669896, 1)
CONDCONT_DEF(HTcalculator, 124823640);
#endif
