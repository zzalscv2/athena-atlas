/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWDCSDBDATA_H
#define MUONCONDDATA_NSWDCSDBDATA_H

// STL includes
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

    // helper functions
    static DcsFsmState getFsmStateEnum(std::string fsmState);
    static std::string getFsmStateStrg(DcsFsmState fsmState);
 
private:
    
    int identToModuleIdx(const Identifier& chan_id) const;
    // Copied from https://gitlab.cern.ch/atlas/athena/-/blob/master/MuonSpectrometer/MuonDetDescr/MuonReadoutGeometry/MuonReadoutGeometry/MuonDetectorManager.h
    enum sTgcGMRanges {
            NsTgStatEta = 6,      /// 3 x 2 sides (-3,-2,-1 and 1,2,3)
            NsTgStEtaOffset = 3,  /// needed offest to map (-3,-2,-1,1,2,3) to (0,1,2,3,4,5)
            NsTgStatPhi = 16,     // large and small sector together
            NsTgcStatLay = 4,   // 4 wedges of stgcs
            NsTgcChannelTypes =3, // Pads / Wires / Strips
            NsTgChamberLayer = 2
        };
    enum mmGMRanges {
            NMMcStatEta = 4,      /// 2 x 2 sides (-2,-1 and 1,2)
            NMMcStEtaOffset = 2,  /// needed offest to map (-2,-1,1,2) to (0,1,2,3)
            NMMcStatPhi = 16,     // large and small sector together
            NMMcStatLay = 4, /// 4 wedges of micromegas
            NMMcChamberLayer = 2
        };
    
    static constexpr int s_NumMaxSTgcElemets = NsTgStatEta * NsTgStatPhi * NsTgChamberLayer *NsTgcStatLay * NsTgcChannelTypes;
    static constexpr int s_NumMaxMMElements = NMMcStatEta * NMMcStatPhi * NMMcChamberLayer *NMMcStatLay;

    // containers
    struct DcsModule{
        std::vector<std::unique_ptr<DcsConstants>> channels{};
        Identifier layer_id{0};
    };
    using ChannelDcsMapMMG = std::array<DcsModule, s_NumMaxMMElements >;
    using ChannelDcsMapSTG = std::array<DcsModule, s_NumMaxSTgcElemets>;
    ChannelDcsMapMMG m_data_hv_mmg{};
    ChannelDcsMapMMG m_data_hv_mmd{};
    ChannelDcsMapSTG m_data_hv_stg{};

    // ID helpers
    const MmIdHelper&   m_mmIdHelper;
    const sTgcIdHelper& m_stgcIdHelper;

};

std::ostream& operator<<(std::ostream& ostr, const NswDcsDbData::DcsConstants& obj);

CLASS_DEF( NswDcsDbData , 99551304 , 1 )
CLASS_DEF( CondCont<NswDcsDbData> , 125092872 , 1 )

#endif
