/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonIdHelpers/RpcIdHelper.h"
RpcIdHelper::RpcIdHelper() : MuonIdHelper("RpcIdHelper") {}

// Initialize dictionary
int RpcIdHelper::initialize_from_dictionary(const IdDictMgr& dict_mgr) {
    int status = 0;
    constexpr int detHashSize = sizeof(m_detectorElement_hashes) / sizeof(unsigned int);
    constexpr int modHashSize = sizeof(m_module_hashes) / sizeof(unsigned int);
    for (int h = 0; h < detHashSize ; ++h) {
        unsigned int* e = &(m_detectorElement_hashes[0][0][0][0][0])+ h;
        (*e) = -1;
    }
    for (int h = 0; h < modHashSize ; ++h) {
        unsigned int* e = &(m_module_hashes[0][0][0][0])+ h;
        (*e) = -1;
    }

    // Check whether this helper should be reinitialized
    if (!reinitialize(dict_mgr)) {
        ATH_MSG_INFO("Request to reinitialize not satisfied - tags have not changed");
        return (0);
    } else {
        ATH_MSG_DEBUG("(Re)initialize");
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

    IdDictField* field = m_dict->find_field("doubletR");
    if (field) {
        m_DOUBLETR_INDEX = field->m_index;
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'doubletR' field ");
        status = 1;
    }

    field = m_dict->find_field("doubletZ");
    if (field) {
        m_DOUBLETZ_INDEX = field->m_index;
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'doubletZ' field ");
        status = 1;
    }

    field = m_dict->find_field("doubletPhi");
    if (field) {
        m_DOUBLETPHI_INDEX = field->m_index;
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'doubletPhi' field ");
        status = 1;
    }

    field = m_dict->find_field("rpcGasGap");
    if (field) {
        m_GASGAP_INDEX = field->m_index;
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'rpcGasGap' field ");
        status = 1;
    }

    field = m_dict->find_field("rpcMeasuresPhi");
    if (field) {
        m_MEASURESPHI_INDEX = field->m_index;
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'rpcMeasuresPhi' field ");
        status = 1;
    }

    field = m_dict->find_field("rpcStrip");
    if (field) {
        m_CHANNEL_INDEX = field->m_index;
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'rpcStrip' field ");
        status = 1;
    }

    // reinitialze the module context
    m_MODULE_INDEX = m_DOUBLETR_INDEX;
    m_DETECTORELEMENT_INDEX = m_DOUBLETZ_INDEX;

    // save an index to the first region of rpc
    IdDictGroup* rpcGroup = m_dict->find_group("rpc");
    if (!rpcGroup) {
        ATH_MSG_ERROR("Cannot find rpc group");
    } else {
        m_GROUP_INDEX = rpcGroup->regions()[0]->m_index;
    }

    const IdDictRegion& region = *m_dict->m_regions[m_GROUP_INDEX];
    m_eta_impl = region.m_implementation[m_ETA_INDEX];
    m_phi_impl = region.m_implementation[m_PHI_INDEX];
    m_tec_impl = region.m_implementation[m_TECHNOLOGY_INDEX];
    m_dbr_impl = region.m_implementation[m_DOUBLETR_INDEX];
    m_dbz_impl = region.m_implementation[m_DOUBLETZ_INDEX];
    m_dbp_impl = region.m_implementation[m_DOUBLETPHI_INDEX];
    m_gap_impl = region.m_implementation[m_GASGAP_INDEX];
    m_mea_impl = region.m_implementation[m_MEASURESPHI_INDEX];
    m_str_impl = region.m_implementation[m_CHANNEL_INDEX];

    ATH_MSG_DEBUG(" RPC decode index and bit fields for each level: " << std::endl
                                                                      << " muon        " << m_muon_impl.show_to_string() << std::endl
                                                                      << " station     " << m_sta_impl.show_to_string() << std::endl
                                                                      << " eta         " << m_eta_impl.show_to_string() << std::endl
                                                                      << " phi         " << m_phi_impl.show_to_string() << std::endl
                                                                      << " technology  " << m_tec_impl.show_to_string() << std::endl
                                                                      << " TR          " << m_dbr_impl.show_to_string() << std::endl
                                                                      << " TZ          " << m_dbz_impl.show_to_string() << std::endl
                                                                      << " TPHI        " << m_dbp_impl.show_to_string() << std::endl
                                                                      << " gas gap     " << m_gap_impl.show_to_string() << std::endl
                                                                      << " phi         " << m_mea_impl.show_to_string() << std::endl
                                                                      << " strip       " << m_str_impl.show_to_string());

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

    // Build MultiRange down to "doubletR" for all (muon) regions
    ExpandedIdentifier region_id;
    region_id.add(muonField);
    Range prefix;
    MultiRange muon_range = m_dict->build_multirange(region_id, prefix, "doubletR");

    if (muon_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to doubletR: "
                     << "MultiRange size is " << muon_range.size());
    } else {
        ATH_MSG_ERROR("Muon MultiRange is empty");
    }

    // Build MultiRange down to "detectorElement" for all mdt regions

    ExpandedIdentifier detectorElement_region;
    detectorElement_region.add(muonField);
    Range detectorElement_prefix;
    MultiRange muon_detectorElement_range = m_dict->build_multirange(detectorElement_region, detectorElement_prefix, "doubletPhi");
    if (muon_detectorElement_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to detectorElement: "
                     << "DetectorElement MultiRange size is " << muon_detectorElement_range.size());
    } else {
        ATH_MSG_ERROR("Muon RPC ReadoutElement MultiRange is empty");
        return 1;
    }

    // Build MultiRange down to "rpcStrip" for all rpc regions
    ExpandedIdentifier rpc_region;
    rpc_region.add(muonField);
    Range rpc_prefix;
    MultiRange muon_channel_range = m_dict->build_multirange(rpc_region, rpc_prefix, "rpcStrip");

    if (muon_channel_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to rpcStrip: "
                     << "MultiRange size is " << muon_channel_range.size());
    } else {
        ATH_MSG_ERROR("Muon RPC channel MultiRange is empty");
        return 1;
    }

    // build RPC module ranges
    // Find the regions that have a "RPC doubletR field" and save them
    int rpcField = -1;
    status = m_dict->get_label_value("technology", "RPC", rpcField);

    for (int i = 0; i < (int)muon_range.size(); ++i) {
        const Range& range = muon_range[i];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)rpcField)) {
                m_full_module_range.add(range);
                ATH_MSG_DEBUG("field size is " << (int)range.cardinality() << " field index = " << i);
            }
        }
    }

    for (int j = 0; j < (int)muon_detectorElement_range.size(); ++j) {
        const Range& range = muon_detectorElement_range[j];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)rpcField)) {
                m_full_detectorElement_range.add(range);
                ATH_MSG_DEBUG("detectorElement field size is " << (int)range.cardinality() << " field index = " << j);
            }
        }
    }

    for (int j = 0; j < (int)muon_channel_range.size(); ++j) {
        const Range& range = muon_channel_range[j];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)rpcField)) {
                m_full_channel_range.add(range);
                ATH_MSG_DEBUG("channel field size is " << (int)range.cardinality() << " field index = " << j);
            }
        }
    }

    // test to see that the multi range is not empty
    if (m_full_module_range.size() == 0) {
        ATH_MSG_ERROR("RPC MultiRange ID is empty for modules");
        status = 1;
    } else {
        ATH_MSG_DEBUG(" full module range size is " << m_full_module_range.size());
    }

    /// test to see that the detectorElement multi range is not empty
    if (m_full_detectorElement_range.size() == 0) {
        ATH_MSG_ERROR("MDT MultiRange ID is empty for detector elements");
        status = 1;
    }

    // test to see that the multi range is not empty
    if (m_full_channel_range.size() == 0) {
        ATH_MSG_ERROR("RPC MultiRange ID is empty for channels");
        status = 1;
    } else {
        ATH_MSG_DEBUG(" full channel range size is " << m_full_channel_range.size());
    }

    // Setup the hash tables for RPC
    ATH_MSG_INFO("Initializing RPC hash indices ... ");
    status = init_hashes();
    status = init_detectorElement_hashes();  // doubletZ
    status = init_id_to_hashes();

    // Setup hash tables for finding neighbors
    ATH_MSG_INFO("Initializing RPC hash indices for finding neighbors ... ");
    status = init_neighbors();

    // retrieve the maximum number of gas gaps
    ExpandedIdentifier expId;
    IdContext gasGap_context(expId, 0, m_GASGAP_INDEX);
    for (const auto& id : m_detectorElement_vec) {  // channel Identifiers not filled for RPCs, thus using detector element ones
        if (!get_expanded_id(id, expId, &gasGap_context)) {
            for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
                const Range& range = m_full_channel_range[i];
                if (range.match(expId)) {
                    const Range::field& gap_field = range[m_GASGAP_INDEX];
                    if (gap_field.has_maximum()) {
                        unsigned int max = (gap_field.get_maximum());
                        if (m_gasGapMax == UINT_MAX)
                            m_gasGapMax = max;
                        else if (max > m_gasGapMax)
                            m_gasGapMax = max;
                    }
                }
            }
        }
    }
    if (m_gasGapMax == UINT_MAX) {
        ATH_MSG_ERROR("No maximum number of RPC gas gaps was retrieved");
        status = 1;
    } else {
        ATH_MSG_DEBUG(" Maximum number of RPC gas gaps is " << m_gasGapMax);
    }
    m_init = true;
    return (status);
}

