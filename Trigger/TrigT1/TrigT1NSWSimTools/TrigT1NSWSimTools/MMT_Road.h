/*
 *   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MMT_ROAD_H
#define MMT_ROAD_H

#include "MMT_Hit.h"
#include <cmath>
#include <numeric>

class MMT_Road {
  public:
    MMT_Road(const char sector, const int roadSize, const int UpX, const int DownX, const int UpUV, const int DownUV, const int xthr, const int uvthr,
             const int iroadx, const int iroadu = -1, const int iroadv = -1);
    ~MMT_Road()=default;

    void addHits(std::vector<std::shared_ptr<MMT_Hit> > &hits);
    double avgSofX() const;
    double avgSofUV(const int uv1, const int uv2) const;
    double avgZofUV(const int uv1, const int uv2) const;
    bool checkCoincidences(const int bcwind) const;
    unsigned int countHits() const { return m_road_hits.size(); }
    unsigned int countRealHits() const;
    unsigned int countUVHits(bool flag) const;
    unsigned int countXHits(bool flag) const;
    bool evaluateLowRes() const;
    bool horizontalCheck() const;
    void incrementAge(const int bcwind);
    const std::vector<std::unique_ptr<MMT_Hit> >& getHitVector() const { return m_road_hits; }
    int getRoadSize() const { return m_roadSize; }
    int getRoadSizeUpX() const { return m_roadSizeUpX; }
    int getRoadSizeDownX() const { return m_roadSizeDownX; }
    int getRoadSizeUpUV() const { return m_roadSizeUpUV; }
    int getRoadSizeDownUV() const { return m_roadSizeDownUV; }
    char getSector() const { return m_sector; }
    int getXthreshold() const { return m_xthr; }
    int getUVthreshold() const { return m_uvthr; }
    int iRoadx() const { return m_iroadx; }
    int iRoadu() const { return m_iroadu; }
    int iRoadv() const { return m_iroadv; }
    bool matureCheck(const int bcwind) const;
    double mxl() const;
    void reset();
    bool stereoCheck() const;

  private:
    int m_iroadx;
    int m_iroadu;
    int m_iroadv;
    char m_sector;
    int m_xthr, m_uvthr;
    int m_roadSize, m_roadSizeUpX, m_roadSizeDownX, m_roadSizeUpUV, m_roadSizeDownUV;
    std::vector<std::unique_ptr<MMT_Hit> > m_road_hits;
};
#endif
