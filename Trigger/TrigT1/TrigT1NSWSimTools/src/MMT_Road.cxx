/*
 *   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "TrigT1NSWSimTools/MMT_Road.h"

MMT_Road::MMT_Road(const char sector, const int roadSize, const int UpX, const int DownX, const int UpUV, const int DownUV, const int xthr, const int uvthr,
                   const float pitch, const float eta1, const float eta2, const int iroadx, const int iroadu, const int iroadv) {
  m_sector = sector;
  m_iroad  = iroadx;
  m_iroadx = iroadx;
  m_iroadu = (iroadu != -1) ? iroadu : iroadx;
  m_iroadv = (iroadv != -1) ? iroadv : iroadx;
  m_xthr = xthr;
  m_uvthr = uvthr;

  m_roadSize = roadSize;
  m_roadSizeUpX = UpX;
  m_roadSizeDownX = DownX;
  m_roadSizeUpUV = UpUV;
  m_roadSizeDownUV = DownUV;
  m_pitch = pitch;
  m_innerRadiusEta1 = eta1;
  m_innerRadiusEta2 = eta2;
}

void MMT_Road::addHits(std::vector<std::shared_ptr<MMT_Hit> > &hits) {
  for (const auto &hit_i : hits) {
    int bo = hit_i->getPlane();
    bool has_hit = false;
    if( this->containsNeighbors(hit_i.get()) ) {
      for (const auto &hit_j : m_road_hits) {
        if (hit_j->getPlane() == bo) {
          has_hit = true;
          break;
        }
      }
      if (hit_i->isNoise() == false) {
        int erase_me = -1;
        for (unsigned int j = 0; j < m_road_hits.size(); j++) {
          if (m_road_hits[j]->getPlane() == bo && m_road_hits[j]->isNoise()) {
            erase_me = j;
            has_hit = false;
            break;
          }
        }
        if (erase_me > -1) m_road_hits.erase(m_road_hits.begin() + erase_me);
      }

      if (has_hit) continue;
      auto hit = std::make_unique<MMT_Hit>(hit_i.get());
      m_road_hits.push_back(std::move(hit));
      m_road_hits.back()->setAge(0);
    }
  }
}

bool MMT_Road::containsNeighbors(const MMT_Hit* hit) const {

  if (this->getSector() != hit->getSector()) return false;

  int iroad = 0;
  unsigned short int olow = 0, ohigh = 0;
  if (hit->isX()) {
    iroad = this->iRoadx();
    olow  = this->getRoadSizeDownX();
    ohigh = this->getRoadSizeUpX();
  }
  else if (hit->isU()) {
    iroad = this->iRoadu();
    olow  = this->getRoadSizeDownUV();
    ohigh = this->getRoadSizeUpUV();
  }
  else if (hit->isV()) {
    iroad = this->iRoadv();
    olow  = this->getRoadSizeDownUV();
    ohigh = this->getRoadSizeUpUV();
  }
  else return false;

  float R = (std::abs(hit->getStationEta()) == 1) ? m_innerRadiusEta1 : m_innerRadiusEta2;
  double slow  = (R + (this->getRoadSize()*iroad     + 0.5 - olow )*this->getPitch() + hit->getShift())*hit->getOneOverZ();
  double shigh = (R + (this->getRoadSize()*(iroad+1) + 0.5 + ohigh)*this->getPitch() + hit->getShift())*hit->getOneOverZ();

  if (hit->getRZSlope() > 0.) return (hit->getRZSlope() >= slow && hit->getRZSlope() < shigh);
  else return (hit->getRZSlope() >= shigh && hit->getRZSlope() < slow);
}

double MMT_Road::avgSofX() const {
  std::vector<double> sl;
  for (const auto &hit : m_road_hits) {
    int bo = hit->getPlane();
    if (bo < 2 || bo > 5) sl.push_back(hit->getRZSlope());
  }
  double avg_x = std::accumulate(sl.begin(), sl.end(), 0.0)/(double)sl.size();
  return avg_x;
}

double MMT_Road::avgSofUV(const int uv1, const int uv2) const {
  std::vector<double> sl;
  for (const auto &hit : m_road_hits) {
    int bo = hit->getPlane();
    if (bo == uv1 || bo == uv2) sl.push_back(hit->getRZSlope());
  }
  double avg_uv = std::accumulate(sl.begin(), sl.end(), 0.0)/(double)sl.size();
  return avg_uv;
}

double MMT_Road::avgZofUV(const int uv1, const int uv2) const {
  std::vector<double> zs;
  for (const auto &hit : m_road_hits) {
    int bo = hit->getPlane();
    if (bo == uv1 || bo == uv2) zs.push_back(hit->getZ());
  }
  double avg_z = std::accumulate(zs.begin(), zs.end(), 0.0)/(double)zs.size();
  return avg_z;
}

bool MMT_Road::checkCoincidences(const int &bcwind) const {
  bool passHorizontalCheck = this->horizontalCheck();
  bool passStereoCheck = this->stereoCheck();
  bool passMatureCheck = this->matureCheck(bcwind);
  return (passHorizontalCheck && passStereoCheck && passMatureCheck);
}

unsigned int MMT_Road::countRealHits() const {
  int nreal = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->isNoise() == false) nreal++;
  }
  return nreal;
}

unsigned int MMT_Road::countUVHits(bool flag) const {
  unsigned int nuv = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->getPlane() == 2 || hit->getPlane() == 4) {
      if (hit->isNoise() == flag) nuv++;
    }
    if (hit->getPlane() == 3 || hit->getPlane() == 5) {
      if (hit->isNoise() == flag) nuv++;
    }
  }
  return nuv;
}

unsigned int MMT_Road::countXHits(bool flag) const {
  unsigned int nx = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->getPlane() < 2 || hit->getPlane() > 5) {
      if (hit->isNoise() == flag) nx++;
    }
  }
  return nx;
}

bool MMT_Road::evaluateLowRes() const {
  unsigned int nhits1 = 0, nhits2 = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->getPlane() < 4 && !hit->isNoise()) nhits1++;
    else if (hit->getPlane() > 3 && !hit->isNoise()) nhits2++;
  }
  return (nhits1 < 4 || nhits2 < 4);
}

bool MMT_Road::horizontalCheck() const {
  int nx1 = 0, nx2 = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->getPlane() >-1 && hit->getPlane() < 2) nx1++;
    if (hit->getPlane() > 5 && hit->getPlane() < 8) nx2++;
  }
  return (nx1 > 0 && nx2 > 0 && (nx1+nx2) >= this->getXthreshold());
}

void MMT_Road::incrementAge(const int &bcwind) {
  std::vector<unsigned int> old_ihits;
  for (unsigned int j = 0; j < m_road_hits.size(); j++) {
    m_road_hits[j]->setAge(m_road_hits[j]->getAge() +1);
    if (m_road_hits[j]->getAge() > (bcwind-1)) old_ihits.push_back(j);
  }
  for (int j = old_ihits.size()-1; j > -1; j--) m_road_hits.erase(m_road_hits.begin()+j);
}

bool MMT_Road::matureCheck(const int &bcwind) const {
  for (const auto &hit : m_road_hits) {
    if (hit->getAge() == (bcwind - 1)) return true;
  }
  return false;
}

double MMT_Road::mxl() const {
  std::vector<double> ys, zs;
  for (const auto &hit : m_road_hits) {
    int bo = hit->getPlane();
    if (bo < 2 || bo > 5) {
      ys.push_back(hit->getR());
      zs.push_back(hit->getZ());
    }
  }
  double mxl = 0;
  double avg_z = std::accumulate(zs.begin(), zs.end(), 0.0)/(double)zs.size();
  double sum_sq_z = std::inner_product(zs.begin(), zs.end(), zs.begin(), 0.0);
  for (unsigned int i = 0; i < ys.size(); i++) mxl += ys[i]*( (zs[i]-avg_z) / (sum_sq_z - zs.size()*std::pow(avg_z,2)) );

  return mxl;
}

void MMT_Road::reset() {
  if (!m_road_hits.empty()) m_road_hits.clear();
}

bool MMT_Road::stereoCheck() const {

  if (this->getUVthreshold() == 0) return true;

  int nu = 0, nv = 0;
  for (const auto &hit : m_road_hits) {
    if (hit->getPlane() == 2 || hit->getPlane() == 4) nu++;
    if (hit->getPlane() == 3 || hit->getPlane() == 5) nv++;
  }

  return (nu > 0 && nv > 0 && (nu+nv) >= this->getUVthreshold());
}
