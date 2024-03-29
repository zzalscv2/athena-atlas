/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Detector Description
 -----------------------------------------
***************************************************************************/

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "IdDict/IdDictDefs.h"
#include "AtlasDetectorIDHelper.h"
#include <stdio.h>
#include <assert.h>

AtlasDetectorID::AtlasDetectorID(const std::string& name) :
  AthMessaging(name)
{}

AtlasDetectorID::~AtlasDetectorID()
{
    delete m_helper;
}

Identifier
AtlasDetectorID::indet        (void) const
{

    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack    (indet_field_value(), result);
    return (result);
}

Identifier
AtlasDetectorID::lar          (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack    (lar_field_value(), result);
    return (result);
}

Identifier
AtlasDetectorID::tile         (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack    (tile_field_value(), result);
    return (result);
}

Identifier
AtlasDetectorID::muon() const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack    (muon_field_value(), result);
    return (result);
}

Identifier
AtlasDetectorID::calo(void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack    (calo_field_value(), result);
    return (result);
}


Identifier
AtlasDetectorID::pixel        (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack       (indet_field_value(), result);
    m_indet_part_impl.pack(m_PIXEL_ID, result);
    return (result);
}

Identifier
AtlasDetectorID::sct          (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack       (indet_field_value(), result);
    m_indet_part_impl.pack(m_SCT_ID, result);
    return (result);
}

Identifier
AtlasDetectorID::trt          (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack       (indet_field_value(), result);
    m_indet_part_impl.pack(m_TRT_ID, result);
    return (result);
}

Identifier
AtlasDetectorID::hgtd        (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack       (indet_field_value(), result);
    m_indet_part_impl.pack(m_HGTD_ID, result);
    return (result);
}

Identifier
AtlasDetectorID::lumi        (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack       (indet_field_value(), result);
    m_indet_part_impl.pack(m_LUMI_ID, result);
    return (result);
}

Identifier
AtlasDetectorID::lar_em           (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack     (lar_field_value(), result);
    m_lar_part_impl.pack(m_LAR_EM_ID, result);
    return (result);
}


Identifier
AtlasDetectorID::lar_lvl1        (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack     (calo_field_value(), result);
    m_calo_side_impl.pack(-1, result);
    return (result);
}

Identifier
AtlasDetectorID::lar_dm          (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack     (calo_field_value(), result);
    m_calo_side_impl.pack(-4, result);
    return (result);
}

Identifier
AtlasDetectorID::tile_dm         (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack     (calo_field_value(), result);
    m_calo_side_impl.pack(-5, result);
    return (result);
}

Identifier
AtlasDetectorID::lar_hec          (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack     (lar_field_value(), result);
    m_lar_part_impl.pack(m_LAR_HEC_ID, result);
    return (result);
}

Identifier
AtlasDetectorID::lar_fcal          (void) const
{
    Identifier result((Identifier::value_type)0);
    // Pack field
    m_det_impl.pack     (lar_field_value(), result);
    m_lar_part_impl.pack(m_LAR_FCAL_ID, result);
    return (result);
}

Identifier
AtlasDetectorID::mdt() const
{
    // THIS METHOD SHOULD BE REMOVED !!! DOESN'T MAKE SENSE TO HAVE AN MDT ID

    Identifier result((Identifier::value_type)0);
    return (result);
}

Identifier
AtlasDetectorID::csc() const
{
    Identifier result((Identifier::value_type)0);
    // THIS METHOD SHOULD BE REMOVED !!! DOESN'T MAKE SENSE TO HAVE AN CSC ID
    return (result);
}

Identifier
AtlasDetectorID::rpc() const
{
    Identifier result((Identifier::value_type)0);
    // THIS METHOD SHOULD BE REMOVED !!! DOESN'T MAKE SENSE TO HAVE AN RPC ID
    return (result);
}

Identifier
AtlasDetectorID::tgc() const
{
    Identifier result((Identifier::value_type)0);
    // THIS METHOD SHOULD BE REMOVED !!! DOESN'T MAKE SENSE TO HAVE AN TGC ID
    return (result);
}

Identifier
AtlasDetectorID::stgc() const
{
    Identifier result((Identifier::value_type)0);
    // THIS METHOD SHOULD BE REMOVED !!! DOESN'T MAKE SENSE TO HAVE AN STGC ID
    return (result);
}

Identifier
AtlasDetectorID::mm() const
{
    Identifier result((Identifier::value_type)0);
    // THIS METHOD SHOULD BE REMOVED !!! DOESN'T MAKE SENSE TO HAVE AN MM ID
    return (result);
}

/// IdContext (indicates id length) for detector systems
IdContext
AtlasDetectorID::detsystem_context (void) const
{
    ExpandedIdentifier id;
    return (IdContext(id, 0, m_DET_INDEX));
}

/// IdContext (indicates id length) for sub-detector
IdContext
AtlasDetectorID::subdet_context  (void) const
{
    ExpandedIdentifier id;
    return (IdContext(id, 0, m_SUBDET_INDEX));
}


