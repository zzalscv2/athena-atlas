/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_TRIGTRACKSEEDGENERATOR_H
#define TRIGINDETPATTRECOTOOLS_TRIGTRACKSEEDGENERATOR_H

#include<vector>
#include<algorithm>

#include "TrigInDetPattRecoTools/TrigCombinatorialSettings.h"
#include "TrigInDetEvent/TrigSiSpacePointBase.h"

class TrigInDetTriplet;

typedef struct IndexedSP {
public :
IndexedSP() : m_pSP(0), m_idx(-1) {};
IndexedSP(const TrigSiSpacePointBase* p, int idx) : m_pSP(p), m_idx(idx) {};
IndexedSP(const IndexedSP& isp) : m_pSP(isp.m_pSP), m_idx(isp.m_idx) {};

  void set(const TrigSiSpacePointBase* p, int idx) {
    m_pSP = p;
    m_idx = idx;
  }

  const TrigSiSpacePointBase* m_pSP;
  int m_idx;

} INDEXED_SP;

typedef struct LPhiSector {

public:
  
  struct compareZ {
    bool operator()(const INDEXED_SP* p1, const INDEXED_SP* p2) {
      return p1->m_pSP->z()<p2->m_pSP->z();
    }
  };

  struct compareR {
    bool operator()(const INDEXED_SP* p1, const INDEXED_SP* p2) {
      return p1->m_pSP->r()<p2->m_pSP->r();
    }
  };

  struct compareRless {
    bool operator()(const INDEXED_SP* p1, const INDEXED_SP* p2) {
      return p1->m_pSP->r()>p2->m_pSP->r();
    }
  };

  struct greaterThanZ {
    bool operator()(float z, const INDEXED_SP* const& p) const {
      return z < p->m_pSP->z();
    }
  };

  struct smallerThanZ {
    bool operator()(const INDEXED_SP* const& p, float z) const {
      return p->m_pSP->z() < z;
    }
  };

  struct greaterThanR {
    bool operator()(float r, const INDEXED_SP* const& p) const {
      return r < p->m_pSP->r();
    }
  };

  struct greaterThanR_i {
    bool operator()(const INDEXED_SP* const& p, float r) const {
      return r < p->m_pSP->r();
    }
  };

  struct smallerThanR {
    bool operator()(const INDEXED_SP* const& p, float r) const {
      return p->m_pSP->r() < r;
    }
  };

  struct smallerThanR_i {
    bool operator()(float r, const INDEXED_SP* const& p) const {
      return p->m_pSP->r() < r;
    }
  };

  //LPhiSector() : m_nSP(0) {m_phiSlices.clear();}
LPhiSector(int nPhiSlices) : m_nSP(0) {
  std::vector<const INDEXED_SP*> d;
  m_phiSlices.resize(nPhiSlices, d);
  m_phiThreeSlices.resize(nPhiSlices, d);
  if(nPhiSlices == 1) {//special case
    m_threeIndices[1].push_back(0);
    m_threeIndices[0].push_back(-1);
    m_threeIndices[2].push_back(-1);
  }
  if(nPhiSlices == 2) {//special case
    m_threeIndices[1].push_back(0);
    m_threeIndices[0].push_back(-1);
    m_threeIndices[2].push_back(1);
    m_threeIndices[1].push_back(1);
    m_threeIndices[0].push_back(0);
    m_threeIndices[2].push_back(-1);
  }
  if(nPhiSlices > 2) {
    for(int phiIdx=0;phiIdx<nPhiSlices;phiIdx++) {
      m_threeIndices[1].push_back(phiIdx);
      if(phiIdx>0) m_threeIndices[0].push_back(phiIdx-1);
      else m_threeIndices[0].push_back(nPhiSlices-1);
      if(phiIdx<nPhiSlices-1) m_threeIndices[2].push_back(phiIdx+1);
      else m_threeIndices[2].push_back(0);
    }
  }
}

LPhiSector(const LPhiSector& ps) : m_nSP(ps.m_nSP), m_phiSlices(ps.m_phiSlices), m_phiThreeSlices(ps.m_phiThreeSlices) {
  for(int i=0;i<3;i++) {
    m_threeIndices[i] = ps.m_threeIndices[i];
  }
}

  const LPhiSector& operator = (const LPhiSector& ps) {
    m_nSP = ps.m_nSP;
    m_phiSlices = ps.m_phiSlices;
    m_phiThreeSlices = ps.m_phiThreeSlices;
    for(int i=0;i<3;i++) {
      m_threeIndices[i] = ps.m_threeIndices[i];
    }
    return *this;
  }

  void reset() {
    for(std::vector<std::vector<const INDEXED_SP*> >::iterator it = m_phiSlices.begin();it!=m_phiSlices.end();++it) {
      (*it).clear();
    }
    for(std::vector<std::vector<const INDEXED_SP*> >::iterator it = m_phiThreeSlices.begin();it!=m_phiThreeSlices.end();++it) {
      (*it).clear();
    }
    m_nSP = 0;
  }

  void addSpacePoint(int phiIndex, const INDEXED_SP* p) {
    m_nSP++;
    m_phiSlices[phiIndex].push_back(p);
    for(int i=0;i<3;i++) {
      if(m_threeIndices[i][phiIndex]==-1) continue;//to account for special cases nPhiSlices=1,2
      m_phiThreeSlices[m_threeIndices[i][phiIndex]].push_back(p);
    }
  }

  void sortSpacePoints(bool isBarrel) {
    for(std::vector<std::vector<const INDEXED_SP*> >::iterator it=m_phiSlices.begin();it!=m_phiSlices.end();++it) {
      if((*it).empty()) continue;
      if(isBarrel) std::sort(it->begin(), it->end(), compareZ());
      else std::sort(it->begin(), it->end(), compareR());
    }
    for(std::vector<std::vector<const INDEXED_SP*> >::iterator it=m_phiThreeSlices.begin();it!=m_phiThreeSlices.end();++it) {
      if((*it).empty()) continue;
      if(isBarrel) std::sort(it->begin(), it->end(), compareZ());
      else std::sort(it->begin(), it->end(), compareR());
    }
  }

  void sortSpacePoints(bool isBarrel, bool isPositive) {
    for(std::vector<std::vector<const INDEXED_SP*> >::iterator it=m_phiSlices.begin();it!=m_phiSlices.end();++it) {
      if((*it).empty()) continue;
      if(isBarrel) std::sort(it->begin(), it->end(), compareZ());
      else {
	if(isPositive) std::sort(it->begin(), it->end(), compareRless());
	else std::sort(it->begin(), it->end(), compareR());
      }
    }
    for(std::vector<std::vector<const INDEXED_SP*> >::iterator it=m_phiThreeSlices.begin();it!=m_phiThreeSlices.end();++it) {
      if((*it).empty()) continue;
      if(isBarrel) std::sort(it->begin(), it->end(), compareZ());
      else {
	if(isPositive) std::sort(it->begin(), it->end(), compareRless());
	else std::sort(it->begin(), it->end(), compareR());
      }
    }
  }

  int size() const { return m_nSP; }
  
  int m_nSP;
  std::vector<std::vector<const INDEXED_SP*> > m_phiSlices;
  std::vector<std::vector<const INDEXED_SP*> > m_phiThreeSlices;
  std::vector<int> m_threeIndices[3];

private:
  LPhiSector() {};

} L_PHI_SECTOR;

