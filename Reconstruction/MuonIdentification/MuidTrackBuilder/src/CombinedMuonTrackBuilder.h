/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////////
// CombinedMuonTrackBuilder
//  AlgTool gathering  material effects along a combined muon track, in
//  particular the TSOS'es representing the calorimeter energy deposit and
//  Coulomb scattering.
//  The resulting track is fitted at the IP
//
//////////////////////////////////////////////////////////////////////////////

#ifndef MUIDTRACKBUILDER_COMBINEDMUONTRACKBUILDER_H
#define MUIDTRACKBUILDER_COMBINEDMUONTRACKBUILDER_H

#include <atomic>

#include "CombinedMuonTrackFitter.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MuidInterfaces/ICombinedMuonTrackBuilder.h"
#include "MuidInterfaces/IMuidCaloEnergy.h"
#include "MuidInterfaces/IMuidCaloTrackStateOnSurface.h"
#include "MuidInterfaces/IMuonAlignmentUncertTool.h"
#include "MuidInterfaces/IMuonTrackQuery.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/IMdtDriftCircleOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonErrorOptimisationTool.h"
#include "MuonRecToolInterfaces/IMuonHoleRecoveryTool.h"
#include "MuonRecToolInterfaces/IMuonTrackCleaner.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TrkDetDescrInterfaces/ITrackingVolumesSvc.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkExInterfaces/IIntersector.h"
#include "TrkExInterfaces/IPropagator.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TrkToolInterfaces/ITrkMaterialProviderTool.h"
#include "TrkTrack/TrackInfo.h"
#include "TrkiPatFitterUtils/IMaterialAllocator.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
class CaloEnergy;
class MessageHelper;

namespace Trk {
    class PerigeeSurface;
    class PseudoMeasurementOnTrack;
    class RecVertex;
    class TrackStateOnSurface;
}  // namespace Trk

namespace Rec {

    class CombinedMuonTrackBuilder : public CombinedMuonTrackFitter, virtual public ICombinedMuonTrackBuilder {
    public:
        CombinedMuonTrackBuilder(const std::string& type, const std::string& name, const IInterface* parent);
        virtual ~CombinedMuonTrackBuilder();

        virtual StatusCode initialize() override;
        virtual StatusCode finalize() override;

        /** ICombinedMuonTrackBuilder interface: build and fit combined ID/Calo/MS track */
        virtual std::unique_ptr<Trk::Track> combinedFit(const EventContext& ctx, const Trk::Track& indetTrack, const Trk::Track& extrapolatedTrack,
                                                        const Trk::Track& spectrometerTrack) const override;

        /** ICombinedMuonTrackBuilder interface:
            build and fit indet track extended to include MS Measurement set.
            Adds material effects as appropriate plus calo energy-loss treatment */
        virtual std::unique_ptr<Trk::Track> indetExtension(const EventContext& ctx,const Trk::Track& indetTrack, const Trk::MeasurementSet& spectrometerMeas,                                                           
                                                           std::unique_ptr<Trk::TrackParameters> innerParameters,                                                           
                                                           std::unique_ptr<Trk::TrackParameters> middleParameters,                                                           
                                                           std::unique_ptr<Trk::TrackParameters> outerParameters) const override;

        /** ICombinedMuonTrackBuilder interface:
            propagate to perigee adding calo energy-loss and material to MS track */
        virtual std::unique_ptr<Trk::Track> standaloneFit(const EventContext& ctx, const Trk::Track& spectrometerTrack,
                                                          const Amg::Vector3D& bs, const Trk::Vertex* vertex) const override;

        /** ICombinedMuonTrackBuilder interface:
            refit a track removing any indet measurements with optional addition of pseudoMeasurements */
        virtual std::unique_ptr<Trk::Track> standaloneRefit(const EventContext& ctx, const Trk::Track& combinedTrack, const Amg::Vector3D& vec) const override;

       

    private:
        std::unique_ptr<Trk::Track> addIDMSerrors(const Trk::Track* track) const;

        void appendSelectedTSOS(Trk::TrackStates& trackStateOnSurfaces,
                                Trk::TrackStates::const_iterator begin,
                                Trk::TrackStates::const_iterator end) const;

        const CaloEnergy* caloEnergyParameters(const Trk::Track* combinedTrack, const Trk::Track* muonTrack,
                                               const Trk::TrackParameters*& combinedEnergyParameters,
                                               const Trk::TrackParameters*& muonEnergyParameters) const;