int RpcIdHelper::init_id_to_hashes() {
    unsigned int hash_max = module_hash_max();
    for (unsigned int i = 0; i < hash_max; ++i) {
        Identifier id = m_module_vec[i];
        int station = stationName(id);
        int eta = stationEta(id) + 10;  // for negative etas
        int phi = stationPhi(id);
        int dR = doubletR(id);
        m_module_hashes[station][eta - 1][phi - 1][dR - 1] = i;
    }

    hash_max = detectorElement_hash_max();
    for (unsigned int i = 0; i < hash_max; ++i) {
        Identifier id = m_detectorElement_vec[i];
        int station = stationName(id);
        int eta = stationEta(id) + 10;  // for negative eta
        int phi = stationPhi(id);
        int dR = doubletR(id);
        int zIdx = zIndex(id);
        m_detectorElement_hashes[station][eta - 1][phi - 1][dR - 1][zIdx - 1] = i;
    }
    return 0;
}

int RpcIdHelper::get_module_hash(const Identifier& id, IdentifierHash& hash_id) const {
    // Identifier moduleId = elementID(id);
    // IdContext context = module_context();
    // return get_hash(moduleId,hash_id,&context);
    int station = stationName(id);
    int eta = stationEta(id) + 10;  // for negative etas
    int phi = stationPhi(id);
    int dR = doubletR(id);
    hash_id = m_module_hashes[station][eta - 1][phi - 1][dR - 1];
    return 0;
}