typedef struct LPhi_Storage {

public:

  LPhi_Storage(int nPhiSectors, int nLayers) {
    m_layers.reserve(nLayers);
    for(int i = 0;i<nLayers;i++) m_layers.push_back(L_PHI_SECTOR(nPhiSectors));
  }

  void addSpacePoint(int phiIdx, int layerId, const INDEXED_SP* p) {
    m_layers[layerId].addSpacePoint(phiIdx, p);
  }

  void reset() {
    for(std::vector<L_PHI_SECTOR>::iterator it=m_layers.begin();it!=m_layers.end();++it) {
      if((*it).m_nSP==0) continue;
      (*it).reset();
    }
  }
   
  void sortSpacePoints(const std::vector<TrigInDetSiLayer>& layerGeometry) {
    int layerId = 0;
    for(std::vector<L_PHI_SECTOR>::iterator it=m_layers.begin();it!=m_layers.end();++it,layerId++) {
      if((*it).m_nSP==0) continue;
      (*it).sortSpacePoints(layerGeometry[layerId].m_type==0);
    }
  }

  void sortSpacePoints2(const std::vector<TrigInDetSiLayer>& layerGeometry) {
    int layerId = 0;
    for(std::vector<L_PHI_SECTOR>::iterator it=m_layers.begin();it!=m_layers.end();++it,layerId++) {
      if((*it).m_nSP==0) continue;
      (*it).sortSpacePoints(layerGeometry[layerId].m_type==0, layerGeometry[layerId].m_refCoord > 0);
    }
  }

  std::vector<L_PHI_SECTOR> m_layers;

} L_PHI_STORAGE;