int
AtlasDetectorID::get_id          (const IdentifierHash& /*hash_id*/,
                                  Identifier& /*id*/,
                                  const IdContext* /*context*/) const
{
    return (0);
}

int
AtlasDetectorID::get_hash        (const Identifier& /*id*/,
                                  IdentifierHash& /*hash_id*/,
                                  const IdContext* /*context*/) const
{
    return (0);
}

int
AtlasDetectorID::register_dict_tag        (const IdDictMgr& dict_mgr,
                                           const std::string& dict_name)
{
    // Register version of dictionary dict_name

    // Access dictionary by name
    IdDictDictionary* dict = dict_mgr.find_dictionary(dict_name);
    if (!dict) return(1);
    // Add in dict name, file name and version
    m_dict_names.push_back(dict_name);
    m_file_names.push_back(dict->file_name());
    m_dict_tags.push_back(dict->dict_tag());
    return (0);
}

/// Test whether an idhelper should be reinitialized based on the
/// change of tags
bool
AtlasDetectorID::reinitialize             (const IdDictMgr& dict_mgr)
{
    // If no tag has been registered, then reinitialize
    if (m_dict_tags.size() == 0) return (true);

    // If no dict names have been registered, then reinitialize
    if (m_dict_names.size() == 0) return (true);

    // Loop over dict names and check version tags
    if (m_dict_names.size() != m_dict_tags.size()) {
        ATH_MSG_ERROR("reinitialize: dict names and tags vectors not the same length ");
        ATH_MSG_ERROR("names: " << m_dict_names.size() << " tags: " << m_dict_tags.size());
    }
    for (unsigned int i = 0; i < m_dict_names.size(); ++i) {
        // Access dictionary by name
        IdDictDictionary* dict = dict_mgr.find_dictionary(m_dict_names[i]);
        if (!dict) {
            ATH_MSG_ERROR("reinitialize: could not find dict -  " << m_dict_names[i]);
            return(false);
        }
        if (m_dict_tags[i] != dict->dict_tag()) {
            // Remove all memory of versions
            m_dict_names.clear();
            m_dict_tags.clear();
            m_file_names.clear();
            return (true);
        }
    }

    // Tags match - don't reinitialize
    return (false);
}



int
AtlasDetectorID::initialize_from_dictionary(const IdDictMgr& dict_mgr)
{

    // Register version of ATLAS dictionary
    if (register_dict_tag(dict_mgr, "ATLAS")) return(1);

    // Initialize helper, needed for init of AtlasDetectorID
    if(!m_helper) {
        m_helper = new AtlasDetectorIDHelper(m_msgSvc);
    }

    if(m_helper->initialize_from_dictionary(dict_mgr, m_quiet)) return (1);

    // Initialize level indices and id values from dicts
    if(initLevelsFromDict(dict_mgr)) return (1);

    m_is_initialized_from_dict = true;

    if (!m_quiet) {
        ATH_MSG_INFO("initialize_from_dictionary - OK");
    }

    return (0);
}


std::string
AtlasDetectorID::dictionaryVersion  (void) const
{
    return (m_dict_version);
}