int RpcIdHelper::get_detectorElement_hash(const Identifier& id, IdentifierHash& hash_id) const {
    // Identifier detectorElementId = detectorElementID(id);
    // IdContext context = detectorElement_context();
    // return get_hash(detectorElementId,hash_id,&context);
    int station = stationName(id);
    int eta = stationEta(id) + 10;  // for negative eta
    int phi = stationPhi(id);
    int dR = doubletR(id);
    int zIdx = zIndex(id);
    hash_id = m_detectorElement_hashes[station][eta - 1][phi - 1][dR - 1][zIdx - 1];
    return 0;
}

void RpcIdHelper::idChannels(const Identifier& id, std::vector<Identifier>& vect) const {
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

// Access to min and max of level ranges

int RpcIdHelper::stationEtaMin(const Identifier& id) const {
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
        return (result);
    }
    return 999;  // default
}

int RpcIdHelper::stationEtaMax(const Identifier& id) const {
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
        return (result);
    }
    return -999;
}

int RpcIdHelper::stationPhiMin(const Identifier& id) const {
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

int RpcIdHelper::stationPhiMax(const Identifier& id) const {
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

int RpcIdHelper::doubletRMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext doubletR_context(expId, 0, m_DOUBLETR_INDEX);
    if (!get_expanded_id(id, expId, &doubletR_context)) {
        for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
            const Range& range = m_full_module_range[i];
            if (range.match(expId)) {
                const Range::field& r_field = range[m_DOUBLETR_INDEX];
                if (r_field.has_minimum()) { return (r_field.get_minimum()); }
            }
        }
    }
    // Failed to find the min
    return 999;
}

int RpcIdHelper::doubletRMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext doubletR_context(expId, 0, m_DOUBLETR_INDEX);
    if (!get_expanded_id(id, expId, &doubletR_context)) {
        for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
            const Range& range = m_full_module_range[i];
            if (range.match(expId)) {
                const Range::field& r_field = range[m_DOUBLETR_INDEX];
                if (r_field.has_maximum()) { return (r_field.get_maximum()); }
            }
        }
    }
    // Failed to find the max
    return -999;
}

int RpcIdHelper::doubletZMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext doubletZ_context(expId, 0, m_DOUBLETZ_INDEX);
    if (!get_expanded_id(id, expId, &doubletZ_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& z_field = range[m_DOUBLETZ_INDEX];
                if (z_field.has_minimum()) { return (z_field.get_minimum()); }
            }
        }
    }
    // Failed to find the min
    return 999;
}

int RpcIdHelper::doubletZMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext doubletZ_context(expId, 0, m_DOUBLETZ_INDEX);
    if (!get_expanded_id(id, expId, &doubletZ_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& z_field = range[m_DOUBLETZ_INDEX];
                if (z_field.has_maximum()) { return (z_field.get_maximum()); }
            }
        }
    }
    // Failed to find the max
    return -999;
}

int RpcIdHelper::doubletPhiMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext doubletPhi_context(expId, 0, m_DOUBLETPHI_INDEX);
    if (!get_expanded_id(id, expId, &doubletPhi_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& dphi_field = range[m_DOUBLETPHI_INDEX];
                if (dphi_field.has_minimum()) { return (dphi_field.get_minimum()); }
            }
        }
    }
    // Failed to find the min
    return 999;
}

int RpcIdHelper::doubletPhiMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext doubletPhi_context(expId, 0, m_DOUBLETPHI_INDEX);
    if (!get_expanded_id(id, expId, &doubletPhi_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& dphi_field = range[m_DOUBLETPHI_INDEX];
                if (dphi_field.has_maximum()) { return (dphi_field.get_maximum()); }
            }
        }
    }
    // Failed to find the max
    return -999;
}

int RpcIdHelper::gasGapMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext gasGap_context(expId, 0, m_GASGAP_INDEX);
    if (!get_expanded_id(id, expId, &gasGap_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& gas_field = range[m_GASGAP_INDEX];
                if (gas_field.has_minimum()) { return (gas_field.get_minimum()); }
            }
        }
    }
    // Failed to find the min
    return 999;
}

int RpcIdHelper::gasGapMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext gasGap_context(expId, 0, m_GASGAP_INDEX);
    if (!get_expanded_id(id, expId, &gasGap_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& gap_field = range[m_GASGAP_INDEX];
                if (gap_field.has_maximum()) { return (gap_field.get_maximum()); }
            }
        }
    }
    // Failed to find the max
    return -999;
}