typedef struct InternalSoA {

public:

InternalSoA() : m_spi(0), m_spo(0), m_r(0), m_u(0), m_v(0), m_t(0), m_ti(0), m_to(0), m_tCov(0), m_sorted_sp(0),
    m_sorted_sp_type(0),
    m_sorted_sp_t(0) {}

  ~InternalSoA() {
  }

  void clear() {
    delete[] m_spi;
    delete[] m_spo;
    delete[] m_r;
    delete[] m_u;
    delete[] m_v;
    delete[] m_t;
    delete[] m_ti;
    delete[] m_to;
    delete[] m_tCov;
    delete[] m_sorted_sp;
    delete[] m_sorted_sp_type;
    delete[] m_sorted_sp_t;
    m_spi = 0;
    m_spo = 0;
    m_r = 0;
    m_u = 0;
    m_v = 0;
    m_t = 0;
    m_ti = 0;
    m_to = 0;
    m_tCov = 0;
    m_sorted_sp = 0;
    m_sorted_sp_type = 0;
    m_sorted_sp_t = 0;
  }
  
  void resize(const int spSize) {

    m_spi = new const TrigSiSpacePointBase*[spSize];
    m_spo = new const TrigSiSpacePointBase*[spSize];
    m_r = new double[spSize];
    m_u = new double[spSize];
    m_v = new double[spSize];
    m_t = new double[spSize];
    m_ti = new double[spSize];
    m_to = new double[spSize];
    m_tCov = new double[spSize];
    m_sorted_sp = new const TrigSiSpacePointBase*[spSize];
    m_sorted_sp_type = new int[spSize];
    m_sorted_sp_t = new double[spSize];
  }

  const TrigSiSpacePointBase** m_spi;
  const TrigSiSpacePointBase** m_spo;
  double* m_r;
  double* m_u;
  double* m_v;
  double* m_t;
  double* m_ti;
  double* m_to;
  double* m_tCov;
  const TrigSiSpacePointBase** m_sorted_sp;
  int* m_sorted_sp_type;
  double* m_sorted_sp_t;

} INTERNAL_SOA;


typedef std::pair<std::vector<const INDEXED_SP*>::const_iterator, std::vector<const INDEXED_SP*>::const_iterator> SP_RANGE;

typedef class TrigTrackSeedGenerator {

 public:

  TrigTrackSeedGenerator(const TrigCombinatorialSettings&);
  ~TrigTrackSeedGenerator(); 

  TrigTrackSeedGenerator(const TrigTrackSeedGenerator&) = delete;
  TrigTrackSeedGenerator& operator=(const TrigTrackSeedGenerator&) = delete;

  void loadSpacePoints(const std::vector<TrigSiSpacePointBase>&);
  void createSeeds(const IRoiDescriptor*);
  void createSeeds(const IRoiDescriptor*, const std::vector<float>& vZv);
  void getSeeds(std::vector<TrigInDetTriplet>&);

private:

  std::vector<INDEXED_SP> m_spStorage;
  std::vector<float> m_minTau;
  std::vector<float> m_maxTau;

  bool validateLayerPairNew(int, int, float, float); 
  bool getSpacepointRange(int, const std::vector<const INDEXED_SP*>&, SP_RANGE&);
  int processSpacepointRange(int, const INDEXED_SP*, bool, const SP_RANGE&, const IRoiDescriptor*);
  int processSpacepointRangeZv(const INDEXED_SP*, bool, const SP_RANGE&, bool, const float&, const float&);
  void createTriplets(const TrigSiSpacePointBase*, int, int, std::vector<TrigInDetTriplet>&, const IRoiDescriptor*);
  void createTripletsNew(const TrigSiSpacePointBase*, int, int, std::vector<TrigInDetTriplet>&, const IRoiDescriptor*);
  void createConfirmedTriplets(const TrigSiSpacePointBase*, int, int, std::vector<TrigInDetTriplet>&, const IRoiDescriptor*);
  void storeTriplets(std::vector<TrigInDetTriplet>&);

  const TrigCombinatorialSettings& m_settings;
  double m_phiSliceWidth;
  double m_minDeltaRadius, m_maxDeltaRadius, m_maxDeltaRadiusConf, m_zTol;

  L_PHI_STORAGE* m_pStore;

  INTERNAL_SOA m_SoA;

  double m_CovMS, m_minR_squ, m_dtPreCut;

  std::vector<TrigInDetTriplet> m_triplets;

  float m_zMinus, m_zPlus, m_minCoord, m_maxCoord;

  int m_nInner, m_nOuter;
  std::vector<int> m_innerMarkers, m_outerMarkers;
} TRIG_TRACK_SEED_GENERATOR;

#endif // not TRIGINDETPATTRECOTOOLS_TRIGTRACKSEEDGENERATOR_H
