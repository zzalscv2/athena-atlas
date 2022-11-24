/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 properties of a plane based detector allowing for a stereo angle
 ----------------------------------------------------------------------
***************************************************************************/

#ifndef MUONREADOUTGEOMETRY_MUONCHANNELDESIGN_H
#define MUONREADOUTGEOMETRY_MUONCHANNELDESIGN_H

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "EventPrimitives/EventPrimitives.h"
#include "MuonReadoutGeometry/GlobalUtilities.h"
#include "cmath"
namespace MuonGM {

    struct MuonChannelDesign {
    public:
        MuonChannelDesign();
        enum class ChannelType { etaStrip, phiStrip };
        enum class DetType { MM, STGC, Other };
        ChannelType type{ ChannelType::etaStrip };
        DetType  detType{ DetType::Other };
        int nch{-1};             // total #of active strips
        double inputPitch{0.};   // we use this param to define the pitch for MM
        double inputWidth{0.};
        double inputLength{0.};
        double firstPitch{0.};   // Pitch of 1st strip or number of wires in 1st group
        double groupWidth{0.};   // Number of Wires per group
        double nGroups{0.};      // Number of Wire groups
        double wireCutout{0.};
        double thickness{0.};    // gas thickness
        double yCutout{0.};
        int nMissedTopEta{0};    // #of strips that are not connected to any FE boards (MM)
        int nMissedBottomEta{0};
        int nMissedTopStereo{0};
        int nMissedBottomStereo{0};
        int totalStrips{0};      // total strips per MM module

        /// distance to readout 
        double distanceToReadout(const Amg::Vector2D& pos) const;

        /// distance to channel - residual 
        double distanceToChannel(const Amg::Vector2D& pos, int nChannel) const;

        /// calculate local channel number, range 1=nstrips like identifiers. Returns -1 if out of range 
        int channelNumber(const Amg::Vector2D& pos) const;

        /// calculate local wire group number, range 1=64 like identifiers. Returns -1 if out of range 
        int wireGroupNumber(const Amg::Vector2D& pos) const;

        /// calculate the sTGC wire number. The method can return a value outside the range [1, nch].
        int wireNumber(const Amg::Vector2D& pos) const;

        /// calculate local channel width 
        double channelWidth() const;
       
        /// set the trapezoid dimensions 
        void defineTrapezoid(double HalfShortY, double HalfLongY, double HalfHeight);
        void defineTrapezoid(double HalfShortY, double HalfLongY, double HalfHeight, double sAngle);

        /// returns the stereo angle 
        double stereoAngle() const { return m_sAngle; }

        /// returns whether the stereo angle is non-zero 
        double hasStereoAngle() const {return m_hasStereo;}

        /// returns the rotation matrix between eta <-> phi layer 
        const AmgSymMatrix(2)& rotation() const { return m_rotMat; }

        /// calculate channel length for a given channel number 
        double channelLength(int channel) const;
        
        /// calculate channel length on the given side of the x axis (for MM stereo strips) 
        double channelHalfLength(int st, bool left) const;

        /// thickness of gas gap 
        double gasGapThickness() const { return thickness; }

        /// Returns the center on the strip
        bool center(int channel, Amg::Vector2D& pos) const;
        /// Returns the left edge of the strip
        bool leftEdge(int channel, Amg::Vector2D& pos) const;
        /// Returns the right edge of the strip
        bool rightEdge(int channel, Amg::Vector2D& pos) const;
        /// Returns the number of missing top channels
        int numberOfMissingTopStrips() const;
        /// Returns the number of missing bottom strips
        int numberOfMissingBottomStrips() const;

        // radial distance (active area)
        double xSize() const;
        // top length (active area)
        double maxYSize() const;
        // bottom length (active area)
        double minYSize() const;
        
        //// In the context of passivation the trapezoid is rotated by 90 degrees again
        ////  The short parallel edge (Left edge in the nominal description of the design) is the bottom
        ////      top edge -> left edge
        ///       bottom edge -> right edge
        /// a rectangle with height H parallel to the inclined edges is passivated
        double passivatedLength(double passivWidth, bool left)  const;
        /// Passivation is applied parallell to
        double passivatedHeight(double passivHeight, bool edge) const;

