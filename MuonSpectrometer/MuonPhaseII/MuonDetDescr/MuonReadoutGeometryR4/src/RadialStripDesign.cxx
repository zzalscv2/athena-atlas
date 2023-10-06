/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/RadialStripDesign.h>
namespace MuonGMR4{
    bool RadialStripDesign::operator<(const RadialStripDesign& other) const {
        if (other.m_strips.size() != m_strips.size()) {
            return m_strips.size() < other.m_strips.size();
        }
        for (unsigned int strip = 0; strip < m_strips.size(); ++strip) {
            if (m_strips[strip].distOnBottom != other.m_strips[strip].distOnBottom) {
                return m_strips[strip].distOnBottom < other.m_strips[strip].distOnBottom;
            }
            if (m_strips[strip].distOnTop != other.m_strips[strip].distOnTop) {
                return m_strips[strip].distOnTop < other.m_strips[strip].distOnTop;
            }
        }
        return static_cast<const StripDesign&>(*this) < other;
    }
    Amg::Vector2D RadialStripDesign::stripPosition(int stripNum) const {
        if (stripNum >= static_cast<int>(m_strips.size())) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The strip number "<<stripNum
                           <<"is out of range.");
            return Amg::Vector2D::Zero();
        }
        const stripEdges& edges = m_strips[stripNum];
        return 0.5* (m_bottomLeft  + edges.distOnBottom * m_dirBotEdge + 
                     m_topLeft + edges.distOnTop*m_dirTopEdge);
    }
    void RadialStripDesign::addStrip(const double posOnBottom,
                                     const double posOnTop) {
        
        m_strips.resize(m_strips.size() +1);
        stripEdges& edges{m_strips.back()};
        edges.distOnBottom = posOnBottom / (2. * shortHalfHeight());
        edges.distOnBottom = posOnTop / (2. * longHalfHeight());
    }
    Amg::Vector2D RadialStripDesign::stripDir(int stripNumber) const {
        const int stripCh = (stripNumber - firstStripNumber());
        if (stripCh < 0 ||  stripCh > numStrips()) {
            ATH_MSG_WARNING("stripDir() -- Invalid strip number given "<<stripNumber<<" allowed range ["
                         <<firstStripNumber()<<";"<<numStrips()<<"]");
            return Amg::Vector2D::Zero();
        }
        const stripEdges& edges = m_strips[stripCh];        
        return ((m_bottomLeft  + edges.distOnBottom * m_dirBotEdge)  - 
                     (m_topLeft + edges.distOnTop*m_dirTopEdge) ).unit();
    } 
}