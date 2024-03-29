/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_GNN_GEOMETRY_H
#define TRIGINDETPATTRECOTOOLS_GNN_GEOMETRY_H

#include<vector>
#include<map>
#include<algorithm>

#include "FasTrackConnector.h"

class TrigInDetSiLayer;

class TrigFTF_GNN_Layer {
public:
  TrigFTF_GNN_Layer(const TrigInDetSiLayer&, float, int);
  ~TrigFTF_GNN_Layer();

  int getEtaBin(float, float) const;

  float getMinBinRadius(int) const;
  float getMaxBinRadius(int) const;

  int num_bins() const {return m_bins.size();} 

  bool verifyBin(const TrigFTF_GNN_Layer*, int, int, float, float) const;

  const TrigInDetSiLayer& m_layer;
  std::vector<int> m_bins;//eta-bin indices
  std::vector<float> m_minRadius;
  std::vector<float> m_maxRadius;
  std::vector<float> m_minBinCoord;
  std::vector<float> m_maxBinCoord;

  float m_minEta, m_maxEta;

protected:

  float m_etaBinWidth, m_phiBinWidth;

  float m_r1, m_z1, m_r2, m_z2;
  int m_nBins;
  float m_etaBin;
  

};

class TrigFTF_GNN_Geometry {
public:
  TrigFTF_GNN_Geometry(const std::vector<TrigInDetSiLayer>&, const FASTRACK_CONNECTOR*);
  ~TrigFTF_GNN_Geometry();
  
  const TrigFTF_GNN_Layer* getTrigFTF_GNN_LayerByKey(unsigned int) const;
  const TrigFTF_GNN_Layer* getTrigFTF_GNN_LayerByIndex(int) const;

  int num_bins() const {return m_nEtaBins;}

protected:

  const TrigFTF_GNN_Layer* addNewLayer(const TrigInDetSiLayer&, int);

  float m_etaBinWidth;

  std::map<unsigned int, TrigFTF_GNN_Layer*> m_layMap;
  std::vector<TrigFTF_GNN_Layer*> m_layArray;

  int m_nEtaBins;

};


#endif
