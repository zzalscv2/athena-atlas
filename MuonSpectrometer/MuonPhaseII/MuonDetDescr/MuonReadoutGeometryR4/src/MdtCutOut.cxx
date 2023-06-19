
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MdtCutOut.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
namespace MuonGMR4{
    bool MdtCutOut::operator<(const MdtCutOut& other) const {
        if (layer != other.layer) return layer < other.layer;
        return lastTube <  other.firstTube; 
    }
    bool operator<(const MdtCutOut& cutout, const IdentifierHash& hash){
        const uint16_t lay = MdtReadoutElement::layerNumber(hash);
        const uint16_t tube = MdtReadoutElement::tubeNumber(hash);
        if (lay != cutout.layer) return cutout.layer < lay;
        return cutout.lastTube < tube; 
    }
    bool operator<(const IdentifierHash& hash, const MdtCutOut& cutout){
        const uint16_t lay = MdtReadoutElement::layerNumber(hash);
        const uint16_t tube = MdtReadoutElement::tubeNumber(hash);
        if (lay != cutout.layer) return lay < cutout.layer;
        return tube < cutout.firstTube;
    }
    std::ostream& operator<<(std::ostream& ostr, const MdtCutOut& cutout) {
        ostr<<" cut tubes ";
        if (cutout.firstTube != cutout.lastTube) ostr<<" ["<<
            static_cast<int>(cutout.firstTube)<<"-"<<
            static_cast<int>(cutout.lastTube)<<"]";
        else ostr<<static_cast<int>(cutout.firstTube);
        ostr<<" in layer "<<static_cast<int>(cutout.layer);
        ostr<<" left X: "<<cutout.leftX<<", right X:"<<cutout.rightX;
        return ostr;
    }

}