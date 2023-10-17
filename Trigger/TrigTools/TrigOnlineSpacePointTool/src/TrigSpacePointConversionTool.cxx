/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/PixelID.h" 

#include "TrkSpacePoint/SpacePoint.h"
#include "TrkSpacePoint/SpacePointCollection.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "TrigInDetEvent/TrigSiSpacePointBase.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "TrigInDetToolInterfaces/ITrigL2LayerNumberTool.h"

#include "SpacePointConversionUtils.h"

#include "IRegionSelector/IRegSelTool.h"

#include "TrigSpacePointConversionTool.h"



TrigSpacePointConversionTool::TrigSpacePointConversionTool(const std::string& t, 
					     const std::string& n,
					     const IInterface*  p ) : 
  AthAlgTool(t,n,p)
{
  declareInterface< ITrigSpacePointConversionTool >( this );

  declareProperty( "DoPhiFiltering",         m_filter_phi = true );
  declareProperty( "UseBeamTilt",            m_useBeamTilt = true );
  declareProperty( "UseNewLayerScheme",      m_useNewScheme = false );
  declareProperty( "PixelSP_ContainerName",  m_pixelSpacePointsContainerKey = std::string("PixelTrigSpacePoints"));
  declareProperty( "SCT_SP_ContainerName",   m_sctSpacePointsContainerKey = "SCT_TrigSpacePoints" );
  declareProperty( "UsePixelSpacePoints",    m_usePixelSpacePoints = true );
  declareProperty( "UseSctSpacePoints",      m_useSctSpacePoints = true );
}

StatusCode TrigSpacePointConversionTool::initialize() {

  StatusCode sc = AthAlgTool::initialize();

  ATH_MSG_INFO("In initialize...");

  //  sc = serviceLocator()->service( m_regionSelectorName, m_regionSelector);
  //  if ( sc.isFailure() ) {
  //    ATH_MSG_FATAL("Unable to retrieve RegionSelector Service  " << m_regionSelectorName);
  //    return sc;
  //  }

  ATH_CHECK(m_regsel_pix.retrieve());
  ATH_CHECK(m_regsel_sct.retrieve());

  sc=m_layerNumberTool.retrieve();
  if(sc.isFailure()) {
    ATH_MSG_ERROR("Could not retrieve "<<m_layerNumberTool);
    return sc;
  }

  sc = detStore()->retrieve(m_atlasId, "AtlasID");
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Could not get ATLAS ID helper"); 
    return sc;
  } 

  sc = detStore()->retrieve(m_pixelId, "PixelID");
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Could not get Pixel ID helper"); 
    return sc;
  } 

  sc = detStore()->retrieve(m_sctId, "SCT_ID");
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Could not get SCT ID helper"); 
    return sc;
  } 
 
  ATH_CHECK(m_beamSpotKey.initialize());

  if (!m_usePixelSpacePoints && !m_useSctSpacePoints) {
    ATH_MSG_FATAL("Both usePixelSpacePoints and useSctSpacePoints set to False. At least one needs to be True");
    return StatusCode::FAILURE;
  }
  if (!m_useSctSpacePoints) ATH_MSG_INFO("Only converting Pixel spacepoints => PPP seeds only");
  if (!m_usePixelSpacePoints) ATH_MSG_INFO("Only converting SCT spacepoints => SSS seeds only");
  ATH_CHECK(m_pixelSpacePointsContainerKey.initialize(m_usePixelSpacePoints));
  ATH_CHECK(m_sctSpacePointsContainerKey.initialize(m_useSctSpacePoints));

  ATH_MSG_INFO("TrigSpacePointConversionTool initialized ");

  return sc;
}

StatusCode TrigSpacePointConversionTool::finalize() {

  StatusCode sc = AthAlgTool::finalize(); 
  return sc;
}


