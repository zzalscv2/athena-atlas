/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_GNN_DATA_STORAGE_H
#define TRIGINDETPATTRECOTOOLS_GNN_DATA_STORAGE_H

#include<vector>
#include<map>
#include<algorithm>

#define MAX_SEG_PER_NODE 30 //was 30
#define N_SEG_CONNS  6 //was 6
#define N_PHI_BINS 60

#include "TrigInDetEvent/TrigSiSpacePointBase.h"

class TrigFTF_GNN_Geometry;

class TrigFTF_GNN_Node {
public:

  struct CompareByPhi {

    bool operator()(const TrigFTF_GNN_Node* n1, const TrigFTF_GNN_Node* n2) {
      return n1->m_sp.phi() < n2->m_sp.phi();
    }

  };

  TrigFTF_GNN_Node(const TrigSiSpacePointBase&, float, float);
  ~TrigFTF_GNN_Node();
  

 inline void addIn(int i) {
    if(m_in.size()<MAX_SEG_PER_NODE) {
      m_in.push_back(i);
    }
  }

  inline void addOut(int i) {
    if(m_out.size()<MAX_SEG_PER_NODE) {
      m_out.push_back(i);
    }
  }
  
  inline bool isConnector() const {
    if(m_in.empty() || m_out.empty()) return false;
    return true;
  }

  inline bool isFull() const {
    if(m_in.size()==MAX_SEG_PER_NODE && m_out.size()==MAX_SEG_PER_NODE) return true;
    else return false;
  }

  const TrigSiSpacePointBase& m_sp;
  
  std::vector<unsigned int> m_in;//indices of the edges in the edge storage
  std::vector<unsigned int> m_out;
  float m_minCutOnTau, m_maxCutOnTau;

};

class TrigFTF_GNN_EtaBin {
public:
  TrigFTF_GNN_EtaBin();
  ~TrigFTF_GNN_EtaBin();

  void sortByPhi();

  bool empty() const {
    return m_vn.empty();
  }
  
  void generatePhiIndexing(float);
  
  std::vector<TrigFTF_GNN_Node*> m_vn;
  std::vector<std::pair<float, unsigned int> > m_vPhiNodes;

};

class TrigFTF_GNN_DataStorage {
public:
  TrigFTF_GNN_DataStorage(const TrigFTF_GNN_Geometry&, float);
  ~TrigFTF_GNN_DataStorage();

  int addSpacePoint(const TrigSiSpacePointBase&, bool);
  unsigned int numberOfNodes() const;
  void getConnectingNodes(std::vector<const TrigFTF_GNN_Node*>&);
  void sortByPhi();
  void generatePhiIndexing(float);


  const TrigFTF_GNN_EtaBin& getEtaBin(int idx) const {
    if(idx >= static_cast<int>(m_etaBins.size())) idx = idx-1;
    return m_etaBins.at(idx);
  }

protected:

  const TrigFTF_GNN_Geometry& m_geo;
  float m_phiBinWidth;

  std::vector<TrigFTF_GNN_EtaBin> m_etaBins; 

};

class TrigFTF_GNN_Edge {
public:

  struct CompareLevel {
  public:
    bool operator()(const TrigFTF_GNN_Edge* pS1, const TrigFTF_GNN_Edge* pS2) {
      return pS1->m_level > pS2->m_level;
    }
  };

  TrigFTF_GNN_Edge() : m_n1(nullptr), m_n2(nullptr), m_level(-1), m_next(-1), m_nNei(0) {};

  inline void initialize(TrigFTF_GNN_Node* n1, TrigFTF_GNN_Node* n2) {
    m_n1 = n1; 
    m_n2 = n2;
    m_level = 1;
    m_next = 1;
    m_nNei = 0;
  }


  TrigFTF_GNN_Node* m_n1;
  TrigFTF_GNN_Node* m_n2;
  
  char m_level, m_next;

  unsigned char m_nNei;
  float m_p[4];
  
  unsigned int m_vNei[N_SEG_CONNS];//global indices of the connected edges

};

#endif
