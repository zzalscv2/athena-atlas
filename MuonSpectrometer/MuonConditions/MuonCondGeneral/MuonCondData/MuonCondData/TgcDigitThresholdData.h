/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDDATA_TGCDIGITTHRESHOLDDATA_H
#define MUONCONDDATA_TGCDIGITTHRESHOLDDATA_H

//STL includes
#include <map>

//Athena includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h"
#include "AthenaBaseComps/AthMessaging.h"

/**
 *  Conditions object to cache 
 **/
class TgcDigitThresholdData: public AthMessaging {
    public:
        TgcDigitThresholdData(const Muon::IMuonIdHelperSvc* idHelperSvc);
        ~TgcDigitThresholdData() = default;

        bool setThreshold(const Identifier& layerId, const double threshold);
        
        double getThreshold(const Identifier& channelId) const;

    
    private:
        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        std::unordered_map<Identifier, double> m_thresholds{};
};

CLASS_DEF( TgcDigitThresholdData , 142756895 , 1 );
CONDCONT_DEF( TgcDigitThresholdData , 226980043 );
#endif