        std::unique_ptr<Trk::Track> createExtrapolatedTrack(
            const EventContext& ctx, const Trk::Track& spectrometerTrack, const Trk::TrackParameters& parameters,
            Trk::ParticleHypothesis particleHypothesis, Trk::RunOutlierRemoval runOutlier,
            const std::vector<std::unique_ptr<const Trk::TrackStateOnSurface>>& trackStateOnSurfaces, const Trk::RecVertex* vertex,
            const Trk::RecVertex* mbeamAxis, const Trk::PerigeeSurface* mperigeeSurface, const Trk::Perigee* seedParameter = nullptr) const;

        std::unique_ptr<Trk::Track> createIndetTrack(const Trk::TrackInfo& info,
                                                     const Trk::TrackStates* tsos) const;
        /// Summarizes the available information about the ID track,
        ///            the deposited calorimeter energies and the track states into a new track
        ///   --> m_materialUpdator as only tool called , but does not provide an interface method with ctx thus far
        std::unique_ptr<Trk::Track> createMuonTrack(const EventContext& ctx, const Trk::Track& muonTrack,
                                                    const Trk::TrackParameters* parameters, std::unique_ptr<CaloEnergy> caloEnergy,
                                                    const Trk::TrackStates* tsos) const;

        std::unique_ptr<Trk::TrackStateOnSurface> createPhiPseudoMeasurement(const EventContext& ctx, const Trk::Track& track) const;

        std::vector<std::unique_ptr<const Trk::TrackStateOnSurface>> createSpectrometerTSOS(const EventContext& ctx, const Trk::Track& spectrometerTrack) const;

        std::unique_ptr<Trk::TrackStateOnSurface> entrancePerigee(const EventContext& ctx, const Trk::TrackParameters* parameters) const;

        std::unique_ptr<Trk::TrackParameters> extrapolatedParameters(const EventContext& ctx, bool& badlyDeterminedCurvature,
                                                                           const Trk::Track& spectrometerTrack,
                                                                           const Trk::RecVertex* mvertex,
                                                                           const Trk::PerigeeSurface* mperigeeSurface) const;

        void finalTrackBuild(const EventContext& ctx, std::unique_ptr<Trk::Track>& track) const;

        void momentumUpdate(std::unique_ptr<Trk::TrackParameters>& parameters, double updatedP, bool directionUpdate = false,
                            double deltaPhi = 0., double deltaTheta = 0.) const;

        // double normalizedChi2(const Trk::Track& track) const;
        std::unique_ptr<Trk::Track> reallocateMaterial(const EventContext& ctx, const Trk::Track& spectrometerTrack) const;
        void replaceCaloEnergy(const CaloEnergy* caloEnergy, Trk::Track* track) const;
        void removeSpectrometerMaterial(std::unique_ptr<Trk::Track>& track) const;

        static std::unique_ptr<Trk::PseudoMeasurementOnTrack> vertexOnTrack(const Trk::TrackParameters& parameters,
                                                                            const Trk::RecVertex* vertex, const Trk::RecVertex* mbeamAxis);

        void dumpCaloEloss(const Trk::Track* track, const std::string& txt) const;
        
        // helpers, managers, tools
        ToolHandle<Rec::IMuidCaloEnergy> m_caloEnergyParam{
            this,
            "CaloEnergyParam",
            "",
        };        
        ToolHandle<Muon::IMuonClusterOnTrackCreator> m_cscRotCreator{
            this,
            "CscRotCreator",
            "",
        };

        ToolHandle<Muon::IMuonClusterOnTrackCreator> m_muClusterRotCreator{this, "MuonRotCreator", ""};

        ToolHandle<Trk::IExtrapolator> m_extrapolator{
            this,
            "Extrapolator",
            "Trk::Extrapolator/AtlasExtrapolator",
        };        
        ToolHandle<Trk::IMaterialAllocator> m_materialAllocator{
            this,
            "MaterialAllocator",
            "",
        };
        ToolHandle<Muon::IMdtDriftCircleOnTrackCreator> m_mdtRotCreator{
            this,
            "MdtRotCreator",
            "",
        };        
        ToolHandle<Muon::IMuonHoleRecoveryTool> m_muonHoleRecovery{
            this,
            "MuonHoleRecovery",
            "",
        };
        ToolHandle<Trk::IPropagator> m_propagator{
            this,
            "Propagator",
            "Trk::IntersectorWrapper/IntersectorWrapper",
        };
        ToolHandle<Trk::IPropagator> m_propagatorSL{
            this,
            "SLPropagator",
            "Trk::StraightLinePropagator/MuonStraightLinePropagator",
        };
        
        /// ToolHandles to retrieve the uncertainties for theta and phi for
        /// the scattering uncertainties

