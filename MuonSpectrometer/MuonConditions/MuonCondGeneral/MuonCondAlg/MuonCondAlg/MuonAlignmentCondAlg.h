/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_MUONALIGNMENTCONDALG_H
#define MUONCONDALG_MUONALIGNMENTCONDALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Blob.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/ReadCondHandleKey.h"
// typedefs for A/BLineMapContainer
#include "CoralUtilities/blobaccess.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "nlohmann/json.hpp"


class MuonAlignmentCondAlg : public AthAlgorithm {
public:
    MuonAlignmentCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MuonAlignmentCondAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;


private:
    /** Attaches the dependencies of the Alignment keys onto the A & Bline container*/
    StatusCode attachDependencies(const EventContext& ctx,
                              SG::WriteCondHandle<ALineContainer>& alines,
                              SG::WriteCondHandle<BLineContainer>& blines ) const;
    /**
     *  Load the Alignment data from the legacy format where the channels are parsed line wise 
     *  The data is then transferred into a modern JSON blob
     * */
    StatusCode loadDataFromLegacy(const std::string& data, nlohmann::json& json, bool loadBLines) const;
    /**
     *  Parse the JSON blob to fill the A & B Line containers
     * */
    StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                ALineContainer& writeALineCdo, 
                                BLineContainer& writeBLineCdo) const;
    /**
     *  Retrieves the alignment parameters from a COOL folder
     * */
    StatusCode loadCoolFolder(const EventContext& ctx,
                              const SG::ReadCondHandleKey<CondAttrListCollection>& key,
                              ALineContainer& writeALineCdo, 
                              BLineContainer& writeBLineCdo) const;
    
    // Read Handles
    SG::ReadCondHandleKeyArray<CondAttrListCollection> m_alignKeys{this, "MuonAlignmentKeys", {
                                                                  "/MUONALIGN/MDT/BARREL",
                                                                  "/MUONALIGN/MDT/ENDCAP/SIDEA",
                                                                  "/MUONALIGN/MDT/ENDCAP/SIDEC",
                                                                  "/MUONALIGN/TGC/SIDEA",
                                                                  "/MUONALIGN/TGC/SIDEC",
                                                                  },
                                                                  "Folder names where the alignment paramters are stored"};



    SG::ReadCondHandleKey<CondAttrListCollection> m_readCscILinesKey{this, "ReadCscILinesKey", "/MUONALIGN/CSC/ILINES",
                                                                     "Key of input CSC/ILINES condition data"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readMdtAsBuiltParamsKey{this, "ReadMdtAsBuiltParamsKey", "/MUONALIGN/MDT/ASBUILTPARAMS",
                                                                            "Key of MDT/ASBUILTPARAMS input condition data"};
    // Write Handles
    SG::WriteCondHandleKey<ALineContainer> m_writeALineKey{this, "WriteALineKey", "ALineContainer",
                                                              "Key of output muon alignment ALine condition data"};
    SG::WriteCondHandleKey<BLineContainer> m_writeBLineKey{this, "WriteBLineKey", "BLineContainer",
                                                              "Key of output muon alignment BLine condition data"};
    SG::WriteCondHandleKey<CscInternalAlignmentMapContainer> m_writeILineKey{this, "WriteILineKey", "CscInternalAlignmentMapContainer",
                                                                             "Key of output muon alignment CSC/ILine condition data"};
    SG::WriteCondHandleKey<MdtAsBuiltMapContainer> m_writeMdtAsBuiltKey{this, "WriteMdtAsBuiltKey", "MdtAsBuiltMapContainer",
                                                                     "Key of output muon alignment MDT/AsBuilt condition data"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
   
    Gaudi::Property<std::vector<std::string>> m_parlineFolder{this, "ParlineFolders", {}, "Database folders"};
                                                              

    Gaudi::Property<bool> m_dumpILines{this, "DumpILines", false};
    Gaudi::Property<bool> m_ILinesFromDb{this, "ILinesFromCondDB", false};

    bool m_MdtAsBuiltRequested {false};
    bool m_ILineRequested {false};
    Gaudi::Property<std::string> m_aLinesFile{this, "ALinesFile", "" };
    Gaudi::Property<std::string> m_asBuiltFile{this, "AsBuiltFile", ""};

    // new folder format 2020
    Gaudi::Property<bool> m_newFormat2020 {this, "NewFormat2020", false, 
                          "The database folders are given in the new JSON format"};
     /// @brief Load the alignment parameters from a JSON file
    Gaudi::Property<std::string> m_readFromJSON{this,"readFromJSON", "",
                                 "Reads the A & B lines parameters from a JSON file instead from COOL"};

    
    StatusCode loadAlignILines(const std::string& folderName);
    StatusCode loadAlignILinesData(const std::string& folderName, const std::string& data, nlohmann::json& json);
    StatusCode loadMdtAlignAsBuilt(const std::string& folderName);

    void setAsBuiltFromAscii(MdtAsBuiltMapContainer* writeCdo) const;
    void dumpILines(const std::string& folderName, CscInternalAlignmentMapContainer* writeCdo);
};

#endif
