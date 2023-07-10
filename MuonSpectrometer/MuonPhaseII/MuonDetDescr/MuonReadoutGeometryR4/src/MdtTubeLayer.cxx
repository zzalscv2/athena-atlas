/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MdtTubeLayer.h>
#include <GeoModelKernel/GeoTube.h>

namespace MuonGMR4{

MdtTubeLayer::MdtTubeLayer(const PVConstLink layer):
    m_layerNode{std::move(layer)} {
        
    for (unsigned int ch = 0; ch < m_layerNode->getNChildNodes(); ++ch) {
        const GeoGraphNode* const* node = m_layerNode->getChildNode(ch);
        const GeoSerialTransformer* trans = dynamic_cast<const GeoSerialTransformer*>(*node);
        if (trans) m_serialTrans = trans;
    }
    if (!m_serialTrans) {
        std::stringstream except{};
        except<<__FILE__<<": "<<__LINE__<<" No serial transformer found.";
        throw std::out_of_range(except.str());
    }   
}

const Amg::Transform3D MdtTubeLayer::layerTransform() const {
    return m_layerNode->getX();
}
PVConstLink MdtTubeLayer::getTubeNode(unsigned int tube) const {
     if (tube >= nTubes()) {
        std::stringstream except{};
        except<<__FILE__<<":"<<__LINE__<<" "<<m_layerNode->getLogVol()->getName()<<" has only "<<nTubes()<<" tubes. But "<<tube<<" is requested. Please check.";
        throw std::out_of_range(except.str());
    }
    return m_layerNode->getChildVol(tube);
}
const Amg::Transform3D MdtTubeLayer::tubeTransform(unsigned int tube) const {    
    if (tube >= nTubes()) {
        std::stringstream except{};
        except<<__FILE__<<":"<<__LINE__<<" "<<m_layerNode->getLogVol()->getName()<<" has only "<<nTubes()<<" tubes. But "<<tube<<" is requested. Please check.";
        throw std::out_of_range(except.str()); 
    }
    return layerTransform() * m_serialTrans->getTransform(tube);
}
unsigned int MdtTubeLayer::nTubes() const {
    return m_layerNode->getNChildVols();
}
double MdtTubeLayer::tubeHalfLength(unsigned int tube) const {
    const PVConstLink child = getTubeNode(tube);
    const GeoShape* shape = child->getLogVol()->getShape();
    const GeoTube* tubeShape = static_cast<const GeoTube*>(shape);
    return tubeShape->getZHalfLength();    
}   
}

