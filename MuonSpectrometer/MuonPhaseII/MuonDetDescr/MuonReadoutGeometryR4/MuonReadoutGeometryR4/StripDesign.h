/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_STRIPDESIGN_H
#define MUONREADOUTGEOMETRYR4_STRIPDESIGN_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <GeoModelUtilities/TransientConstSharedPtr.h>
#include <AthenaBaseComps/AthMessaging.h>
#include <optional>

namespace MuonGMR4 {
    /* 
     * Generic class describing the layout of a Muon strip detector (E.g. the Rpcs, Micromegas, Tgc, sTgc)
     * The local coordinate system is defined such that the strip lanes are parallel to the y-axis and the
     * positive x-axis points to the next adjacent strip. The StripDesign also describes the stereo layers of the
     * Micromega detectors. There, the strips are rotated around their nominal strip center. The center position is then
     * adapted accordingly to always return the geometrical bisect of each strip lane
     * 
     * In terms of geometrical layout it is assumed that the strips are mounted onto a trapezoid where the two parallel edges are
     * parallel to the strip lanes. Its center is the origin of the local coordinate system. Further it's assumed that there is no
     * passivation w.r.t. to the slopy edges. i.e. the strips end with the edge of the panel. 
     * By specifing the position of the first strip center w.r.t. trapezoid center, the strip pitch, the number of all strips and
     * the global number of the first strip w.r.t. the global strip numbering scheme, the layout of the strip detector is completely
     * determined.
    */ 
    class StripDesign;
    using StripDesignPtr = GeoModel::TransientConstSharedPtr<StripDesign>;
    class StripDesign: public AthMessaging {
        public:
            StripDesign();
            virtual ~StripDesign() = default;

            /// Distance between two adjacent strips
            double stripPitch() const;
            /// Width of a strip
            double stripWidth() const;
            /// Number of strips on the panel
            int numStrips() const;
            /// Defines the layout of the strip detector by specifing the position of the first strip w.r.t.
            /// the layer center, the pitch to the next strip, the corresponding width of each strip,
            /// the total number of strips and finally the number of the first strip in the global numbering scheme
            void defineStripLayout(Amg::Vector2D&& posFirst,
                                   const double stripPitch,
                                   const double stripWidth,
                                   const int numStrips,
                                   const int numFirst = 1);
       
            /// Returns the half height of the strip panel
            double halfWidth() const;
            /// Returns the shorter half height of the panel
            double shortHalfHeight() const;
            /// Returns the longer half height of the panel
            double longHalfHeight() const;
            
            /// Returns whether a stereo angle is defined
            bool hasStereoAngle() const;
            /// Returns the value of the stereo angle
            double stereoAngle() const;
            /// Returns whether the trapezoid is flipped
            bool isFlipped() const;

            /// Defines the edges of the trapezoid
            void defineTrapezoid(double HalfShortY, double HalfLongY, double HalfHeight);
            /// Defines the edges of the trapezoid with stereo angle
            void defineTrapezoid(double HalfShortY, double HalfLongY, double HalfHeight, double sAngle);
            /// Flips the edges of the trapezoid boundaries by 90 degrees clockwise
            void flipTrapezoid();

    
            /// Returns the distance to the strip center along x
            double distanceToStrip(const Amg::Vector2D& pos, int strip) const;

            /// Calculates the number of the strip whose center is closest to the given point. 
            /// If the point is outside of the panel, -1 is returned
            int stripNumber(const Amg::Vector2D& pos) const;

            /// Returns the number of the first strip
            int firstStripNumber() const;

            /// Returns the left edge of the strip (Global numbering scheme)
            std::optional<Amg::Vector2D> leftEdge(int stripNumb) const;
            /// Returns the right edge of the strip (Global numbering scheme)
            std::optional<Amg::Vector2D> rightEdge(int stripNumb) const;
            /// Returns the bisector of the strip (Global numbering scheme)
            std::optional<Amg::Vector2D> center(int stripNumb) const;
            /// Odering operator
            bool operator<(const StripDesign& other) const;
            /// Returns length of the strip
            double stripLength(int stripNumb) const;
        protected:
            /// Calculates the position of a given strip (Local numbering scheme)
            virtual Amg::Vector2D stripPosition(int stripNum) const;
            /// Returns the intersection of a given strip with the left or right edge of the trapezoid
            /// If uncapped is set to false and the strip is a routed strip, then the intersection onto
            /// the corresponding bottom / top edges are returned
            Amg::Vector2D leftInterSect(int stripNum, bool uncapped = false) const;
            Amg::Vector2D rightInterSect(int stripNum, bool uncapped = false) const;
            
