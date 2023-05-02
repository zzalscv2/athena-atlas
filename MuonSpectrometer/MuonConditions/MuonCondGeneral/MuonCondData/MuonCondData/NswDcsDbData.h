/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWDCSDBDATA_H
#define MUONCONDDATA_NSWDCSDBDATA_H

// STL includes
#include <set>
#include <vector>

// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 
#include "MuonIdHelpers/IMuonIdHelperSvc.h"


class NswDcsDbData {  

public:
    enum class DcsTechType{
        MMG, // MM channels
        MMD, // MM drift channels
        STG  // sTGC
    };
    enum class DcsDataType{
        HV,
        LV
    };
    enum class DcsFsmState{
        NONE,
        UNKNOWN,
        OFF,
        ON,
        STANDBY,
        DEAD,
        UNPLUGGED,
        RAMP_UP,
        RAMP_DOWN,
        TRIP,
        RECOVERY,
        LOCKED
    };
    /// Helper struct to cache all dcs constants 
    /// in a common place of the memory
    struct DcsConstants{
       float v0set{0.f};
       float v1set{0.f};
       DcsFsmState fsmState{DcsFsmState::NONE};
    };
    
    NswDcsDbData(const MmIdHelper& mmHelper, const sTgcIdHelper& stgcHelper);
    virtual ~NswDcsDbData() = default;

    // setting functions
    void setData(const DcsTechType tech, const Identifier& chnlId, DcsConstants constants);
    
    // retrieval functions
    
    //// Retrieves the list of all identifiers for which calibration channels are available
    std::vector<Identifier> getChannelIds(const DcsTechType tech, const std::string& side) const;
    /// Retrieves the calibration constant for a particular readout channel.
    const DcsConstants* getDataForChannel(const DcsTechType tech, const Identifier& channelId) const; 
    
    /// Returns whether the channel is alive, i.e. DCS state on, etc...
    bool isGood(const Identifier& channelId, bool issTgcQ1OuterHv = false) const;
    bool isGoodHv(const Identifier& channelId, bool issTgcQ1OuterHv = false) const;

    
    // helper functions
    static DcsFsmState getFsmStateEnum(std::string fsmState);
    static std::string getFsmStateStrg(DcsFsmState fsmState);
 
private:
    
    unsigned int identToModuleIdx(const Identifier& chan_id) const;
    
    // containers
    struct DcsModule{
        std::vector<std::unique_ptr<DcsConstants>> channels{};
        Identifier layer_id{0};
    };
    using ChannelDcsMap = std::vector<DcsModule>;
    ChannelDcsMap m_data_hv_mmg{};
    ChannelDcsMap m_data_hv_mmd{};
    ChannelDcsMap m_data_hv_stg{};

    // ID helpers
    const MmIdHelper&   m_mmIdHelper;
    const sTgcIdHelper& m_stgcIdHelper;

};

std::ostream& operator<<(std::ostream& ostr, const NswDcsDbData::DcsConstants& obj);

CLASS_DEF( NswDcsDbData , 99551304 , 1 )
CLASS_DEF( CondCont<NswDcsDbData> , 125092872 , 1 )

#endif
