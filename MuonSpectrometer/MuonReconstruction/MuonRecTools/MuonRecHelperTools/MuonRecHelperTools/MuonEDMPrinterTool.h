/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRECHELPERTOOLS_H
#define MUONRECHELPERTOOLS_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPattern/MuonPatternCollection.h"
#include "MuonPattern/MuonPatternCombinationCollection.h"
#include "MuonPrepRawData/MdtPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "MuonSegment/MuonSegmentCombinationCollection.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"


namespace Trk {
class Track;
class MuonTrackSummary;
class ResidualPull;
class MeasurementBase;
class PrepRawData;
class MaterialEffectsBase;
class TrackStateOnSurface;
}  // namespace Trk

namespace Muon {
class MuonSegment;
class MuonSegmentCombination;
class MuonPattern;
class MuonPatternCombination;
class MuonPatternChamberIntersect;
/**
   @brief Helper tool to print EDM objects to string in a fix format

*/
class MuonEDMPrinterTool : public AthAlgTool {
  public:
    /** @brief constructor */
    MuonEDMPrinterTool(const std::string&, const std::string&, const IInterface*);

    /** @brief destructor */
    ~MuonEDMPrinterTool()= default;

    /** @brief AlgTool initilize */
    StatusCode initialize();

    /** @brief access to tool interface */
    static const InterfaceID& interfaceID()
    {
        static const InterfaceID IID_MuonEDMPrinterTool("Muon::MuonEDMPrinterTool", 1, 0);
        return IID_MuonEDMPrinterTool;
    }

    /** @brief print track parameters to string */
    std::string print(const Trk::Track& track) const;

    /** @brief print stations on track to string */
    std::string printStations(const Trk::Track& track) const;

    /** @brief print stations on track to string */
    std::string print(const Trk::MuonTrackSummary& summary) const;

    /** @brief produce a string indicating who produced the track */
    static std::string printPatRec(const Trk::Track& track) ;

    /** @brief print segment parameters to string */
    std::string print(const MuonSegment& segment) const;

    /** @brief print vector of segments  */
    std::string print(const std::vector<const MuonSegment*>& segs) const;
    std::string print(std::vector<std::unique_ptr<MuonSegment> >& segs) const;

    /** @brief print Muon MeasurementBase to string */
    std::string print(const Trk::MeasurementBase& measurement) const;

    /** @brief print Muon PrepRawData to string */
    std::string print(const Trk::PrepRawData& prd) const;

    /** @brief print vector of measurement base to string */
    std::string print(const std::vector<const Trk::MeasurementBase*>& measurements) const;

    /** @brief print MuonSegmentCombinationCollection */
    std::string print(const MuonSegmentCombinationCollection& combiCol) const;

    /** @brief print MuonSegmentCombination */
    std::string print(const MuonSegmentCombination& combi) const;

    /** @brief print MuonPattern */
    std::string print(const MuonPattern& pattern) const;

    /** @brief print MuonPatternCollection */
    std::string print(const MuonPatternCollection& patCol) const;

    /** @brief print MuonPatternCollection */
    std::string print(const MuonPrdPatternCollection& patCol) const;

    /** @brief print MuonPatternCombination */
    std::string print(const MuonPatternCombination& pattern) const;

    /** @brief print MuonPatternCombinationCollection */
    std::string print(const MuonPatternCombinationCollection& combiCol) const;

    /** @brief print MuonPatternChamberIntersect */
    std::string print(const MuonPatternChamberIntersect& intersect) const;

    /** @brief print Trk::TrackParameters */
    static std::string print(const Trk::TrackParameters& pars) ;

    /** @brief  print ResidualPull object to string*/
    static std::string print(const Trk::ResidualPull& resPull) ;

    /** @brief print the material effects object to the string */
    static std::string print(const Trk::MaterialEffectsBase& mat) ;
    /** @brief print the alignment effects on track object to the string*/
    static std::string print(const Trk::AlignmentEffectsOnTrack& aeot) ;
    
    /** @brief print the track state on surface to the string */
    std::string print(const Trk::TrackStateOnSurface& tsos) const;

    /** @brief print measurements on track to string */
    std::string printMeasurements(const Trk::Track& track) const;
    
    /** @brief print data part of Muon MeasurementBase to string */
    std::string printData(const Trk::MeasurementBase& measurement) const;

    /** @brief print identifier part of Muon MeasurementBase to string */
    std::string printId(const Trk::MeasurementBase& measurement) const;

  private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this,
        "MuonIdHelperSvc",
        "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
    };
    ServiceHandle<IMuonEDMHelperSvc> m_edmHelperSvc{
        this,
        "edmHelper",
        "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
        "Handle to the service providing the IMuonEDMHelperSvc interface",
    };

    ToolHandle<Trk::IResidualPullCalculator> m_pullCalculator{
        this,
        "ResidualPullCalculator",
        "Trk::ResidualPullCalculator/ResidualPullCalculator",
    };

    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{
        this,
        "DetectorManagerKey",
        "MuonDetectorManager",
        "Key of input MuonDetectorManager condition data",
    };
    SG::ReadHandleKey<MdtPrepDataContainer> m_mdtKey{
        this,
        "MdtPrdCollection",
        "MDT_DriftCircles",
        "MDT PRD Container",
    };
    SG::ReadHandleKey<RpcPrepDataContainer> m_rpcKey{
        this,
        "RpcPrdCollection",
        "RPC_Measurements",
        "RPC PRD Container",
    };
    SG::ReadHandleKey<TgcPrepDataContainer> m_tgcKey{
        this,
        "TgcPrdCollection",
        "TGC_Measurements",
        "TGC PRD Container",
    };
};

}  // namespace Muon
#endif
