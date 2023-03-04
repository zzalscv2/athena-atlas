/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCABLINGDATA_MDTCSMCRATE_H
#define MUONCABLINGDATA_MDTCSMCRATE_H

#include <MuonCablingData/MdtCablingData.h>
#include <MuonCablingData/MdtMezzanineCard.h>

/// @brief MdtCSMCrate - The signals from the mezzanine cards are sent to the
///                      CSM units. The position of each card is identified by
///                      the tdcId on CSM.
///
/// The MdtCSMCrate class bundles all the mezzanines on the chamber to translate
/// between online and offline Identifiers

class MdtCSMCrate {
    public:
        /// Struct to map the tdcs onto the chamber.
        /// Each chip is designated by its id, 
        /// the first tube number in the bottom layer, and
        /// the mezzanine layout.
        struct TdcCard {
            uint8_t tdcId{0};
            uint8_t firstTube{0};
            std::shared_ptr<const MdtMezzanineCard> mezzanine{nullptr};
        };
        ///@brief Standard constructor of the crate taking the cabling data
        ///       identifier encoding the MDT multilayer ID and the MROD+CSM ID 
        MdtCSMCrate(const MdtCablingData& id);       
        
        /// @brief checks whether all of the tdcs are well mapped
        bool checkConsistency(MsgStream& msg) const;
        
        /// @brief  Returns the fields of the offline multilayer identifier        
        const MdtCablingOffData& offlineId() const {
            return static_cast<const MdtCablingOffData&>(m_Id);
        }
        
        /// @brief  Returns the fields of the online chamber identifier 
        const MdtCablingOnData& csmId() const {
            return static_cast<const MdtCablingOnData&>(m_Id);
        }
        
        /// @brief  Picks up the offline identifier in the MdtCablingData and transforms it
        ///         to the online Identifier + tdc channel
        /// @param channel Cabling data object containing valid offline Id fields + tubelayer + tube
        /// @param msg Log object
        /// @return Flag whether the conversion succeeded or not
        bool getOnlineId(MdtCablingData& channel, MsgStream& msg) const;
        
        /// @brief  Picks up the online identifier in the MdtCablingData and transforms it
        ///         to the multilayer identifier + tubelayer + tube
        /// @param channel Cabling data object containing valid online Id fields + tdc channel
        /// @param msg Log object
        /// @return Flag whether the conversion succeeded or not
        bool getOfflineId(MdtCablingData& channel, MsgStream& msg) const;

        bool addTdc(TdcCard&& card, MsgStream& msg);

    private:
        /// Chamber identifier
        MdtCablingData m_Id{};
        /// @brief  Vector of tdc chips
        std::vector<TdcCard> m_tdcs{};
};

#endif