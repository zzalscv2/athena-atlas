/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include <limits>

#define WRITE_SETTER(ELE_TYPE, SETTER, STORAGE_VEC)                            \
    StatusCode MuonDetectorManager::SETTER(ElementPtr<ELE_TYPE> element) {     \
        if (!element) {                                                        \
            ATH_MSG_FATAL(__func__ << " -- nullptr is given.");                \
            return StatusCode::FAILURE;                                        \
        }                                                                      \
        ATH_CHECK(element->initElement());                                     \
        size_t idx = static_cast<unsigned int>(element->identHash());          \
        if (idx >= STORAGE_VEC.size())                                         \
            STORAGE_VEC.resize(idx + 1);                                       \
        std::unique_ptr<ELE_TYPE>& new_element = STORAGE_VEC[idx];             \
        if (new_element) {                                                     \
            ATH_MSG_FATAL("The detector element "                              \
                          << m_idHelperSvc->toStringDetEl(element->identify()) \
                          << " has already been added before");                \
            return StatusCode::FAILURE;                                        \
        }                                                                      \
        new_element = std::move(element);                                      \
        return StatusCode::SUCCESS;                                            \
    }
#define ADD_DETECTOR(ELE_TYPE, STORAGE_VEC) \
    WRITE_SETTER(ELE_TYPE, add##ELE_TYPE, STORAGE_VEC)

namespace {
    constexpr unsigned int minOne = std::numeric_limits<unsigned int>::max();
}
namespace MuonGMR4 {
MuonDetectorManager::MuonDetectorManager()
    : AthMessaging{"MuonDetectorManagerR4"} {
    if (!m_idHelperSvc.retrieve().isSuccess()) {
        ATH_MSG_FATAL(__func__
                      << "()  -- Failed to retrieve the Identifier service");
        throw std::runtime_error("MuonIdHelperSvc does not exists");
    }
    setName("MuonR4");
}
IdentifierHash MuonDetectorManager::buildHash(const Identifier& id) const {
    if (m_idHelperSvc->isMdt(id))
        return buildHash(id, m_idHelperSvc->mdtIdHelper());
    else if (m_idHelperSvc->isRpc(id))
        return buildHash(id, m_idHelperSvc->rpcIdHelper());
    else if (m_idHelperSvc->isTgc(id))
        return buildHash(id, m_idHelperSvc->tgcIdHelper());
    else if (m_idHelperSvc->issTgc(id))
        return buildHash(id, m_idHelperSvc->stgcIdHelper());
    else if (m_idHelperSvc->isMM(id))
        return buildHash(id, m_idHelperSvc->mmIdHelper());
    else if (m_idHelperSvc->isCsc(id))
        return buildHash(id, m_idHelperSvc->cscIdHelper());
    return IdentifierHash{minOne};
}
IdentifierHash MuonDetectorManager::buildHash(
    const Identifier& id, const MuonIdHelper& idHelper) const {
    IdentifierHash hash{minOne};
    if (idHelper.get_detectorElement_hash(id, hash)) {
        ATH_MSG_WARNING("Could not construct an Identifier hash from "
                        << m_idHelperSvc->toString(id));
    }
    return hash;
}
ADD_DETECTOR(MdtReadoutElement, m_mdtEles);

unsigned int MuonDetectorManager::getNumTreeTops() const {
    return m_treeTopVector.size();
}

PVConstLink MuonDetectorManager::getTreeTop(unsigned int i) const {
    return m_treeTopVector[i];
}

void MuonDetectorManager::addTreeTop(PVConstLink pv) {
    m_treeTopVector.push_back(pv);
}

}  // namespace MuonGMR4
#undef WRITE_SETTER
#undef ADD_DETECTOR