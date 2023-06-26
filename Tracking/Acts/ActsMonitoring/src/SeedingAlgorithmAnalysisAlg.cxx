/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SeedingAlgorithmAnalysisAlg.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "SiSPSeededTrackFinderData/SiSpacePointsSeedMakerEventData.h"

ActsTrk::SeedingAlgorithmAnalysisAlg::SeedingAlgorithmAnalysisAlg(const std::string& name,
                                                   ISvcLocator *pSvcLocator)
: AthMonitorAlgorithm(name, pSvcLocator)
{}

StatusCode ActsTrk::SeedingAlgorithmAnalysisAlg::initialize() {
    ATH_MSG_DEBUG( "Initializing SeedingAlgorithmAnalysisAlg" );

    ATH_CHECK(m_seedingTools.retrieve());

    if (m_monitoringGroupNames.size() != m_seedingTools.size()) {
        ATH_MSG_ERROR("Number of monitoring tools not equal to number of seeding tools! Please check!");
        return StatusCode::FAILURE;
    }

    return AthMonitorAlgorithm::initialize();
}

StatusCode ActsTrk::SeedingAlgorithmAnalysisAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG(" In SeedingAlgorithmAnalysisAlg::fillHistograms()" );

    for (unsigned int index(0); index < m_seedingTools.size() ; ++index) {
        const auto& seedingTool = m_seedingTools[index];

        std::array<Monitored::Timer<std::chrono::milliseconds>, TimeMonitoringType::AllTypes> timeMonitors = {
            Monitored::Timer<std::chrono::milliseconds>("TIME_initTimeFirstIter"),
            Monitored::Timer<std::chrono::milliseconds>("TIME_initTimeSecondIter"),
            Monitored::Timer<std::chrono::milliseconds>("TIME_SSS"),
            Monitored::Timer<std::chrono::milliseconds>("TIME_PPP")
        };

        auto eventNumber = Monitored::Scalar<int>("eventNumber", ctx.eventID().event_number());

        InDet::SiSpacePointsSeedMakerEventData seedEventData;

        timeMonitors[TimeMonitoringType::StripSeedInitialisation].start();
        seedingTool->newEvent(ctx, seedEventData, 0);
        timeMonitors[TimeMonitoringType::StripSeedInitialisation].stop();
        auto stripSeedInitialisationTime = Monitored::Scalar<float>("stripSeedInitialisationTime", static_cast<float>(timeMonitors[TimeMonitoringType::StripSeedInitialisation]));

        auto numberStripSpacePoints = Monitored::Scalar<int>("numberStripSpacePoints", seedEventData.nsaz);

        timeMonitors[TimeMonitoringType::StripSeedProduction].start();
        seedingTool->find3Sp(ctx, seedEventData, {});
        timeMonitors[TimeMonitoringType::StripSeedProduction].stop();
        auto stripSeedProductionTime = Monitored::Scalar<float>("stripSeedProductionTime", static_cast<float>(timeMonitors[TimeMonitoringType::StripSeedProduction]));

        auto numberStripSeeds = Monitored::Scalar<int>("numberStripSeeds", 0);
        if (not seedEventData.l_ITkSpacePointForSeed.empty())
            numberStripSeeds = seedEventData.i_ITkSeeds.size();
        else if (not seedEventData.v_ActsSpacePointForSeed.empty())
            numberStripSeeds = seedEventData.nsazv;

        timeMonitors[TimeMonitoringType::PixelSeedInitialisation].start();
        seedingTool->newEvent(ctx, seedEventData, 1);
        timeMonitors[TimeMonitoringType::PixelSeedInitialisation].stop();
        auto pixelSeedInitialisationTime = Monitored::Scalar<float>("pixelSeedInitialisationTime", static_cast<float>(timeMonitors[TimeMonitoringType::PixelSeedInitialisation]));

        auto numberPixelSpacePoints = Monitored::Scalar<int>("numberPixelSpacePoints", seedEventData.nsaz);

        timeMonitors[TimeMonitoringType::PixelSeedProduction].start();
        seedingTool->find3Sp(ctx, seedEventData, {});
        timeMonitors[TimeMonitoringType::PixelSeedProduction].stop();
        auto pixelSeedProductionTime = Monitored::Scalar<float>("pixelSeedProductionTime", static_cast<float>(timeMonitors[TimeMonitoringType::PixelSeedProduction]));

        auto numberPixelSeeds = Monitored::Scalar<int>("numberPixelSeeds", 0);
        if (not seedEventData.l_ITkSpacePointForSeed.empty())
            numberPixelSeeds = seedEventData.i_ITkSeeds.size();
        else if (not seedEventData.v_ActsSpacePointForSeed.empty())
            numberPixelSeeds = seedEventData.nsazv;

        auto monGroup = getGroup(m_monitoringGroupNames[index]);
        fill(monGroup,
             eventNumber,
             stripSeedInitialisationTime,
             stripSeedProductionTime,
             pixelSeedInitialisationTime,
             pixelSeedProductionTime,
             numberPixelSpacePoints,
             numberStripSpacePoints,
             numberPixelSeeds,
             numberStripSeeds);
    }

    return StatusCode::SUCCESS;
}
