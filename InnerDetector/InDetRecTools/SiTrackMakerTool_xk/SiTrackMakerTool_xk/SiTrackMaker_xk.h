// -*- C++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiTrackMaker_xk
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 22/03/2005 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiTrackMaker_xk_H
#define SiTrackMaker_xk_H

#include "InDetRecToolInterfaces/ISiTrackMaker.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "InDetRecToolInterfaces/ISeedToTrackConversionTool.h"
#include "InDetRecToolInterfaces/ISiCombinatorialTrackFinder.h"
#include "InDetRecToolInterfaces/ISiDetElementsRoadMaker.h"
#include "TrkCaloClusterROI/ROIPhiRZContainer.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "SiSPSeededTrackFinderData/SiTrackMakerEventData_xk.h"

#include "GaudiKernel/ToolHandle.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <array>
#include <iosfwd>
#include <list>
#include <vector>
#include <mutex>

class MsgStream;

namespace InDet{

  class SiTrackMakerEventData_xk;

  /**
  @class SiTrackMaker_xk

  InDet::SiTrackMaker_xk is algorithm which produce Trk::Track started
  from 3 space points information of SCT and Pixels
  in the road of InDetDD::SiDetectorElement* sorted in propagation order.

  In AthenaMT, event dependent cache inside SiTrackMaker_xk
  is not preferred. SiTrackMakerEventData_xk class holds
  event dependent data for SiTrackMaker_xk.
  Its object is instantiated in SiSPSeededTrackFinder::execute.

  @author Igor.Gavrilenko@cern.ch
  */

  class SiTrackMaker_xk final:
    public extends<AthAlgTool, ISiTrackMaker>
    {

      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////

    public:

      ///////////////////////////////////////////////////////////////////
      /// @name Standard tool methods
      ///////////////////////////////////////////////////////////////////
      //@{
      SiTrackMaker_xk
      (const std::string&,const std::string&,const IInterface*);
      virtual ~SiTrackMaker_xk() = default;
      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;
      //@}

      ///////////////////////////////////////////////////////////////////
      /// @name Main methods for local track finding
      ///////////////////////////////////////////////////////////////////
      //@{
      virtual std::list<Trk::Track*>
      getTracks(const EventContext& ctx, SiTrackMakerEventData_xk& data, const std::vector<const Trk::SpacePoint*>& Sp) const override;

      virtual std::list<Trk::Track*>
      getTracks(const EventContext& ctx, SiTrackMakerEventData_xk& data, const Trk::TrackParameters& Tp, const std::vector<Amg::Vector3D>& Gp) const override;

      virtual void newEvent(const EventContext& ctx, SiTrackMakerEventData_xk& data, bool PIX, bool SCT) const override;
      virtual void newTrigEvent(const EventContext& ctx, SiTrackMakerEventData_xk& data, bool PIX, bool SCT) const override;

      virtual void endEvent(SiTrackMakerEventData_xk& data) const override;
      //@}

      ///////////////////////////////////////////////////////////////////
      /// @name Print internal tool parameters and status
      ///////////////////////////////////////////////////////////////////
      //@{
      MsgStream& dump(SiTrackMakerEventData_xk& data, MsgStream& out) const override;
      //@}

    private:

      /// @name Disallow default constructor, copy constructor and assignment operator
      //@{
      SiTrackMaker_xk() = delete;
      SiTrackMaker_xk(const SiTrackMaker_xk&) =delete;
      SiTrackMaker_xk &operator=(const SiTrackMaker_xk&) = delete;
      //@}

      ///////////////////////////////////////////////////////////////////
      // Protected Data
      ///////////////////////////////////////////////////////////////////

      /// @name Tool handles
      //@{
      ToolHandle<InDet::ISiDetElementsRoadMaker> m_roadmaker{this, "RoadTool", "InDet::SiDetElementsRoadMaker_xk"};
      ToolHandle<InDet::ISiCombinatorialTrackFinder> m_tracksfinder{this, "CombinatorialTrackFinder", "InDet::SiCombinatorialTrackFinder_xk"};
      ToolHandle<InDet::ISeedToTrackConversionTool> m_seedtrack{this, "SeedToTrackConversion", "InDet::SeedToTrackConversionTool"};
      //@}

      /// @name Data handles
      //@{
      SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
      SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};
      SG::ReadHandleKey<ROIPhiRZContainer> m_caloCluster{this, "EMROIPhiRZContainer", ""};
      SG::ReadHandleKey<ROIPhiRZContainer> m_caloHad{this, "HadROIPhiRZContainer", ""};
      //@}

