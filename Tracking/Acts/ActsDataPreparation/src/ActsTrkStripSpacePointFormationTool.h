/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_ACTSTRKSTRIPSPACEPOINTFORMATIONTOOL_H
#define ACTSTRK_DATAPREPARATION_ACTSTRKSTRIPSPACEPOINTFORMATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IStripSpacePointFormationTool.h"
#include "StripInformationHelper.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SiSpacePointFormation/SiElementPropertiesTable.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include <string>

class SCT_ID;

namespace ActsTrk {
  
    /// @class ActsTrkStripSpacePointFormationTool
    /// Tool to produce strip space points.
    /// Strip space points are made by combining clusters from pairs of
    /// overlapping detectors. The access to overlapping detector elements is
    /// possible using the ContainerAccessor.
    /// The user can choose just to process the detector element and
    /// its opposite on the stereo layer, or also to consider overlaps with the
    /// four nearest neighbours of the opposite elements.
    ///
    /// Space points are then recorded to storegate as StripSP

    class ActsTrkStripSpacePointFormationTool: public extends<AthAlgTool, ActsTrk::IStripSpacePointFormationTool> {
    public:
        /// @name AthAlgTool methods
        //@{
        ActsTrkStripSpacePointFormationTool(const std::string& type,
                                     const std::string& name,
                                     const IInterface* parent);
        virtual ~ActsTrkStripSpacePointFormationTool() = default;
        virtual StatusCode initialize() override;
        //@}

        /// @name Production of space points
        //@{
      virtual StatusCode produceSpacePoints(const EventContext& ctx,
					    const xAOD::StripClusterContainer& clusterContainer,
					    const InDet::SiElementPropertiesTable& properties,
					    const InDetDD::SiDetectorElementCollection& elements,
					    const Amg::Vector3D& beamSpotVertex,
					    std::vector<StripSP>& spacePoints,
					    std::vector<StripSP>& overlapSpacePoints,
					    bool processOverlaps) const override;
        //@}

    private:

        /// @name Production of space points
        //@{
        StatusCode
	  fillStripSpacePoints(const std::array<const InDetDD::SiDetectorElement*,nNeighbours>& neighbourElements,
			       const std::array<std::vector<std::pair<const xAOD::StripCluster*, size_t>>,nNeighbours>& neighbourClusters,
			       const std::array<double, 14>& overlapExtents,
			       const Amg::Vector3D& beamSpotVertex,
			       std::vector<StripSP>& spacePoints,
			       std::vector<StripSP>& overlapSpacePoints ) const;
	
	StatusCode makeStripSpacePoint(std::vector<StripSP>&,
				       const StripInformationHelper& firstInfo,
				       const StripInformationHelper& secondInfo,
				       bool isEndcap,
				       double limit,
				       double slimit) const;

        void updateRange(const InDetDD::SiDetectorElement* element1,
                         const InDetDD::SiDetectorElement* element2,
                         double& stripLengthGapTolerance, double& min, double& max) const;

        double offset(const InDetDD::SiDetectorElement* element1,
                      const InDetDD::SiDetectorElement* element2,
                      double& stripLengthGapTolerance) const;

        void correctPolarRange(const InDetDD::SiDetectorElement* element,
                               double& min, double& max,
                               size_t& minStrip, size_t& maxStrip) const;

        std::pair<Amg::Vector3D, Amg::Vector3D > getStripEnds(const xAOD::StripCluster* cluster,
                                                              const InDetDD::SiDetectorElement* element,
                                                              size_t& stripIndex) const;
        //@}

        /// @name Id helpers
        //@{
        const SCT_ID* m_stripId{};
        //@}

        /// @name tool handles
        //@{
        /// @brief Using Lorentz angle tool
        ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool{this, "LorentzAngleTool", "SiLorentzAngleTool/SCTLorentzAngleTool", "Tool to retreive Lorentz angle of SCT"};
        //@}

        /// @name Configuration flags
        //@{
        BooleanProperty m_allClusters{this, "AllClusters", false, "Process all clusters without limits."};
        //@}

        /// @name Cut parameters
        /// @brief The following are ranges within which clusters must lie to
        /// make a spacepoint. Opposite and eta neighbours
        /// clusters must lie within range of each other.
        /// Phi clusters must lie in region of each wafer separately.
        //@{
        FloatProperty m_overlapLimitOpposite{this, "OverlapLimitOpposite", 2.8, "Overlap limit for opposite-neighbour."};
        FloatProperty m_overlapLimitPhi{this, "OverlapLimitPhi", 5.64, "Overlap limit for phi-neighbours."};
        FloatProperty m_overlapLimitEtaMin{this, "OverlapLimitEtaMin", 1.68, "Low overlap limit for eta-neighbours."};
        FloatProperty m_overlapLimitEtaMax{this, "OverlapLimitEtaMax", 3.0, "High overlap limit for eta-neighbours."};
        /// @brief The following are parameters to build the space points.
        FloatProperty m_stripLengthTolerance{this, "StripLengthTolerance", 0.01};
        FloatProperty m_stripGapParameter{this, "StripGapParameter", 0.0015, "Recommend 0.001 - 0.0015 for ITK geometry"};
        //@}

  };

}

#endif // ACTSTRKSPACEPOINTFORMATIONTOOL_ACTSTRKSTRIPSPACEPOINTFORMATIONTOOL_H
