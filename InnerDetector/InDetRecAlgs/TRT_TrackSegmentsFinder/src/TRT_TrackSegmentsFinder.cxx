/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "TRT_TrackSegmentsFinder/TRT_TrackSegmentsFinder.h"


#include "TRT_ReadoutGeometry/TRT_BaseElement.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#include <memory>

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::TRT_TrackSegmentsFinder::TRT_TrackSegmentsFinder
(const std::string& name,ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode InDet::TRT_TrackSegmentsFinder::initialize()
{
  // Get tool for drift circles seeds maker
  //
  ATH_CHECK( m_segmentsMakerTool.retrieve() );
  ATH_CHECK( m_roadtool.retrieve( DisableTool{ !m_useCaloSeeds }) );

  ATH_CHECK( m_caloClusterROIKey.initialize(m_useCaloSeeds) );
  ATH_CHECK( m_foundSegmentsKey.initialize() );

  ////////////////////////////////////////////////////////////////////////////////
  ATH_CHECK( m_fieldCondObjInputKey.initialize());
  ////////////////////////////////////////////////////////////////////////////////

  // Get output print level
  //
  if (msgLvl(MSG::DEBUG)) {
     MsgStream& out = msg(MSG::DEBUG);
     out << std::endl;
     dumptools(out);
     out << endmsg;
  }
  m_nsegmentsTotal = 0;
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////

StatusCode InDet::TRT_TrackSegmentsFinder::execute(const EventContext &ctx) const
{
  std::unique_ptr<Trk::SegmentCollection> found_segments(std::make_unique<Trk::SegmentCollection>());
  std::unique_ptr<InDet::ITRT_TrackSegmentsMaker::IEventData> event_data_p;
  InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap map;
  if(!m_useCaloSeeds) {
    event_data_p = m_segmentsMakerTool->newEvent(ctx);
    m_segmentsMakerTool->find    (ctx, *event_data_p, map);
    // Loop through all segments and reconsrtucted segments collection preparation
    //
    Trk::Segment* segment = nullptr;
    while((segment = m_segmentsMakerTool->next(*event_data_p))) {
      found_segments->push_back(segment);
    }
  } else   {
    Amg::Vector3D PSV(0.,0.,0.); Trk::PerigeeSurface PS(PSV);
    std::vector<IdentifierHash>      vTR;
    SG::ReadHandle<ROIPhiRZContainer> calo_rois(m_caloClusterROIKey, ctx);
    if (!calo_rois.isValid()) {
       ATH_MSG_FATAL("Failed to get EM Calo cluster collection " << m_caloClusterROIKey );
       return StatusCode::FAILURE;
    }
    MagField::AtlasFieldCache fieldCache;
    SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCondObjInputKey, ctx};
    const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};
    if (fieldCondObj == nullptr) {
       ATH_MSG_ERROR("InDet::TRT_TrackExtensionTool_xk::findSegment: Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCondObjInputKey.key());
       return StatusCode::FAILURE;
    }
    fieldCondObj->getInitializedCache (fieldCache);
    for (const ROIPhiRZ &roi : *calo_rois) {
        if (std::abs(roi[0])>M_PI) continue; // reject duplicates;
        std::unique_ptr<Trk::TrackParameters> par = PS.createUniqueTrackParameters(0., 0., roi.phi(), roi.theta(), 0., std::nullopt);
        // Get AtlasFieldCache
        // TRT detector elements road builder
        //
        const auto & DE = m_roadtool->detElementsRoad(ctx, fieldCache, *par, Trk::alongMomentum, map);
	     if(int(DE.size()) < m_minNumberDCs) continue;
	     vTR.clear();
        vTR.reserve(DE.size());
        for (const InDetDD::TRT_BaseElement*d: DE) {
           vTR.push_back(d->identifyHash());
        }
        event_data_p = m_segmentsMakerTool->newRegion(ctx, vTR);
	     m_segmentsMakerTool->find(ctx, *event_data_p,map);
	     // Loop through all segments and reconsrtucted segments collection preparation
        Trk::Segment* segment = nullptr;
	     while((segment = m_segmentsMakerTool->next(*event_data_p))) {
	        found_segments->push_back(segment);
	     }
      }//end of loopover *calo
  }
  if (event_data_p) {
     m_segmentsMakerTool->endEvent(*event_data_p);
  }

  // gather stat before found segments are moved.
  if (msgLvl(MSG::DEBUG)) {
     MsgStream& out = msg(MSG::DEBUG);
     out << std::endl;
     dumpevent(out,found_segments->size());
     out << endmsg;
  }
  m_nsegmentsTotal+=found_segments->size();

  if (SG::WriteHandle( m_foundSegmentsKey, ctx ).record(std::move(found_segments)).isFailure()) {
     ATH_MSG_ERROR("Could not save TRT segments " << m_foundSegmentsKey.key() );
      return StatusCode::FAILURE;
  }

  // Print common event information
  //
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////

StatusCode InDet::TRT_TrackSegmentsFinder::finalize()
{
  if (msgLvl(MSG::INFO)) {
     MsgStream& out = msg(MSG::INFO);
     out << std::endl;
     dumpevent(out,m_nsegmentsTotal);
     out << endmsg;
  }
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Dumps conditions information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::TRT_TrackSegmentsFinder::dumptools( MsgStream& out ) const
{
  int n = 65-m_segmentsMakerTool.type().size();
  std::string s1; for(int i=0; i<n; ++i) s1.append(" "); s1.append("|");
  n     = 65-m_foundSegmentsKey.key().size();
  std::string s2; for(int i=0; i<n; ++i) s2.append(" "); s2.append("|");

  out<<"|----------------------------------------------------------------"
     <<"----------------------------------------------------|"
     <<std::endl;
  out<<"| Tool for TRT track segments finding             | "<<m_segmentsMakerTool.type()<<s1
     <<std::endl;
  out<<"| Location of output segments                     | "<<m_foundSegmentsKey.key()<<s2
     <<std::endl;
  out<<"|----------------------------------------------------------------"
     <<"----------------------------------------------------|"
     <<std::endl;

  return out;
}

///////////////////////////////////////////////////////////////////
// Dumps event information into the ostream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::TRT_TrackSegmentsFinder::dumpevent( MsgStream& out, int nsegments) const
{
  out<<"|-------------------------------------------------------------------";
  out<<"-----------------------------|"
     <<std::endl;

  out<< ((m_useCaloSeeds)
         ? "| TRT track segments found with calo seeds        |"
         : "| TRT track segments found without calo seeds     |")
       <<std::setw(7)<<nsegments
       <<"                                       |"
       <<std::endl;
  out<<"|-------------------------------------------------------------------";
  out<<"-----------------------------|"
     <<std::endl;
  return out;
}






