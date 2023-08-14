/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_TRIG_COMBINATORIAL_SETTINGS_H
#define TRIGINDETPATTRECOTOOLS_TRIG_COMBINATORIAL_SETTINGS_H

class IRoiDescriptor;
#include <vector>
#include "TrigInDetPattRecoEvent/TrigInDetSiLayer.h"
#include "TrigInDetPattRecoTools/TrigSeedML_LUT.h"
#include "TrigInDetPattRecoTools/FasTrackConnector.h"
#include "TrigInDetPattRecoTools/GNN_Geometry.h"

typedef struct TrigCombinatorialSettings {
public:
  TrigCombinatorialSettings() {//provides some defaults

    //default silicon geometry 

    m_maxBarrelPix = 3;
    m_minEndcapPix = 7;
    m_maxEndcapPix = 10;
    m_maxSiliconLayer = 19;

    m_doubletD0Max         = 5.0;

    m_tripletD0Max      = 4.0; 
    m_tripletD0_PPS_Max = 1.7; 
    m_tripletPtMin      = 2500.0;//was 1000.0
    m_tripletDoPSS      = false; // Allow Pixel SCT SCT seeds?
    m_tripletDoPPS      = true; // Allow Pixel Pixel SCT seeds?
    m_tripletDoConfirm  = false; // Use another Pixel spacepoint to confirm a triplet
    m_curv_delta        = 0.001; //max difference in track curvature for triplet confirmation
    m_doubletFilterRZ   = true;
    m_tripletDtCut      = 3.0;//in sigmas of mult.scattering for m_tripletPtMin track at eta=0
    m_magFieldZ = 2.0;//switch to configured value
    m_nMaxPhiSlice = 53;
    m_maxTripletBufferLength = 3;

    m_zvError = 10.0;
    m_zvErrorEndcap = m_zvError;
    m_LRTmode=false;
    m_layerGeometry.clear();
    m_useTrigSeedML = 0;
    m_useEtaBinning = false;
    m_maxEC_len = 1.5;
    m_vLUT.clear();
  }

  int m_maxBarrelPix, m_minEndcapPix, m_maxEndcapPix, m_maxSiliconLayer;

  float m_doubletD0Max;
  float m_doublet_dR_Max;
  float m_doublet_dR_Max_Confirm;
  float m_magFieldZ;
  float m_tripletD0Max;
  float m_tripletD0_PPS_Max;
  float m_tripletPtMin;
  float m_seedRadBinWidth;
  bool  m_tripletDoPSS;
  bool  m_tripletDoPPS;
  bool  m_tripletDoConfirm;
  float m_curv_delta;
  bool  m_doubletFilterRZ;
  float m_tripletDtCut;
  int m_nMaxPhiSlice;
  unsigned int m_maxTripletBufferLength;

  float m_zvError;
  float m_zvErrorEndcap;
  bool m_LRTmode;
  bool m_useEtaBinning;

  const FASTRACK_CONNECTOR* m_conn;
  const TrigFTF_GNN_Geometry*       m_geo;

  std::vector<TrigInDetSiLayer> m_layerGeometry;

  int m_useTrigSeedML;
  std::vector<TrigSeedML_LUT> m_vLUT;
  float m_maxEC_len;

} TRIG_COMBINATORIAL_SETTINGS;



#endif
