#include "MuonEventTPCnv/MuonRDO/NSW_PadTriggerDataContainerCnv_p1.h"

namespace Muon {
void NSW_PadTriggerDataContainerCnv_p1::persToTrans(const NSW_PadTriggerDataContainer_p1* persistentObj, NSW_PadTriggerDataContainer* transientObj, MsgStream &log) {
    if (log.level() <= MSG::VERBOSE) {
        log << MSG::VERBOSE <<
            "Converting persistent NSW_PadTriggerDataContainer_p1 to transient NSW_PadTriggerDataContainer" << endmsg;
    }
    for (const auto& pCollection : persistentObj->m_collections) {
        auto tCollection = std::make_unique<NSW_PadTriggerData>(
            pCollection.m_sourceid,
            pCollection.m_flags,
            pCollection.m_ec,
            pCollection.m_fragid,
            pCollection.m_secid,
            pCollection.m_spare,
            pCollection.m_orbit,
            pCollection.m_bcid,
            pCollection.m_l1id,
            pCollection.m_orbitid,
            pCollection.m_orbit1,
            pCollection.m_status,
            pCollection.m_hit_n,
            pCollection.m_pfeb_n,
            pCollection.m_trigger_n,
            pCollection.m_bcid_n,
            pCollection.m_hit_relbcid,
            pCollection.m_hit_pfeb,
            pCollection.m_hit_tdschannel,
            pCollection.m_hit_vmmchannel,
            pCollection.m_hit_vmm,
            pCollection.m_hit_padchannel,
            pCollection.m_pfeb_addr,
            pCollection.m_pfeb_nchan,
            pCollection.m_pfeb_disconnected,
            pCollection.m_trigger_bandid,
            pCollection.m_trigger_phiid,
            pCollection.m_trigger_relbcid,
            pCollection.m_bcid_rel,
            pCollection.m_bcid_status,
            pCollection.m_bcid_multzero,
            pCollection.m_bcid_multiplicity
            );
        if(transientObj->addCollection(tCollection.release(), transientObj->numberOfCollections()).isFailure()) {
            throw std::runtime_error{ "Could not add collection to transient container!" };
        }
    }
}


void NSW_PadTriggerDataContainerCnv_p1::transToPers(const NSW_PadTriggerDataContainer* transientObj, NSW_PadTriggerDataContainer_p1* persistentObj, MsgStream &log) {
    if (log.level() <= MSG::VERBOSE) {
        log << MSG::VERBOSE <<
            "Converting transient NSW_PadTriggerDataContainer to persistent NSW_PadTriggerDataContainer_p1" << endmsg;
    }
    persistentObj->m_collections.reserve(transientObj->size());
    // Iterate over collections
    for (const NSW_PadTriggerData* tCollection : *transientObj) {
        NSW_PadTriggerData_p1 pCollection{};
        pCollection.m_sourceid = tCollection->getSourceid();
        pCollection.m_flags = tCollection->getFlags();
        pCollection.m_ec = tCollection->getEc();
        pCollection.m_fragid = tCollection->getFragid();
        pCollection.m_secid = tCollection->getSecid();
        pCollection.m_spare = tCollection->getSpare();
        pCollection.m_orbit = tCollection->getOrbit();
        pCollection.m_bcid = tCollection->getBcid();
        pCollection.m_l1id = tCollection->getL1id();
        pCollection.m_orbitid = tCollection->getOrbitid();
        pCollection.m_orbit1 = tCollection->getOrbit1();
        pCollection.m_status = tCollection->getStatus();
        pCollection.m_hit_n = tCollection->getNumberOfHits();
        pCollection.m_pfeb_n = tCollection->getNumberOfPfebs();
        pCollection.m_trigger_n = tCollection->getNumberOfTriggers();
        pCollection.m_bcid_n = tCollection->getNumberOfBcids();
        pCollection.m_hit_relbcid = tCollection->getHitRelBcids();
        pCollection.m_hit_pfeb = tCollection->getHitPfebs();
        pCollection.m_hit_tdschannel = tCollection->getHitTdsChannels();
        pCollection.m_hit_vmmchannel = tCollection->getHitVmmChannels();
        pCollection.m_hit_vmm = tCollection->getHitVmms();
        pCollection.m_hit_padchannel = tCollection->getHitPadChannels();
        pCollection.m_pfeb_addr = tCollection->getPfebAddrs();
        pCollection.m_pfeb_nchan = tCollection->getPfebNChannels();
        pCollection.m_pfeb_disconnected = tCollection->getPfebDisconnecteds();
        pCollection.m_trigger_bandid = tCollection->getTriggerBandIds();
        pCollection.m_trigger_phiid = tCollection->getTriggerPhiIds();
        pCollection.m_trigger_relbcid = tCollection->getTriggerRelBcids();
        pCollection.m_bcid_rel = tCollection->getBcidRels();
        pCollection.m_bcid_status = tCollection->getBcidStatuses();
        pCollection.m_bcid_multzero = tCollection->getBcidMultZeros();
        pCollection.m_bcid_multiplicity = tCollection->getBcidMultiplicities();
        persistentObj->m_collections.push_back(std::move(pCollection));
    }
}

} // namespace Muon
