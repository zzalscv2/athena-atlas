// -*- C++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiCombinatorialTrackFinder_xk
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 10/04/2007 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiCombinatorialTrackFinder_xk_H
#define SiCombinatorialTrackFinder_xk_H

#include "InDetRecToolInterfaces/ISiCombinatorialTrackFinder.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SiSPSeededTrackFinderData/SiCombinatorialTrackFinderData_xk.h"
#include "SiSPSeededTrackFinderData/SiDetElementBoundaryLink_xk.h"
#include "SiSPSeededTrackFinderData/SiDetElementBoundaryLinks_xk.h"
#include "SiSPSeededTrackFinderData/SiTrajectory_xk.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

#include "GaudiKernel/ToolHandle.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <map>
#include <vector>

class MsgStream;

namespace InDet {

 /**
  @class SiCombinatorialTrackFinder_xk 

  InDet::SiCombinatorialTrackFinder_xk is algorithm which produce track-finding
  in the road of InDetDD::SiDetectorElement* of SCT and Pixels
  sorted in propagation order.

  In AthenaMT, event dependent cache inside SiCombinatorialTrackFinder_xk
  is not preferred. SiCombinatorialTrackFinderData_xk class holds
  event dependent data for SiCombinatorialTrackFinder_xk.
  Its object is instantiated in SiSPSeededTrackFinder::execute
  through SiTrackMakerEventData_xk.

  @author Igor.Gavrilenko@cern.ch     
  */

  class SiCombinatorialTrackFinder_xk final: 

    public extends<AthAlgTool, ISiCombinatorialTrackFinder>
    {
      friend class SiTrajectory_xk;

      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////
      
    public:
      
      ///////////////////////////////////////////////////////////////////
      /// @name Standard tool methods
      ///////////////////////////////////////////////////////////////////
      //@{
     SiCombinatorialTrackFinder_xk(const std::string&, const std::string&,
                                   const IInterface*);
     virtual ~SiCombinatorialTrackFinder_xk() = default;
     virtual StatusCode initialize() override;
     virtual StatusCode finalize() override;
     //@}

     ///////////////////////////////////////////////////////////////////
     /// @name Main methods for local track finding
     ///////////////////////////////////////////////////////////////////
     //@{

     virtual const std::list<Trk::Track*>& getTracks(
         SiCombinatorialTrackFinderData_xk& data, const Trk::TrackParameters&,
         const std::vector<const Trk::SpacePoint*>&,
         const std::vector<Amg::Vector3D>&,
         std::vector<const InDetDD::SiDetectorElement*>&, 
         const TrackQualityCuts&,
         const EventContext& ctx) const override;

     virtual const std::list<Trk::Track*>& getTracks(
         SiCombinatorialTrackFinderData_xk& data, const Trk::TrackParameters&,
         const std::vector<const Trk::SpacePoint*>&,
         const std::vector<Amg::Vector3D>&,
         std::vector<const InDetDD::SiDetectorElement*>&,
         std::multimap<const Trk::PrepRawData*, const Trk::Track*>&,
         const EventContext& ctx) const override;

     virtual const std::list<Trk::Track*>& getTracksWithBrem(
         SiCombinatorialTrackFinderData_xk& data, const Trk::TrackParameters&,
         const std::vector<const Trk::SpacePoint*>&,
         const std::vector<Amg::Vector3D>&,
         std::vector<const InDetDD::SiDetectorElement*>&,
         std::multimap<const Trk::PrepRawData*, const Trk::Track*>&, bool,
         const EventContext& ctx) const override;

     virtual double pTseed(SiCombinatorialTrackFinderData_xk& data,
                           const Trk::TrackParameters&,
                           const std::vector<const Trk::SpacePoint*>&,
                           const EventContext&) const override;

     virtual void newEvent(
         const EventContext& ctx,
         SiCombinatorialTrackFinderData_xk& data) const override;
     virtual void newEvent(const EventContext& ctx,
                           SiCombinatorialTrackFinderData_xk& data,
                           Trk::TrackInfo,
                           const TrackQualityCuts&) const override;

     virtual void endEvent(
         SiCombinatorialTrackFinderData_xk& data) const override;
     //@}

     ///////////////////////////////////////////////////////////////////
     /// @name Print internal tool parameters and status
     ///////////////////////////////////////////////////////////////////
     //@{
     MsgStream& dump(SiCombinatorialTrackFinderData_xk& data,
                     MsgStream& out) const override;
     //@}

    private:
      
      ///////////////////////////////////////////////////////////////////
      // Protected Data
      ///////////////////////////////////////////////////////////////////

      /// @name Tool handles
      //@{
      ToolHandle<IInDetConditionsTool> m_pixelCondSummaryTool{this, "PixelSummaryTool",
          "PixelConditionsSummaryTool"};
      ToolHandle<IInDetConditionsTool> m_sctCondSummaryTool{this, "SctSummaryTool",
          "InDetSCT_ConditionsSummaryTool/SCT_ConditionsSummaryTool", "Tool to retrieve SCT Conditions summary"};
      ToolHandle<Trk::IBoundaryCheckTool> m_boundaryCheckTool{this, "BoundaryCheckTool",
          "InDet::InDetBoundaryCheckTool", "Boundary checking tool for detector sensitivities"};

      PublicToolHandle<Trk::IPatternParametersPropagator> m_proptool{this, "PropagatorTool",
          "Trk::RungeKuttaPropagator/InDetPropagator"};
      PublicToolHandle<Trk::IPatternParametersUpdator> m_updatortool{this, "UpdatorTool",
          "Trk::KalmanUpdator_xk/InDetPatternUpdator"};
      PublicToolHandle<Trk::IRIO_OnTrackCreator> m_riocreator{this, "RIOonTrackTool",
          "Trk::RIO_OnTrackCreator/RIO_OnTrackCreator"};
      //@}

      /// @name Data handles
      //@{
      SG::ReadHandleKey<InDet::PixelClusterContainer> m_pixcontainerkey{this, "PixelClusterContainer",
          "PixelClusters"};
      SG::ReadHandleKey<InDet::SCT_ClusterContainer> m_sctcontainerkey{this, "SCT_ClusterContainer",
          "SCT_Clusters"};
      SG::ReadCondHandleKey<InDet::SiDetElementBoundaryLinks_xk> m_boundaryPixelKey{this, "PixelDetElementBoundaryLinks_xk",
          "PixelDetElementBoundaryLinks_xk", "Key of InDet::SiDetElementBoundaryLinks_xk for Pixel"};
      SG::ReadCondHandleKey<InDet::SiDetElementBoundaryLinks_xk> m_boundarySCTKey{this, "SCT_DetElementBoundaryLinks_xk",
          "SCT_DetElementBoundaryLinks_xk", "Key of InDet::SiDetElementBoundaryLinks_xk for SCT"};
      // For P->T converter of SCT_Clusters
      SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};
      //@}