            Amg::Vector2D leftInterSect(const Amg::Vector2D& stripPos, bool uncapped = false) const;
            Amg::Vector2D rightInterSect(const Amg::Vector2D& stripPos, bool uncapped = false) const;

            /// Returns the geometrical center of a given strip
            Amg::Vector2D stripCenter(int stripNum) const;
        private:
            void setStereoAngle(double stereo);
            /// Shift between the 0-th readout channel and the first strip described by the panel
            int m_channelShift{1};
            /// Number of all strips
            int m_numStrips{0};
            /// Distance between 2 adjacent strip centers
            double m_stripPitch{0.};
            /// Width of each strip line
            double m_stripWidth{0.};
            /// First strip position
            Amg::Vector2D m_firstStripPos{Amg::Vector2D::Zero()};
  
            /// Orientiation of the strips along the panel
            Amg::Vector2D m_stripDir{Amg::Vector2D::UnitY()};
            /// Vector pointing from strip N to the next strip
            Amg::Vector2D m_stripNormal{Amg::Vector2D::UnitX()};
            
            /// Flag telling whether the trapezoid has been flipped
            bool m_isFlipped{false};
            /// Flag telling whether the strip design has a stereo angle or not
            bool m_hasStereo{false};
            /// Stereo angle of the strip design
            double m_stereoAngle{0.};
            /// Matrix to translate from nominal -> stereo frame
            AmgSymMatrix(2) m_stereoRotMat{AmgSymMatrix(2)::Identity()};
            /// Matrixt to translate from stereo -> nominal frame
            AmgSymMatrix(2) m_nominalRotMat{AmgSymMatrix(2)::Identity()};
        protected:    
            /// Bottom left point of the trapezoid
            Amg::Vector2D m_bottomLeft{Amg::Vector2D::Zero()};
            /// Top right point of the trapezoid
            Amg::Vector2D m_topLeft{Amg::Vector2D::Zero()};
        private:
            /// Bottom right point of the trapezoid
            Amg::Vector2D m_topRight{Amg::Vector2D::Zero()};
           /// Bottom right point of the trapezoid
            Amg::Vector2D m_bottomRight{Amg::Vector2D::Zero()};
        protected:
            /// Vector describing the top edge of the trapzoid (top left -> top right)
            Amg::Vector2D m_dirTopEdge{Amg::Vector2D::Zero()};
            /// Vector describing the bottom edge of the trapezoid (bottom left -> bottom right)
            Amg::Vector2D m_dirBotEdge{Amg::Vector2D::Zero()};
        private:   
            /// Vector describing the left adge of the trapezoid (bottom left -> top left)
            Amg::Vector2D m_dirLeftEdge{Amg::Vector2D::UnitY()};
            /// Vector describing the right edge of the trapezoid (bottom right -> top right)
            Amg::Vector2D m_dirRightEdge{Amg::Vector2D::UnitY()};
            /// Length of the edge connecting the short with the long egde 
            double m_lenSlopEdge{0.};            
            /// Trapezoid dimensions
            double m_shortHalfY{0.};
            double m_longHalfY{0.};
            double m_halfX{0.};
    };
    
    struct StripDesignSorter{
        bool operator()(const StripDesignPtr&a, const StripDesignPtr& b) const {
             return (*a) < (*b);
        }
        bool operator()(const StripDesign&a ,const StripDesign& b) const {
            return a < b;
        }
    };
  
    std::ostream& operator<<(std::ostream& ostr, const StripDesign& design);
}
#include <MuonReadoutGeometryR4/StripDesign.icc>
#endif