/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ATHENA_INNERDETECTOR_TOOLS_BOUNDARYCHECKTOOL
#define ATHENA_INNERDETECTOR_TOOLS_BOUNDARYCHECKTOOL

#include "AthenaBaseComps/AthAlgTool.h"

#include "InDetConditionsSummaryService/IInDetConditionsTool.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetRecToolInterfaces/IInDetTestPixelLayerTool.h"

#include "TrkParameters/TrackParameters.h"
#include "TrkToolInterfaces/IBoundaryCheckTool.h"
#include "InDetReadoutGeometry/SiDetectorElementStatus.h"

#include "GeoModelInterfaces/IGeoModelSvc.h"
class SCT_ID;

namespace InDet {
    class InDetBoundaryCheckTool : public AthAlgTool, virtual public Trk::IBoundaryCheckTool {
        public:
            InDetBoundaryCheckTool(
                const std::string &,
                const std::string &,
                const IInterface *
            );

            virtual StatusCode initialize() override;
            virtual StatusCode finalize() override;

            virtual Trk::BoundaryCheckResult boundaryCheck(
                const Trk::TrackParameters &
            ) const override;
        private:
            bool isAlivePixel(
                const InDetDD::SiDetectorElement &element,
                const Trk::TrackParameters &parameters
            ) const;

            bool isAliveSCT(
                const InDetDD::SiDetectorElement &element,
                const Trk::TrackParameters &parameters
            ) const;

            bool isBadSCTChipStrip(
                const InDet::SiDetectorElementStatus *,
                const Identifier &,
                const Trk::TrackParameters &,
                const InDetDD::SiDetectorElement &
            ) const;

            Trk::BoundaryCheckResult boundaryCheckSiElement(
                const InDetDD::SiDetectorElement &,
                const Trk::TrackParameters &
            ) const;

            SG::ReadHandle<InDet::SiDetectorElementStatus> getSCTDetElStatus(const EventContext& ctx) const;

            ServiceHandle<IGeoModelSvc> m_geoModelSvc;

            ToolHandle<IInDetConditionsTool> m_sctCondSummaryTool{
                this,
                "SctSummaryTool",
                "SCT_ConditionsSummaryTool/InDetSCT_ConditionsSummaryTool",
                "Tool to retrieve SCT Conditions summary"
            };

            ToolHandle<IInDetTestPixelLayerTool> m_pixelLayerTool{
                this,
                "PixelLayerTool",
		"InDet::InDetTestPixelLayerTool",
		"Tool to retrieve pixel conditions summary"
	    };

            const AtlasDetectorID *m_atlasId;

            /** eta and phi tolerances **/
            Gaudi::Property<double> m_etaTol{this, "ToleranceEta", 3.0};
            Gaudi::Property<double> m_phiTol{this, "TolerancePhi", 3.0};

            /** Control usage of pixel and SCT info */
            Gaudi::Property<bool> m_usePixel{this, "UsePixel", true};
            Gaudi::Property<bool> m_useSCT{this, "UseSCT", true};

            /** Control check of bad SCT chip (should be false for ITk Strip) */
            Gaudi::Property<bool> m_checkBadSCT{this, "CheckBadSCT", true};

            /** @brief Optional read handle to get status data to test whether a SCT detector element is good.
             * If set to e.g. SCTDetectorElementStatus the event data will be used instead of the SCT conditions summary tool.
             */
            SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_sctDetElStatus
                {this, "SCTDetElStatus", "", "Key of SiDetectorElementStatus for SCT"};

            const SCT_ID*                m_sctID{nullptr};

    };

    inline SG::ReadHandle<InDet::SiDetectorElementStatus> InDetBoundaryCheckTool::getSCTDetElStatus(const EventContext& ctx) const {
       SG::ReadHandle<InDet::SiDetectorElementStatus> sctDetElStatus;
       if (!m_sctDetElStatus.empty()) {
          sctDetElStatus = SG::ReadHandle<InDet::SiDetectorElementStatus>(m_sctDetElStatus, ctx);
          if (!sctDetElStatus.isValid()) {
             std::stringstream msg;
             msg << "Failed to get " << m_sctDetElStatus.key() << " from StoreGate in " << name();
             throw std::runtime_error(msg.str());
          }
       }
       return sctDetElStatus;
    }
}

#endif
