/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCELLCORRECTION_CALOCELLRANDOMIZER_H
#define CALOCELLCORRECTION_CALOCELLRANDOMIZER_H

#include "CaloUtils/CaloCellCorrection.h"

#include "CaloInterface/ICalorimeterNoiseTool.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

//class CaloCell;
//class ICalorimeterNoiseTool;

class CaloCellRandomizer : public CaloCellCorrection

{

public:

  CaloCellRandomizer(const std::string& type,
                     const std::string& name,
                     const IInterface* parent);

  virtual ~CaloCellRandomizer() {};

  virtual StatusCode initialize() override;
  
  virtual void MakeCorrection (CaloCell* theCell) override;


private:


  ToolHandle<ICalorimeterNoiseTool> m_noiseTool;

  int m_corrSampleMin;
  int m_corrSampleMax;
  float m_fractionSigma;
  bool m_GaussRand;
  bool m_GaussRandGroupedSamples;
  bool m_GaussRandShifted;
  bool m_GaussRandShifted_Custom;
  float m_shift_EMB;
  float m_shift_EMEC;
  float m_shift_HEC;
  float m_shift_TileBar;
  float m_shift_TileGap;
  float m_shift_TileExt;
  float m_shift_FCAL;

  ServiceHandle<IAthRNGSvc> m_athRNGSvc;
  std::string m_randomStream;
  ATHRNG::RNGWrapper* m_randomEngine;
};

#endif
