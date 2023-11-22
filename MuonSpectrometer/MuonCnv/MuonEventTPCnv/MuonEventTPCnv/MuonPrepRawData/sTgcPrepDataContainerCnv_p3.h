/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef sTgcPREPDATACONTAINERCNV_p3_H
#define sTgcPREPDATACONTAINERCNV_p3_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "MuonEventTPCnv/MuonPrepRawData/MuonPRD_Container_p2.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkEventCnvTools/IEventCnvSuperTool.h"


#include <iostream>

class StoreGateSvc;
class sTgcIdHelper;


namespace Muon{
/** Class to handle the conversion of the transient sTgcPrepDataContainer into its persistent representation.
It is a copy of sTgcPrepDataContainerCnv_p1*/
class sTgcPrepDataContainerCnv_p3 : public T_AthenaPoolTPCnvBase<Muon::sTgcPrepDataContainer, Muon::sTgcPrepDataContainer_p3>
{
public:
    sTgcPrepDataContainerCnv_p3() = default;

    virtual void	persToTrans(const Muon::sTgcPrepDataContainer_p3* persCont,
        Muon::sTgcPrepDataContainer* transCont,
        MsgStream &log) ;
    virtual void	transToPers(const Muon::sTgcPrepDataContainer* transCont,
        Muon::sTgcPrepDataContainer_p3* persCont,
        MsgStream &log) ;

    virtual Muon::sTgcPrepDataContainer* createTransient(const Muon::sTgcPrepDataContainer_p3* persObj, MsgStream& log);


private:
    const MuonGM::sTgcReadoutElement* getReadOutElement(const Identifier& id ) const;
    const sTgcIdHelper *m_sTgcId{nullptr};
    StoreGateSvc *m_storeGate{nullptr};
    ToolHandle  < Trk::IEventCnvSuperTool >   m_eventCnvTool{"Trk::EventCnvSuperTool/EventCnvSuperTool"}; 
    bool m_isInitialized{false};
    StatusCode initialize(MsgStream &log);
};
}


#endif