int RpcIdHelper::measuresPhiMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext measuresPhi_context(expId, 0, m_MEASURESPHI_INDEX);
    if (!get_expanded_id(id, expId, &measuresPhi_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& mphi_field = range[m_MEASURESPHI_INDEX];
                if (mphi_field.has_minimum()) { return (mphi_field.get_minimum()); }
            }
        }
    }
    // Failed to find the min
    return 999;
}

int RpcIdHelper::measuresPhiMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext measuresPhi_context(expId, 0, m_MEASURESPHI_INDEX);
    if (!get_expanded_id(id, expId, &measuresPhi_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& mphi_field = range[m_MEASURESPHI_INDEX];
                if (mphi_field.has_maximum()) { return (mphi_field.get_maximum()); }
            }
        }
    }
    // Failed to find the max
    return -999;
}

int RpcIdHelper::stripMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext strip_context(expId, 0, m_CHANNEL_INDEX);
    if (!get_expanded_id(id, expId, &strip_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& strip_field = range[m_CHANNEL_INDEX];
                if (strip_field.has_minimum()) { return (strip_field.get_minimum()); }
            }
        }
    }
    // Failed to find the min
    return 999;
}

int RpcIdHelper::stripMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext strip_context(expId, 0, m_CHANNEL_INDEX);
    if (!get_expanded_id(id, expId, &strip_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& strip_field = range[m_CHANNEL_INDEX];
                if (strip_field.has_maximum()) { return (strip_field.get_maximum()); }
            }
        }
    }
    // Failed to find the max
    return -999;
}

// Public validation of levels

bool RpcIdHelper::valid(const Identifier& id) const {
    if (!validElement(id)) return false;

    int dbz = doubletZ(id);
    if ((dbz < doubletZMin(id)) || (dbz > doubletZMax(id))) {
        ATH_MSG_DEBUG("Invalid doubletZ=" << dbz << " doubletZMin=" << doubletZMin(id) << " doubletZMax=" << doubletZMax(id));
        return false;
    }

    int dbp = doubletPhi(id);
    if ((dbp < doubletPhiMin(id)) || (dbp > doubletPhiMax(id))) {
        ATH_MSG_DEBUG("Invalid doubletPhi=" << dbp << " doubletPhiMin=" << doubletPhiMin(id) << " doubletPhiMax=" << doubletPhiMax(id));
        return false;
    }

    int gasG = gasGap(id);
    if ((gasG < gasGapMin(id)) || (gasG > gasGapMax(id))) {
        ATH_MSG_DEBUG("Invalid gasGap=" << gasG << " gasGapMin=" << gasGapMin(id) << " gasGapMax=" << gasGapMax(id));
        return false;
    }

    int mPhi = measuresPhi(id);
    if ((mPhi < measuresPhiMin(id)) || (mPhi > measuresPhiMax(id))) {
        ATH_MSG_DEBUG("Invalid measuresPhi=" << mPhi << " measuresPhiMin=" << measuresPhiMin(id)
                                             << " measuresPhiMax=" << measuresPhiMax(id));
        return false;
    }

    int str = strip(id);
    if ((str < stripMin(id)) || (str > stripMax(id))) {
        ATH_MSG_DEBUG("Invalid strip=" << str << " stripMin=" << stripMin(id) << " stripMax=" << stripMax(id));
        return false;
    }
    return true;
}
bool RpcIdHelper::isStNameInTech(const std::string& stationName) const { return stationName[0] == 'B'; }
bool RpcIdHelper::validElement(const Identifier& id) const {
    int station = stationName(id);
    if (!validStation(station)) {
        ATH_MSG_DEBUG("Invalid stationName=" << stationNameString(station));
        return false;
    }

    int eta = stationEta(id);
    if (eta < stationEtaMin(id) || eta > stationEtaMax(id)) {
        ATH_MSG_DEBUG("Invalid stationEta=" << eta << " for stationName=" << stationNameString(station)
                                            << " stationEtaMin=" << stationEtaMin(id) << " stationEtaMax=" << stationEtaMax(id));
        return false;
    }

    int phi = stationPhi(id);
    if ((phi < stationPhiMin(id)) || (phi > stationPhiMax(id))) {
        ATH_MSG_DEBUG("Invalid stationPhi=" << phi << " for stationName=" << stationNameString(station)
                                            << " stationPhiMin=" << stationPhiMin(id) << " stationPhiMax=" << stationPhiMax(id));
        return false;
    }

    int dbr = doubletR(id);
    if ((dbr < doubletRMin(id)) || (dbr > doubletRMax(id))) {
        ATH_MSG_DEBUG("Invalid doubletR=" << dbr << " for stationName=" << stationNameString(station) << " doubletRMin=" << doubletRMin(id)
                                          << " doubletRMax=" << doubletRMax(id));
        return false;
    }
    return true;
}

