/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDDATA_DigitEffiData_H
#define MUONCONDDATA_DigitEffiData_H

//Athena includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h"
#include "AthenaBaseComps/AthMessaging.h"

/**
 * Container class that stores the efficiencies of the sTgcs. 
 * Each gasGap has its own efficiency assigned. 
*/
class DigitEffiData: public AthMessaging {
    public:
        DigitEffiData(const Muon::IMuonIdHelperSvc* idHelperSvc);
        /// Returns the signal generation efficiency of the sTgc channel
        double getEfficiency(const Identifier& channelId) const;
        /// Sets the efficiency of the sTgc gasGap. Returns false if the efficiency
        /// would be overwritten
        StatusCode setEfficiency(const Identifier& gasGapId, const double effi);
    private:
        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        using EffiMap = std::unordered_map<Identifier, double>; 
        EffiMap m_effiData{};
};

CLASS_DEF( DigitEffiData , 122326933 , 1 );
CONDCONT_DEF( DigitEffiData , 136592013 );
#endif