        /// Set the position of the first strip along the x-axis
        void setFirstPos(const double pos);
        /// Returns the position of the first strip along the x-axis
        double firstPos() const;

        /// Given an local position, lpos, which falls on strip #i, this function 
        /// expresses it w.r.t. the intersection of eta strip #i and stereo strip #i:
        /// - relative x is within [-0.5, 0.5]*pitch along the local x coordinate,
        /// - relative y within [-1, 1] along the local y coordinate.
        /// These coordinates are ready to be fed to the NswAsBuilt::StripCalculator        
        bool positionWithinStrip(const Amg::Vector2D& lpos, Amg::Vector2D& rel_pos) const;

    private:
        /// calculate local channel position for a given channel number 
        bool channelPosition(int channel, Amg::Vector2D& pos) const;
        ///Returns the intersection of the strip with the left edge
        bool leftInterSect(int channel, Amg::Vector2D& pos) const;
        /// Returns the right edge of the strip
        bool rightInterSect(int channel, Amg::Vector2D& pos) const;
        /// Returns the geometrical strip center
        bool geomCenter(int channel, Amg::Vector2D& pos) const;

        /// calculate local stereo angle 
        void setStereoAngle(double sAngle);
       
        bool m_hasStereo{false};
        double m_sAngle{0.};     // stereo angle
        /// Direction of the strips
        Amg::Vector2D m_stereoDir{0,1};        
        /// Direction pointing to the next strips
        Amg::Vector2D m_stereoNormal{1.,0.};
        /// Position of the first measurement
        double m_firstPos{0.};
        /// Vector describing the right edge of the trapzoid
        Amg::Vector2D m_topEdge{Amg::Vector2D::Zero()};
        /// Vector describing the left edge of the trapezoid
        Amg::Vector2D m_bottomEdge{Amg::Vector2D::Zero()};
        /// Bottom left point of the trapezoid
        Amg::Vector2D m_bottomLeft{Amg::Vector2D::Zero()};
        /// Bottom right point of the trapezoid
        Amg::Vector2D m_bottomRight{Amg::Vector2D::Zero()};
        /// Top right point of the trapezoid
        Amg::Vector2D m_topLeft{Amg::Vector2D::Zero()};
        /// Bottom right point of the trapezoid
        Amg::Vector2D m_topRight{Amg::Vector2D::Zero()};

        AmgSymMatrix(2) m_rotMat{AmgSymMatrix(2)::Identity()};

        double m_maxHorSize{0.};   // Maximum length of the horizontal edges
        double m_xSize{0.};        // radial distance (active area)
        double m_minYSize{0.};     // bottom length (active area)
        double m_maxYSize{0.};     // top length (active area)
     
    };

    //============================================================================
    inline double MuonChannelDesign::distanceToChannel(const Amg::Vector2D& pos, int chNum) const {
        Amg::Vector2D cen{Amg::Vector2D::Zero()};
        if (!center(chNum, cen)) return std::numeric_limits<float>::max();
        
        return (pos - cen).x();
    }
    

    //============================================================================
    inline double MuonChannelDesign::distanceToReadout(const Amg::Vector2D& pos) const {
        throw std::runtime_error("MuonChannelDesign::distanceToReadout() - is not properly defined");
        return pos.dot(pos);
    }



    //============================================================================
    inline int MuonChannelDesign::channelNumber(const Amg::Vector2D& pos) const {
        static const Amg::Vector2D x_axis{1.,0.};
           
        int chNum{-1};
        if (type == ChannelType::etaStrip && detType == DetType::MM) {
            // ** MM strips: keeping cases outside the active area, but within the envelope, 
            // to avoid warnings from MuonPRDTest. Those channels are removed from digitization.
            const Amg::Vector2D posInEta = m_rotMat.inverse()*pos;
            const double xMid = (posInEta - firstPos()*x_axis).dot(m_stereoNormal) / m_stereoNormal.x();
            const int missedBottom =  numberOfMissingBottomStrips();

            // first position is always 1/2 pitch above the center of the first active strip
            chNum = static_cast<int>(std::floor( xMid  / inputPitch)) + missedBottom + 2;
            if (chNum < 1 || chNum > totalStrips) chNum = -1;

        } else if (type == ChannelType::phiStrip && detType == DetType::STGC) {

            // ** sTGC wires: return the wire group
            chNum = wireGroupNumber(pos);

        } else {

            // ** All other cases
            chNum = (int)std::floor( (pos.x() - firstPos()) / inputPitch ) + 2;
            if (chNum < 1 || chNum > nch) chNum = -1;
        }
   
        return chNum;
    }


