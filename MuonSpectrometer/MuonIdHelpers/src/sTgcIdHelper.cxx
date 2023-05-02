/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonIdHelpers/sTgcIdHelper.h"

/*******************************************************************************/
// Constructor/Destructor
sTgcIdHelper::sTgcIdHelper() : MuonIdHelper("sTgcIdHelper") {
    m_module_hashes.fill(-1);
    m_detectorElement_hashes.fill(-1);
}
/*******************************************************************************/
// Initialize dictionary
int sTgcIdHelper::initialize_from_dictionary(const IdDictMgr& dict_mgr) {
    int status = 0;

    // Check whether this helper should be reinitialized
    if (!reinitialize(dict_mgr)) {
        ATH_MSG_INFO("Request to reinitialize not satisfied - tags have not changed");
        return (0);
    } else {
        ATH_MSG_DEBUG("(Re)initialize ");
    }

    // init base object
    if (AtlasDetectorID::initialize_from_dictionary(dict_mgr)) return 1;

    // Register version of the MuonSpectrometer dictionary
    if (register_dict_tag(dict_mgr, "MuonSpectrometer")) return 1;

    m_dict = dict_mgr.find_dictionary("MuonSpectrometer");
    if (!m_dict) {
        ATH_MSG_ERROR(" initialize_from_dict - cannot access MuonSpectrometer dictionary ");
        return 1;
    }

    // Initialize some of the field indices
    if (initLevelsFromDict()) return 1;

    int index = technologyIndex("STGC");
    if (index == -1) {
        ATH_MSG_DEBUG("initLevelsFromDict - there are no sTGC entries in the dictionary! ");
        return 0;
    }

    IdDictField* field = m_dict->find_field("stgcMultilayer");
    if (field) {
        m_DETECTORELEMENT_INDEX = field->m_index;
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'stgcMultilayer' field ");
        status = 1;
    }

    field = m_dict->find_field("stgcGasGap");
    if (field) {
        m_GASGAP_INDEX = field->m_index;
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'stgcGasGap' field ");
        status = 1;
    }

    field = m_dict->find_field("stgcChannelType");
    if (field) {
        m_CHANNELTYPE_INDEX = field->m_index;
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'channelType' field ");
        status = 1;
    }

    field = m_dict->find_field("stgcChannel");
    if (field) {
        m_CHANNEL_INDEX = field->m_index;
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'channel' field ");
        status = 1;
    }

    // reinitialize the module ndex
    // m_DETECTORELEMENT_INDEX = m_MODULE_INDEX;

    // save an index to the first region of tgc
    IdDictGroup* stgcGroup = m_dict->find_group("stgc");
    if (!stgcGroup) {
        ATH_MSG_ERROR("Cannot find stgc group");
    } else {
        m_GROUP_INDEX = stgcGroup->regions()[0]->m_index;
    }

    const IdDictRegion& region = *m_dict->m_regions[m_GROUP_INDEX];
    m_eta_impl = region.m_implementation[m_ETA_INDEX];
    m_phi_impl = region.m_implementation[m_PHI_INDEX];
    m_tec_impl = region.m_implementation[m_TECHNOLOGY_INDEX];
    m_mplet_impl = region.m_implementation[m_DETECTORELEMENT_INDEX];
    m_gap_impl = region.m_implementation[m_GASGAP_INDEX];
    m_typ_impl = region.m_implementation[m_CHANNELTYPE_INDEX];
    m_cha_impl = region.m_implementation[m_CHANNEL_INDEX];

    ATH_MSG_DEBUG(" sTGC decode index and bit fields for each level: " << std::endl
                                                                       << " muon        " << m_muon_impl.show_to_string() << std::endl
                                                                       << " station     " << m_sta_impl.show_to_string() << std::endl
                                                                       << " eta         " << m_eta_impl.show_to_string() << std::endl
                                                                       << " phi         " << m_phi_impl.show_to_string() << std::endl
                                                                       << " technology  " << m_tec_impl.show_to_string() << std::endl
                                                                       << " multilayer   " << m_mplet_impl.show_to_string() << std::endl
                                                                       << " gasgap      " << m_gap_impl.show_to_string() << std::endl
                                                                       << " channelType " << m_typ_impl.show_to_string() << std::endl
                                                                       << " channel     " << m_cha_impl.show_to_string());

    //
    // Build multirange for the valid set of identifiers
    //

    // Find value for the field MuonSpectrometer
    int muonField = -1;
    const IdDictDictionary* atlasDict = dict_mgr.find_dictionary("ATLAS");
    if (atlasDict->get_label_value("subdet", "MuonSpectrometer", muonField)) {
        ATH_MSG_ERROR("Could not get value for label 'MuonSpectrometer' of field 'subdet' in dictionary " << atlasDict->m_name);
        return 1;
    }

    // Build MultiRange down to "technology" for all (muon) regions
    ExpandedIdentifier region_id;
    region_id.add(muonField);
    Range prefix;
    MultiRange muon_range = m_dict->build_multirange(region_id, prefix, "technology");
    if (muon_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to Technology: "
                     << "MultiRange size is " << muon_range.size());
    } else {
        ATH_MSG_ERROR("Muon sTGC detector element MultiRange is empty");
    }

    // Build MultiRange down to "detector element" for all sTGC regions
    ExpandedIdentifier detectorElement_region;
    detectorElement_region.add(muonField);
    Range detectorElement_prefix;
    MultiRange muon_detectorElement_range = m_dict->build_multirange(detectorElement_region, detectorElement_prefix, "stgcMultilayer");
    if (muon_detectorElement_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to detector element: "
                     << "Multilayer MultiRange size is " << muon_detectorElement_range.size());
    } else {
        ATH_MSG_ERROR("Muon sTGC detector element MultiRange is empty");
    }

    // Build MultiRange down to "channel" for all sTGC regions
    ExpandedIdentifier stgc_region;
    stgc_region.add(muonField);
    Range stgc_prefix;
    MultiRange muon_channel_range = m_dict->build_multirange(stgc_region, stgc_prefix, "stgcChannel");
    if (muon_channel_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to channel: "
                     << "MultiRange size is " << muon_channel_range.size());
    } else {
        ATH_MSG_ERROR("Muon sTGC detector MultiRange is empty for channels");
    }

    // build sTGC module ranges
    // Find the regions that have a "technology field" that matches the sTGC and save them
    int stgcField = -1;
    status = m_dict->get_label_value("technology", "STGC", stgcField);

    for (int i = 0; i < (int)muon_range.size(); ++i) {
        const Range& range = muon_range[i];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)stgcField)) {
                m_full_module_range.add(range);
                ATH_MSG_DEBUG("field size is " << (int)range.cardinality() << " field index = " << i);
            }
        }
    }

    for (int j = 0; j < (int)muon_detectorElement_range.size(); ++j) {
        const Range& range = muon_detectorElement_range[j];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)stgcField)) {
                m_full_detectorElement_range.add(range);
                ATH_MSG_DEBUG("detector element field size is " << (int)range.cardinality() << " field index = " << j);
            }
        }
    }

    for (int j = 0; j < (int)muon_channel_range.size(); ++j) {
        const Range& range = muon_channel_range[j];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)stgcField)) {
                m_full_channel_range.add(range);
                ATH_MSG_DEBUG("channel field size is " << (int)range.cardinality() << " field index = " << j);
            }
        }
    }

    // test to see that the multi range is not empty
    if (m_full_module_range.size() == 0) {
        ATH_MSG_ERROR("sTGC MultiRange ID is empty for modules");
        status = 1;
    }

    // test to see that the detector element multi range is not empty
    if (m_full_detectorElement_range.size() == 0) {
        ATH_MSG_ERROR("sTGC MultiRange ID is empty for detector elements");
        status = 1;
    }

    // test to see that the multi range is not empty
    if (m_full_channel_range.size() == 0) {
        ATH_MSG_ERROR("sTGC MultiRange ID is empty for channels");
        status = 1;
    }

    // Setup the hash tables for sTGC
    ATH_MSG_INFO("Initializing sTGC hash indices ... ");
    status = init_hashes();
    status = init_detectorElement_hashes();  // same as module hash
    status = init_id_to_hashes();

    /*
    //comment out this bit to test the identifiers

    status = init_channel_hashes();
    std::cout << " looping over identifiers " << std::endl;
    const_id_iterator it = channel_begin();
    const_id_iterator it_end =  channel_end();
    for( ;it!=it_end;++it ){
    if( !is_stgc(*it) )  (*m_Log) << MSG::DEBUG << "BadStgc: not sTGC    " <<  print_to_string(*it) << endmsg;
    if( !valid(*it) )  (*m_Log) << MSG::DEBUG << "BadStgc: not valid " <<  print_to_string(*it) << endmsg;
    }

    */

    // Setup hash tables for finding neighbors
    ATH_MSG_INFO("Initializing sTGC hash indices for finding neighbors ... ");
    status = init_neighbors();

    m_init = true;
    return (status);
}  // end sTgcIdHelper::initialize_from_dictionary
/*******************************************************************************/
inline unsigned int sTgcIdHelper::moduleHashIdx(const Identifier& id) const{
    /// Unfold the array [A][B][C] by
    /// a * BxC + b * C + c
    constexpr unsigned int C = s_phiDim;
    constexpr unsigned int BxC = C*s_etaDim;    
    const int stEta = stationEta(id);
    return (stationName(id) - m_stationShift)*BxC + (stEta + s_etaDim/2 - (stEta>0))*C + (stationPhi(id) -1);
}
inline unsigned int sTgcIdHelper::detEleHashIdx(const Identifier& id) const{
    return moduleHashIdx(id)  *s_mlDim +  (multilayer(id) -1);
}