bool RpcIdHelper::validPad(const Identifier& id) const {
    if (!validElement(id)) return false;

    int dbz = doubletZ(id);
    if ((dbz < doubletZMin(id)) || (dbz > doubletZMax(id))) {
        ATH_MSG_DEBUG("Invalid doubletZ=" << dbz << " doubletZMin=" << doubletZMin(id) << " doubletZMax=" << doubletZMax(id));
        return false;
    }

    int dbp = doubletPhi(id);
    if ((dbp < doubletPhiMin(id)) || (dbp > doubletPhiMax(id))) {
        ATH_MSG_DEBUG("Invalid doubletPhi=" << dbp << " doubletPhiMin=" << doubletPhiMin(id) << " doubletPhiMax=" << doubletPhiMax(id));
        return false;
    }
    return true;
}

// Private validation of levels

bool RpcIdHelper::validElement(const Identifier& id, int stationName, int stationEta, int stationPhi, int doubletR) const {
    if (!validStation(stationName)) {
        ATH_MSG_VERBOSE("Invalid stationName=" << stationNameString(stationName));
        return false;
    }
    if (stationEta < stationEtaMin(id) || stationEta > stationEtaMax(id)) {
        ATH_MSG_VERBOSE("Invalid stationEta=" << stationEta << " for stationName=" << stationNameString(stationName)
                                              << " stationEtaMin=" << stationEtaMin(id) << " stationEtaMax=" << stationEtaMax(id));
        return false;
    }
    if ((stationPhi < stationPhiMin(id)) || (stationPhi > stationPhiMax(id))) {
        ATH_MSG_VERBOSE("Invalid stationPhi=" << stationPhi << " for stationName=" << stationNameString(stationName)
                                              << " stationPhiMin=" << stationPhiMin(id) << " stationPhiMax=" << stationPhiMax(id));
        return false;
    }
    if ((doubletR < doubletRMin(id)) || (doubletR > doubletRMax(id))) {
        ATH_MSG_VERBOSE("Invalid doubletR=" << doubletR << " for stationName=" << stationNameString(stationName)
                                            << " doubletRMin=" << doubletRMin(id) << " doubletRMax=" << doubletRMax(id));
        return false;
    }
    return true;
}

// Check values down to detector element level

bool RpcIdHelper::validChannel(const Identifier& id, int stationName, int stationEta, int stationPhi, int doubletR, int doubletZ,
                               int doubletPhi, int gasGap, int measuresPhi, int strip) const {
    if (!validElement(id, stationName, stationEta, stationPhi, doubletR)) return false;

    if ((doubletZ < doubletZMin(id)) || (doubletZ > doubletZMax(id))) {
        ATH_MSG_VERBOSE("Invalid doubletZ=" << doubletZ << " doubletZMin=" << doubletZMin(id) << " doubletZMax=" << doubletZMax(id));
        return false;
    }
    if ((doubletPhi < doubletPhiMin(id)) || (doubletPhi > doubletPhiMax(id))) {
        ATH_MSG_VERBOSE("Invalid doubletPhi=" << doubletPhi << " doubletPhiMin=" << doubletPhiMin(id)
                                              << " doubletPhiMax=" << doubletPhiMax(id));
        return false;
    }
    if ((gasGap < gasGapMin(id)) || (gasGap > gasGapMax(id))) {
        ATH_MSG_VERBOSE("Invalid gasGap=" << gasGap << " gasGapMin=" << gasGapMin(id) << " gasGapMax=" << gasGapMax(id));
        return false;
    }
    if ((measuresPhi < measuresPhiMin(id)) || (measuresPhi > measuresPhiMax(id))) {
        ATH_MSG_VERBOSE("Invalid measuresPhi=" << measuresPhi << " measuresPhiMin=" << measuresPhiMin(id)
                                               << " measuresPhiMax=" << measuresPhiMax(id));
        return false;
    }
    if ((strip < stripMin(id)) || (strip > stripMax(id))) {
        ATH_MSG_VERBOSE("Invalid strip=" << strip << " stripMin=" << stripMin(id) << " stripMax=" << stripMax(id));
        return false;
    }
    return true;
}

// Check values down to the pad

bool RpcIdHelper::validPad(const Identifier& id, int stationName, int stationEta, int stationPhi, int doubletR, int doubletZ,
                           int doubletPhi) const {
    if (!validElement(id, stationName, stationEta, stationPhi, doubletR)) return false;

    if ((doubletZ < doubletZMin(id)) || (doubletZ > doubletZMax(id))) {
        ATH_MSG_DEBUG("Invalid doubletZ=" << doubletZ << " doubletZMin=" << doubletZMin(id) << " doubletZMax=" << doubletZMax(id));
        return false;
    }
    if ((doubletPhi < doubletPhiMin(id)) || (doubletPhi > doubletPhiMax(id))) {
        ATH_MSG_DEBUG("Invalid doubletPhi=" << doubletPhi << " doubletPhiMin=" << doubletPhiMin(id)
                                            << " doubletPhiMax=" << doubletPhiMax(id));
        return false;
    }
    return true;
}