    //============================================================================
    inline int MuonChannelDesign::wireGroupNumber(const Amg::Vector2D& pos) const {
    
        // The wires in the 1st gas volume of QL1, QS1 can not be read for digits
        if (type != ChannelType::phiStrip || detType != DetType::STGC) return -1;
        
        // Get the wire number associated to the position
        int wire_number = wireNumber(pos);
        
        // Find the wire group associated to the wire number.
        // For wires, firstPitch is the number of wires in the 1st group.
        int grNumber{-1};
        if (wire_number <= firstPitch) {
            grNumber = 1;  
        } else {
            grNumber = (wire_number - 1 - firstPitch) / groupWidth + 2; // 20 wires per group,
            // If a hit is positionned after the last wire but inside the gas volume.
            // This is really a check for the few mm on the fringe of the gas volume
            // Especially important for QL3. We still consider the digit active.
            if (grNumber > nGroups && pos.x() < 0.5 * m_maxYSize) grNumber = nGroups;
        }

        // If hit is in an inactive wire region of QL1/QS1, return 63 (invalid).
        // This allows better tracking of hits.
        if (wireCutout != 0. && pos.y() < 0.5 * m_xSize - wireCutout) return 63;
        if (grNumber < 1 || grNumber > nGroups) return -1;
        
        return grNumber;
    }


    //============================================================================
    inline int MuonChannelDesign::wireNumber(const Amg::Vector2D& pos) const {
    
        // Only determine wire number for sTGC wire surfaces
        int wire_number{-1};
        if (type == ChannelType::phiStrip && detType == DetType::STGC) {
            if ((pos.x() > -0.5 * m_maxYSize) && (pos.x() < firstPos())) { // before first wire
                wire_number = 1;
            } else {
                wire_number = (pos.x() - firstPos()) / inputPitch + 1;
                // Print a warning if the wire number is outside the range [1, nch]
                if (wire_number < 1 || wire_number > nch) {
                    MsgStream log(Athena::getMessageSvc(), "MuonChannelDesign");
                    if (log.level() <= MSG::DEBUG) {
                        log << MSG::DEBUG << "sTGC wire number out of range: wire number = " << wire_number << " local pos = (" << pos.x() << ", " << pos.y() << ")" << endmsg;
                    }
                }
            }
        }

        return wire_number;
    }


