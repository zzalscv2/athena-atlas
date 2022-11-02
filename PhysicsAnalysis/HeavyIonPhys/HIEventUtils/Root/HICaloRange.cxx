/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "HIEventUtils/HICaloRange.h"
#include "CaloGeoHelpers/CaloSampling.h"

HICaloRange::HICaloRange()
{
  initializeRange();
}

const HICaloRange& HICaloRange::getRange()
{
  static const HICaloRange range = HICaloRange();
  return range;
}

bool HICaloRange::LayerInRange(float eta, int layer)
{
  const float eta_abs=std::abs(eta);
  return ( (eta_abs > m_range.at(layer).first) && (eta_abs < m_range.at(layer).second) );
}

void HICaloRange::initializeRange()
{
  m_range = {
    {CaloSampling::PreSamplerB, range_t(0,1.6)}, //0
    {CaloSampling::EMB1, range_t(0,1.5)}, //1
    {CaloSampling::EMB2, range_t(0,1.5)}, //2
    {CaloSampling::EMB3, range_t(0,1.4)}, //3

    {CaloSampling::PreSamplerE, range_t(1.5,1.8)}, //4
    {CaloSampling::EME1, range_t(1.3,2.5)}, //5
    {CaloSampling::EME2, range_t(1.3,3.2)}, //6
    {CaloSampling::EME3, range_t(1.5,3.2)}, //7

    {CaloSampling::HEC0, range_t(1.5,3.3)}, //8
    {CaloSampling::HEC1, range_t(1.5,3.1)}, //9
    {CaloSampling::HEC2, range_t(1.6,3.1)}, //10
    {CaloSampling::HEC3, range_t(1.7,3.3)}, //11

    {CaloSampling::TileBar0, range_t(0,1)}, //12
    {CaloSampling::TileBar1, range_t(0,0.9)}, //13
    {CaloSampling::TileBar2, range_t(0,0.7)}, //14

    {CaloSampling::TileGap1, range_t(0.9,1)}, //15
    {CaloSampling::TileGap2, range_t(0.8,0.9)}, //16
    {CaloSampling::TileGap3, range_t(1,1.6)}, //17

    {CaloSampling::TileExt0, range_t(1.1,1.6)}, //18
    {CaloSampling::TileExt1, range_t(1,1.5)}, //19
    {CaloSampling::TileExt2, range_t(0.9,1.3)}, //20

    {CaloSampling::FCAL0, range_t(3,5)}, //21
    {CaloSampling::FCAL1, range_t(3.1,5)}, //22
    {CaloSampling::FCAL2, range_t(3.2,5)}, //23
  };
}
