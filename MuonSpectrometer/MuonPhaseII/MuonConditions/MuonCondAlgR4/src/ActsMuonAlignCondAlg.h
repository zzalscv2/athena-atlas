/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDALGR4_ACTSMUONALIGNCONDALG_H
#define MUONCONDALGR4_ACTSMUONALIGNCONDALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/CondHandleKeyArray.h>

#include <MuonStationGeoHelpers/IMuonStationLayerSurfaceTool.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <ActsGeometryInterfaces/RawGeomAlignStore.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonAlignmentData/CorrContainer.h>

/* The ActsMuonAlignCondAlg takes the ALineContainer and translates this into the  
 *  a GeoAlignmentStore. The store is filled with the AlignableTransforms of the ReadoutGeometry
 *  which are connected with the A-line transformations of the ALineContainer.
 **/
class ActsMuonAlignCondAlg: public AthReentrantAlgorithm {
public:
      ActsMuonAlignCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
      virtual ~ActsMuonAlignCondAlg() = default;
      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;
      virtual bool isReEntrant() const override { return false; }

private:
    SG::ReadCondHandleKey<ALineContainer> m_readKey{this, "ReadKey", "ALineContainer",
                                                    "Key of the ALine container created from the DB"};

    std::vector<ActsTrk::DetectorType> m_techs{};
    SG::WriteCondHandleKeyArray<ActsTrk::RawGeomAlignStore> m_writeKeys{this, "WriteKeys", {},
                                                                        "Keys of the alignment technologies"};
    Gaudi::Property<std::string> m_keyToken{this, "CondKeyToken","ActsAlignContainer",
                                            "Common name token of all written alignment objects (e.g.) MdtActsAlignContainer"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    PublicToolHandle<MuonGMR4::IMuonStationLayerSurfaceTool> m_surfaceProvTool{this, "LayerGeoTool", ""};

    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
};


#endif
