/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONMDT_CABLING_MICROMEGA_CABLINGMAP_H
#define MUONMDT_CABLING_MICROMEGA_CABLINGMAP_H

#include <MuonCablingData/MicroMegaZebraData.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

#include <optional>

/**********************************************
 *
 * @brief MM cabling map data object
 *
 **********************************************/

class MicroMega_CablingMap {
   public:
    MicroMega_CablingMap(const Muon::IMuonIdHelperSvc* svc);

    // The following function corrects the MM cabling. It takes an identifier as
    // input (nominal identifer from the decoder) and returns
    //      The same identifier if there is no shift needed for this channel
    //      The new identifier containing the strip shift if a shift is needed
    //      A nullopt if the correction would move a channel outside the defined
    //      range of channel to be shifted (e.g. the first channel of a zebra
    //      connector which would be shifted into the channel range of the
    //      previous connector wich is physically impossible.)
    std::optional<Identifier> correctChannel(const Identifier& id,
                                             MsgStream& msg) const;

    // Function to add a range of channels to be shifted to the MM cabling map.
    bool addConnector(const Identifier& gapID,
                      const MicroMegaZebraData& connector, MsgStream& msg);

   private:
    const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};

    // Map holding the MM cabling correction map. The key is the identifier of
    // the gas gap and the value is a  MicroMegaZebraSet object containing the
    // information on which channel range needs to be moved
    using LookUpMap = std::map<Identifier, MicroMegaZebraSet>;
    LookUpMap m_cablingMap{};
};
std::ostream& operator<<(std::ostream& ostr,
                         const MicroMegaZebraData& connector);
CLASS_DEF(MicroMega_CablingMap, 85614785, 1);
#include "AthenaKernel/CondCont.h"
CONDCONT_DEF(MicroMega_CablingMap, 164588835);

#endif