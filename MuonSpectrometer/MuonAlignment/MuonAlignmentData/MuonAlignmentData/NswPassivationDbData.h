/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWPASSIVATIONDBDATA_H
#define MUONCONDDATA_NSWPASSIVATIONDBDATA_H

// STL includes
#include <unordered_map>

// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h"
#include "Identifier/Identifier.h"

// Forward declarations
class MmIdHelper;


class NswPassivationDbData {

public:
    /// Helper struct to save the four passivation values of each PCB
    struct PCBPassivation{        
        double left{0.};
        double right{0.};
        double top{0.};
        double bottom{0.};        
        bool valid{false};
    };
    
    NswPassivationDbData(const MmIdHelper&);
    virtual ~NswPassivationDbData() = default;

    // setting functions
    void setData(const Identifier& chnlId, const int pcb, const float indiv, const float extra, const std::string& position);

    // retrieval functions
    std::vector<Identifier> getChannelIds() const;
    const PCBPassivation& getPassivation(const Identifier& id) const;
private:
    /// ID helpers
    const MmIdHelper& m_mmIdHelper;
    // containers
    using PassivationMap = std::unordered_map<Identifier::value_type, PCBPassivation>;
    PassivationMap m_data{};

   

};

CLASS_DEF( NswPassivationDbData , 183672311 , 1 );
CONDCONT_DEF( NswPassivationDbData , 139639445 );

#endif
