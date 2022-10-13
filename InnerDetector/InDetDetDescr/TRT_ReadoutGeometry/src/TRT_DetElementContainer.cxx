
/*                                                                                                                                          
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_ReadoutGeometry/TRT_DetElementContainer.h"
#include "TRT_ReadoutGeometry/TRT_BarrelElement.h"
#include "TRT_ReadoutGeometry/TRT_EndcapElement.h"

namespace InDetDD{

  TRT_DetElementContainer::TRT_DetElementContainer():
    m_trtcoll(),
    m_trtnum(nullptr)
  {
    clear();
  }

  TRT_DetElementContainer::~TRT_DetElementContainer()
  {
    clear();
  }

  void TRT_DetElementContainer::setNumerology(const TRT_Numerology* mynum) {
    m_trtnum=mynum;
  }

  const TRT_DetElementCollection* TRT_DetElementContainer::getElements() const
  { 
    return &m_trtcoll;
  }

  const TRT_Numerology* TRT_DetElementContainer::getTRTNumerology() const
  {
    return m_trtnum;
  }

  const TRT_BarrelElement* TRT_DetElementContainer::getBarrelDetElement(unsigned int positive,
								 unsigned int moduleIndex,
								 unsigned int phiIndex,
								 unsigned int strawLayerIndex) const {
    if ( positive >= 2 || moduleIndex >= NMODMAX
	 || phiIndex>=NPHIMAX || strawLayerIndex >= NSTRAWLAYMAXBR) return nullptr;

    return m_baArray[positive][moduleIndex][phiIndex][strawLayerIndex];
  }

  const TRT_EndcapElement* TRT_DetElementContainer::getEndcapDetElement(unsigned int positive,
								 unsigned int wheelIndex,
								 unsigned int strawLayerIndex,
								 unsigned int phiIndex) const {

    if ( positive >= 2 || wheelIndex >= NWHEELMAX
	 || phiIndex>=NPHIMAX || strawLayerIndex >= NSTRAWLAYMAXEC) return nullptr;

    return m_ecArray[positive][wheelIndex][strawLayerIndex][phiIndex];
  }

  void TRT_DetElementContainer::addBarrelElement(TRT_BarrelElement *barrel) 
  {
    // check if the element has already been added
    if (std::find(m_trtcoll.begin(), m_trtcoll.end(), barrel) != m_trtcoll.end()) return;
    // check if something was stored at the given indices 
    TRT_BarrelElement* arrayElement = m_baArray
      [barrel->getCode().isPosZ()]
      [barrel->getCode().getModuleIndex()]
      [barrel->getCode().getPhiIndex()]
      [barrel->getCode().getStrawLayerIndex()];
    if (arrayElement != nullptr) {
      m_trtcoll.erase(std::remove(m_trtcoll.begin(), m_trtcoll.end(), arrayElement));
      delete arrayElement;
    }
    m_baArray[barrel->getCode().isPosZ()]
      [barrel->getCode().getModuleIndex()]
      [barrel->getCode().getPhiIndex()]
      [barrel->getCode().getStrawLayerIndex()] = barrel;
    m_trtcoll.push_back(barrel);
  }

  void TRT_DetElementContainer::addEndcapElement(TRT_EndcapElement *endcap)
  {
    // check if the element has already been added
    if (std::find(m_trtcoll.begin(), m_trtcoll.end(), endcap) != m_trtcoll.end()) return;
    // check if something was stored at the given indices 
    TRT_EndcapElement* arrayElement = m_ecArray
      [endcap->getCode().isPosZ()]
      [endcap->getCode().getWheelIndex()]
      [endcap->getCode().getStrawLayerIndex()]
      [endcap->getCode().getPhiIndex()];
    if (arrayElement != nullptr) {
      m_trtcoll.erase(std::remove(m_trtcoll.begin(), m_trtcoll.end(), arrayElement));
      delete arrayElement;
    }
    m_ecArray[endcap->getCode().isPosZ()]
      [endcap->getCode().getWheelIndex()]
      [endcap->getCode().getStrawLayerIndex()]
      [endcap->getCode().getPhiIndex()] = endcap;
    m_trtcoll.push_back(endcap);
  }

  void TRT_DetElementContainer::clear()
  {
    for (auto *p : m_trtcoll) {
      delete p;
    }
    m_trtcoll.clear();
    for (auto & ec : m_baArray) {
      for (auto & mod : ec) {
        for (auto & phi : mod) {
          for (auto & sLay : phi) {
            sLay = nullptr;
          }
        }
      }
    }
    for (auto & ec : m_ecArray) {
      for (auto & whe : ec) {
        for (auto & sLay : whe) {
          for(auto & phi : sLay) {
            phi = nullptr;
          }
        }
      }
    }
  }
     
}
