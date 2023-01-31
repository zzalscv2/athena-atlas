/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "src/OrthogonalSeedingTool.h"

namespace ActsTrk {

  OrthogonalSeedingTool::OrthogonalSeedingTool(const std::string& type,
    const std::string& name,
    const IInterface* parent)
    : base_class(type, name, parent)
  {}

  StatusCode OrthogonalSeedingTool::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG(" *  Used by SeedFinderOrthogonalConfig");
    ATH_MSG_DEBUG("   " << m_minPt);
    ATH_MSG_DEBUG("   " << m_cotThetaMax);
    ATH_MSG_DEBUG("   " << m_deltaRMinTopSP);
    ATH_MSG_DEBUG("   " << m_deltaRMaxTopSP);
    ATH_MSG_DEBUG("   " << m_deltaRMinBottomSP);
    ATH_MSG_DEBUG("   " << m_deltaRMaxBottomSP);

    ATH_MSG_DEBUG("   " << m_impactMax);
    ATH_MSG_DEBUG("   " << m_sigmaScattering);
    ATH_MSG_DEBUG("   " << m_maxPtScattering);
    ATH_MSG_DEBUG("   " << m_maxSeedsPerSpM);

    ATH_MSG_DEBUG("   " << m_collisionRegionMin);
    ATH_MSG_DEBUG("   " << m_collisionRegionMax);
    ATH_MSG_DEBUG("   " << m_phiMin);
    ATH_MSG_DEBUG("   " << m_phiMax);
    ATH_MSG_DEBUG("   " << m_zMin);
    ATH_MSG_DEBUG("   " << m_zMax);
    ATH_MSG_DEBUG("   " << m_rMax);
    ATH_MSG_DEBUG("   " << m_rMin);

    ATH_MSG_DEBUG("   " << m_rMinMiddle);
    ATH_MSG_DEBUG("   " << m_rMaxMiddle);
    ATH_MSG_DEBUG("   " << m_deltaPhiMax);
    ATH_MSG_DEBUG("   " << m_deltaZMax);

    ATH_MSG_DEBUG("   " << m_interactionPointCut);
    ATH_MSG_DEBUG("   " << m_seedConfirmation);
    ATH_MSG_DEBUG("   " << m_skipPreviousTopSP);
    ATH_MSG_DEBUG("   " << m_radLengthPerSeed);

    ATH_MSG_DEBUG(" *  Used by SeedFilterConfig" );
    ATH_MSG_DEBUG("   " << m_deltaInvHelixDiameter);
    ATH_MSG_DEBUG("   " << m_impactWeightFactor);
    ATH_MSG_DEBUG("   " << m_zOriginWeightFactor);
    ATH_MSG_DEBUG("   " << m_compatSeedWeight);
    ATH_MSG_DEBUG("   " << m_deltaRMin);
    ATH_MSG_DEBUG("   " << m_maxSeedsPerSpM);
    ATH_MSG_DEBUG("   " << m_compatSeedLimit);

    ATH_MSG_DEBUG("   " << m_curvatureSortingInFilter);
    ATH_MSG_DEBUG("   " << m_seedWeightIncrement);
    ATH_MSG_DEBUG("   " << m_numSeedIncrement);

    ATH_MSG_DEBUG("   " << m_seedConfirmationInFilter);
    ATH_MSG_DEBUG("   " << m_maxSeedsPerSpMConf);
    ATH_MSG_DEBUG("   " << m_maxQualitySeedsPerSpMConf);
    ATH_MSG_DEBUG("   " << m_useDeltaRorTopRadius);

    ATH_MSG_DEBUG(" *  Used by SeedFilterConfig" );
    ATH_MSG_DEBUG("   " << m_seedConfCentralZMin);
    ATH_MSG_DEBUG("   " << m_seedConfCentralZMax);
    ATH_MSG_DEBUG("   " << m_seedConfCentralRMax);
    ATH_MSG_DEBUG("   " << m_seedConfCentralNTopLargeR);
    ATH_MSG_DEBUG("   " << m_seedConfCentralNTopSmallR);
    ATH_MSG_DEBUG("   " << m_seedConfCentralMinBottomRadius);
    ATH_MSG_DEBUG("   " << m_seedConfCentralMaxZOrigin);
    ATH_MSG_DEBUG("   " << m_seedConfCentralMinImpact);