    //============================================================================
    inline bool MuonChannelDesign::positionWithinStrip(const Amg::Vector2D& lpos, Amg::Vector2D& rel_pos) const {

        if (type != ChannelType::etaStrip) return false;

        const int istrip = channelNumber(lpos);
        if (istrip < 0) return false;

        // get the intersection of the eta and stereo strips in the local reference frame
        Amg::Vector2D chan_pos{Amg::Vector2D::Zero()};
        if (!channelPosition(istrip, chan_pos)) return false;
        chan_pos = rotation() * chan_pos;    

        // strip edge in the local reference frame
        Amg::Vector2D edge_pos{Amg::Vector2D::Zero()}; 
        if ( !((lpos.y() > 0) ? rightEdge(istrip, edge_pos) : leftEdge(istrip, edge_pos)) ) return false;

        rel_pos[0] = (lpos - chan_pos).x() / inputWidth;
        rel_pos[1] = (lpos - chan_pos).y() / std::abs( edge_pos.y() - chan_pos.y() );
        return true;
    }
    
    
    //============================================================================
    inline bool MuonChannelDesign::channelPosition(int st, Amg::Vector2D& pos) const {

        if (type == ChannelType::phiStrip) {

            if (st < 1 || st > nch) return false;
 
            if (detType == DetType::STGC) {  
             
                /// sTGC Wires: return the center of the wire group (not the wire)
                if (st > nGroups || st == 63) return false; // 63 is defined as an unactive digit

                // calculate the end of the first wire group (accounts for staggering).
                // for wires, firstPitch is the number of wires in the first group
                double firstX = firstPos() + (firstPitch - 1) * inputPitch;  
                double locX{0.};

                if (st == 1) {               
                    // first group: average the starting and ending x of the group
                    locX = 0.5 * (-0.5 * m_maxYSize + firstX);
                } else if (st == nGroups)  { 
                    // last group: average the starting and ending x of the group
                    locX = 0.5 * (0.5 * m_maxYSize + firstX + (nGroups - 2) * groupWidth * inputPitch);
                } else {
                    locX = firstX + groupWidth * inputPitch * (st - 1.5);
                }

                pos[0] = locX;
                pos[1] = 0.;

            } else {

                /// Default case for phi wires
                double dY   = 0.5 * (m_maxYSize - m_minYSize);
                double locY = firstPos() + (st-1)*inputPitch;
                double locX{0.};

                if (std::abs(locY) > 0.5*m_minYSize) {
                    locX = 0.5 * m_xSize *(1. - (0.5*m_maxYSize - std::abs(locY)) / dY);
                }
                pos[0] = locY;
                pos[1] = locX;
            }

        } else if (detType == DetType::MM) {
                
            /// MM eta strips (strip numbering starts at 1)
            const int nMissedBottom = numberOfMissingBottomStrips();
            if (st <= nMissedBottom || st > nMissedBottom + nch) return false;
            
            // firstPos is 1/2 pitch above the center of the first active strip.
            static const Amg::Vector2D x_axis{1.,0.};
            pos = (firstPos()  + inputPitch* (st - nMissedBottom - 1.5)) * x_axis;
        } else {

            /// sTGC and default case for eta strips
            if (st < 1 || st > nch) return false;

            double x = firstPos() + (st - 1.5)*inputPitch;
            if (detType == DetType::STGC) {
                if (firstPitch == inputPitch && st == nch) x -= inputWidth/4.; // case last strip is a half-strip
                if (firstPitch  < inputPitch && st ==   1) x += inputWidth/4.; // case first strip is a half-strip
            }

            pos[0] = x;
            pos[1] = 0;
        }

        return true;
    }
    
    //============================================================================
    inline double MuonChannelDesign::channelHalfLength(int st, bool left) const {
        Amg::Vector2D cen{Amg::Vector2D::Zero()}, side{Amg::Vector2D::Zero()};
        if (!geomCenter(st,cen) || (left&& !leftInterSect(st, side)) ||  (!left && !rightInterSect(st, side)))  return -0.5;
        if (type == ChannelType::etaStrip && detType == DetType::MM && (st < 1 || st > totalStrips)) return -0.5;           
        // default case:
        return (cen - side).mag();
    }
    

    //============================================================================
    inline double MuonChannelDesign::channelLength(int st) const {
        Amg::Vector2D left{Amg::Vector2D::Zero()}, right{Amg::Vector2D::Zero()};
        if(!leftInterSect(st,left)|| !rightInterSect(st, right)) return -0.5;
        return (left - right).mag();       
    }


    //============================================================================
    inline double MuonChannelDesign::channelWidth() const {

        if (detType == DetType::STGC) {
            // eta strips: return the pitch (3.2mm), not the width (2.7mm)
            // phi wires: return width of full wire group
            return (type == ChannelType::etaStrip) ? inputPitch : groupWidth * inputPitch;  
        }

        return inputWidth;
    }
    
    //============================================================================
    inline bool  MuonChannelDesign::geomCenter(int channel, Amg::Vector2D& pos) const {
        Amg::Vector2D l_pos{Amg::Vector2D::Zero()}, r_pos{Amg::Vector2D::Zero()};
        if (!leftInterSect(channel,l_pos) || !rightInterSect(channel,r_pos)) return false;
        pos = 0.5 * (l_pos + r_pos);
        return true;
    }

