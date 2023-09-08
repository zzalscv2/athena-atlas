/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONIDHELPERSVC_H
#define MUON_MUONIDHELPERSVC_H

#include "AthenaBaseComps/AthService.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace Muon {
    /**
       @brief Helper service that creates muon Identifiers and can be used to print Identifiers

    */
    class MuonIdHelperSvc : public extends<AthService, IMuonIdHelperSvc> {
    public:
        /** @brief default AlgService constructor */
        MuonIdHelperSvc(const std::string& name, ISvcLocator* svc);

        /** @brief destructor */
        virtual ~MuonIdHelperSvc() override = default;

        /** @brief AlgTool initilize */
        virtual StatusCode initialize() override;

        /** @brief print all fields to string */
        virtual std::string toString(const Identifier& id) const override;

        /** @brief print all fields up to technology to string */
        virtual std::string toStringTech(const Identifier& id) const override;

        /** @brief print all fields up to stationName to string */
        virtual std::string toStringStation(const Identifier& id) const override;

        /** @brief print all fields up to chamber to string */
        virtual std::string toStringChamber(const Identifier& id) const override;

        /** @brief print all fields up to detector element to string */
        virtual std::string toStringDetEl(const Identifier& id) const override;

        /** @brief print all fields up to gas gap to string */
        virtual std::string toStringGasGap(const Identifier& id) const override;

        /** @brief print chamber name to string */
        virtual std::string chamberNameString(const Identifier& id) const override;

        /** @brief returns whether this is a Muon Identifier or not */
        virtual bool isMuon(const Identifier& id) const override;

        /** @brief returns whether this is a MDT Identifier or not */
        virtual bool isMdt(const Identifier& id) const override;

        /** @brief returns whether this is a MM Identifier or not */
        virtual bool isMM(const Identifier& id) const override;

        /** @brief returns whether this is a CSC Identifier or not */
        virtual bool isCsc(const Identifier& id) const override;

        /** @brief returns whether this is a RPC Identifier or not */
        virtual bool isRpc(const Identifier& id) const override;

        /** @brief returns whether this is a TGC Identifier or not */
        virtual bool isTgc(const Identifier& id) const override;

        /** @brief returns whether this is a sTGC Identifier or not */
        virtual bool issTgc(const Identifier& id) const override;

        /** @brief returns whether this is a sMDT Identifier or not */
        virtual bool issMdt(const Identifier& id) const override;

        /** @brief returns whether this Identifier belongs to an MDT with HPTDC or not
            NOTE that in Run4, no HPTDCs at all are planned to be present any more,
            so this function should be obsolete from Run4 onwards */
        virtual bool hasHPTDC(const Identifier& id) const override;

        /** @brief returns whether channel measures phi or not */
        virtual bool measuresPhi(const Identifier& id) const override;

        /** @brief returns whether this is an endcap Identifier or not */
        virtual bool isEndcap(const Identifier& id) const override;

        /** @brief returns whether trigger chamber id or not */
        virtual bool isTrigger(const Identifier& id) const override;

        /** @brief returns whether this is a small chamber, always returns true for TGCs */
        virtual bool isSmallChamber(const Identifier& id) const override;

        /** @brief returns gas gap: gasGap for RPC + TGC, wireLayer for CSC, tube for MDT */
        virtual int gasGap(const Identifier& id) const override;

        /** @brief create a chamber ID */
        virtual Identifier chamberId(const Identifier& id) const override;

        /** @brief create a detector element ID */
        virtual Identifier detElId(const Identifier& id) const override;

        /** @brief create a gasGap ID (will return layer Id for MDTs) */
        virtual Identifier gasGapId(const Identifier& id) const override;

        /** @brief create a layer ID, returns tube id for the MDTs */
        virtual Identifier layerId(const Identifier& id) const override;

        /** @brief access to MdtIdHelper */
        virtual const MdtIdHelper& mdtIdHelper() const override { return *m_mdtIdHelper; }

        /** @brief access to RpcIdHelper */
        virtual const RpcIdHelper& rpcIdHelper() const override { return *m_rpcIdHelper; }

        /** @brief access to TgcIdHelper */
        virtual const TgcIdHelper& tgcIdHelper() const override { return *m_tgcIdHelper; }

        /** @brief access to CscIdHelper */
        virtual const CscIdHelper& cscIdHelper() const override { return *m_cscIdHelper; }

        /** @brief access to TgcIdHelper */
        virtual const sTgcIdHelper& stgcIdHelper() const override { return *m_stgcIdHelper; }

        /** @brief access to CscIdHelper */
        virtual const MmIdHelper& mmIdHelper() const override { return *m_mmIdHelper; }

        /** @brief calculate chamber index from Identifier */
        virtual MuonStationIndex::ChIndex chamberIndex(const Identifier& id) const override;

        /** @brief calculate station index from Identifier */
        virtual MuonStationIndex::StIndex stationIndex(const Identifier& id) const override;

        /** @brief calculate phi index from Identifier (not supported for MDT hits) */
        virtual MuonStationIndex::PhiIndex phiIndex(const Identifier& id) const override;

        /** @brief calculate detector region index from Identifier */
        virtual MuonStationIndex::DetectorRegionIndex regionIndex(const Identifier& id) const override;

        /** @brief calculate layer index from Identifier */
        virtual MuonStationIndex::LayerIndex layerIndex(const Identifier& id) const override;

        /** @brief calculate layer index from Identifier */
        virtual MuonStationIndex::TechnologyIndex technologyIndex(const Identifier& id) const override;

        /** @brief Return stationPhi for all technologies */
        virtual int stationPhi(const Identifier& id) const override;

        /** @brief Return stationEta for all technologies */
        virtual int stationEta(const Identifier& id) const override;

        /** @brief Return the station name string for all technologies */
        virtual std::string stationNameString(const Identifier& id) const override;

        /** @brief Return stationName for all technologies */
        virtual int stationName(const Identifier& id) const override;

        /** @brief Return stationRegion for all technologies */
        virtual int stationRegion(const Identifier& id) const override;

        /** @brief return sector number 1-16, odd=large, even=small */
        virtual int sector(const Identifier& id) const override;

        bool hasRPC() const override;
        bool hasTGC() const override;
        bool hasMDT() const override;

        bool hasCSC() const override;
        bool hasSTGC() const override;
        bool hasMM() const override;

        /** @brief Returns the module hash associated to an Identifier */
        virtual IdentifierHash moduleHash(const Identifier& id) const override;
        /** @brief Returns the detector element hash associated to an Identifier */
        virtual IdentifierHash detElementHash(const Identifier& id) const override;

        
    private:
        IdentifierHash moduleHash(const MuonIdHelper& idHelper, const Identifier& id) const;
        IdentifierHash detElementHash(const MuonIdHelper& idHelper, const Identifier& id) const;
        
        /// Sub detector specific IdHelpers
        const RpcIdHelper* m_rpcIdHelper{nullptr};
        const TgcIdHelper* m_tgcIdHelper{nullptr};
        const CscIdHelper* m_cscIdHelper{nullptr};
        const MdtIdHelper* m_mdtIdHelper{nullptr};
        const MmIdHelper* m_mmIdHelper{nullptr};
        const sTgcIdHelper* m_stgcIdHelper{nullptr};

        const MuonIdHelper* m_primaryHelper{nullptr};

        Gaudi::Property<bool> m_hasMDT{this, "HasMDT", true, "Specify whether the current detector layour contains MDT chambers"};
        Gaudi::Property<bool> m_hasRPC{this, "HasRPC", true, "Specify whether the current detector layour contains RPC chambers"};
        Gaudi::Property<bool> m_hasTGC{this, "HasTGC", true, "Specify whether the current detector layour contains TGC chambers"};        
        Gaudi::Property<bool> m_hasCSC{this, "HasCSC", true, "Specify whether current detector layout contains CSC chambers"};
        Gaudi::Property<bool> m_hasSTGC{this, "HasSTGC", true, "Specify whether current detector layout contains STGC chambers"};
        Gaudi::Property<bool> m_hasMM{this, "HasMM", true, "Specify whether current detector layout contains MicroMegas chambers"};

        ServiceHandle<StoreGateSvc> m_detStore{this, "DetectorStore", "DetectorStore"};

        struct StationNameData {
            std::string stationName{};
            bool isEndcap{false};
            bool isSmall{false};
            MuonStationIndex::ChIndex chIndex{};
            MuonStationIndex::StIndex stIndex{};
        };
        std::vector<StationNameData> m_stationNameData;
        std::vector<MuonStationIndex::TechnologyIndex> m_technologies;

        int m_BIS_stat{-1};
        std::set<Identifier> m_smdt_stat{};
    };

}  // namespace Muon

#endif  // MUON_MUONIDHELPERSVC_H