int RpcIdHelper::init_detectorElement_hashes(void) {
    //
    // create a vector(s) to retrieve the hashes for compact ids. For
    // the moment, we implement a hash for detector channels
    //

    // detector element hash
    IdContext context = detectorElement_context();
    unsigned int nids = 0;
    std::set<Identifier> ids;
    for (unsigned int i = 0; i < m_full_detectorElement_range.size(); ++i) {
        const Range& range = m_full_detectorElement_range[i];
        Range::const_identifier_factory first = range.factory_begin();
        Range::const_identifier_factory last = range.factory_end();
        for (; first != last; ++first) {
            Identifier id;
            get_id((*first), id);
            Identifier doubletZ_id = doubletZID(id);
            int dZ = doubletZ(id);
            int corrected_doubletZ = zIndex(id);
            bool isInserted = false;
            if (dZ == corrected_doubletZ) {
                isInserted = ids.insert(doubletZ_id).second;
                if (!isInserted)
                    ATH_MSG_DEBUG("init_detectorElement_hashes "
                                  << "Please check the dictionary for possible duplication for " << id);
            } else {
                isInserted = ids.insert(id).second;
                if (!isInserted) {
                    ATH_MSG_ERROR("init_detectorElement_hashes "
                                  << " Error: duplicated id for detector element id. nid " << (int)nids << " doubletPhi ID " << id);
                    return 1;
                }
            }
            nids++;
        }
    }
    m_detectorElement_hash_max = ids.size();
    ATH_MSG_INFO("The detector element hash max is " << (int)m_detectorElement_hash_max);
    m_detectorElement_vec.resize(m_detectorElement_hash_max);

    nids = 0;
    std::set<Identifier>::const_iterator first = ids.begin();
    std::set<Identifier>::const_iterator last = ids.end();
    for (; first != last && nids < m_detectorElement_vec.size(); ++first) {
        m_detectorElement_vec[nids] = (*first);
        nids++;
    }

    return (0);
}

Identifier RpcIdHelper::elementID(int stationName, int stationEta, int stationPhi, int doubletR) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(rpc_field_value(), result);
    m_dbr_impl.pack(doubletR, result);
    return result;
}

