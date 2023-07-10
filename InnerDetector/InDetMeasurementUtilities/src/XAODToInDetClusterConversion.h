/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETRIOMAKER_XAODTOINDETCLUSTERCONVERSION_H
#define INDETRIOMAKER_XAODTOINDETCLUSTERCONVERSION_H

//STL
#include <string>

//Gaudi
#include "GaudiKernel/ToolHandle.h"

// Base class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

//InDet
//can't fwd declare this, needed for typedef to Pixel_RDO_Container
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"

#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetPrepRawData/SiClusterContainer.h"

#include "InDetCondTools/ISiLorentzAngleTool.h"

class PixelID;
class SCT_ID;

namespace InDet {

class XAODToInDetClusterConversion 
  : public AthReentrantAlgorithm {
 public:
  
  /// Constructor with parameters:
  XAODToInDetClusterConversion(const std::string &name,ISvcLocator *pSvcLocator);

  //@name Usual algorithm methods
  //@{
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  /**    @name Disallow default instantiation, copy, assignment */
  //@{
  //@}
  XAODToInDetClusterConversion() = delete;
  XAODToInDetClusterConversion(const XAODToInDetClusterConversion&) = delete;
  XAODToInDetClusterConversion &operator=(const XAODToInDetClusterConversion&) = delete;
   //@}

 private:
  StatusCode convertPixelClusters(const EventContext& ctx) const;
  StatusCode convertStripClusters(const EventContext& ctx) const;

 private:
  const PixelID* m_pixelID {}; 
  const SCT_ID* m_stripID {};

  ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool {this, "LorentzAngleTool", "SiLorentzAngleTool/SCTLorentzAngleTool", "Tool to retrieve Lorentz angle of SCT"};

  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey {this, "PixelDetEleCollKey", "ITkPixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_inputPixelClusterContainerKey {this, "InputPixelClustersName", "ITkPixelClusters", "name of the input xAOD pixel cluster container"};

  SG::WriteHandleKey<InDet::PixelClusterContainer> m_outputPixelClusterContainerKey {this, "OutputPixelClustersName", "ITkPixelClusters", "name of the output InDet pixel cluster container"};
  SG::WriteHandleKey< InDet::SiClusterContainer > m_pixelClusterContainerLinkKey {this, "PixelClustersLinkName", "ITkPixelClusters"};

  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_stripDetEleCollKey {this, "StripDetEleCollKey", "ITkStripDetectorElementCollection", "Key of SiDetectorElementCollection for Strip"};
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_inputStripClusterContainerKey {this, "InputStripClustersName", "ITkStripClusters", "name of the input xAOD strip cluster container"};

  SG::WriteHandleKey<InDet::SCT_ClusterContainer> m_outputStripClusterContainerKey {this, "OutputStripClustersName", "ITkStripClusters", "name of the output InDet pixel cluster container"};
  SG::WriteHandleKey< InDet::SiClusterContainer > m_stripClusterContainerLinkKey {this, "StripClustersLinkName", "ITkStripClusters"};

  Gaudi::Property<bool> m_processPixel {this, "ProcessPixel", true};
  Gaudi::Property<bool> m_processStrip {this, "ProcessStrip", true};
};

}

#endif // INDETRIOMAKER_XAODTOINDETCLUSTERCONVERSION_H

