/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMdtRdoToPrepDataToolCore_H
#define MUONMdtRdoToPrepDataToolCore_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MdtCalibSvc/MdtCalibrationSvcSettings.h"
#include "MdtCalibSvc/MdtCalibrationTool.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonMDT_CnvTools/IMDT_RDO_Decoder.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRDO/MdtCsmContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandleKey.h"
class MdtDigit;
class MdtCalibHit;

namespace MuonGM {
    class MdtReadoutElement;
}

namespace Muon {
    /** @class MdtRdoToPrepDataToolCore

        This is for the Doxygen-Documentation.
        Please delete these lines and fill in information about
        the Algorithm!
        Please precede every member function declaration with a
        short Doxygen comment stating the purpose of this function.

        @author  Edward Moyse <Edward.Moyse@cern.ch>
    */

    class MdtRdoToPrepDataToolCore : public extends<AthAlgTool, IMuonRdoToPrepDataTool> {
    public:
        MdtRdoToPrepDataToolCore(const std::string&, const std::string&, const IInterface*);

        /** default destructor */
        virtual ~MdtRdoToPrepDataToolCore() = default;

        /** standard Athena-Algorithm method */
        virtual StatusCode initialize() override;

        /** Decode method - declared in Muon::IMuonRdoToPrepDataTool*/
        virtual StatusCode decode(std::vector<IdentifierHash>& idVect, std::vector<IdentifierHash>& selectedIdVect) const override;
        // new decode method for Rob based readout
        virtual StatusCode decode(const std::vector<uint32_t>& robIds) const override;

        // dump methods for debugging
        virtual void printInputRdo() const override;

    protected:
        void printPrepDataImpl(const Muon::MdtPrepDataContainer* mdtPrepDataContainer) const;

        Muon::MdtDriftCircleStatus getMdtDriftRadius(const MdtDigit& digit, double& radius, double& errRadius,
                                                     const MuonGM::MdtReadoutElement* descriptor,
                                                     const MuonGM::MuonDetectorManager* muDetMgr) const;
        /// method to get the twin tube 2nd coordinate
        Muon::MdtDriftCircleStatus getMdtTwinPosition(const MdtDigit& prompt_digit, const MdtDigit& twin_digit, double& radius,
                                                      double& errRadius, double& zTwin, double& errZTwin, bool& twinIsPrompt,
                                                      const MuonGM::MuonDetectorManager* muDetMgr) const;

        // decode method for Rob based readout
        StatusCode decode(const EventContext& ctx, const std::vector<IdentifierHash>& multiLayerHashInRobs) const;

        /// Helper struct to steer which collections were added by
        /// this tool and which already existed before hand
        struct ModfiablePrdColl {
            ModfiablePrdColl() = default;
            ModfiablePrdColl(Muon::MdtPrepDataContainer* cont) : prd_cont{cont} {}
            /// Creates a new MdtPrepDataCollection, if it's neccessary
            /// and also possible. Nullptr is returned if the collection
            /// cannot be modified
            MdtPrepDataCollection* createCollection(const Identifier& id, const MdtIdHelper& id_helper, MsgStream& msg);
            /// Copy the non-empty collections into the created prd container. Fill the id_hash vector with
            /// the corresponding hashes
            StatusCode finalize(std::vector<IdentifierHash>& id_hash, MsgStream& msg);

            Muon::MdtPrepDataContainer* prd_cont{nullptr};

            using PrdCollMap = std::map<IdentifierHash, std::unique_ptr<MdtPrepDataCollection>>;
            PrdCollMap addedCols{};
        };

        StatusCode processCsm(ModfiablePrdColl& mdtPrepDataContainer, const MdtCsm* rdoColl,
                              const MuonGM::MuonDetectorManager* muDetMgr) const;

        StatusCode processCsmTwin(ModfiablePrdColl& mdtPrepDataContainer, const MdtCsm* rdoColll,
                                  const MuonGM::MuonDetectorManager* muDetMgr) const;

        /// Creates the prep data container to be written
        ModfiablePrdColl setupMdtPrepDataContainer(const EventContext& ctx) const;
        /// Is the identifier disabled due to BMG cut outs
        bool deadBMGChannel(const Identifier& channelId) const;

        /// Loads the input RDO container from StoreGate
        const MdtCsmContainer* getRdoContainer(const EventContext& ctx) const;

        void processPRDHashes(const EventContext& ctx, ModfiablePrdColl& mdtPrepDataContainer,
                              const std::vector<IdentifierHash>& chamberHashInRobs) const;

        bool handlePRDHash(const EventContext& ctx, ModfiablePrdColl& mdtPrepDataContainer, IdentifierHash rdoHash) const;

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        /// MDT calibration service
        ToolHandle<MdtCalibrationTool> m_calibrationTool{this, "CalibrationTool", "MdtCalibrationTool"};
        std::unique_ptr<MdtCalibrationSvcSettings> m_mdtCalibSvcSettings{std::make_unique<MdtCalibrationSvcSettings>()};

        /// MdtPrepRawData containers
        SG::WriteHandleKey<Muon::MdtPrepDataContainer> m_mdtPrepDataContainerKey{this, "OutputCollection", "MDT_DriftCircles"};

        SG::ReadHandleKey<MdtCsmContainer> m_rdoContainerKey{this, "RDOContainer", "MDTCSM"};

        /** member variables for algorithm properties: */
        Gaudi::Property<bool> m_calibratePrepData{this, "CalibratePrepData", true};  //!< toggle on/off calibration of MdtPrepData
        Gaudi::Property<bool> m_decodeData{this, "DecodeData", true};  //!< toggle on/off the decoding of MDT RDO into MdtPrepData
        bool m_sortPrepData = false;                                   //!< Toggle on/off the sorting of the MdtPrepData

        ToolHandle<Muon::IMDT_RDO_Decoder> m_mdtDecoder{this, "Decoder", "Muon::MdtRDO_Decoder/MdtRDO_Decoder"};

        bool m_BMGpresent{false};
        int m_BMGid{-1};

        // + TWIN TUBE
        Gaudi::Property<bool> m_useTwin{this, "UseTwin", true};
        Gaudi::Property<bool> m_useAllBOLTwin{this, "UseAllBOLTwin", false};
        Gaudi::Property<bool> m_use1DPrepDataTwin{this, "Use1DPrepDataTwin", false};
        Gaudi::Property<bool> m_twinCorrectSlewing{this, "TwinCorrectSlewing", false};
        Gaudi::Property<bool> m_discardSecondaryHitTwin{this, "DiscardSecondaryHitTwin", false};
        int m_twin_chamber[2][3][36]{};
        int m_secondaryHit_twin_chamber[2][3][36]{};
        // - TWIN TUBE

        std::map<Identifier, std::vector<Identifier>> m_DeadChannels;
        void initDeadChannels(const MuonGM::MdtReadoutElement* mydetEl);

        SG::ReadCondHandleKey<MuonMDT_CablingMap> m_readKey{this, "ReadKey", "MuonMDT_CablingMap", "Key of MuonMDT_CablingMap"};

        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                         "Key of input MuonDetectorManager condition data"};
        /// This is the key for the cache for the MDT PRD containers, can be empty
        SG::UpdateHandleKey<MdtPrepDataCollection_Cache> m_prdContainerCacheKey;
    };
}  // namespace Muon

#endif