StatusCode TrigSpacePointConversionTool::getSpacePoints(const IRoiDescriptor& internalRoI, 
							std::vector<TrigSiSpacePointBase>& output, int& nPix, int& nSct, const EventContext& ctx, std::map<Identifier, std::vector<long int> > *clustermap) const {

  output.clear();
  
  const SpacePointContainer* pixelSpacePointsContainer = nullptr;
  if (m_usePixelSpacePoints) {
    SG::ReadHandle<SpacePointContainer> pixHandle(m_pixelSpacePointsContainerKey, ctx);
    ATH_CHECK(pixHandle.isValid());
    pixelSpacePointsContainer = pixHandle.ptr();
  }
  const SpacePointContainer* sctSpacePointsContainer = nullptr;
  if (m_useSctSpacePoints) {
    SG::ReadHandle<SpacePointContainer> sctHandle(m_sctSpacePointsContainerKey, ctx);
    ATH_CHECK(sctHandle.isValid());
    sctSpacePointsContainer = sctHandle.ptr();
  }

  std::vector<IdentifierHash> listOfPixIds;
  std::vector<IdentifierHash> listOfSctIds;
        
  //  m_regionSelector->DetHashIDList(PIXEL, internalRoI, listOfPixIds); 
  //  m_regionSelector->DetHashIDList(SCT, internalRoI, listOfSctIds); 

  m_regsel_pix->HashIDList( internalRoI, listOfPixIds );
  m_regsel_sct->HashIDList( internalRoI, listOfSctIds );

 
  int offsets[3];

  offsets[0] = m_layerNumberTool->offsetEndcapPixels();
  offsets[1] = m_layerNumberTool->offsetBarrelSCT();
  offsets[2] = m_layerNumberTool->offsetEndcapSCT();
    
  FTF::LayerCalculator lc(m_atlasId, m_pixelId, m_sctId, offsets);
    
  //filter spacepoints to reject those beyound internalRoI boundaries
      
  nPix = 0;
  nSct = 0;
  if ( clustermap!=nullptr ) { 

    ATH_MSG_DEBUG("LRT Mode: clustermap supplied and being used to remove spacepoints from clusters already on tracks");
    //  In LRT mode a cluster map is supplied to enable removal of clusters on tracks.
    FTF::RoI_Filter filter(output, lc, &internalRoI, m_filter_phi, clustermap);
    FTF::SpacePointSelector<FTF::RoI_Filter> selector(filter);
    
    if(m_useNewScheme) {
      if (m_usePixelSpacePoints)  nPix=selector.select(*pixelSpacePointsContainer,listOfPixIds, m_layerNumberTool->pixelLayers());
      if (m_useSctSpacePoints)  nSct=selector.select(*sctSpacePointsContainer,listOfSctIds, m_layerNumberTool->sctLayers());
    }
    else {
      if (m_usePixelSpacePoints)  nPix=selector.select(*pixelSpacePointsContainer,listOfPixIds);
      if (m_useSctSpacePoints)  nSct=selector.select(*sctSpacePointsContainer,listOfSctIds);
    }


  } else {
    FTF::RoI_Filter filter(output, lc, &internalRoI, m_filter_phi);
    FTF::SpacePointSelector<FTF::RoI_Filter> selector(filter);
    
    if(m_useNewScheme) {
      if (m_usePixelSpacePoints) nPix=selector.select(*pixelSpacePointsContainer,listOfPixIds, m_layerNumberTool->pixelLayers());
      if (m_useSctSpacePoints) nSct=selector.select(*sctSpacePointsContainer,listOfSctIds, m_layerNumberTool->sctLayers());
    }
    else {
      if (m_usePixelSpacePoints) nPix=selector.select(*pixelSpacePointsContainer,listOfPixIds);
      if (m_useSctSpacePoints) nSct=selector.select(*sctSpacePointsContainer,listOfSctIds);
    }
  }
  if(!m_useBeamTilt) shiftSpacePoints(output, ctx);
  else transformSpacePoints(output, ctx);

  ATH_MSG_DEBUG("Returning "<<nPix<< " Pixel Spacepoints and "<<nSct<< " SCT SpacePoints");
  return StatusCode::SUCCESS;
}


void TrigSpacePointConversionTool::shiftSpacePoints(std::vector<TrigSiSpacePointBase>& output, const EventContext& ctx) const {

  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };  
  const Amg::Vector3D &vertex = beamSpotHandle->beamPos();
  double shift_x = vertex.x() - beamSpotHandle->beamTilt(0)*vertex.z();
  double shift_y = vertex.y() - beamSpotHandle->beamTilt(1)*vertex.z();

  std::for_each(output.begin(), output.end(), FTF::SpacePointShifter(shift_x, shift_y));

}


void TrigSpacePointConversionTool::transformSpacePoints(std::vector<TrigSiSpacePointBase>& output, const EventContext& ctx) const {

  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
  const Amg::Vector3D &origin = beamSpotHandle->beamPos();
  double tx = tan(beamSpotHandle->beamTilt(0));
  double ty = tan(beamSpotHandle->beamTilt(1));

  double phi   = atan2(ty,tx);
  double theta = acos(1.0/sqrt(1.0+tx*tx+ty*ty));
  double sint = sin(theta);
  double cost = cos(theta);
  double sinp = sin(phi);
  double cosp = cos(phi);
  
  std::array<float, 4> xtrf{}, ytrf{}, ztrf{};

  xtrf[0] = float(origin.x());
  xtrf[1] = float(cost*cosp*cosp+sinp*sinp);
  xtrf[2] = float(cost*sinp*cosp-sinp*cosp);
  xtrf[3] =-float(sint*cosp);
  
  ytrf[0] = float(origin.y());
  ytrf[1] = float(cost*cosp*sinp-sinp*cosp);
  ytrf[2] = float(cost*sinp*sinp+cosp*cosp);
  ytrf[3] =-float(sint*sinp);
  
  ztrf[0] = float(origin.z());
  ztrf[1] = float(sint*cosp);
  ztrf[2] = float(sint*sinp);
  ztrf[3] = float(cost);

  std::for_each(output.begin(), output.end(), FTF::SpacePointTransform(xtrf, ytrf, ztrf));

}