int sTgcIdHelper::init_id_to_hashes() {
    for (const Identifier& id : m_module_vec) m_stationShift = std::min(m_stationShift, 1u* stationName(id));
    unsigned int hash_max = module_hash_max();
    for (unsigned int i = 0; i < hash_max; ++i) {
        const Identifier& id = m_module_vec[i];
        const unsigned idx = moduleHashIdx(id);
        if (idx >= m_module_hashes.size() || m_module_hashes[idx] < hash_max){
            ATH_MSG_FATAL("Failed to assign module hash to "<<show_to_string(id));
            return 1;
        }
        m_module_hashes[idx] = i;
    }

    hash_max = detectorElement_hash_max();
    for (unsigned int i = 0; i < hash_max; ++i) {
        const Identifier& id = m_detectorElement_vec[i];
        const unsigned idx = detEleHashIdx(id);
        if (idx >= m_detectorElement_hashes.size() || m_detectorElement_hashes[idx] < hash_max){
            ATH_MSG_FATAL("Failed to assign detector hash to "<<show_to_string(id));
            return 1;
        }
        m_detectorElement_hashes[idx] = i;
    }
    return 0;
}  // end sTgcIdHelper::init_id_to_hashes()
/*******************************************************************************/
int sTgcIdHelper::get_module_hash(const Identifier& id, IdentifierHash& hash_id) const {
    const unsigned int idx = moduleHashIdx(id);
    if (idx >= m_module_hashes.size()) return 1;
    hash_id = m_module_hashes[idx];
    return 0;
}  // end sTgcIdHelper::get_module_hash
/*******************************************************************************/
int sTgcIdHelper::get_detectorElement_hash(const Identifier& id, IdentifierHash& hash_id) const {
    const unsigned int idx = detEleHashIdx(id);
    if (idx >= m_detectorElement_hashes.size()) return 1;
    hash_id = m_detectorElement_hashes[idx];
    return 0;
    // return get_module_hash(id, hash_id);
}

