/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>

namespace MuonGMR4 {
  
bool doesNotDeform(const Amg::Transform3D& trans) {
    for (unsigned int d = 0; d < 3 ; ++d) {
        const double defLength = Amg::Vector3D::Unit(d).dot(trans.linear() * Amg::Vector3D::Unit(d));
        if (std::abs(defLength - 1.) > std::numeric_limits<float>::epsilon()) {
            return false;
        }
    }
    return true;
}
bool isIdentity(const Amg::Transform3D& trans) {
   return doesNotDeform(trans) && trans.translation().mag() < std::numeric_limits<float>::epsilon();
}

}  // namespace MuonGMR4