      /** @brief Optional read handle to get status data to test whether a pixel detector element is good.
       * If set to e.g. PixelDetectorElementStatus the event data will be used instead of the pixel conditions summary tool.
       */
      SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_pixelDetElStatus
         {this, "PixelDetElStatus", "", "Key of SiDetectorElementStatus for Pixel"};

      /** @brief Optional read handle to get status data to test whether a SCT detector element is good.
       * If set to e.g. SCTDetectorElementStatus the event data will be used instead of the SCT conditions summary tool.
       */
      SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_sctDetElStatus
         {this, "SCTDetElStatus", "", "Key of SiDetectorElementStatus for SCT"};

      /// @name Properties
      //@{
      BooleanProperty m_usePIX{this, "usePixel", true};
      BooleanProperty m_useSCT{this, "useSCT", true};
      BooleanProperty m_ITkGeometry{this, "ITkGeometry", false};
      BooleanProperty m_doFastTracking{this, "doFastTracking", false};
      StringProperty m_fieldmode{this, "MagneticFieldMode", "MapSolenoid", "Mode of magnetic field"};
      DoubleProperty m_qualityCut{this, "TrackQualityCut", 9.3, "Simple track quality cut"};
      FloatProperty m_minPtCut{this, "MinFinalPtCut", 100, "Cut on the pt of the final track. Must be >0 to avoid NANs when computing eta."};
      float  m_minPt2Cut=0;
      BooleanProperty m_writeHolesFromPattern{this, "writeHolesFromPattern", false,"Flag to activate writing hole info from the pattern recognition"}; 
      //@}

      /// @name Data members, which are updated in only initialize
      //@{
      int m_outputlevel{0};
      Trk::MagneticFieldProperties m_fieldprop; //!< Magnetic field properties
      //@}

      ///Array entries for data.statistic counter 
      typedef enum {
        TwoCluster,
        WrongRoad,
        WrongInit,
        CantFindTrk,
        NotNewTrk,
        BremAttempt,
        NumberOfStats,
        Success,
      } EStat_t;

      ///////////////////////////////////////////////////////////////////
      // Methods 
      ///////////////////////////////////////////////////////////////////

      EStat_t findTrack(
          SiCombinatorialTrackFinderData_xk& data, const Trk::TrackParameters&,
          const std::vector<const Trk::SpacePoint*>&,
          const std::vector<Amg::Vector3D>&,
          std::vector<const InDetDD::SiDetectorElement*>&,
          std::multimap<const Trk::PrepRawData*, const Trk::Track*>&,
          const EventContext&) const;

      static void getTrackQualityCuts(SiCombinatorialTrackFinderData_xk& data,
                                      const TrackQualityCuts&);

      Trk::Track* convertToTrack(SiCombinatorialTrackFinderData_xk& data, const EventContext& ctx) const;
      Trk::Track* convertToNextTrack(
          SiCombinatorialTrackFinderData_xk& data) const;

      void magneticFieldInit();

      static bool spacePointsToClusters(
          const std::vector<const Trk::SpacePoint*>&,
          std::vector<const InDet::SiCluster*>&,
          std::optional<std::reference_wrapper<std::vector<const InDetDD::SiDetectorElement*>>> = std::nullopt);

      void detectorElementLinks(
          std::vector<const InDetDD::SiDetectorElement*>&,
          std::vector<const InDet::SiDetElementBoundaryLink_xk*>&,
          const EventContext& ctx) const;

      MsgStream& dumpconditions(MsgStream& out) const;
      static MsgStream& dumpevent(SiCombinatorialTrackFinderData_xk& data, MsgStream& out) ;

      void initializeCombinatorialData(const EventContext& ctx, SiCombinatorialTrackFinderData_xk& data) const;
      virtual void fillStatistic(SiCombinatorialTrackFinderData_xk& data, std::array<bool,NumberOfStats>& information) const override;

    };

} // end of name space

#endif // SiCombinatorialTrackFinder_xk_H
