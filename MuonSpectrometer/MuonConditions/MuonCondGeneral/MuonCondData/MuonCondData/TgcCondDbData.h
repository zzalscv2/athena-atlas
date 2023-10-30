/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDDATA_TGCCONDDBDATA_H
#define MUONCONDDATA_TGCCONDDBDATA_H

//STL includes
#include <string>
#include <set>

//Athena includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "Identifier/Identifier.h"
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 

/**
 *  Conditions object to mark switched-off Tgc gasGaps
*/
class TgcCondDbData {

public:
    TgcCondDbData(const Muon::IMuonIdHelperSvc* idHelperSvc);
    virtual ~TgcCondDbData() = default;
    /// Declare all channels wires + strips in a gasGap as dead
    void setDeadGasGap(const Identifier& id);
    /// Returns whether the channelId does not belong to a dead gasGap
    bool isGood(const Identifier & channelId) const;

private:
    const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
    std::unordered_set<Identifier> m_cachedDeadStationsId{};

};

CLASS_DEF( TgcCondDbData, 130737053, 1);
CONDCONT_DEF( TgcCondDbData , 178252645 );
#endif
