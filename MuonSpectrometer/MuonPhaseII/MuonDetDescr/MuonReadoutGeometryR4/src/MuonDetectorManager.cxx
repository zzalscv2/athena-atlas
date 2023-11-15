/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include <limits>

#define WRITE_SETTER(ELE_TYPE, SETTER, STORAGE_VEC)                                 \
    StatusCode MuonDetectorManager::SETTER(ElementPtr<ELE_TYPE> element) {          \
        if (!element) {                                                             \
            ATH_MSG_FATAL(__func__ << " -- nullptr is given.");                     \
            return StatusCode::FAILURE;                                             \
        }                                                                           \
        ATH_CHECK(element->initElement());                                          \
        size_t idx = static_cast<unsigned int>(element->identHash());               \
        if (idx >= STORAGE_VEC.size())                                              \
            STORAGE_VEC.resize(idx + 1);                                            \
        std::unique_ptr<ELE_TYPE>& new_element = STORAGE_VEC[idx];                  \
        if (new_element) {                                                          \
            ATH_MSG_FATAL("The detector element "                                   \
                          << m_idHelperSvc->toStringDetEl(element->identify())      \
                          << " has already been added before "                      \
                          <<m_idHelperSvc->toStringDetEl(new_element->identify())); \
            return StatusCode::FAILURE;                                             \
        }                                                                           \
        new_element = std::move(element);                                           \
        return StatusCode::SUCCESS;                                                 \
    }
#define ADD_DETECTOR(ELE_TYPE, STORAGE_VEC)                                         \
    WRITE_SETTER(ELE_TYPE, add##ELE_TYPE, STORAGE_VEC)                              \
                                                                                    \
    std::vector<const ELE_TYPE*> MuonDetectorManager::getAll##ELE_TYPE##s() const { \
         std::vector<const ELE_TYPE*> allElements{};                                \
         allElements.reserve(STORAGE_VEC.size());                                   \
         for (const std::unique_ptr<ELE_TYPE>& ele : STORAGE_VEC) {                 \
             if (ele) allElements.push_back(ele.get());                             \
         }                                                                          \
         return allElements;                                                        \
    }

namespace {
    /// Helper function to copy the radout elements from a technology into the 
    /// vector of all readout elements.
    template <class ReadoutEle> void insert(std::vector<const ReadoutEle*>&& tech_eles,
                                            std::vector<const MuonGMR4::MuonReadoutElement*>& all_eles){
        all_eles.reserve(all_eles.capacity() + tech_eles.size());
        all_eles.insert(all_eles.end(), 
                        std::make_move_iterator(tech_eles.begin()), 
                        std::make_move_iterator(tech_eles.end()));
    }
}
namespace MuonGMR4 {
MuonDetectorManager::MuonDetectorManager()
    : AthMessaging{"MuonDetectorManagerR4"} {
    if (!m_idHelperSvc.retrieve().isSuccess()) {
        ATH_MSG_FATAL(__func__<< "()  -- Failed to retrieve the Identifier service");
        throw std::runtime_error("MuonIdHelperSvc does not exist");
    }
    setName("MuonR4");
}
 std::vector<const MuonReadoutElement*> MuonDetectorManager::getAllReadoutElements() const {
    std::vector<const MuonReadoutElement*> allEles{};
    insert(getAllMdtReadoutElements(), allEles);
    insert(getAllTgcReadoutElements(), allEles);
    insert(getAllRpcReadoutElements(), allEles);
    insert(getAllsTgcReadoutElements(), allEles);
    return allEles;
}
ADD_DETECTOR(MdtReadoutElement, m_mdtEles);
ADD_DETECTOR(TgcReadoutElement, m_tgcEles);
ADD_DETECTOR(RpcReadoutElement, m_rpcEles);
ADD_DETECTOR(sTgcReadoutElement, m_sTgcEles);

unsigned int MuonDetectorManager::getNumTreeTops() const {
    return m_treeTopVector.size();
}

PVConstLink MuonDetectorManager::getTreeTop(unsigned int i) const {
    return m_treeTopVector[i];
}

void MuonDetectorManager::addTreeTop(PVConstLink pv) {
    m_treeTopVector.push_back(pv);
}
const Muon::IMuonIdHelperSvc* MuonDetectorManager::idHelperSvc() const {
    return m_idHelperSvc.get();
}
std::vector<ActsTrk::DetectorType> MuonDetectorManager::getDetectorTypes() const {
    std::vector<ActsTrk::DetectorType> types{};
    if (!m_mdtEles.empty()) types.push_back(ActsTrk::DetectorType::Mdt);
    if (!m_tgcEles.empty()) types.push_back(ActsTrk::DetectorType::Tgc);
    if (!m_rpcEles.empty()) types.push_back(ActsTrk::DetectorType::Rpc);
    if (!m_sTgcEles.empty()) types.push_back(ActsTrk::DetectorType::sTgc);
    return types;
}
const MuonReadoutElement* MuonDetectorManager::getReadoutElement(const Identifier& id) const {
    if (m_idHelperSvc->isMdt(id)) return getMdtReadoutElement(id);
    else if (m_idHelperSvc->isRpc(id)) return getRpcReadoutElement(id);
    else if (m_idHelperSvc->isTgc(id)) return getTgcReadoutElement(id);
    else if (m_idHelperSvc->issTgc(id)) return getsTgcReadoutElement(id);
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Not a muon detector element "<<m_idHelperSvc->toString(id));
    return nullptr;
}

}  // namespace MuonGMR4
#undef WRITE_SETTER
#undef ADD_DETECTOR