    //============================================================================
    inline bool MuonChannelDesign::leftInterSect(int channel, Amg::Vector2D& pos) const {
       /// Nominal channel position
       Amg::Vector2D chanPos{Amg::Vector2D::Zero()};
       if (!channelPosition(channel, chanPos)) return false;
       std::optional<double>  lambda =intersect<2>(chanPos, m_stereoDir, m_bottomLeft, m_bottomEdge);
       if (!lambda) return false;
       /// If the channel is a stereo channel && lamda is either smaller 0 or longer
       /// then the bottom edge, then it's a routed strip
       if (m_hasStereo && ( (*lambda) < 0. || (*lambda) > m_maxHorSize)) { 
            const Amg::Vector2D e_y{0., 1.};                  
            const std::optional<double> bottom_line =intersect<2>(m_stereoDir.x() > 0.? m_bottomLeft:  m_bottomRight, e_y, chanPos, m_stereoDir);
            if (bottom_line)  {
                pos = chanPos + (*bottom_line)* m_stereoDir;
                return true;
            }
       }
       pos = (m_bottomLeft + (*lambda) * m_bottomEdge); 
       return true;
    }

    //============================================================================
    inline bool MuonChannelDesign::rightInterSect(int channel, Amg::Vector2D& pos) const {
        /// Nominal channel position
        Amg::Vector2D chanPos{Amg::Vector2D::Zero()};
        if (!channelPosition(channel, chanPos)) return false;
        /// We expect lambda to be positive
        const std::optional<double> lambda =intersect<2>(chanPos, m_stereoDir, m_topRight, m_topEdge);
        if (!lambda) return false;
        /// If the channel is a stereo channel && lamda is either smaller 0 or longer
        /// then the bottom edge, then it's a routed strip
        if (m_hasStereo&& ( (*lambda) < 0  || (*lambda) > m_maxHorSize)) { 
            const Amg::Vector2D e_y{0., 1.};              
            const std::optional<double> top_line =intersect<2>(m_stereoDir.x() >  0. ? m_topRight: m_topLeft, e_y, chanPos, m_stereoDir);
            if (top_line) {
                pos = chanPos + (*top_line) * m_stereoDir;
                return true;
            }
       }
       pos = (m_topRight + (*lambda) * m_topEdge);
       return true; 
    }
    
    //============================================================================
    inline int MuonChannelDesign::numberOfMissingTopStrips() const{ return !m_hasStereo  ? nMissedTopEta : nMissedTopStereo; }        
    inline int MuonChannelDesign::numberOfMissingBottomStrips() const { return !m_hasStereo ? nMissedBottomEta : nMissedBottomStereo;}
    inline double MuonChannelDesign::xSize() const {return m_xSize;}
    inline double MuonChannelDesign::maxYSize() const {return m_maxYSize;}
    inline double MuonChannelDesign::minYSize() const {return m_minYSize;}
    inline double MuonChannelDesign::firstPos() const {return m_firstPos;}

    //============================================================================
    inline bool MuonChannelDesign::center(int channel, Amg::Vector2D& pos) const {
        if (!geomCenter(channel, pos)) return false;
        pos = m_rotMat * pos;
        return true;
    }
    
    //============================================================================
    inline bool MuonChannelDesign::leftEdge(int channel, Amg::Vector2D& pos) const {
        if (!leftInterSect(channel, pos)) return false;
        pos = m_rotMat * pos;
        return true;
    }

    //============================================================================
    inline bool MuonChannelDesign::rightEdge(int channel, Amg::Vector2D& pos) const {
        if (!rightInterSect(channel, pos)) return false;
        pos = m_rotMat * pos;
        return true;
    }
    
    
    //============================================================================
    // The passivated length 
    inline double MuonChannelDesign::passivatedLength(double passivWidth, bool left)  const{
        const Amg::Vector2D& edge = (left ? m_bottomEdge : m_topEdge);
        return passivWidth * std::abs(m_stereoDir.x() *edge.y() - m_stereoDir.y()* edge.x());
    }
    
    //============================================================================
    inline double MuonChannelDesign::passivatedHeight(double passivHeight, bool edge_pcb) const{
        return passivHeight*std::abs(edge_pcb? m_stereoDir.x() : 1.);
    }

}  // namespace MuonGM
#endif  // MUONREADOUTGEOMETRY_MUONCHANNELDESIGN_H
