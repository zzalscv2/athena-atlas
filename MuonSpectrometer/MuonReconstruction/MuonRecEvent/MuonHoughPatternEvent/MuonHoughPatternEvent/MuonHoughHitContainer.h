/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHOUGHPATTERNEVENT_MUONHOUGHHITCONTAINER_H
#define MUONHOUGHPATTERNEVENT_MUONHOUGHHITCONTAINER_H

#include "MuonHoughPatternEvent/MuonHoughHit.h"
#include <memory>

namespace Trk {
    class PrepRawData;
}

class MuonHoughHitContainer {
    /** MuonHoughHitContainer does own its hits all added hits should be 'newed',
        except when m_ownhits==false */

public:
    /** constructor, flag for deleting hits at destructor */
    MuonHoughHitContainer() = default;

    /** destructor */
    ~MuonHoughHitContainer() = default;

    /** returns Hit at position hitno */
    std::shared_ptr<MuonHoughHit> getHit(int hitno) const;
    /** returns hit vector */
    const std::vector<std::shared_ptr<MuonHoughHit>>& getHits() const;

    /** add hit to container */
    void addHit(const std::shared_ptr<MuonHoughHit>& hit);

    /** remove hit from container */
    void removeHit(unsigned int hitno);

    /** returns hitid of hit hitno */
    int getHitId(unsigned int hitno) const;
    /** returns x position of hit hitno*/
    double getHitx(unsigned int hitno) const;
    /** returns y position of hit hitno*/
    double getHity(unsigned int hitno) const;
    /** returns z position of hit hitno*/
    double getHitz(unsigned int hitno) const;

    /** returns radius of hit hitno*/
    double getRadius(unsigned int hitno) const;
    /** returns theta of hit hitno*/
    double getTheta(unsigned int hitno) const;
    /** returns phi of hit hitno*/
    double getPhi(unsigned int hitno) const;
    /** returns weight of hit hitno*/
    double getWeight(unsigned int hitno) const;
    /** returns the orignal weight of hit hitno*/
    double getOrigWeight(unsigned int hitno) const;

    /** returns if hit hitno measures phi*/
    bool getMeasuresPhi(unsigned int hitno) const;
    /** returns preprawdata pointer of hit hitno*/
    const Trk::PrepRawData* getPrd(unsigned int hitno) const;

    /** returns detectortechnology in string of hit hitno*/
    std::string getWhichDetector(unsigned int hitno) const;
    /** returns detectortechnology of hit hitno*/
    MuonHough::DetectorTechnology getDetectorId(unsigned int hitno) const;
    /** returns size of hitcontainer */
    unsigned int size() const;
    /** returns if hitcontainer is empty */
    bool empty() const;
    /** allocates memory for hitvector*/
    void reserve(int size);

    /** returns number of rpc hits in container */
    int getRPChitno() const;
    /** returns number of rpc eta hits in container */
    int getRPCetahitno() const;
    /** returns number of mdt hits in container */
    int getMDThitno() const;
    /** returns number of csc hits in container */
    int getCSChitno() const;
    /** returns number of tgc hits in container */
    int getTGChitno() const;

protected:
    /** vector of hits in container */
    std::vector<std::shared_ptr<MuonHoughHit>> m_hit;

  
};

inline std::shared_ptr<MuonHoughHit> MuonHoughHitContainer::getHit(int hitno) const { return m_hit.at(hitno); }
inline const std::vector<std::shared_ptr<MuonHoughHit>>& MuonHoughHitContainer::getHits() const { return m_hit; }
inline int MuonHoughHitContainer::getHitId(unsigned int hitno) const { return m_hit[hitno]->getId(); }
inline double MuonHoughHitContainer::getHitx(unsigned int hitno) const { return m_hit[hitno]->getHitx(); }
inline double MuonHoughHitContainer::getHity(unsigned int hitno) const { return m_hit[hitno]->getHity(); }
inline double MuonHoughHitContainer::getHitz(unsigned int hitno) const { return m_hit[hitno]->getHitz(); }
inline double MuonHoughHitContainer::getRadius(unsigned int hitno) const { return m_hit[hitno]->getRadius(); }

inline double MuonHoughHitContainer::getTheta(unsigned int hitno) const { return m_hit[hitno]->getTheta(); }
inline double MuonHoughHitContainer::getPhi(unsigned int hitno) const { return m_hit[hitno]->getPhi(); }
inline double MuonHoughHitContainer::getWeight(unsigned int hitno) const { return m_hit[hitno]->getWeight(); }
inline double MuonHoughHitContainer::getOrigWeight(unsigned int hitno) const { return m_hit[hitno]->getOrigWeight(); }

inline unsigned int MuonHoughHitContainer::size() const { return m_hit.size(); }
inline bool MuonHoughHitContainer::empty() const { return m_hit.empty(); }
inline void MuonHoughHitContainer::reserve(int size) { m_hit.reserve(size); }
inline MuonHough::DetectorTechnology MuonHoughHitContainer::getDetectorId(unsigned int hitno) const { return m_hit[hitno]->getDetectorId(); }
inline std::string MuonHoughHitContainer::getWhichDetector(unsigned int hitno) const { return m_hit[hitno]->getWhichDetector(); }
inline bool MuonHoughHitContainer::getMeasuresPhi(unsigned int hitno) const { return m_hit[hitno]->getMeasuresPhi(); }
inline const Trk::PrepRawData* MuonHoughHitContainer::getPrd(unsigned int hitno) const { return m_hit[hitno]->getPrd(); }

#endif  // MUONHOUGHPATTERNEVENT_MUONHOUGHHITCONTAINER_H