      /// @name Properties
      //@{
      BooleanProperty m_seedsfilter{this, "UseSeedFilter", true, "Use seed filter"};
      StringProperty m_fieldmode{this, "MagneticFieldMode", "MapSolenoid", "Mode of magnetic field"};
      StringProperty m_patternName{this, "TrackPatternRecoInfo", "SiSPSeededFinder", "Name of the pattern recognition"};
      BooleanProperty m_usePix{this, "usePixel", true, "flags to set whether to use pixel/sct cluster, irrespective of what is in event"};
      BooleanProperty m_useSct{this, "useSCT", true};
      BooleanProperty m_useassoTool{this, "UseAssociationTool", false, "Use prd-track association tool"};
      BooleanProperty m_cosmicTrack{this, "CosmicTrack", false, "Is it cosmic track"};
      BooleanProperty m_multitracks{this, "doMultiTracksProd", false};
      BooleanProperty m_useBremModel{this, "useBremModel", false};
      BooleanProperty m_useCaloSeeds{this, "doCaloSeededBrem", false};
      BooleanProperty m_useSSSfilter{this, "useSSSseedsFilter", true};
      BooleanProperty m_useHClusSeed{this, "doHadCaloSeedSSS", false, "Hadronic Calorimeter Seeds"};
      BooleanProperty m_ITKGeometry{this, "ITKGeometry", false, "ITK geometry"};
      BooleanProperty m_seedsegmentsWrite{this, "SeedSegmentsWrite", false, "Call seed to track conversion"};

      DoubleProperty m_xi2max{this, "Xi2max", 15., "max Xi2 for updators"};
      DoubleProperty m_xi2maxNoAdd{this, "Xi2maxNoAdd", 35., "max Xi2 for clusters"};
      DoubleProperty m_xi2maxlink{this, "Xi2maxlink", 200., "max Xi2 for clusters"};
      DoubleProperty m_pTmin{this, "pTmin", 500., "min pT"};
      DoubleProperty m_pTminBrem{this, "pTminBrem", 1000., "min pT for Brem mode"};
      DoubleProperty m_distmax{this, "MaxDistanceForSCTsp", 5.};
      DoubleProperty m_xi2multitracks{this, "Xi2maxMultiTracks", 3., "max Xi2 for multi tracks"};
      IntegerProperty m_nholesmax{this, "nHolesMax", 2, "Max number holes"};
      IntegerProperty m_dholesmax{this, "nHolesGapMax", 2, "Max holes gap"};
      IntegerProperty m_nclusmin{this, "nClustersMin", 6,  "Min number clusters"};
      IntegerProperty m_nwclusmin{this, "nWeightedClustersMin", 6, "Min umber weighted clusters(pix=2 sct=1)"};
      DoubleProperty m_phiWidth{this, "phiWidth", 0.3};
      DoubleProperty m_etaWidth{this, "etaWidth", 0.3};
      DoubleArrayProperty m_etabins{this, "etaBins", {}, "eta bins"};
      DoubleArrayProperty m_ptbins{this, "pTBins", {}, "pT bins"};
      //@}

      /// @name Data members, which are updated only in initialize method
      //@{
      Trk::TrackInfo m_trackinfo;
      bool m_heavyion{false}; // Is it heavy ion events
      Trk::MagneticFieldMode m_fieldModeEnum{Trk::FullField};
      //@}

      ///////////////////////////////////////////////////////////////////
      // Counters
      /////////////////////////////////////////////////////////////////////

