/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometry/MuonChannelDesign.h"
namespace MuonGM{
    MuonChannelDesign::MuonChannelDesign() {
        m_rotMat.setIdentity();
    }
    void  MuonChannelDesign::defineTrapezoid(double HalfShortY, double HalfLongY, double HalfHeight, double sAngle){
        defineTrapezoid(HalfShortY,HalfLongY, HalfHeight);
        setStereoAngle(sAngle);
    }
     void MuonChannelDesign::setStereoAngle(double sAngle) {
        m_sAngle = sAngle;
        m_hasStereo = (sAngle !=0.);
        Eigen::Rotation2D rot{sAngle};
        m_stereoDir = rot * m_stereoDir;
        m_stereoNormal = rot * m_stereoNormal;
        m_rotMat = Eigen::Rotation2D{-sAngle};
    }

     void MuonChannelDesign::defineTrapezoid(double HalfShortY, double HalfLongY, double HalfHeight) {
        m_bottomLeft = Amg::Vector2D{-HalfHeight, -HalfShortY};
        m_bottomRight = Amg::Vector2D{HalfHeight, -HalfLongY};
        m_topLeft = Amg::Vector2D{-HalfHeight, HalfShortY};        
        m_topRight = Amg::Vector2D{HalfHeight, HalfLongY};
       
        m_bottomEdge = (m_bottomRight - m_bottomLeft).unit();
        m_topEdge =   (m_topLeft - m_topRight);
        m_maxHorSize = std::hypot(m_topEdge.x(), m_topEdge.y());
        m_topEdge = m_topEdge.unit();
       
       
        m_xSize = 2.*HalfHeight;     // radial length (active area)
        m_minYSize = 2.*HalfShortY;  // bottom length (active area)
        m_maxYSize = 2.*HalfLongY;   // top length (active area)
        inputLength = m_minYSize;
    }
    void MuonChannelDesign::setFirstPos(const double firstPos) {
         m_firstPos = firstPos;
    }
}