/*******************************************************************************/
Identifier sTgcIdHelper::multilayerID(const Identifier& channelID) const {
    assert(is_stgc(channelID));
    Identifier result(channelID);
    m_gap_impl.reset(result);
    m_typ_impl.reset(result);
    m_cha_impl.reset(result);
    return result;
}
/*******************************************************************************/
Identifier sTgcIdHelper::multilayerID(const Identifier& moduleID, int multilayer) const {
    Identifier result(moduleID);
    resetAndSet(m_mplet_impl, multilayer, result);
    return result;
}
Identifier sTgcIdHelper::multilayerID(const Identifier& moduleID, int multilayer, bool& isValid) const {
    try {
        const Identifier result = multilayerID(moduleID, multilayer);
        isValid = validElement(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
/*******************************************************************************/
void sTgcIdHelper::idChannels(const Identifier& id, std::vector<Identifier>& vect) const {
    vect.clear();
    Identifier parent = parentID(id);
    for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
        const Range& range = m_full_channel_range[i];
        Range::const_identifier_factory first = range.factory_begin();
        Range::const_identifier_factory last = range.factory_end();
        for (; first != last; ++first) {
            Identifier child;
            get_id((*first), child);
            if (parentID(child) == parent) vect.push_back(child);
        }
    }
}
/*******************************************************************************/
int sTgcIdHelper::stationEtaMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext eta_context(expId, 0, m_ETA_INDEX);
    if (!get_expanded_id(id, expId, &eta_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
            const Range& range = m_full_module_range[i];
            if (range.match(expId)) {
                const Range::field& eta_field = range[m_ETA_INDEX];
                if (eta_field.has_minimum()) {
                    int etamin = eta_field.get_minimum();
                    if (-999 == result) {
                        result = etamin;
                    } else {
                        if (etamin < result) result = etamin;
                    }
                }
            }
        }
        return result;
    }
    return 999;  // default
}  // end TgcIdHelper::stationEtaMin
/*******************************************************************************/
int sTgcIdHelper::stationEtaMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext eta_context(expId, 0, m_ETA_INDEX);
    if (!get_expanded_id(id, expId, &eta_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
            const Range& range = m_full_module_range[i];
            if (range.match(expId)) {
                const Range::field& eta_field = range[m_ETA_INDEX];
                if (eta_field.has_maximum()) {
                    int etamax = eta_field.get_maximum();
                    if (result < etamax) result = etamax;
                }
            }
        }
        return result;
    }
    return -999;
}  // end TgcIdHelper::stationEtaMax
/*******************************************************************************/
int sTgcIdHelper::stationPhiMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext phi_context(expId, 0, m_PHI_INDEX);

    if (!get_expanded_id(id, expId, &phi_context)) {
        for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
            const Range& range = m_full_module_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_PHI_INDEX];
                if (phi_field.has_minimum()) { return (phi_field.get_minimum()); }
            }
        }
    }
    // Failed to find the min
    return 999;
}
/*******************************************************************************/
int sTgcIdHelper::stationPhiMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext phi_context(expId, 0, m_PHI_INDEX);

    if (!get_expanded_id(id, expId, &phi_context)) {
        for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
            const Range& range = m_full_module_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_PHI_INDEX];
                if (phi_field.has_maximum()) { return (phi_field.get_maximum()); }
            }
        }
    }
    // Failed to find the max
    return -999;
}
/*******************************************************************************/
int sTgcIdHelper::numberOfMultilayers(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext context = technology_context();
    if (!get_expanded_id(id, expId, &context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& multilayer_field = range[m_DETECTORELEMENT_INDEX];
                if (multilayer_field.has_maximum()) {
                    int multilayermax = multilayer_field.get_maximum();
                    if (result < multilayermax) result = multilayermax;
                }
            }
        }
        return result;
    }
    return -999;
}
/*******************************************************************************/
int sTgcIdHelper::multilayerMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext multilayer_context(expId, 0, m_DETECTORELEMENT_INDEX);
    if (!get_expanded_id(id, expId, &multilayer_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& multilayer_field = range[m_DETECTORELEMENT_INDEX];
                if (multilayer_field.has_minimum()) {
                    int multilayermin = multilayer_field.get_minimum();
                    if (-999 == result) {
                        result = multilayermin;
                    } else {
                        if (multilayermin < result) result = multilayermin;
                    }
                }
            }
        }
        return result;
    }
    return 999;  /// default
}
/*******************************************************************************/
int sTgcIdHelper::multilayerMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext multilayer_context(expId, 0, m_DETECTORELEMENT_INDEX);
    if (!get_expanded_id(id, expId, &multilayer_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& multilayer_field = range[m_DETECTORELEMENT_INDEX];
                if (multilayer_field.has_maximum()) {
                    int multilayermax = multilayer_field.get_maximum();
                    if (result < multilayermax) result = multilayermax;
                }
            }
        }
        return result;
    }
    return -999;
}
/*******************************************************************************/
int sTgcIdHelper::gasGapMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext gasgap_context(expId, 0, m_GASGAP_INDEX);
    if (!get_expanded_id(id, expId, &gasgap_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& gasgap_field = range[m_GASGAP_INDEX];
                if (gasgap_field.has_minimum()) {
                    int gasgapmin = gasgap_field.get_minimum();
                    if (-999 == result) {
                        result = gasgapmin;
                    } else {
                        if (gasgapmin < result) result = gasgapmin;
                    }
                }
            }
        }
        return result;
    }
    return 999;
}
/*******************************************************************************/
int sTgcIdHelper::gasGapMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext gasgap_context(expId, 0, m_GASGAP_INDEX);
    if (!get_expanded_id(id, expId, &gasgap_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& gasgap_field = range[m_GASGAP_INDEX];
                if (gasgap_field.has_maximum()) { return (gasgap_field.get_maximum()); }
            }
        }
    }
    // Failed to find the max
    return -999;
}
/*******************************************************************************/
int sTgcIdHelper::channelTypeMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext channeltype_context(expId, 0, m_CHANNELTYPE_INDEX);
    if (!get_expanded_id(id, expId, &channeltype_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& channeltype_field = range[m_CHANNELTYPE_INDEX];
                if (channeltype_field.has_minimum()) {
                    int channeltypemin = channeltype_field.get_minimum();
                    if (-999 == result) {
                        result = channeltypemin;
                    } else {
                        if (channeltypemin < result) result = channeltypemin;
                    }
                }
            }
        }
        return result;
    }
    return 999;
}
/*******************************************************************************/
int sTgcIdHelper::channelTypeMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext channeltype_context(expId, 0, m_CHANNELTYPE_INDEX);
    if (!get_expanded_id(id, expId, &channeltype_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& channeltype_field = range[m_CHANNELTYPE_INDEX];
                if (channeltype_field.has_maximum()) {
                    int channeltypemax = channeltype_field.get_maximum();
                    if (result < channeltypemax) result = channeltypemax;
                }
            }
        }
        return result;
    }
    return -999;
}
/*******************************************************************************/
int sTgcIdHelper::channelMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext channel_context(expId, 0, m_CHANNEL_INDEX);
    if (!get_expanded_id(id, expId, &channel_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& channel_field = range[m_CHANNEL_INDEX];
                if (channel_field.has_minimum()) {
                    int channelmin = channel_field.get_minimum();
                    if (-999 == result) {
                        result = channelmin;
                    } else {
                        if (channelmin < result) result = channelmin;
                    }
                }
            }
        }
        return result;
    }
    return 999;
}
/*******************************************************************************/
int sTgcIdHelper::channelMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext channel_context(expId, 0, m_CHANNEL_INDEX);
    if (!get_expanded_id(id, expId, &channel_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& channel_field = range[m_CHANNEL_INDEX];
                if (channel_field.has_maximum()) {
                    int channelmax = channel_field.get_maximum();
                    if (result < channelmax) result = channelmax;
                }
            }
        }
        return result;
    }
    return -999;
}
/*******************************************************************************/
int sTgcIdHelper::padEta(const Identifier& id) const {
    int ChannelType = channelType(id);
    int PadEta = 0;
    if (ChannelType == Pad) {
        int Channel = channel(id);
        PadEta = ((Channel - 1) % PadEtaMax) + 1;
    }
    return PadEta;
}
/*******************************************************************************/
int sTgcIdHelper::padPhi(const Identifier& id) const {
    int ChannelType = channelType(id);
    int PadPhi = 0;
    if (ChannelType == Pad) {
        int Channel = channel(id);
        PadPhi = ((Channel - 1) / PadEtaMax) + 1;
    }
    return PadPhi;
}
/*******************************************************************************/
// validation of levels
bool sTgcIdHelper::valid(const Identifier& id) const {
    if (!validElement(id)) return false;

    int mplet = multilayer(id);
    if ((mplet < multilayerMin(id)) || (mplet > multilayerMax(id))) {
        ATH_MSG_DEBUG("Invalid multilayer=" << mplet << " multilayerMin=" << multilayerMin(id) << " multilayerMax=" << multilayerMax(id));
        return false;
    }

    int gasG = gasGap(id);
    if (gasG < gasGapMin(id) || gasG > gasGapMax(id)) {
        ATH_MSG_DEBUG("Invalid gasGap=" << gasG << " gasGapMin=" << gasGapMin(id) << " gasGapMax=" << gasGapMax(id));
        return false;
    }

    int channeltype = channelType(id);
    if (channeltype < channelTypeMin(id) || channeltype > channelTypeMax(id)) {
        ATH_MSG_DEBUG("Invalid channelType=" << channeltype << " channelTypeMin=" << channelTypeMin(id)
                                             << " channelTypeMax=" << channelTypeMax(id));
        return false;
    }

    int element = channel(id);
    if (element < channelMin(id) || element > channelMax(id)) {
        ATH_MSG_DEBUG("Invalid channel=" << element << " channelMin=" << channelMin(id) << " channelMax=" << channelMax(id));
        return false;
    }

    if (channeltype == Pad) {
        int PadEta = padEta(id);
        int PadPhi = padPhi(id);
        if (PadEta < PadEtaMin || PadEta > PadEtaMax) {
            ATH_MSG_DEBUG("Invalid padEta=" << PadEta << " padEtaMin=" << PadEtaMin << " padEtaMax=" << PadEtaMax);
            return false;
        }
        if (PadPhi < PadPhiMin || PadPhi > PadPhiMax) {
            ATH_MSG_DEBUG("Invalid padEPhi" << PadPhi << " padPhiMin=" << PadPhiMin << " padPhiMax=" << PadPhiMax);
            return false;
        }
    }

    return true;
}  // end sTgcIdHelper::valid
/*******************************************************************************/
bool sTgcIdHelper::isStNameInTech(const std::string& name) const { return 'S' == name[0] && 'T' == name[1]; }
bool sTgcIdHelper::validElement(const Identifier& id) const {
    int station = stationName(id);
    if (!validStation(station)) {
        ATH_MSG_DEBUG("Invalid stationName=" << stationNameString(station));
        return false;
    }

    int eta = stationEta(id);
    if (eta < stationEtaMin(id) || eta > stationEtaMax(id)) {
        ATH_MSG_DEBUG("Invalid stationEta=" << eta << " for stationName=" << stationNameString(station) << " stationIndex=" << station
                                            << " stationEtaMin=" << stationEtaMin(id) << " stationEtaMax=" << stationEtaMax(id));
        return false;
    }

    int phi = stationPhi(id);
    if (phi < stationPhiMin(id) || phi > stationPhiMax(id)) {
        ATH_MSG_DEBUG("Invalid stationPhi=" << phi << " for stationName=" << stationNameString(station) << " stationIndex=" << station
                                            << " stationPhiMin=" << stationPhiMin(id) << " stationPhiMax=" << stationPhiMax(id));
        return false;
    }
    return true;

}  // end sTgcIdHelper::validElement
/*******************************************************************************/
// Private validation of levels
bool sTgcIdHelper::validElement(const Identifier& id, int stationName, int stationEta, int stationPhi) const {
    if (!validStation(stationName)) {
        ATH_MSG_DEBUG("Invalid stationName=" << stationNameString(stationName));
        return false;
    }
    if (stationEta < stationEtaMin(id) || stationEta > stationEtaMax(id)) {
        ATH_MSG_DEBUG("Invalid stationEta=" << stationEta << " for stationName=" << stationNameString(stationName)
                                            << " stationIndex=" << stationName << " stationEtaMin=" << stationEtaMin(id)
                                            << " stationEtaMax=" << stationEtaMax(id));
        return false;
    }
    if (stationPhi < stationPhiMin(id) || stationPhi > stationPhiMax(id)) {
        ATH_MSG_DEBUG("Invalid stationPhi=" << stationPhi << " for stationName=" << stationNameString(stationName)
                                            << " stationIndex=" << stationName << " stationPhiMin=" << stationPhiMin(id)
                                            << " stationPhiMax=" << stationPhiMax(id));
        return false;
    }
    return true;
}  // end sTgcIdHelper::validElement
/*******************************************************************************/
// Check values down to readout channel level
bool sTgcIdHelper::validChannel(const Identifier& id, int stationName, int stationEta, int stationPhi, int multilayer, int gasGap,
                                int channelType, int channel) const {
    if (!validElement(id, stationName, stationEta, stationPhi)) return false;

    if ((multilayer < multilayerMin(id)) || (multilayer > multilayerMax(id))) {
        ATH_MSG_DEBUG("Invalid multilayer=" << multilayer << " multilayerMin=" << multilayerMin(id)
                                            << " multilayerMax=" << multilayerMax(id));
        return false;
    }

    if (gasGap < gasGapMin(id) || gasGap > gasGapMax(id)) {
        ATH_MSG_DEBUG("Invalid gasGap=" << gasGap << " gasGapMin=" << gasGapMin(id) << " gasGapMax=" << gasGapMax(id));
        return false;
    }
    if (channelType < channelTypeMin(id) || channelType > channelTypeMax(id)) {
        ATH_MSG_DEBUG("Invalid channelType=" << channelType << " channelTypeMin=" << channelTypeMin(id)
                                             << " channelTypeMax=" << channelTypeMax(id));
        return false;
    }
    if (channel < channelMin(id) || channel > channelMax(id)) {
        ATH_MSG_DEBUG("Invalid channel=" << channel << " channelMin=" << channelMin(id) << " channelMax=" << channelMax(id));
        return false;
    }
    if (channelType == Pad) {
        int PadEta = padEta(id);
        int PadPhi = padPhi(id);
        if (PadEta < PadEtaMin || PadEta > PadEtaMax) {
            ATH_MSG_DEBUG("Invalid padEta=" << PadEta << " padEtaMin=" << PadEtaMin << " padEtaMax=" << PadEtaMax);
            return false;
        }
        if (PadPhi < PadPhiMin || PadPhi > PadPhiMax) {
            ATH_MSG_DEBUG("Invalid padEPhi" << PadPhi << " padPhiMin=" << PadPhiMin << " padPhiMax=" << PadPhiMax);
            return false;
        }
    }
    return true;
}  // end sTgcIdHelper::validChannel
/*******************************************************************************/
// Check values down to readout channel level
bool sTgcIdHelper::validChannel(const Identifier& id, int stationName, int stationEta, int stationPhi, int multilayer, int gasGap,
                                int channelType, int padEta, int padPhi) const {
    if (!validElement(id, stationName, stationEta, stationPhi)) return false;

    if ((multilayer < multilayerMin(id)) || (multilayer > multilayerMax(id))) {
        ATH_MSG_DEBUG("Invalid multilayer=" << multilayer << " multilayerMin=" << multilayerMin(id)
                                            << " multilayerMax=" << multilayerMax(id));
        return false;
    }

    if (gasGap < gasGapMin(id) || gasGap > gasGapMax(id)) {
        ATH_MSG_DEBUG("Invalid gasGap=" << gasGap << " gasGapMin=" << gasGapMin(id) << " gasGapMax=" << gasGapMax(id));
        return false;
    }
    if (channelType != Pad) {
        ATH_MSG_DEBUG("Invalid channelType=" << channelType << " channelType must be " << Pad << " when specifying padEta and padPhi");
        return false;
    }
    int channel = (padPhi - 1) * PadEtaMax + padEta;
    if (channel < channelMin(id) || channel > channelMax(id)) {
        ATH_MSG_DEBUG("Invalid channel=" << channel << " channelMin=" << channelMin(id) << " channelMax=" << channelMax(id)
                                         << " for given padEta=" << padEta << " and padPhi=" << padPhi);
        return false;
    }
    if (padEta < PadEtaMin || padEta > PadEtaMax) {
        ATH_MSG_DEBUG("Invalid padEta=" << padEta << " padEtaMin=" << PadEtaMin << " padEtaMax=" << PadEtaMax);
        return false;
    }
    if (padPhi < PadPhiMin || padPhi > PadPhiMax) {
        ATH_MSG_DEBUG("Invalid padEPhi" << padPhi << " padPhiMin=" << PadPhiMin << " padPhiMax=" << PadPhiMax);
        return false;
    }
    return true;
}  // end sTgcIdHelper::validChannel
   /*******************************************************************************/
   // Construct ID from components
