/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#ifndef BEAMSPOTCONDITIONSDATA_BEAMSPOTDATA_H
#define BEAMSPOTCONDITIONSDATA_BEAMSPOTDATA_H

#include "AthenaKernel/CondCont.h"
#include "AthenaKernel/CLASS_DEF.h"

#include "GeoPrimitives/GeoPrimitives.h"

#ifndef SIMULATIONBASE
#include "VxVertex/RecVertex.h"
#endif

namespace InDet
{

  class BeamSpotData final //Class declared as final because no virtual destructor is used
                           //if you want to inherit, remove final keyword and add virtual destructor
  {
  public:
    BeamSpotData(int status, float posX, float posY, float posZ,
		 float sigmaX, float sigmaY, float sigmaZ,
		 float tiltX, float tiltY, float sigmaXY);

    BeamSpotData() = delete; //Forbid undefined behaviour

    const Amg::Vector3D& beamPos() const;

   /**
    * @brief Returns the beam sigma for the i-th error matrix element
    * @return The matrix element as float
    * @param i the requested index
    */
    float beamSigma(int i) const;

    float beamSigmaXY() const;

   /**
    * @brief Returns the beam sigma for the i+3-th error matrix element (the 'tilt')
    * @return The (i + 3)th element as float
    * @param i the requested index after the addition of 3
    */
    float beamTilt(int i) const;

    int beamStatus() const;

#ifndef SIMULATIONBASE
    const Trk::RecVertex& beamVtx() const;
#endif
    

  private:

    int m_status;
    float m_posX;
    float m_posY;
    float m_posZ;
    float m_sigmaX;
    float m_sigmaY;
    float m_sigmaZ;
    float m_tiltX;
    float m_tiltY;
    float m_sigmaXY;

    std::vector<float> m_errPar;

    Amg::Vector3D m_beamPos;

#ifndef SIMULATIONBASE
    Trk::RecVertex m_vertex;
#endif

  };

  inline const Amg::Vector3D& BeamSpotData::beamPos() const { return m_beamPos; }

  inline float BeamSpotData::beamSigma(int i) const { return m_errPar[i]; }

  inline float BeamSpotData::beamSigmaXY() const { return m_errPar[5]; }

  inline float BeamSpotData::beamTilt(int i) const { return m_errPar[3+i]; }

  inline int BeamSpotData::beamStatus() const { return m_status; }

#ifndef SIMULATIONBASE
  inline const Trk::RecVertex& BeamSpotData::beamVtx() const { return m_vertex; }
#endif

}

CLASS_DEF(InDet::BeamSpotData,188569010,1)
CONDCONT_DEF(InDet::BeamSpotData,34275740);


#endif