bool
AtlasDetectorID::is_indet       (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( id.fields() > 0 ){
        if ( id[0] == m_INDET_ID) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_lar                 (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( id.fields() > 0 ){
        if ( id[0] == m_LAR_ID) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_tile                (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( id.fields() > 0 ){
        if ( id[0] == m_TILE_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_muon                (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( id.fields() > 0 ){
        if ( id[0] == m_MUON_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_calo                (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( id.fields() > 0 ){
        if ( id[0] == m_CALO_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_pixel       (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( is_indet(id) && id.fields() > 1 ){
        if ( id[1] == m_PIXEL_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_sct         (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( is_indet(id) && id.fields() > 1 ){
        if ( id[1] == m_SCT_ID ) return(true);
    }
    return result;
}

bool
AtlasDetectorID::is_trt         (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( is_indet(id) && id.fields() > 1 ){
        if ( id[1] == m_TRT_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_hgtd        (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( is_indet(id) && id.fields() > 1 ){
        if ( id[1] == m_HGTD_ID) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_lumi        (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( is_indet(id) && id.fields() > 1 ){
        if ( id[1] == m_LUMI_ID) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_plr         (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( is_lumi(id) ){
        if ( id[2] == m_LUMI_PLR_ID) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_lar_em      (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( is_lar(id) && id.fields() > 1 ){
        if ( abs(id[1]) == m_LAR_EM_ID) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_lar_hec             (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( is_lar(id) && id.fields() > 1 ){
        if ( abs(id[1]) == m_LAR_HEC_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_lar_fcal            (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( is_lar(id) && id.fields() > 1 ){
        if ( abs(id[1]) == m_LAR_FCAL_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_lar_minifcal        (const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( is_lar_fcal(id) && id.fields() > 3 ){
        if ( abs(id[3]) == 0 ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_mdt(const ExpandedIdentifier& id) const
{

    bool result = false;
    if ( id[0] == m_MUON_ID ) {
        if ( id[4] == m_MDT_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_csc(const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( id[0] == m_MUON_ID ) {
        if ( id[4] == m_CSC_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_rpc(const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( id[0] == m_MUON_ID )  {
        if ( id[4] == m_RPC_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_tgc(const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( id[0] == m_MUON_ID ) {
        if ( id[4] == m_TGC_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_stgc(const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( id[0] == m_MUON_ID ) {
        if ( id[4] == m_STGC_ID ) result = true;
    }
    return result;
}

bool
AtlasDetectorID::is_mm(const ExpandedIdentifier& id) const
{
    bool result = false;
    if ( id[0] == m_MUON_ID ) {
        if ( id[4] == m_MM_ID ) result = true;
    }
    return result;
}

// Short print out of any identifier:
void
AtlasDetectorID::show           (Identifier id,
                                 const IdContext* context,
                                 char sep ) const
{
    ATH_MSG_INFO(show_to_string(id, context, sep));
}

// or provide the printout in string form
std::string
AtlasDetectorID::show_to_string (Identifier id,
                                 const IdContext* context,
                                 char sep ) const
{
    // Do a generic printout of identifier

    std::string result("Unable to decode id");
    unsigned int max_index = (context) ? context->end_index()  : 999;

    if (!m_is_initialized_from_dict) return  (result);

    // Find the dictionary to use:
    IdDictDictionary*   dict = 0;
    ExpandedIdentifier expId;
    ExpandedIdentifier prefix;  // default is null prefix
    Identifier compact = id;

    if (is_indet(id)) {
        dict = m_indet_dict;
    }
    else if (is_lar(id)) {
        dict = m_lar_dict;
    }
    else if (is_tile(id)) {
        dict = m_tile_dict;
    }
    else if (is_muon(id)) {
        dict = m_muon_dict;
    }
    else if (is_lvl1_trig_towers(id) ||
             is_lvl1_online(id) ||
             is_lar_dm(id) ||
             is_tile_dm(id)) {
        dict = m_calo_dict;
    }
    else if (is_forward(id)) {
        dict = m_fwd_dict;
    }



    if (!dict) return (result);

    if (dict->unpack(compact,
                     prefix,
                     max_index,
                     expId)) {
        return (result);
    }

    bool first = true;
    char temp[20];

    result = "";
    if ('.' == sep) result = "[";
    for (unsigned int i = 0; i < expId.fields(); ++i) {
        if (first) first = false;
        else result += sep;
        sprintf (temp, "%d", expId[i]);
        result += temp;
    }
    if ('.' == sep) result += "]";

//      result += " compact [";
//      sprintf (temp, "0x%x", (unsigned int)compact);
//      result += temp;
//      result += "]";

    return (result);
}



void
AtlasDetectorID::print (Identifier id,
                        const IdContext* context) const
{
    ATH_MSG_INFO(print_to_string(id, context));
}

std::string
AtlasDetectorID::print_to_string        (Identifier id,
                                         const IdContext* context) const
{
    // Print out for any Atlas identifier
    std::string result;
    if (m_is_initialized_from_dict) {

        // Do a generic printout of identifier from dictionary
        unsigned int max_index = (context) ? context->end_index()  : 999;

        // Find the dictionary to use:
        IdDictDictionary*       dict = 0;
        ExpandedIdentifier expId;
        ExpandedIdentifier prefix;  // default is null prefix
        Identifier compact = id;

        if (is_indet(id)) {
            dict = m_indet_dict;
        }
        else if (is_lar(id)) {
            dict = m_lar_dict;
        }
        else if (is_tile(id)) {
            dict = m_tile_dict;
        }
        else if (is_muon(id)) {
            dict = m_muon_dict;
        }
        else if (is_lvl1_trig_towers(id) ||
                 is_lvl1_online(id) ||
                 is_lar_dm(id) ||
                 is_tile_dm(id)) {
            dict = m_calo_dict;
        }
        else if (is_forward(id)) {
            dict = m_fwd_dict;
        }


        if (!dict) return (result);

        if (dict->unpack(compact,
                         prefix,
                         max_index,
                         " ",
                         result)) {
            return (result);
        }
    }
    return (result);
}

///  Dictionary name
std::vector<std::string>
AtlasDetectorID::dict_names         (void) const
{
    return (m_dict_names);
}

///  File name
std::vector<std::string>
AtlasDetectorID::file_names         (void) const
{
    return (m_file_names);
}

///  Version tag for subdet dictionary
std::vector<std::string>
AtlasDetectorID::dict_tags      (void) const
{
    return (m_dict_tags);
}

bool            AtlasDetectorID::do_checks      (void) const
{
    return (m_do_checks);
}

void            AtlasDetectorID::set_do_checks  (bool do_checks)
{
    m_do_checks = do_checks;
}

bool            AtlasDetectorID::do_neighbours  (void) const
{
    return (m_do_neighbours);
}

void            AtlasDetectorID::set_do_neighbours      (bool do_neighbours)
{
    m_do_neighbours = do_neighbours;
}

void            AtlasDetectorID::setMessageSvc  (IMessageSvc* msgSvc)
{
    m_msgSvc = msgSvc ;
}

void            AtlasDetectorID::set_quiet  (bool quiet)
{
    m_quiet = quiet ;
}

void
AtlasDetectorID::setDictVersion  (const IdDictMgr& dict_mgr, const std::string& name)
{
    const IdDictDictionary* dict = dict_mgr.find_dictionary (name);

    m_dict_version = dict->m_version;
}

std::string
AtlasDetectorID::to_range (const ExpandedIdentifier& id) const
{

    // Build a string from the contents of an identifier

    int fields = id.fields();
    char temp[10] = "";
    std::string result("");

    for (int i = 0; i < fields; ++i) {
        sprintf( temp, "%d", id[i]);
        if (i > 0) result += '/'; // add '/' only if NOT last one
        result += temp;
    }

    return result;
}

int
AtlasDetectorID::initLevelsFromDict(const IdDictMgr& dict_mgr)
{

    // Set do_checks flag
    if (dict_mgr.do_checks()) m_do_checks = true;
    // Set do_neighbours flag
    if (!dict_mgr.do_neighbours()) m_do_neighbours = false;

    IdDictLabel* label = 0;
    IdDictField* field = 0;

    // Find out from the dictionary the detector and subdetector
    // levels and id values
    m_DET_INDEX             = 999;
    m_SUBDET_INDEX          = 999;
    m_MUON_SUBDET_INDEX     = 999;
    m_INDET_ID              = -1;
    m_LAR_ID                = -1;
    m_TILE_ID               = -1;
    m_MUON_ID               = -1;
    m_PIXEL_ID              = -1;
    m_SCT_ID                = -1;
    m_TRT_ID                = -1;
    m_HGTD_ID               = -1;
    m_FWD_ID                = -1;
    m_ALFA_ID               = -1;
    m_BCM_ID                = -1;
    m_LUCID_ID              = -1;
    m_ZDC_ID                = -1;
    m_LAR_EM_ID             = -1;
    m_LAR_HEC_ID            = -1;
    m_LAR_FCAL_ID           = -1;
    m_LAR_FCAL_MODULE_INDEX = 999;
    m_MDT_ID                = -1;
    m_CSC_ID                = -1;
    m_RPC_ID                = -1;
    m_TGC_ID                = -1;
    m_STGC_ID               = -1;
    m_MM_ID                 = -1;

    // Save generic dict for top levels
    IdDictDictionary*   top_dict = 0;

//      // Initialize the DET INDEX and det ids from the Atlas dict
//      m_atlas_dict = dict_mgr.find_dictionary ("ATLAS");
//      if(!m_atlas_dict) {
//      std::cout << " AtlasDetectorID::initLevelsFromDict - cannot access ATLAS dictionary "
//                << std::endl;
//      return (1);
//      }

    // Get det ids

    // Initialize ids for InDet subdet
    m_indet_dict = dict_mgr.find_dictionary ("InnerDetector");
    if(!m_indet_dict) {
        ATH_MSG_WARNING("initLevelsFromDict - cannot access InnerDetector dictionary");
    }
    else {

        // Found InDet dict

        top_dict = m_indet_dict;  // save as top_dict

        // Check if this is High Luminosity LHC layout
	//should just use std::string::contains once that is available... (C++23)
	std::string versionString = m_indet_dict->m_version;
        m_isHighLuminosityLHC = (versionString.find("ITk") != std::string::npos || versionString.find("P2-RUN4") != std::string::npos);

        // Get InDet subdets

        field = m_indet_dict->find_field("part");
        if (!field) {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'part' field for InnerDetector dictionary");
            return (1);
        }

        label = field->find_label("Pixel");
        if (label) {
            if (label->m_valued) {
                m_PIXEL_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label Pixel does NOT have a value ");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'Pixel' label");
            return (1);
        }

        label = field->find_label("SCT");
        if (label) {
            if (label->m_valued) {
                m_SCT_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label SCT does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'SCT' label");
            return (1);
        }
        if (!m_isHighLuminosityLHC) {
            label = field->find_label("TRT");
            if (label) {
                if (label->m_valued) {
                    m_TRT_ID = label->m_value;
                }
                else {
                    ATH_MSG_ERROR("initLevelsFromDict - label TRT does NOT have a value");
                    return (1);
                }
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - unable to find 'TRT' label");
                return (1);
            }
        }
        if(m_isHighLuminosityLHC) {
            if(versionString.find("PLR") != std::string::npos || versionString.find("P2-RUN4") != std::string::npos) { // do not look for this unless using ITKHGTDPLR dictionary which contains "LuminosityDetectors"
                label = field->find_label("LuminosityDetectors");
                if (label) {
                    if (label->m_valued) {
                        m_LUMI_ID = label->m_value;
                    }
                    else {
                        ATH_MSG_ERROR("initLevelsFromDict - label LuminosityDetectors does NOT have a value");
                        return (1);
                    }
                }
                else {
                    ATH_MSG_ERROR("initLevelsFromDict - unable to find 'LuminosityDetectors' label ");
                    return (1);
                }
            }

            label = field->find_label("HGTD");
            if (label) {
                if (label->m_valued) {
                    m_HGTD_ID = label->m_value;
                }
                else {
                    ATH_MSG_ERROR("initLevelsFromDict - label HGTD does NOT have a value");
                    return (1);
                }
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - unable to find 'HGTD' label");
                return (1);
            }
        }

    }



    // Initialize ids for Forward dets
    m_fwd_dict = dict_mgr.find_dictionary ("ForwardDetectors");
    if(!m_fwd_dict) {
        ATH_MSG_WARNING("initLevelsFromDict - cannot access ForwardDetectors dictionary");
    }
    else {

        // Found ForwardDetectors dict

        if (!top_dict) top_dict = m_fwd_dict;  // save as top_dict

        // Get Forward subdets

        field = m_fwd_dict->find_field("part");
        if (!field) {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'part' field for ForwardDetectors dictionary");
            return (1);
        }

        label = field->find_label("ALFA");
        if (label) {
            if (label->m_valued) {
                m_ALFA_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label ALFA does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'ALFA' label");
            return (1);
        }

        label = field->find_label("BCM");
        if (label) {
            if (label->m_valued) {
                m_BCM_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label BCM does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'BCM' label");
            return (1);
        }
        label = field->find_label("LUCID");
        if (label) {
            if (label->m_valued) {
                m_LUCID_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label LUCID does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'LUCID' label");
            return (1);
        }
        label = field->find_label("ZDC");
        if (label) {
            if (label->m_valued) {
                m_ZDC_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label ZDC does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'ZDC' label");
            return (1);
        }
    }

    // Initialize ids for LAr detectors
    m_lar_dict = dict_mgr.find_dictionary ("LArCalorimeter");
    if(!m_lar_dict) {
        ATH_MSG_WARNING("initLevelsFromDict -  cannot access LArCalorimeter dictionary");
    }
    else {
        // Found LAr dict

        if (!top_dict) top_dict = m_lar_dict;  // save as top_dict

        field = m_lar_dict->find_field("part");
        if (!field) {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'part' field for LArCalorimeter dictionary");
            return (1);
        }

        label = field->find_label("LArEM");
        if (label) {
            if (label->m_valued) {
                m_LAR_EM_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label LArEM does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'LArEM' label");
            return (1);
        }
        label = field->find_label("LArHEC");
        if (label) {
            if (label->m_valued) {
                m_LAR_HEC_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label LArHEC does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'LArHEC' label");
            return (1);
        }
        label = field->find_label("LArFCAL");
        if (label) {
            if (label->m_valued) {
                m_LAR_FCAL_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label LArFCAL does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'LArFCAL' label");
            return (1);
        }
        // set

        field = m_lar_dict->find_field("module") ;
        if (field) {
            m_LAR_FCAL_MODULE_INDEX = field->m_index ;
        }
        else {
            ATH_MSG_DEBUG("initLevelsFromDict - unable to find 'module' field for miniFCAL");
            // return (1); For the moment, this is not an
            // error. Cannot be an error until miniFCAL is always in
            // the xml files.
        }
    }


    // Initialize ids for Tile calo
    m_tile_dict = dict_mgr.find_dictionary ("TileCalorimeter");
    if(!m_tile_dict) {
        ATH_MSG_WARNING("initLevelsFromDict -  cannot access TileCalorimeter dictionary");
    }
    else {
        // File Tile

        if (!top_dict) top_dict = m_tile_dict;  // save as top_dict

    }

    // Initialize ids for Muon detectors
    m_muon_dict = dict_mgr.find_dictionary ("MuonSpectrometer");
    if(!m_muon_dict) {
        ATH_MSG_WARNING("initLevelsFromDict - cannot access MuonSpectrometer dictionary");
    }
    else {
        // Found muon dict

        if (!top_dict) top_dict = m_muon_dict;  // save as top_dict

	/*
	  During initialisation from the dictionary we parse the
	  information which muon stationName belongs to which muon
	  subsystem. This information is not entirely encoded in the
	  identifiers however it is frequently tested during
	  reconstruction. In order to speed up these checks this info
	  is now stored locally into m_muon_tech_bits.
	  P.Fleischmann 04.04.2013
	 */
	if (m_muon_tech_bits.empty()) {

	  // we only need to load this once
	  field = m_muon_dict->find_field("stationName");
	  if (field) {
	    size_type nStationNames = field->get_label_number();
	    std::string stationNameString;
	    std::vector<IdDictLabel*> stationNameLabels = field->m_labels;

	    // first check for the maximum value assigned to any stationName
	    int stationNameIndex;
	    int maxStationNameIndex = -1;
	    for (size_type i = 0; i < nStationNames; ++i) {
	      if (stationNameLabels[i]->m_valued) {
		stationNameIndex = stationNameLabels[i]->m_value;
	      } else {
		// in case no individual values are given,
		// the order inside the dictionary is used
		stationNameIndex = (int)i;
	      }
	      if (stationNameIndex > maxStationNameIndex) {
		maxStationNameIndex = stationNameIndex;
	      }
	    }

	    // the vector may contain gaps (value=0) in case of jumps
	    // in the values
	    m_muon_tech_bits.resize(maxStationNameIndex + 1);
	    std::vector<IdDictRegion*> muonRegions = m_muon_dict->m_all_regions;

	    // loop over all stationNames and search for associations
	    // to technology
	    for (size_type i = 0; i < nStationNames; ++i) {
	      stationNameString = stationNameLabels[i]->m_name;
	      if (stationNameLabels[i]->m_valued) {
		stationNameIndex = stationNameLabels[i]->m_value;
	      } else {
		// in case no individual values are given,
		// the order inside the dictionary is used
		stationNameIndex = (int)i;
	      }

	      // next loop over all regions to look for
	      // stationName <-> technology associations
	      bool found = false;
	      bool stationNameFound = false;
	      bool technologyFound = false;
	      std::string techLabel;
	      for (size_type j = 0; j < muonRegions.size(); ++j) {
		IdDictRegion* region = muonRegions[j];
		std::vector< IdDictRegionEntry * > entries = region->m_entries;

		// loop over all entries of a region to look for
		// stationName and technology information
		stationNameFound = false;
		technologyFound = false;
		for (size_type k = 0; k < entries.size(); ++k) {
		  IdDictRange* range = dynamic_cast<IdDictRange*> (entries[k]);
		  if (range) {
		    if (range->m_field_name == "stationName") {

		      if (range->m_label == stationNameString) {
			// we found a region containing the current stationName
			stationNameFound = true;
			continue;
		      } else {
			// we found a region containing a different stationName,
			// let's skip
			break;
		      }

		    } else if (range->m_field_name == "technology") {
		      technologyFound = true;
		      techLabel = range->m_label;
		    }

		    if (stationNameFound && technologyFound) {
		      // we found a stationName <-> technology association
		      if (techLabel == "MDT") {
			m_muon_tech_bits[stationNameIndex] = AtlasDetDescr::fAtlasMDT;
		      } else if (techLabel == "RPC") {
			m_muon_tech_bits[stationNameIndex] = AtlasDetDescr::fAtlasRPC;
		      } else if (techLabel == "CSC") {
			m_muon_tech_bits[stationNameIndex] = AtlasDetDescr::fAtlasCSC;
		      } else if (techLabel == "TGC") {
			m_muon_tech_bits[stationNameIndex] = AtlasDetDescr::fAtlasTGC;
		      } else if (techLabel == "MM") {
			m_muon_tech_bits[stationNameIndex] = AtlasDetDescr::fAtlasMM;
		      } else if (techLabel == "STGC") {
			m_muon_tech_bits[stationNameIndex] = AtlasDetDescr::fAtlasSTGC;
		      } else {
			m_muon_tech_bits[stationNameIndex] = AtlasDetDescr::fUndefined;
		      }

		      found = true;
		      break;
		    }
		  }
		} // end of loop overregion entries

		if (found) {
		  // no need to continue to look for this stationName,
		  // since each stationName must be uniquely associated
		  // to a technology, except for MDT/PRC
		  break;
		}

	      } // end of loop over regions

	    } // end of loop over stationNames

	  }
	  else {
	      ATH_MSG_ERROR("initLevelsFromDict - unable to find 'stationName' field for MuonSpectrometer dictionary");
          return (1);
	  }
	}
	// end of filling stationName <-> technology associations

        field = m_muon_dict->find_field("technology");
        if (field) {
            m_MUON_SUBDET_INDEX = field->m_index;
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'technology' field for MuonSpectrometer dictionary");
            return (1);
        }

        label = field->find_label("MDT");
        if (label) {
            if (label->m_valued) {
                m_MDT_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label MDT does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'MDT' label");
            return (1);
        }
        label = field->find_label("RPC");
        if (label) {
            if (label->m_valued) {
                m_RPC_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label RPC does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'RPC' label");
        }
        label = field->find_label("TGC");
        if (label) {
            if (label->m_valued) {
                m_TGC_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label TGC does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'TGC' label");
        }
        label = field->find_label("STGC");
        if (label) {
            if (label->m_valued) {
                m_STGC_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label STGC does NOT have a value");
                return (1);
            }
        }
        else {
            if (!m_quiet) {
                ATH_MSG_DEBUG("initLevelsFromDict - there are no sTGC entries in the dictionary!");
            }
        }
        label = field->find_label("MM");
        if (label) {
            if (label->m_valued) {
                m_MM_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label MM does NOT have a value");
                return (1);
            }
        }
        else {
            if (!m_quiet) {
                ATH_MSG_DEBUG("initLevelsFromDict - there are no MM entries in the dictionary!");
            }
        }
        label = field->find_label("CSC");
        if (label) {
            if (label->m_valued) {
                m_CSC_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label CSC does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_DEBUG("initLevelsFromDict - there are no CSC entries in the dictionary!");
            return (1);
        }
    }

    // Initialize id for Calo and fields for lvl1 and dead material
    m_calo_dict = dict_mgr.find_dictionary ("Calorimeter");
    if(!m_calo_dict) {
        ATH_MSG_ERROR("initLevelsFromDict - Warning cannot access Calorimeter dictionary");
    }
    else {
        // Found calo dict

        if (!top_dict) top_dict = m_calo_dict;  // save as top_dict

        // Set lvl1 field for is_lvl1_trig_towers
        int value;
        m_lvl1_field.clear();
        // negative half
        if (m_calo_dict->get_label_value("DetZside", "negative_lvl1_side", value)) {
            ATH_MSG_ERROR("initLevelsFromDict - Could not get value for label 'negative_lvl1_side' of field 'DetZside' in dictionary " << m_calo_dict->m_name);
            return (1);
        }
        m_lvl1_field.add_value(value);
        // positive half
        if (m_calo_dict->get_label_value("DetZside", "positive_lvl1_side", value)) {
            ATH_MSG_ERROR("initLevelsFromDict - Could not get value for label 'positive_lvl1_side' of field 'DetZside' in dictionary " << m_calo_dict->m_name);
            return (1);
        }
        m_lvl1_field.add_value(value);

        // Set lar dead material field for is_lar_dm
        m_lar_dm_field.clear();
        // negative half
        if (m_calo_dict->get_label_value("DetZside", "negative_DMLar_side", value)) {
            ATH_MSG_ERROR("initLevelsFromDict - Could not get value for label 'negative_DMLar_side' of field 'DetZside' in dictionary " << m_calo_dict->m_name);
            return (1);
        }
        m_lar_dm_field.add_value(value);
        // positive half
        if (m_calo_dict->get_label_value("DetZside", "positive_DMLar_side", value)) {
            ATH_MSG_ERROR("initLevelsFromDict - Could not get value for label 'positive_DMLar_side' of field 'DetZside' in dictionary " << m_calo_dict->m_name);
            return (1);
        }
        m_lar_dm_field.add_value(value);

        // Set tile dead material field for is_tile_dm
        m_tile_dm_field.clear();
        // negative half
        if (m_calo_dict->get_label_value("DetZside", "negative_DMTile_side", value)) {
            ATH_MSG_ERROR("initLevelsFromDict - Could not get value for label 'negative_DMTile_side' of field 'DetZside' in dictionary " << m_calo_dict->m_name);
            return (1);
        }
        m_tile_dm_field.add_value(value);
        // positive half
        if (m_calo_dict->get_label_value("DetZside", "positive_DMTile_side", value)) {
            ATH_MSG_ERROR("initLevelsFromDict - Could not get value for label 'positive_DMTile_side' of field 'DetZside' in dictionary " << m_calo_dict->m_name);
            return (1);
        }
        m_tile_dm_field.add_value(value);

        // Set lvl1 field for is_lvl1_online
        m_lvl1_onl_field.clear();

        int notok = m_calo_dict->get_label_value("DetZside", "no_side", value);
        if (notok && !m_quiet) {
            ATH_MSG_DEBUG("initLevelsFromDict -  Could not get value for label 'no_side' of field 'DetZside' in dictionary " << m_calo_dict->m_name);
            //      return (1);
        } else {
            m_lvl1_onl_field.add_value(value);
        }
    }

    // set det/subdet indices
    if (top_dict) {

        field = top_dict->find_field("subdet");
        if (field) {
            m_DET_INDEX = field->m_index;
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict -  - unable to find 'subdet' field from dict "
                          << top_dict->m_name);
            return (1);
        }

        // Get indet id
        label = field->find_label("InnerDetector");
        if (label) {
            if (label->m_valued) {
                m_INDET_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label InnerDetector does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'InnerDetector' label");
            return (1);
        }

        // Get fwd id
        label = field->find_label("ForwardDetectors");
        if (label) {
            if (label->m_valued) {
                m_FWD_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label ForwardDetectors does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'ForwardDetectors' label");
            return (1);
        }


        // Get LAr id
        label = field->find_label("LArCalorimeter");
        if (label) {
            if (label->m_valued) {
                m_LAR_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label LArCalorimeter does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'LArCalorimeter' label");
            return (1);
        }

        // Get Tile id
        label = field->find_label("TileCalorimeter");
        if (label) {
            if (label->m_valued) {
                m_TILE_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label TileCalorimeter does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'TileCalorimeter' label");
            return (1);
        }

        // Get Muon id
        label = field->find_label("MuonSpectrometer");
        if (label) {
            if (label->m_valued) {
                m_MUON_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label MuonSpectrometer does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'MuonSpectrometer' label");
            return (1);
        }

        // Get Calo id
        label = field->find_label("Calorimeter");
        if (label) {
            if (label->m_valued) {
                m_CALO_ID = label->m_value;
            }
            else {
                ATH_MSG_ERROR("initLevelsFromDict - label Calorimeter does NOT have a value");
                return (1);
            }
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find 'Calorimeter' label");
            return (1);
        }

        // Get name of next level
        std::string name;
        if (top_dict->m_name == "InnerDetector") {
            name = "part";
        }
        else if (top_dict->m_name == "Calorimeter") {
            name = "DetZside";
        }
        else if (top_dict->m_name == "LArCalorimeter") {
            name = "part";
        }
        else if (top_dict->m_name == "MuonSpectrometer") {
            name = "stationName";
        }
        else if (top_dict->m_name == "TileCalorimeter") {
            name = "section";
        }
        else if (top_dict->m_name == "ForwardDetectors") {
            name = "part";
        }

        // While we're here, save the index to the sub-detector level
        // ("part" for InDet)
        field = top_dict->find_field(name);
        if (field) {
            m_SUBDET_INDEX = field->m_index;
        }
        else {
            ATH_MSG_ERROR("initLevelsFromDict - unable to find field "
                          << name << " from dict "
                          << top_dict->m_name);
            return (1);
        }
    }
    else {
        ATH_MSG_ERROR("initLevelsFromDict - no top dictionary defined");
        return (1);
    }



//      std::cout << " AtlasDetectorID::initLevelsFromDict "
//            << "Set lvl1 field values: "
//            << (std::string)m_lvl1_field
//            << " lvl1_onl "
//            << (std::string)m_lvl1_onl_field
//            << " lar_dm "
//            << (std::string)m_lar_dm_field
//            << " tile_dm "
//            << (std::string)m_tile_dm_field
//            << std::endl;


    // Set the field implementations

    const IdDictRegion* region = 0;
    size_type region_index =  m_helper->pixel_region_index();
    if (m_indet_dict && AtlasDetectorIDHelper::UNDEFINED != region_index) {

        region                 =  m_indet_dict->m_regions[region_index];

        // Detector
        m_det_impl             = region->m_implementation[m_DET_INDEX];

        // Add on extra values to assure that one has a value per
        // bit. This is needed to avoid an overflow decoding error
        // when a pixel channel id is decoded
        if(m_det_impl.ored_field().get_mode() != Range::field::enumerated) {
            ATH_MSG_ERROR("initLevelsFromDict - ERROR det implementation is not enumerated: "
                          << m_det_impl.show_to_string());
        }

        size_type bits    = m_det_impl.bits();
        size_type nvalues = static_cast<size_type>(1) << bits;
        Range::field det  = m_det_impl.ored_field();
        size_type max     = det.get_maximum ();
        for (size_type i = det.get_values().size(); i < nvalues; ++i) {
            max++;
            det.add_value(max);
        }
        // Replace ored field with modified one
        m_det_impl.set_ored_field(det);
        //std::cout << "set extra bits    "  << std::endl;
        //std::cout << "det               "
        //        << m_det_impl.show_to_string() << std::endl;

        // InDet part
        m_indet_part_impl      = region->m_implementation[m_SUBDET_INDEX];
    }

    // Calo side: LVL1, LAr & Tile DeadMat
    region_index = m_helper->lvl1_region_index();
    if (m_calo_dict && AtlasDetectorIDHelper::UNDEFINED != region_index) {
        region = m_calo_dict->m_regions[region_index];
        m_calo_side_impl       = region->m_implementation[m_SUBDET_INDEX];
    }

    // LAr part
    region_index = m_helper->lar_em_region_index();
    if (m_lar_dict && AtlasDetectorIDHelper::UNDEFINED != region_index) {
        region = m_lar_dict->m_regions[region_index];
        m_lar_part_impl        = region->m_implementation[m_SUBDET_INDEX];
    }

    // LAr part
    region_index = m_helper->lar_fcal_region_index();
    if (m_lar_dict && AtlasDetectorIDHelper::UNDEFINED != region_index &&
        m_LAR_FCAL_MODULE_INDEX != 999) {
        region = m_lar_dict->m_regions[region_index];
        m_lar_fcal_module_impl = region->m_implementation[m_LAR_FCAL_MODULE_INDEX];
    }

    // Muon station name
    region_index = m_helper->mdt_region_index();
    if (m_muon_dict && AtlasDetectorIDHelper::UNDEFINED != region_index) {
        region = m_muon_dict->m_regions[region_index];
        m_muon_station_name_impl = region->m_implementation[m_SUBDET_INDEX];

        // Muon MDT
        m_muon_mdt_impl          = region->m_implementation[m_MUON_SUBDET_INDEX];

        // Muon RPC
        region_index = m_helper->rpc_region_index();
	if (AtlasDetectorIDHelper::UNDEFINED != region_index) {
	  region = m_muon_dict->m_regions[region_index];
	  m_muon_rpc_impl          = region->m_implementation[m_MUON_SUBDET_INDEX];
	}

    }


    /*     std::cout << "decode index and bit fields for each level: " << std::endl;
     std::cout << "det               "
           << m_det_impl.show_to_string() << std::endl;
     std::cout << "indet part        "
           << m_indet_part_impl.show_to_string() << std::endl;
     std::cout << "calo side         "
           << m_calo_side_impl.show_to_string() << std::endl;
     std::cout << "lar part          "
           << m_lar_part_impl.show_to_string() << std::endl;
     std::cout << "muon station name "
           << m_muon_station_name_impl.show_to_string() << std::endl;
     std::cout << "muon mdt          "
           << m_muon_mdt_impl.show_to_string() << std::endl;
     std::cout << "muon rpc          "
           << m_muon_rpc_impl.show_to_string() << std::endl; */


    return (0);
}