Identifier sTgcIdHelper::elementID(int stationName, int stationEta, int stationPhi) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(stgc_field_value(), result);
    return result;
}
Identifier sTgcIdHelper::elementID(int stationName, int stationEta, int stationPhi, bool& isValid) const {
    try {
        const Identifier result = elementID(stationName, stationEta, stationPhi);
        isValid = validElement(result, stationName, stationEta, stationPhi);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

/*******************************************************************************/
Identifier sTgcIdHelper::elementID(const std::string& stationNameStr, int stationEta, int stationPhi, bool& isValid) const {
    return elementID(stationNameIndex(stationNameStr), stationEta, stationPhi, isValid);
}
Identifier sTgcIdHelper::elementID(const std::string& stationNameStr, int stationEta, int stationPhi) const {
    return elementID(stationNameIndex(stationNameStr), stationEta, stationPhi);
}

/*******************************************************************************/
Identifier sTgcIdHelper::elementID(const Identifier& id) const { return parentID(id); }
/*******************************************************************************/
Identifier sTgcIdHelper::channelID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType,
                                   int channel) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(stgc_field_value(), result);
    m_mplet_impl.pack(multilayer, result);
    m_gap_impl.pack(gasGap, result);
    m_typ_impl.pack(channelType, result);
    m_cha_impl.pack(channel, result);

    return result;
}
Identifier sTgcIdHelper::channelID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType,
                                   int channel, bool& isValid) const {
    try {
        const Identifier result = channelID(stationName, stationEta, stationPhi, multilayer, gasGap, channelType, channel);
        isValid = validChannel(result, stationName, stationEta, stationPhi, multilayer, gasGap, channelType, channel);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }    
    return Identifier{0};
}
/*******************************************************************************/
Identifier sTgcIdHelper::channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap,
                                   int channelType, int channel) const {
    return channelID(stationNameIndex(stationNameStr), stationEta, stationPhi, multilayer, gasGap, channelType, channel);
}
Identifier sTgcIdHelper::channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap,
                                   int channelType, int channel, bool& isValid) const {
    return channelID(stationNameIndex(stationNameStr), stationEta, stationPhi, multilayer, gasGap, channelType, channel, isValid);
}

