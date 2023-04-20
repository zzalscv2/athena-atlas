/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCablingData/NrpcCablingData.h"

std::ostream& operator<<(std::ostream& ostr, const NrpcCablingOffData& obj) {
    ostr << "StationIndex: " << static_cast<int>(obj.stationIndex)
         << " eta: " << static_cast<int>(obj.eta)
         << " phi: " << static_cast<int>(obj.phi)
         << " doubletR: " << static_cast<int>(obj.doubletR)
         << " doubletPhi: " << static_cast<int>(obj.doubletPhi)
         << " doubletZ: " << static_cast<int>(obj.doubletZ)
         << " gasGap: " << static_cast<int>(obj.gasGap)
         << " measPhi: " << static_cast<int>(obj.measPhi);
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingOnData& obj) {
    ostr << " subDetector: " << static_cast<int>(obj.subDetector);
    ostr << " tdcSector: " << static_cast<int>(obj.tdcSector);
    ostr << " tdc: " << static_cast<int>(obj.tdc);
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingData& obj) {
    ostr << static_cast<const NrpcCablingOffData&>(obj);
    ostr << " strip:  " << static_cast<int>(obj.strip) << " --- ";
    ostr << static_cast<const NrpcCablingOnData&>(obj);
    ostr << " channelId: " << static_cast<int>(obj.channelId);
    return ostr;
}