      mutable std::mutex            m_counterMutex;
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_totalInputSeeds        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_totalUsedSeeds        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_totalNoTrackPar        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_totalBremSeeds        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_twoClusters           ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_wrongRoad        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_wrongInit        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_noTrack        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_notNewTrack        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_bremAttempt        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_outputTracks        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_extraTracks        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_bremTracks        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<int>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_seedsWithTrack        ATLAS_THREAD_SAFE {};
      mutable std::array<std::atomic<double>,SiCombinatorialTrackFinderData_xk::kNSeedTypes>      m_deSize        ATLAS_THREAD_SAFE {};

      mutable std::vector<std::vector<double>>     m_usedSeedsEta          ATLAS_THREAD_SAFE;
      mutable std::vector<std::vector<double>>     m_seedsWithTracksEta    ATLAS_THREAD_SAFE;

      enum statAllTypes  {
        kTotalInputSeeds,
        kTotalUsedSeeds,
        kTotalNoTrackPar,
        kTotalBremSeeds,
        kTwoClusters,
        kWrongInit,
        kWrongRoad,
        kNoTrack,
        kNotNewTrack,
        kBremAttempt,
        kOutputTracks,
        kExtraTracks,
        kBremTracks,
        kDESize,
        kSeedsWithTracks
      };

      enum kNStatEtaTypes  {
        kUsedSeedsEta,
        kSeedsWithTracksEta
      };

      std::vector<statAllTypes> m_indexToEnum {kTwoClusters,kWrongInit,kWrongRoad,kNoTrack,kNotNewTrack,kBremAttempt};

      ///////////////////////////////////////////////////////////////////
      // Methods
      ///////////////////////////////////////////////////////////////////

      std::unique_ptr<Trk::TrackParameters> getAtaPlane(
        MagField::AtlasFieldCache& fieldCache,
        SiTrackMakerEventData_xk& data,
        bool sss,
        const std::vector<const Trk::SpacePoint*>& SP,
        const EventContext& ctx) const;
      bool globalPositions(const Trk::SpacePoint& s0,
                           const Trk::SpacePoint& s1,
                           const Trk::SpacePoint& s2,
                           double* p0,
                           double* p1,
                           double* p2) const;
      bool globalPosition(const Trk::SpacePoint& sp, const double* dir, double* p) const;
      static void globalDirections(const double* p0, const double* p1, const double* p2, double* d0, double* d1, double* d2) ;
      InDet::TrackQualityCuts setTrackQualityCuts(bool simpleTrack) const;
      static void detectorElementsSelection(SiTrackMakerEventData_xk& data,
                                     std::vector<const InDetDD::SiDetectorElement*>& DE) ;
      bool newSeed(SiTrackMakerEventData_xk& data, const std::vector<const Trk::SpacePoint*>& Sp) const;
      static int  kindSeed(const std::vector<const Trk::SpacePoint*>& Sp)  ;
      static int  rapidity(const std::vector<const Trk::SpacePoint*>& Sp) ;
      static bool isNewTrack(SiTrackMakerEventData_xk& data, Trk::Track* Tr) ;
      bool isCaloCompatible(SiTrackMakerEventData_xk& data) const;
      bool isHadCaloCompatible(SiTrackMakerEventData_xk& data) const;
      static void clusterTrackMap(SiTrackMakerEventData_xk& data, Trk::Track* Tr) ;
      double pTmin(double eta) const;

      MsgStream& dumpStatistics(MsgStream &out) const;
      MsgStream& dumpconditions(MsgStream& out) const;
      static MsgStream& dumpevent(SiTrackMakerEventData_xk& data, MsgStream& out) ;

      /// helper for working with the stat arrays
      template <typename T, size_t N,size_t M> void resetCounter(std::array<std::array<T,M>,N> & a) const{
        for (auto & subarr : a) resetCounter(subarr);
      }
      template <typename T, size_t N> void resetCounter(std::array<T,N> & a) const{
        std::fill(a.begin(),a.end(),0);
      }
    };

} // end of name space

#endif // SiTrackMaker_xk_H
