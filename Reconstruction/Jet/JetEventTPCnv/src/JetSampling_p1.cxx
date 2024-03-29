///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// JetSampling_p1.cxx 
// Implementation file for class JetSampling_p1
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// JetEventTPCnv includes
#include "JetEventTPCnv/JetSampling_p1.h"

/////////////////////////////////////////////////////////////////// 
/// Public methods: 
/////////////////////////////////////////////////////////////////// 

JetSampling_p1::JetSampling_p1()
  : m_ejsPreSamplerB(0)
  , m_ejsPreSamplerE(0)
  , m_ejsEMB1(0)
  , m_ejsEME1(0)  
  , m_ejsEMB2(0)
  , m_ejsEME2(0)
  , m_ejsEMB3(0)
  , m_ejsEME3(0)
  , m_ejsTileBar0(0)
  , m_ejsTileExt0(0)
  , m_ejsTileBar1(0)
  , m_ejsTileExt1(0)
  , m_ejsTileBar2(0)
  , m_ejsTileExt2(0)  
  , m_ejsHEC0(0)
  , m_ejsHEC1(0)
  , m_ejsHEC2(0)
  , m_ejsHEC3(0)
  , m_ejsTileGap1(0)
  , m_ejsTileGap2(0)
  , m_ejsTileGap3(0)
  , m_ejsFCAL0(0)
  , m_ejsFCAL1(0)
  , m_ejsFCAL2(0)
  , m_tot(0)
  , m_ctot(0)
  , m_ehad(0)
  , m_eem(0) 
  , m_eCryo(0)
  , m_eGap(0)
  , m_eScint(0)
  , m_eNull(0)
{
  m_edEMB0Cell.fill(0);
  m_edEMB1Cell.fill(0);
  m_edEMB2Cell.fill(0);
  m_edEME0Cell.fill(0);
  m_edEME1Cell.fill(0);
  m_edEME2Cell.fill(0);
  m_edTile1Cell.fill(0);
  m_edTile2Cell.fill(0);
  m_edHec1Cell.fill(0);
  m_edHec2Cell.fill(0);
  m_edFCal1Cell.fill(0);
  m_edFCal2Cell.fill(0);
  m_ePreSamBCell.fill(0);
  m_ePreSamECell.fill(0);
  m_eEMB1Cell.fill(0);
  m_eEMB2Cell.fill(0);
  m_eEMB3Cell.fill(0);
  m_eEME1Cell.fill(0);
  m_eEME2Cell.fill(0);
  m_eEME3Cell.fill(0);
  m_eTileBar0Cell.fill(0);
  m_eTileBar1Cell.fill(0);
  m_eTileBar2Cell.fill(0);
  m_eTileExt0Cell.fill(0);
  m_eTileExt1Cell.fill(0);
  m_eTileExt2Cell.fill(0);
  m_eHec0Cell.fill(0);
  m_eHec1Cell.fill(0);
  m_eHec2Cell.fill(0);
  m_eHec3Cell.fill(0);
  m_eFCal0Cell.fill(0);
  m_eFCal1Cell.fill(0);
  m_eFCal2Cell.fill(0);
  m_erad.fill(0);
}