        PublicToolHandle<Muon::IMuonAlignmentUncertTool> m_alignUncertTool_theta{this, "AlignmentUncertToolTheta", 
                                                        "Muon::MuonAlignmentUncertTool/MuonAlignmentUncertToolTheta" };
        PublicToolHandle<Muon::IMuonAlignmentUncertTool> m_alignUncertTool_phi{this, "AlignmentUncertToolPhi", 
                                                                                "Muon::MuonAlignmentUncertTool/MuonAlignmentUncertToolPhi"};

       
        ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc{this, "edmHelper", "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
                                                        "Handle to the service providing the IMuonEDMHelperSvc interface"};


        // Read handle for conditions object to get the field cache
        
        SG::ReadCondHandleKey<Trk::TrackingGeometry> m_trackingGeometryReadKey{this, "TrackingGeometryReadKey", "",
                                                                               "Key of the TrackingGeometry conditions data."};


        Trk::MagneticFieldProperties m_magFieldProperties{Trk::FullField};

       
        Gaudi::Property<bool> m_cleanCombined{this, "CleanCombined", true};
        Gaudi::Property<bool> m_cleanStandalone{this, "CleanStandalone", true};

        Gaudi::Property<bool> m_perigeeAtSpectrometerEntrance{this, "PerigeeAtSpectrometerEntrance", false};
        Gaudi::Property<bool> m_reallocateMaterial{this, "ReallocateMaterial", true};

      
        Gaudi::Property<double> m_largeImpact{this, "LargeImpact", 100. * Gaudi::Units::mm};

        Gaudi::Property<double> m_largeMomentumChange{this, "LargeMomentumChange", 0.05};
        Gaudi::Property<double> m_largeMomentumError{this, "LargeMomentumError", 0.15};
        Gaudi::Property<double> m_largePhiError{this, "LargePhiError", 0.020};
        Gaudi::Property<double> m_lineMomentum{this, "LineMomentum", 2. * Gaudi::Units::GeV};
        Gaudi::Property<double> m_lowMomentum{this, "LowMomentum", 10. * Gaudi::Units::GeV};

        Gaudi::Property<double> m_minEnergy{this, "MinEnergy", 0.3 * Gaudi::Units::GeV};
        Gaudi::Property<double> m_numberSigmaFSR{this, "NumberSigmaFSR", 2.5};

        Gaudi::Property<double> m_vertex2DSigmaRPhi{this, "Vertex2DSigmaRPhi", 100. * Gaudi::Units::mm};
        Gaudi::Property<double> m_vertex2DSigmaZ{this, "Vertex2DSigmaZ", 100. * Gaudi::Units::meter};
        Gaudi::Property<double> m_vertex3DSigmaRPhi{this, "Vertex3DSigmaRPhi", 6. * Gaudi::Units::mm};
        Gaudi::Property<double> m_vertex3DSigmaZ{this, "Vertex3DSigmaZ", 60. * Gaudi::Units::mm};
       
        bool m_redoRots{false};

      
        // vertex region and phi modularity for pseudo-measurement constraints
        std::unique_ptr<const Trk::RecVertex> m_beamAxis;
        std::unique_ptr<const Trk::PerigeeSurface> m_perigeeSurface;

        std::unique_ptr<const Trk::RecVertex> m_vertex;

        // counters
        mutable std::atomic_uint m_countAcceptedStandaloneFit{0};
        mutable std::atomic_uint m_countBeamAxis{0};
        mutable std::atomic_uint m_countDegradedStandaloneFit{0};
        mutable std::atomic_uint m_countVertexRegion{0};

        
        Gaudi::Property<bool> m_iterateCombinedTrackFit{this, "IterateCombinedTrackFit", false};
        Gaudi::Property<bool> m_refineELossCombinedTrackFit{this, "RefineELossCombinedTrackFit", true};
        Gaudi::Property<bool> m_refineELossStandAloneTrackFit{this, "RefineELossStandAloneTrackFit", true};
        Gaudi::Property<bool> m_addElossID{this, "AddElossID", true};
        Gaudi::Property<bool> m_addIDMSerrors{this, "AddIDMSerrors", true};
        Gaudi::Property<bool> m_useRefitTrackError{this, "UseRefitTrackError", true};

        const Trk::TrackingVolume* getVolume(const EventContext& ctx, const std::string&& vol_name) const;
               
        /// Helper method to retrieve the CaloTSO from the Material provider
        /// in a memory safe way
        std::vector<std::unique_ptr<const Trk::TrackStateOnSurface>> getCaloTSOSfromMatProvider(const Trk::TrackParameters& track_params,
                                                                                                const Trk::Track& me_track) const;

    };  // end of class CombinedMuonTrackBuilder

}  // end of namespace Rec

#endif  // MUIDTRACKBUILDER_COMBINEDMUONTRACKBUILDER_H
