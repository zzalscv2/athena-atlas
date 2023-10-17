/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGONLINESPACEPOINTTOOL_TRIG_SPACEPOINT_CONVERSION_TOOL_H
#define TRIGONLINESPACEPOINTTOOL_TRIG_SPACEPOINT_CONVERSION_TOOL_H

#include "GaudiKernel/ToolHandle.h"
#include "TrigInDetToolInterfaces/ITrigSpacePointConversionTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include <string>
#include <vector>

#include "TrkSpacePoint/SpacePointContainer.h"
#include "BeamSpotConditionsData/BeamSpotData.h"


#include "IRegionSelector/IRegSelTool.h"

class AtlasDetectorID;
class SCT_ID;
class PixelID;


class ITrigL2LayerNumberTool;

class TrigSpacePointConversionTool : virtual public ITrigSpacePointConversionTool, public AthAlgTool {
 public:

  // standard AlgTool methods
  TrigSpacePointConversionTool(const std::string&,const std::string&,const IInterface*);
  virtual ~TrigSpacePointConversionTool(){};
		
  // standard Athena methods
  StatusCode initialize() override;
  StatusCode finalize() override;

  //concrete implementations
  virtual StatusCode getSpacePoints(const IRoiDescriptor&, std::vector<TrigSiSpacePointBase>&, int&, int&, const EventContext& ctx, std::map<Identifier, std::vector<long int> >*clustermap=nullptr) const override final;
  

 protected:

  ToolHandle<ITrigL2LayerNumberTool> m_layerNumberTool {this, "layerNumberTool", "TrigL2LayerNumberTool"};

  const AtlasDetectorID* m_atlasId = nullptr;
  const SCT_ID*  m_sctId = nullptr;
  const PixelID* m_pixelId = nullptr;

  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

  std::string m_pixelSpContName,m_sctSpContName;// offline/EF containers
  SG::ReadHandleKey<SpacePointContainer> m_sctSpacePointsContainerKey;
  SG::ReadHandleKey<SpacePointContainer> m_pixelSpacePointsContainerKey;

  bool m_filter_phi;
  bool m_useBeamTilt;
  bool m_useNewScheme;
  bool m_usePixelSpacePoints;
  bool m_useSctSpacePoints;

  void shiftSpacePoints(std::vector<TrigSiSpacePointBase>&, const EventContext&) const;
  void transformSpacePoints(std::vector<TrigSiSpacePointBase>&, const EventContext&) const;

  /// new region selector tools
  ToolHandle<IRegSelTool> m_regsel_pix { this, "RegSelTool_Pixel",  "RegSelTool/RegSelTool_Pixel" };
  ToolHandle<IRegSelTool> m_regsel_sct { this, "RegSelTool_SCT",    "RegSelTool/RegSelTool_SCT"   };

};
#endif
