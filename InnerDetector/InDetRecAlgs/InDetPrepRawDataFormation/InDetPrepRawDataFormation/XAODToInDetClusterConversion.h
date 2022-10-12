/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///
/// @file InDetToXAODClusterConversion.h
/// Algorithm to convert InDet::PixelClusters and InDet::SCT_Clusters
/// to xAOD::PixelClusters and xAOD::StripClusters.
/// This is a temporary solution before a proper
/// clustering algorithm is implemented.
///

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

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "InDetPrepRawData/SiClusterContainer.h"

class PixelID;

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
 
 private:
  const PixelID* m_pixelID {}; 

  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "ITkPixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_inputPixelClusterContainerKey {this, "InputPixelClustersName", "ITkPixelClusters", "name of the input xAOD pixel cluster container"};

  SG::WriteHandleKey<InDet::PixelClusterContainer> m_outputPixelClusterContainerKey {this, "OutputPixelClustersName", "ITkPixelClusters", "name of the output InDet pixel cluster container"};
  SG::WriteHandleKey< InDet::SiClusterContainer > m_clusterContainerLinkKey {this, "PixelClustersLinkName", "ITkPixelClusters"};
};

}

#endif // INDETRIOMAKER_INDETTOXAODCLUSTERCONVERSION_H

