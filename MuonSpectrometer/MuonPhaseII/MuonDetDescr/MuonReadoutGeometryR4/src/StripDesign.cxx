/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonReadoutGeometryR4/StripDesign.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <climits>
namespace {
    constexpr double tolerance = 0.001 * Gaudi::Units::mm;
}
/// Helper macro to facilliate the ordering
#define ORDER_PROP(PROP)                                            \
      {                                                             \
        if (std::abs(PROP - other.PROP) > tolerance) {              \
            return PROP < other.PROP;                               \
        }                                                           \
      }

namespace MuonGMR4{
    StripDesign::StripDesign():
        AthMessaging{"MuonStripDesign"} {}
    bool operator<(const StripDesignPtr&a, const StripDesignPtr& b) {
        return (*a) < (*b);
    }
    bool StripDesign::operator<(const StripDesign& other) const {
        ORDER_PROP(firstStripNumber());
        ORDER_PROP(numStrips());
        ORDER_PROP(stripPitch());
        ORDER_PROP(stripWidth());
        ORDER_PROP(halfWidth());
        ORDER_PROP(longHalfHeight());
        ORDER_PROP(shortHalfHeight());
        ORDER_PROP(stereoAngle());
        {
            const Amg::Vector2D dP = m_firstStripPos - other.m_firstStripPos;
            if (std::hypot(dP.x(), dP.y()) > tolerance) {
                if (dP.x() > tolerance) return dP.x()< 0.;
                return dP.y() < 0.;
            }
        }
        return isFlipped() < other.isFlipped();
    }
    std::ostream& operator<<(std::ostream& ostr, const StripDesign& design) {
        design.print(ostr);
        return ostr;
    }
    void StripDesign::print(std::ostream& ostr) const {
        ostr<<"Strip -- number: "<<numStrips()<<", ";        
        ostr<<"pitch: "<<stripPitch()<<", ";
        ostr<<"width: "<<stripWidth()<<", ";
        ostr<<"Dimension  -- width x height:"<<halfWidth() * Gaudi::Units::mm<<" [mm] x ";
        ostr<<shortHalfHeight()<<"/"<<longHalfHeight()<<" [mm], ";
        if (hasStereoAngle()) ostr<<"stereo angle: "<<stereoAngle() / Gaudi::Units::deg<<", ";
        ostr<<"position first strip "<<Amg::toString(center(firstStripNumber()).value_or(Amg::Vector2D::Zero()),1);
        ostr<<"Trapezoid edges "<<Amg::toString(m_bottomLeft,1)<<" - "<<Amg::toString(m_bottomRight, 1)<<" --- ";
        ostr<<Amg::toString(m_topLeft, 1)<<" - "<<Amg::toString(m_topRight, 1);
    }
    void StripDesign::defineTrapezoid(double HalfShortY, double HalfLongY, double HalfHeight, double sAngle){
        defineTrapezoid(HalfShortY,HalfLongY, HalfHeight);
        setStereoAngle(sAngle);
    }

    void StripDesign::setStereoAngle(double sAngle) {
        if (std::abs(sAngle) < std::numeric_limits<float>::epsilon()) return;
        m_stereoAngle = sAngle;
        m_hasStereo = true;
        Eigen::Rotation2D rot{sAngle};
        m_stripDir = rot * m_stripDir;
        m_stripNormal = rot * m_stripNormal;
        m_stereoRotMat = Eigen::Rotation2D{-sAngle};
        m_nominalRotMat = rot;
    }

    void StripDesign::defineTrapezoid(double HalfShortY, double HalfLongY, double HalfHeight) {
        m_bottomLeft = Amg::Vector2D{-HalfHeight, -HalfShortY};
        m_bottomRight = Amg::Vector2D{HalfHeight, -HalfLongY};
        m_topLeft = Amg::Vector2D{-HalfHeight, HalfShortY};
        m_topRight = Amg::Vector2D{HalfHeight, HalfLongY};

        m_dirBotEdge = (m_bottomRight - m_bottomLeft).unit();
        m_dirTopEdge = (m_topRight - m_topLeft);
        m_lenSlopEdge = std::hypot(m_dirTopEdge.x(), m_dirTopEdge.y());
        m_dirTopEdge = m_dirTopEdge.unit();

        m_shortHalfY = HalfShortY;
        m_longHalfY = HalfLongY;
        m_halfX = HalfHeight;       
    }
    void StripDesign::flipTrapezoid() {
        if (m_isFlipped) {
            ATH_MSG_WARNING("It's impossible to flip a trapezoid twice. Swap short and long lengths");
            return;
        }
        m_bottomLeft = Amg::Vector2D{-m_shortHalfY, -m_halfX};
        m_bottomRight = Amg::Vector2D{m_shortHalfY, -m_halfX};
        m_topLeft = Amg::Vector2D{-m_longHalfY, m_halfX};
        m_topRight = Amg::Vector2D{m_longHalfY, m_halfX};

        m_dirBotEdge = (m_bottomRight - m_bottomLeft).unit();
        m_dirTopEdge = (m_topRight - m_topLeft).unit();
        m_dirLeftEdge = (m_topLeft - m_bottomLeft);
        m_lenSlopEdge = std::hypot(m_dirLeftEdge.x(), m_dirLeftEdge.y());
        m_dirLeftEdge = m_dirLeftEdge.unit();
        m_dirRightEdge = (m_topRight - m_bottomRight).unit();
        m_isFlipped = true;
    }
    void StripDesign::defineStripLayout(Amg::Vector2D&& posFirst,
                                        const double stripPitch,
                                        const double stripWidth,
                                        const int numStrips,
                                        const int numFirst) {        
        m_channelShift = numFirst;
        m_numStrips = numStrips;
        m_stripPitch = stripPitch;
        m_stripWidth = stripWidth;
        m_channelShift = numFirst;
        m_firstStripPos = std::move(posFirst);
    }


}
#undef ORDER_PROP 
