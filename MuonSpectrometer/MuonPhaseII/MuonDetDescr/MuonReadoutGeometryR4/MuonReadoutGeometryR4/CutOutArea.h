/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_CUTOUTAREA_H
#define MUONREADOUTGEOMETRYR4_CUTOUTAREA_H
#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>

namespace MuonGMR4{

    struct CutOutArea{
        /// Center of the cutout to be applied
        Amg::Vector3D origin{Amg::Vector3D::Zero()};
        /// Half length a long the X axis
        double halfLengthX{0.};
        /// Half length along the Z axis
        double lengthY{0.};
        ///
        Amg::Vector3D inclanation{Amg::Vector3D::Zero()};
        
    };
    inline std::ostream& operator<<(std::ostream& ostr, const CutOutArea& cut){
        ostr<<"Cut center "<<Amg::toString(cut.origin,1)<<", half X: "<<cut.halfLengthX<<", half Y "<<cut.lengthY;
        ostr<<", min X: "<<(cut.origin.x() - cut.halfLengthX)<<", max X: "<<(cut.origin.x() + cut.halfLengthX);
        ostr<<", min Y: "<<(cut.origin.y() - cut.lengthY)<<", max Y: "<<(cut.origin.y() + cut.lengthY);
        return ostr;
    }


}

#endif