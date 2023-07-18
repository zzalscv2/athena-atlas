/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNMENTDATA_ALINEPAR_H
#define MUONALIGNMENTDATA_ALINEPAR_H

#include "CLHEP/Geometry/Transform3D.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonAlignmentData/MuonAlignmentPar.h"
#include <array>
#include <climits>
#include <iostream>

class ALinePar : public MuonAlignmentPar {
public:
    // Default constructor
    ALinePar() = default;
    // destructor
    virtual ~ALinePar() override = default;
    
    /// amdb frame (s, z, t) = chamber frame (y, z, x)
    enum class Parameter{
       transS = 0, /// Translation along the s-axis
       transZ,     /// Translation along the z-axis
       transT,     /// Translation along the t-axis
       rotS,       /// Rotation around the s-axis
       rotZ,       /// Rotation around the z-axis
       rotT,       /// Rotation around the t-axis
       numPars
    };
    
    void setParameters(float s, float z, float t, float rotS, float rotZ, float rotT);
    float getParameter(const Parameter& p) const{
        return m_payload[static_cast<unsigned int>(p)]; 
    }
    /// Returns the final transformations of the A lines
    HepGeom::Transform3D deltaTransform() const;    
    Amg::Transform3D     delta () const;

    /// @brief  Returns true if at least one of the payload parameters is set
    operator bool () const {
      constexpr float validityCutOff = 1.e-5;
      return std::accumulate(m_payload.begin(),m_payload.end(),0.,
                        [](const float a, const float b){
                          return std::abs(a)+ std::abs(b);
                        }) >validityCutOff;
    }
private:
    std::array<float, static_cast<unsigned int>(Parameter::numPars)> m_payload{};
 
};

std::ostream& operator<<(std::ostream&, const ALinePar& par);


#endif  // MUONALIGNMENTDATA_ALINEPAR_H
