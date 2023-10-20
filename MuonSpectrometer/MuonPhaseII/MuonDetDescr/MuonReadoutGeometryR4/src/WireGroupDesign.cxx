/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/WireGroupDesign.h>
namespace MuonGMR4{
    bool WireGroupDesign::operator<(const WireGroupDesign& other) const {
        if (other.m_groups.size() != m_groups.size()) {
            return m_groups.size() < other.m_groups.size();
        }
        for (unsigned int grp = 0; grp < m_groups.size(); ++grp) {
            if (m_groups[grp].numWires != other.m_groups[grp].numWires) {
                return m_groups[grp].numWires < other.m_groups[grp].numWires;
            }
        }
        return static_cast<const StripDesign&>(*this) < other;
    }
    void WireGroupDesign::declareGroup(const unsigned int numWires) {
        m_groups.emplace_back(numWires, nAllWires());
    }
    unsigned int WireGroupDesign::nAllWires() const {
        return m_groups.empty() ? 0 : m_groups.back().accumlWires + m_groups.back().numWires;
    }
    /// Returns the number of wires in a given group.
    unsigned int WireGroupDesign::numWiresInGroup(unsigned int groupNum) const {
       unsigned int grpIdx = groupNum - firstStripNumber();
       if (grpIdx >= m_groups.size()) {
          ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The group number "<<groupNum
                        <<" is out of range. Expect ["<<firstStripNumber()
                        <<"-"<<m_groups.size()+firstStripNumber()<<").");
          return 0;
       }
       return m_groups[grpIdx].numWires;
    }
    Amg::Vector2D WireGroupDesign::stripPosition(int groupNum) const {
        if (groupNum >= static_cast<int>(m_groups.size())) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The wire group number "<<groupNum
                           <<" is out of range.");
            return Amg::Vector2D::Zero();
        }
        const wireGroup& wireGrp = m_groups[groupNum];
        return StripDesign::stripPosition(0) + 
               (wireGrp.accumlWires + wireGrp.numWires/2)*stripPitch() * Amg::Vector2D::UnitX();
    }
    Amg::Vector2D WireGroupDesign::wirePosition(unsigned int groupNum, 
                                                unsigned int wireNum) const {
       unsigned int grpIdx = groupNum - firstStripNumber();
       if (grpIdx >= m_groups.size()) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The wire group number "<<groupNum
                           <<"is out of range.");
            return Amg::Vector2D::Zero();
        }
        const wireGroup& wireGrp = m_groups[grpIdx];
        if (wireNum >= wireGrp.numWires){
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The wire number "
                        <<wireNum<<" is out of range. Expect in group "<<groupNum<<
                        " [1-"<<wireGrp.numWires<<"] wires.");
            return Amg::Vector2D::Zero();
        }
        return StripDesign::stripPosition(0) + 
               (wireGrp.accumlWires + (wireNum - 1))*stripPitch() * Amg::Vector2D::UnitX();
    }
    Amg::Vector2D WireGroupDesign::leftWireEdge(unsigned int groupNum,
                                                unsigned int wireNum) const {
        return leftInterSect(wirePosition(groupNum, wireNum));
    }
        
    Amg::Vector2D WireGroupDesign::rightWireEdge(unsigned int groupNum,
                                                 unsigned int wireNum) const {
        return rightInterSect(wirePosition(groupNum, wireNum));
    }
}