/*******************************************************************************/
Identifier sTgcIdHelper::channelID(const Identifier& id, int multilayer, int gasGap, int channelType, int channel) const {
    Identifier result = parentID(id);
    resetAndSet(m_mplet_impl, multilayer, result);
    resetAndSet(m_gap_impl, gasGap, result);
    resetAndSet(m_typ_impl, channelType, result);
    resetAndSet(m_cha_impl, channel, result);
    return result;
}
Identifier sTgcIdHelper::channelID(const Identifier& id, int multilayer, int gasGap, int channelType, int channel, bool& isValid) const {
    try{
        const Identifier result = channelID(id, multilayer, gasGap, channelType, channel);
        isValid = valid(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
/*******************************************************************************/
Identifier sTgcIdHelper::padID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType, int padEta,
                               int padPhi) const {
    int channel = -1;
    if (channelType == Pad) {
        channel = (padPhi - 1) * PadEtaMax + padEta;
    } else {
        return Identifier{-1};
    }
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(stgc_field_value(), result);
    m_mplet_impl.pack(multilayer, result);
    m_gap_impl.pack(gasGap, result);
    m_typ_impl.pack(channelType, result);

    m_cha_impl.pack(channel, result);

    return result;
}
Identifier sTgcIdHelper::padID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channelType, int padEta,
                               int padPhi, bool& isValid) const {
    try {
        const Identifier result = padID(stationName, stationEta, stationPhi, multilayer, gasGap, channelType, padEta, padPhi);
        isValid = (channelType) == Pad &&
                  validChannel(result, stationName, stationEta, stationPhi, multilayer, gasGap, channelType, padEta, padPhi);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
/*******************************************************************************/
Identifier sTgcIdHelper::padID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap,
                               int channelType, int padEta, int padPhi) const {
    return padID(stationNameIndex(stationNameStr), stationEta, stationPhi, multilayer, gasGap, channelType, padEta, padPhi);
}

Identifier sTgcIdHelper::padID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap,
                               int channelType, int padEta, int padPhi, bool& isValid) const {
    return padID(stationNameIndex(stationNameStr), stationEta, stationPhi, multilayer, gasGap, channelType, padEta, padPhi, isValid);
}

/*******************************************************************************/
Identifier sTgcIdHelper::padID(const Identifier& id, int multilayer, int gasGap, int channelType, int padEta, int padPhi) const {
    Identifier result(id);
    if (channelType != Pad) return result;
    resetAndSet(m_mplet_impl, multilayer, result);
    resetAndSet(m_gap_impl, gasGap, result);
    resetAndSet(m_typ_impl, channelType, result);
    int channel = (padPhi - 1) * PadEtaMax + padEta;
    resetAndSet(m_cha_impl, channel, result);

    return result;
}
Identifier sTgcIdHelper::padID(const Identifier& id, int multilayer, int gasGap, int channelType, int padEta, int padPhi,
                               bool& isValid) const {
    try {
        const Identifier result = padID(id, multilayer, gasGap, channelType, padEta, padPhi);
        isValid = (channelType == Pad) && valid(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
/*******************************************************************************/
// get parent id from strip or gang identifier
Identifier sTgcIdHelper::parentID(const Identifier& id) const {
    assert(is_stgc(id));
    Identifier result(id);
    m_mplet_impl.reset(result);
    m_gap_impl.reset(result);
    m_typ_impl.reset(result);
    m_cha_impl.reset(result);
    return result;
}
/*******************************************************************************/
// Access to components of the ID
int sTgcIdHelper::multilayer(const Identifier& id) const { return m_mplet_impl.unpack(id); }
/*******************************************************************************/
// Access to components of the ID
int sTgcIdHelper::gasGap(const Identifier& id) const { return m_gap_impl.unpack(id); }
/*******************************************************************************/
int sTgcIdHelper::channelType(const Identifier& id) const { return m_typ_impl.unpack(id); }
/*******************************************************************************/
/** returns measuresPhi */
bool sTgcIdHelper::measuresPhi(const Identifier& id) const { return (channelType(id) == Strip) ? 0 : 1; }
/*******************************************************************************/
int sTgcIdHelper::channel(const Identifier& id) const { return m_cha_impl.unpack(id); }
/*******************************************************************************/
// Access to min and max of level ranges
int sTgcIdHelper::stationEtaMin() const { return StationEtaMin; }
/*******************************************************************************/
int sTgcIdHelper::stationEtaMax() const { return StationEtaMax; }
/*******************************************************************************/
int sTgcIdHelper::stationPhiMin() const { return StationPhiMin; }
/*******************************************************************************/
int sTgcIdHelper::stationPhiMax() const { return StationPhiMax; }
/*******************************************************************************/
int sTgcIdHelper::multilayerMin() const { return MultilayerMin; }
/*******************************************************************************/
int sTgcIdHelper::multilayerMax() const { return MultilayerMax; }
/*******************************************************************************/
int sTgcIdHelper::gasGapMin() const { return GasGapMin; }
/*******************************************************************************/
int sTgcIdHelper::gasGapMax() const { return GasGapMax; }
/*******************************************************************************/
int sTgcIdHelper::channelTypeMin() const { return ChannelTypeMin; }
/*******************************************************************************/
int sTgcIdHelper::channelTypeMax() const { return ChannelTypeMax; }
/*******************************************************************************/
int sTgcIdHelper::channelMin() const { return ChannelMin; }
/*******************************************************************************/
int sTgcIdHelper::channelMax() const { return ChannelMax; }
/*******************************************************************************/
int sTgcIdHelper::padEtaMin() const { return PadEtaMin; }
/*******************************************************************************/
int sTgcIdHelper::padEtaMax() const { return PadEtaMax; }
/*******************************************************************************/
int sTgcIdHelper::padPhiMin() const { return PadPhiMin; }
/*******************************************************************************/
int sTgcIdHelper::padPhiMax() const { return PadPhiMax; }
/*******************************************************************************/
/// Utility methods
int sTgcIdHelper::stgcTechnology() const {
    int stgcField = technologyIndex("STGC");
    if (m_dict) { stgcField = stgc_field_value(); }
    return stgcField;
}
/*******************************************************************************/
bool sTgcIdHelper::LargeSector(int stationName) const { return ('L' == stationNameString(stationName)[2]); }
/*******************************************************************************/
bool sTgcIdHelper::SmallSector(int stationName) const { return ('S' == stationNameString(stationName)[2]); }

/*******************************************************************************/