Identifier RpcIdHelper::elementID(int stationName, int stationEta, int stationPhi, int doubletR, bool& isValid) const {
    try {
        const Identifier result = elementID(stationName, stationEta, stationPhi, doubletR);
        isValid = validElement(result, stationName, stationEta, stationPhi, doubletR);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

Identifier RpcIdHelper::elementID(const std::string& stationNameStr, int stationEta, int stationPhi, int doubletR) const {
    return elementID(stationNameIndex(stationNameStr), stationEta, stationPhi, doubletR);
}
Identifier RpcIdHelper::elementID(const std::string& stationNameStr, int stationEta, int stationPhi, int doubletR, bool& isValid) const {
    return elementID(stationNameIndex(stationNameStr), stationEta, stationPhi, doubletR, isValid);
}

Identifier RpcIdHelper::elementID(const Identifier& id, int doubletR) const {
    Identifier result(id);
    resetAndSet(m_dbr_impl, doubletR, result);
    return result;
}

Identifier RpcIdHelper::elementID(const Identifier& id, int doubletR, bool& isValid) const {
    try {
        const Identifier result = elementID(id, doubletR);
        isValid = validElement(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
Identifier RpcIdHelper::elementID(const Identifier& id) const { return parentID(id); }

/*     Identifier panelID  (const Identifier& padID, int gasGap,) const; */
/*     Identifier panelID  (const Identifier& channelID) const; */
/*     Identifier panelID  (int stationName, int stationEta, int stationPhi, int doubletR, */
/* 		         int doubletZ, int doubletPhi,int gasGap,) const; */

Identifier RpcIdHelper::panelID(int stationName, int stationEta, int stationPhi, int doubletR, int doubletZ, int doubletPhi, int gasGap,
                                int measuresPhi) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(rpc_field_value(), result);
    m_dbr_impl.pack(doubletR, result);
    m_dbz_impl.pack(doubletZ, result);
    m_dbp_impl.pack(doubletPhi, result);
    m_gap_impl.pack(gasGap, result);
    m_mea_impl.pack(measuresPhi, result);
    return result;
}
Identifier RpcIdHelper::panelID(int stationName, int stationEta, int stationPhi, int doubletR, int doubletZ, int doubletPhi, int gasGap,
                                int measuresPhi, bool& isValid) const {
    try {
        const Identifier result = panelID(stationName, stationEta, stationPhi, doubletR, doubletZ, doubletPhi, gasGap, measuresPhi);
        isValid = valid(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

Identifier RpcIdHelper::panelID(const Identifier& channelID) const {
    Identifier result(channelID);
    m_str_impl.reset(result);
    return result;
}

Identifier RpcIdHelper::panelID(const Identifier& padID, int gasGap, int measuresPhi) const {
    Identifier result(padID);
    resetAndSet(m_gap_impl, gasGap, result);
    resetAndSet(m_mea_impl, measuresPhi, result);
    return result;
}
Identifier RpcIdHelper::panelID(const Identifier& padID, int gasGap, int measuresPhi, bool& isValid) const {
    try {
        const Identifier result = panelID(padID, gasGap, measuresPhi);
        isValid = valid(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

Identifier RpcIdHelper::gapID(int stationName, int stationEta, int stationPhi, int doubletR, int doubletZ, int doubletPhi,
                              int gasGap) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(rpc_field_value(), result);
    m_dbr_impl.pack(doubletR, result);
    m_dbz_impl.pack(doubletZ, result);
    m_dbp_impl.pack(doubletPhi, result);
    m_gap_impl.pack(gasGap, result);

    return result;
}
Identifier RpcIdHelper::gapID(int stationName, int stationEta, int stationPhi, int doubletR, int doubletZ, int doubletPhi, int gasGap,
                              bool& isValid) const {
    try {
        const Identifier result = gapID(stationName, stationEta, stationPhi, doubletR, doubletZ, doubletPhi, gasGap);
        isValid = valid(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

Identifier RpcIdHelper::gapID(const Identifier& panelID) const {
    Identifier result(panelID);
    m_mea_impl.reset(result);
    return result;
}

Identifier RpcIdHelper::gapID(const Identifier& padID, int gasGap) const {
    Identifier result(padID);
    resetAndSet(m_gap_impl, gasGap, result);
    return result;
}
Identifier RpcIdHelper::gapID(const Identifier& padID, int gasGap, bool& isValid) const {
    try {
        const Identifier result = gapID(padID, gasGap);
        isValid = valid(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

Identifier RpcIdHelper::channelID(int stationName, int stationEta, int stationPhi, int doubletR, int doubletZ, int doubletPhi, int gasGap,
                                  int measuresPhi, int strip) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(rpc_field_value(), result);
    m_dbr_impl.pack(doubletR, result);
    m_dbz_impl.pack(doubletZ, result);
    m_dbp_impl.pack(doubletPhi, result);
    m_gap_impl.pack(gasGap, result);
    m_mea_impl.pack(measuresPhi, result);
    m_str_impl.pack(strip, result);
    return result;
}
Identifier RpcIdHelper::channelID(int stationName, int stationEta, int stationPhi, int doubletR, int doubletZ, int doubletPhi, int gasGap,
                                  int measuresPhi, int strip, bool& isValid) const {
    try {
        const Identifier result =
            channelID(stationName, stationEta, stationPhi, doubletR, doubletZ, doubletPhi, gasGap, measuresPhi, strip);
        isValid = validChannel(result, stationName, stationEta, stationPhi, doubletR, doubletZ, doubletPhi, gasGap, measuresPhi, strip);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
Identifier RpcIdHelper::channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int doubletR, int doubletZ,
                                  int doubletPhi, int gasGap, int measuresPhi, int strip) const {
    return channelID(stationNameIndex(stationNameStr), stationEta, stationPhi, doubletR, doubletZ, doubletPhi, gasGap, measuresPhi, strip);
}
Identifier RpcIdHelper::channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int doubletR, int doubletZ,
                                  int doubletPhi, int gasGap, int measuresPhi, int strip, bool& isValid) const {
    return channelID(stationNameIndex(stationNameStr), stationEta, stationPhi, doubletR, doubletZ, doubletPhi, gasGap, measuresPhi, strip,
                     isValid);
}

Identifier RpcIdHelper::channelID(const Identifier& id, int doubletZ, int doubletPhi, int gasGap, int measuresPhi, int strip) const {
    // pack fields independently
    Identifier result(id);
    resetAndSet(m_dbz_impl, doubletZ, result);
    resetAndSet(m_dbp_impl, doubletPhi, result);
    resetAndSet(m_gap_impl, gasGap, result);
    resetAndSet(m_mea_impl, measuresPhi, result);
    resetAndSet(m_str_impl, strip, result);
    return result;
}
Identifier RpcIdHelper::channelID(const Identifier& id, int doubletZ, int doubletPhi, int gasGap, int measuresPhi, int strip,
                                  bool& isValid) const {
    try {
        const Identifier result = channelID(id, doubletZ, doubletPhi, gasGap, measuresPhi, strip);
        isValid = valid(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

// get the parent id from the strip identifier
Identifier RpcIdHelper::parentID(const Identifier& id) const {
    assert(is_rpc(id));
    Identifier result(id);
    m_dbz_impl.reset(result);
    m_dbp_impl.reset(result);
    m_gap_impl.reset(result);
    m_mea_impl.reset(result);
    m_str_impl.reset(result);
    return result;
}

// doubletZ Identifier
Identifier RpcIdHelper::doubletZID(const Identifier& id) const {
    assert(is_rpc(id));
    Identifier result(id);
    m_dbp_impl.reset(result);
    m_gap_impl.reset(result);
    m_mea_impl.reset(result);
    m_str_impl.reset(result);
    return result;
}

Identifier RpcIdHelper::padID(int stationName, int stationEta, int stationPhi, int doubletR, int doubletZ, int doubletPhi) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(rpc_field_value(), result);
    m_dbr_impl.pack(doubletR, result);
    m_dbz_impl.pack(doubletZ, result);
    m_dbp_impl.pack(doubletPhi, result);
    return result;
}
Identifier RpcIdHelper::padID(int stationName, int stationEta, int stationPhi, int doubletR, int doubletZ, int doubletPhi,
                              bool& isValid) const {
    try {
        const Identifier result = padID(stationName, stationEta, stationPhi, doubletR, doubletZ, doubletPhi);
        isValid = validPad(result, stationName, stationEta, stationPhi, doubletR, doubletZ, doubletPhi);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

Identifier RpcIdHelper::padID(const Identifier& id, int doubletZ, int doubletPhi) const {
    // pack fields independently
    Identifier result(id);
    resetAndSet(m_dbz_impl, doubletZ, result);
    resetAndSet(m_dbp_impl, doubletPhi, result);
    return result;
}
Identifier RpcIdHelper::padID(const Identifier& id, int doubletZ, int doubletPhi, bool& isValid) const {
    try {
        const Identifier result = padID(id, doubletZ, doubletPhi);
        isValid = validPad(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
// Access to components of the ID

int RpcIdHelper::doubletR(const Identifier& id) const { return m_dbr_impl.unpack(id); }

int RpcIdHelper::doubletZ(const Identifier& id) const { return m_dbz_impl.unpack(id); }

int RpcIdHelper::doubletPhi(const Identifier& id) const { return m_dbp_impl.unpack(id); }

int RpcIdHelper::gasGap(const Identifier& id) const { return m_gap_impl.unpack(id); }

bool RpcIdHelper::measuresPhi(const Identifier& id) const { return m_mea_impl.unpack(id); }

int RpcIdHelper::strip(const Identifier& id) const { return m_str_impl.unpack(id); }

int RpcIdHelper::channel(const Identifier& id) const { return strip(id); }

// Access to min and max of level ranges

int RpcIdHelper::stationEtaMin() { return StationEtaMin; }

int RpcIdHelper::stationEtaMax() { return StationEtaMax; }

int RpcIdHelper::stationPhiMin() { return StationPhiMin; }

int RpcIdHelper::stationPhiMax() { return StationPhiMax; }

int RpcIdHelper::doubletRMin() { return DoubletRMin; }

int RpcIdHelper::doubletRMax() { return DoubletRMax; }

int RpcIdHelper::doubletZMin() { return DoubletZMin; }

int RpcIdHelper::doubletZMax() { return DoubletZMax; }

int RpcIdHelper::doubletPhiMin() { return DoubletPhiMin; }

int RpcIdHelper::doubletPhiMax() { return DoubletPhiMax; }

int RpcIdHelper::gasGapMin() { return GasGapMin; }

int RpcIdHelper::gasGapMax() const { return m_gasGapMax; }

int RpcIdHelper::measuresPhiMin() { return MeasuresPhiMin; }

int RpcIdHelper::measuresPhiMax() { return MeasuresPhiMax; }

int RpcIdHelper::stripMin() { return StripMin; }

int RpcIdHelper::stripMax() { return StripMax; }

/// Utility methods

int RpcIdHelper::rpcTechnology() const {
    int rpcField = technologyIndex("RPC");
    if (m_dict) { rpcField = rpc_field_value(); }
    return rpcField;
}

int RpcIdHelper::zIndex(const Identifier& id) const {
    int station = stationName(id);
    int eta = stationEta(id);
    int dR = doubletR(id);
    int dZ = doubletZ(id);
    int dP = doubletPhi(id);
    const std::string& name = stationNameString(station);
    return zIndex(name, eta, dR, dZ, dP);
}

int RpcIdHelper::zIndex(const std::string& name, int eta, int dR, int dZ, int dP) {
    /** - from Stefania
        BMS5 which has the following structure:
        for dbr=1 there are 3 dbZ, first and second are made of a single
        RpcReadoutElement and the 3rd is made of 2 separate
        RpcReadoutElements corresponding to dbPhi=1,2 respectively ( 2,
        according to the identifier scheme, is at larger global phi than 1)
        for dbr=2 the situation is identical to dbr=1

        and

        BMS6 is done this way:
        at dbr=1, there are only 2 dbZ, the first corrsponding to a single
        RpcReadoutElement, the second one corresponding to two different
        chambers and, therefore, two different RpcReadoutElements;
        in dbr=2, there are 3 doubletZ, the first two are standard (1
        RpcReadoutElemet each), the third one has two chambers -> 2
        RpcReadoutElements

        Notice that 5 and 6 is a subtype naming which does not have any
        correspondence to the offline identifier scheme: so you have to know where
        BMS5 and BMS6 are located in order to treat them in a special way: so ...

        BMS 5 are at StEta = +/- 2 and StPhi = 1,2,3,4,5,8
        BMS 6 are at StEta = +/- 4 and StPhi = 1,2,3,4,5,8
    */
    int dbz_index = dZ;

    if (name == "BMS") {
        if (abs(eta) == 2 && dZ == 3) {
            if (dP == 2) dbz_index++;
        } else if (abs(eta) == 4 && dR == 2 && dZ == 3) {
            if (dP == 2) dbz_index++;
        } else if (abs(eta) == 4 && dR == 1 && dZ == 2) {
            if (dP == 2) dbz_index++;
        }
    }
    return dbz_index;
}