    ATH_MSG_DEBUG("   " << m_seedConfForwardZMin);
    ATH_MSG_DEBUG("   " << m_seedConfForwardZMax);
    ATH_MSG_DEBUG("   " << m_seedConfForwardRMax);
    ATH_MSG_DEBUG("   " << m_seedConfForwardNTopLargeR);
    ATH_MSG_DEBUG("   " << m_seedConfForwardNTopSmallR);
    ATH_MSG_DEBUG("   " << m_seedConfForwardMinBottomRadius);
    ATH_MSG_DEBUG("   " << m_seedConfForwardMaxZOrigin);
    ATH_MSG_DEBUG("   " << m_seedConfForwardMinImpact);

    return StatusCode::SUCCESS;
  }

  StatusCode
  OrthogonalSeedingTool::createSeeds(const EventContext& /*ctx*/,
				     const std::vector<const ActsTrk::SpacePoint*>& spContainer,
				     const Acts::Vector3& beamSpotPos,
				     const Acts::Vector3& bField,
				     ActsTrk::SeedContainer& seedContainer ) const
  {
    auto finderCfg = prepareConfiguration(Acts::Vector2(beamSpotPos[Amg::x], beamSpotPos[Amg::y]),
					  bField);
    Acts::SeedFinderOrthogonal<value_type> finder(finderCfg);

    // Compute seeds
    auto groupSeeds = finder.createSeeds(spContainer);

    // Store seeds
    seedContainer.reserve(groupSeeds.size());
    for( const auto& seed: groupSeeds) {
      std::unique_ptr<seed_type> to_add = std::make_unique<seed_type>(seed);
      seedContainer.push_back(std::move(to_add));  
    }

    return StatusCode::SUCCESS;
  }

  const Acts::SeedFinderOrthogonalConfig< typename OrthogonalSeedingTool::value_type >
  OrthogonalSeedingTool::prepareConfiguration(const Acts::Vector2& beamPos,
					      const Acts::Vector3& bField) const
  {
    // Configuration for Acts::SeedFilter
    Acts::SeedFilterConfig filterCfg;
    filterCfg.deltaInvHelixDiameter = m_deltaInvHelixDiameter;
    filterCfg.impactWeightFactor = m_impactWeightFactor;
    filterCfg.zOriginWeightFactor = m_zOriginWeightFactor;
    filterCfg.compatSeedWeight = m_compatSeedWeight;    
    filterCfg.deltaRMin = m_deltaRMin;
    filterCfg.maxSeedsPerSpM = m_maxSeedsPerSpM;
    filterCfg.compatSeedLimit = m_compatSeedLimit;
    filterCfg.curvatureSortingInFilter = m_curvatureSortingInFilter;
    filterCfg.seedWeightIncrement = m_seedWeightIncrement;
    filterCfg.numSeedIncrement = m_numSeedIncrement;
    filterCfg.seedConfirmation = m_seedConfirmationInFilter;
    filterCfg.maxSeedsPerSpMConf = m_maxSeedsPerSpMConf;
    filterCfg.maxQualitySeedsPerSpMConf = m_maxQualitySeedsPerSpMConf;
    filterCfg.useDeltaRorTopRadius = m_useDeltaRorTopRadius;
    filterCfg.centralSeedConfirmationRange.zMinSeedConf = m_seedConfCentralZMin;
    filterCfg.centralSeedConfirmationRange.zMaxSeedConf = m_seedConfCentralZMax;
    filterCfg.centralSeedConfirmationRange.rMaxSeedConf = m_seedConfCentralRMax;
    filterCfg.centralSeedConfirmationRange.nTopForLargeR = m_seedConfCentralNTopLargeR;
    filterCfg.centralSeedConfirmationRange.nTopForSmallR = m_seedConfCentralNTopSmallR;
    filterCfg.centralSeedConfirmationRange.seedConfMinBottomRadius = m_seedConfCentralMinBottomRadius;
    filterCfg.centralSeedConfirmationRange.seedConfMaxZOrigin = m_seedConfCentralMaxZOrigin;
    filterCfg.centralSeedConfirmationRange.minImpactSeedConf = m_seedConfCentralMinImpact;
    filterCfg.forwardSeedConfirmationRange.zMinSeedConf = m_seedConfForwardZMin;
    filterCfg.forwardSeedConfirmationRange.zMaxSeedConf = m_seedConfForwardZMax;
    filterCfg.forwardSeedConfirmationRange.rMaxSeedConf = m_seedConfForwardRMax;
    filterCfg.forwardSeedConfirmationRange.nTopForLargeR = m_seedConfForwardNTopLargeR;
    filterCfg.forwardSeedConfirmationRange.nTopForSmallR = m_seedConfForwardNTopSmallR;
    filterCfg.forwardSeedConfirmationRange.seedConfMinBottomRadius = m_seedConfForwardMinBottomRadius;
    filterCfg.forwardSeedConfirmationRange.seedConfMaxZOrigin = m_seedConfForwardMaxZOrigin;
    filterCfg.forwardSeedConfirmationRange.minImpactSeedConf = m_seedConfForwardMinImpact;

    // Configuration Acts::SeedFinderOrthogonal
    Acts::SeedFinderOrthogonalConfig<value_type> finderCfg;
    finderCfg.seedFilter =  std::make_shared<Acts::SeedFilter<value_type>>(filterCfg); 
    finderCfg.minPt = m_minPt;
    finderCfg.cotThetaMax = m_cotThetaMax;
    finderCfg.deltaRMinTopSP = m_deltaRMinTopSP;
    finderCfg.deltaRMaxTopSP = m_deltaRMaxTopSP;
    finderCfg.deltaRMinBottomSP = m_deltaRMinBottomSP;
    finderCfg.deltaRMaxBottomSP = m_deltaRMaxBottomSP;
    finderCfg.impactMax = m_impactMax;
    finderCfg.sigmaScattering = m_sigmaScattering;
    finderCfg.maxPtScattering = m_maxPtScattering;
    finderCfg.maxSeedsPerSpM = m_maxSeedsPerSpM;
    finderCfg.collisionRegionMin = m_collisionRegionMin;
    finderCfg.collisionRegionMax = m_collisionRegionMax;
    finderCfg.phiMin = m_phiMin;
    finderCfg.phiMax = m_phiMax;
    finderCfg.zMin = m_zMin;
    finderCfg.zMax = m_zMax;
    finderCfg.rMax = m_rMax;
    finderCfg.rMin = m_rMin;
    finderCfg.rMinMiddle = m_rMinMiddle;
    finderCfg.rMaxMiddle = m_rMaxMiddle;
    finderCfg.deltaPhiMax = m_deltaPhiMax;
    finderCfg.bFieldInZ = bField[2];
    finderCfg.beamPos = beamPos;
    finderCfg.deltaZMax = m_deltaZMax;
    finderCfg.interactionPointCut = m_interactionPointCut;
    finderCfg.seedConfirmation = m_seedConfirmation;
    finderCfg.centralSeedConfirmationRange = filterCfg.centralSeedConfirmationRange;
    finderCfg.forwardSeedConfirmationRange = filterCfg.forwardSeedConfirmationRange;
    finderCfg.skipPreviousTopSP = m_skipPreviousTopSP;
    finderCfg.radLengthPerSeed = m_radLengthPerSeed;

    finderCfg.highland = 13.6 * std::sqrt(finderCfg.radLengthPerSeed) *
      (1 + 0.038 * std::log(finderCfg.radLengthPerSeed));
    float maxScatteringAngle = finderCfg.highland / finderCfg.minPt;
    finderCfg.maxScatteringAngle2 = maxScatteringAngle * maxScatteringAngle;
    finderCfg.pTPerHelixRadius = 300. * finderCfg.bFieldInZ;
    finderCfg.minHelixDiameter2 = std::pow(finderCfg.minPt * 2 /
					   finderCfg.pTPerHelixRadius,
					   2);
    finderCfg.pT2perRadius = std::pow(finderCfg.highland / finderCfg.pTPerHelixRadius, 2);
    return finderCfg;
  }

} // namespace ActsTrk
