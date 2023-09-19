/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcMonitoring/ZdcMonitorAlgorithm.h"
#include "ZdcAnalysis/ZDCPulseAnalyzer.h"

ZdcMonitorAlgorithm::ZdcMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
:AthMonitorAlgorithm(name,pSvcLocator){
    ATH_MSG_DEBUG("calling the constructor of ZdcMonitorAlgorithm");
}


ZdcMonitorAlgorithm::~ZdcMonitorAlgorithm() {}


StatusCode ZdcMonitorAlgorithm::initialize() {

    ATH_MSG_DEBUG("initializing for the monitoring algorithm");

    using namespace Monitored;
    ATH_CHECK( m_ZdcSumContainerKey.initialize() );
    ATH_CHECK( m_ZdcModuleContainerKey.initialize() );
    ATH_CHECK( m_HIEventShapeContainerKey.initialize() );
    
    ATH_CHECK( m_eventTypeKey.initialize() );
    // ATH_CHECK( m_ZdcBCIDKey.initialize() );
    ATH_CHECK( m_DAQModeKey.initialize() );

    ATH_CHECK( m_ZdcSumCalibEnergyKey.initialize() );
    ATH_CHECK( m_ZdcSumAverageTimeKey.initialize() );
    ATH_CHECK( m_ZdcSumUncalibSumKey.initialize() );

    ATH_CHECK( m_ZdcModuleStatusKey.initialize() );
    ATH_CHECK( m_ZdcModuleAmplitudeKey.initialize() );
    ATH_CHECK( m_ZdcModuleTimeKey.initialize() );
    ATH_CHECK( m_ZdcModuleChisqKey.initialize() );
    ATH_CHECK( m_ZdcModuleCalibEnergyKey.initialize() );
    ATH_CHECK( m_ZdcModuleCalibTimeKey.initialize() );

    ATH_CHECK( m_RPDChannelAmplitudeKey.initialize() );
    ATH_CHECK( m_RPDChannelAmplitudeCalibKey.initialize() );
    ATH_CHECK( m_RPDChannelStatusKey.initialize() );

    ATH_CHECK( m_LEDTypeKey.initialize() );
    ATH_CHECK( m_LEDPresampleADCKey.initialize() );
    ATH_CHECK( m_LEDADCSumKey.initialize() );
    ATH_CHECK( m_LEDMaxADCKey.initialize() );
    ATH_CHECK( m_LEDMaxSampleKey.initialize() );
    ATH_CHECK( m_LEDAvgTimeKey.initialize() );
    

    
    m_ZDCModuleToolIndices = buildToolMap<std::vector<int>>(m_tools,"ZdcModuleMonitor",m_nSides,m_nModules);
    m_RPDChannelToolIndices = buildToolMap<std::vector<int>>(m_tools,"RPDChannelMonitor",m_nSides,m_nChannels);
    m_ZDCModuleLEDToolIndices = buildToolMap<std::vector<std::vector<int>>>(m_tools,"ZdcModLEDMonitor",m_LEDNames.size(),m_nSides,m_nModules);
    m_RPDChannelLEDToolIndices = buildToolMap<std::vector<std::vector<int>>>(m_tools,"RPDChanLEDMonitor",m_LEDNames.size(),m_nSides,m_nChannels);

    //---------------------------------------------------
    // initialize superclass

    return AthMonitorAlgorithm::initialize();
    //---------------------------------------------------
    
}


StatusCode ZdcMonitorAlgorithm::fillLEDHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("filling LED histograms");
// ______________________________________________________________________________
    // declaring & obtaining event-level information of interest 
// ______________________________________________________________________________
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx);
    // already checked in fillHistograms that eventInfo is valid
    auto lumiBlock = Monitored::Scalar<uint32_t>("lumiBlock", eventInfo->lumiBlock());
    
// ______________________________________________________________________________
    // declaring & obtaining LED variables of interest for the ZDC modules & RPD channels
    // filling arrays of monitoring tools (module/channel-level)
// ______________________________________________________________________________

    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModules(m_ZdcModuleContainerKey, ctx);

    auto zdcLEDADCSum = Monitored::Scalar<unsigned int>("zdcLEDADCSum",-1000);
    auto zdcLEDMaxADC = Monitored::Scalar<unsigned int>("zdcLEDMaxADC",-1000);
    auto rpdLEDADCSum = Monitored::Scalar<unsigned int>("rpdLEDADCSum",-1000);
    auto rpdLEDMaxADC = Monitored::Scalar<unsigned int>("rpdLEDMaxADC",-1000);

    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, int> LEDADCSumHandle(m_LEDADCSumKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, int> LEDMaxADCHandle(m_LEDMaxADCKey, ctx);


    if (! zdcModules.isValid() ) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcModuleContainerKey);
       return StatusCode::SUCCESS;
    }

    unsigned int iLEDType = 1000;
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> zdcLEDTypeHandle(m_LEDTypeKey, ctx);
    if (!zdcLEDTypeHandle.isAvailable()){
        ATH_MSG_WARNING("CANNOT find the variable " << m_LEDTypeKey << "!");
        return StatusCode::SUCCESS;
    } 

    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_ZdcSumContainerKey, ctx); // already checked in fillHistograms that zdcSums is valid

    for (const auto& zdcSum : *zdcSums) { 
        if (zdcSum->zdcSide() == 0){
            iLEDType = zdcLEDTypeHandle(*zdcSum);
        }
    }
    
    if (iLEDType == 1000){
        ATH_MSG_WARNING("The LED type is unretrieved!");
        return StatusCode::SUCCESS;
    } 
    if (iLEDType >= m_LEDNames.size()){
        ATH_MSG_WARNING("The retrieved LED type is incorrect (larger than 2)!");
        return StatusCode::SUCCESS;
    } 

    for (const auto zdcMod : *zdcModules){
        int iside = (zdcMod->zdcSide() > 0)? 1 : 0;
    
        if (zdcMod->zdcType() == 0){ // zdc
            int imod = zdcMod->zdcModule();
            zdcLEDADCSum = LEDADCSumHandle(*zdcMod);
            zdcLEDMaxADC = LEDMaxADCHandle(*zdcMod);

            fill(m_tools[m_ZDCModuleLEDToolIndices[iLEDType][iside][imod]], lumiBlock, zdcLEDADCSum, zdcLEDMaxADC);
        } 
        else if (zdcMod->zdcType() == 1) { // rpd
            int ichannel = zdcMod->zdcChannel();
            if (ichannel >= m_nChannels){
                ATH_MSG_WARNING("The current channel number exceeds the zero-based limit (15): it is " << ichannel);
                continue;
            }
            rpdLEDADCSum = LEDADCSumHandle(*zdcMod);
            rpdLEDMaxADC = LEDMaxADCHandle(*zdcMod);

            fill(m_tools[m_RPDChannelLEDToolIndices[iLEDType][iside][ichannel]], lumiBlock, rpdLEDADCSum, rpdLEDMaxADC);
        }
    }
    
    return StatusCode::SUCCESS;
}


StatusCode ZdcMonitorAlgorithm::fillPhysicsDataHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("filling physics histograms");
  
    auto zdcTool = getGroup("genZdcMonTool"); // get the tool for easier group filling

// ______________________________________________________________________________
    // declaring & obtaining event-level information of interest 
// ______________________________________________________________________________
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx);
    // already checked in fillHistograms that eventInfo is valid
    auto lumiBlock = Monitored::Scalar<uint32_t>("lumiBlock", eventInfo->lumiBlock());
    
// ______________________________________________________________________________
    // declaring & obtaining variables of interest for the ZDC sums
// ______________________________________________________________________________
    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_ZdcSumContainerKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> ZdcSumCalibEnergyHandle(m_ZdcSumCalibEnergyKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> ZdcSumAverageTimeHandle(m_ZdcSumAverageTimeKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> ZdcSumUncalibSumHandle(m_ZdcSumUncalibSumKey, ctx);
    
    auto zdcEnergySumA = Monitored::Scalar<float>("zdcEnergySumA",-1000.0);
    auto zdcEnergySumC = Monitored::Scalar<float>("zdcEnergySumC",-1000.0);
    auto zdcAvgTimeA = Monitored::Scalar<float>("zdcAvgTimeA",-1000.0);
    auto zdcAvgTimeC = Monitored::Scalar<float>("zdcAvgTimeC",-1000.0);
    auto zdcUncalibSumA = Monitored::Scalar<float>("zdcUncalibSumA",-1000.0);
    auto zdcUncalibSumC = Monitored::Scalar<float>("zdcUncalibSumC",-1000.0);

    if (! zdcSums.isValid() ) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcSumContainerKey);
       return StatusCode::SUCCESS;
    }
    else{
        for (const auto& zdcSum : *zdcSums) { // side -1: C; side 1: A
            
            // skipping the side == 0 global sum
            if (zdcSum->zdcSide() == 1){
                zdcEnergySumA = ZdcSumCalibEnergyHandle(*zdcSum);
                zdcAvgTimeA = ZdcSumAverageTimeHandle(*zdcSum);
                zdcUncalibSumA = ZdcSumUncalibSumHandle(*zdcSum);
            } else if (zdcSum->zdcSide() == -1){
                zdcEnergySumC = ZdcSumCalibEnergyHandle(*zdcSum);
                zdcAvgTimeC = ZdcSumAverageTimeHandle(*zdcSum);
                zdcUncalibSumC = ZdcSumUncalibSumHandle(*zdcSum);
            }
        } // having filled both sides
        
    }

// ______________________________________________________________________________
    // declaring & obtaining variables of interest for the ZDC modules & RPD channels
    // filling arrays of monitoring tools (module/channel-level)
    // updating status bits
// ______________________________________________________________________________

    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModules(m_ZdcModuleContainerKey, ctx);

    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> zdcModuleStatusHandle(m_ZdcModuleStatusKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleAmplitudeHandle(m_ZdcModuleAmplitudeKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleTimeHandle(m_ZdcModuleTimeKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleChisqHandle(m_ZdcModuleChisqKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleCalibEnergyHandle(m_ZdcModuleCalibEnergyKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleCalibTimeHandle(m_ZdcModuleCalibTimeKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDChannelAmplitudeHandle(m_RPDChannelAmplitudeKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> RPDChannelAmplitudeCalibHandle(m_RPDChannelAmplitudeCalibKey, ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> RPDChannelStatusHandle(m_RPDChannelStatusKey, ctx);

    bool zdc_filled = false;

    auto zdcModuleAmp = Monitored::Scalar<float>("zdcModuleAmp", -1000.0);
    auto zdcModuleFract = Monitored::Scalar<float>("zdcModuleFract", -1000.0);
    auto zdcEnergySumCurrentSide = Monitored::Scalar<float>("zdcEnergySumCurrentSide", -1000.0);
    auto zdcModuleTime = Monitored::Scalar<float>("zdcModuleTime", -1000.0);
    auto zdcModuleChisq = Monitored::Scalar<float>("zdcModuleChisq", -1000.0);
    auto zdcModuleChisqOverAmp = Monitored::Scalar<float>("zdcModuleChisqOverAmp", -1000.0);
    auto zdcModuleCalibAmp = Monitored::Scalar<float>("zdcModuleCalibAmp", -1000.0);
    auto zdcModuleCalibTime = Monitored::Scalar<float>("zdcModuleCalibTime", -1000.0);

    auto RPDChannelAmplitude = Monitored::Scalar<float>("RPDChannelAmplitude", -1000.0);
    auto RPDChannelAmplitudeCalib = Monitored::Scalar<float>("RPDChannelAmplitudeCalib", -1000.0);
    auto RPDChannelStatus = Monitored::Scalar<unsigned int>("RPDChannelStatus", -1000);

    std::array<float, m_nStatusBits> statusBitsCount;
    for (int bit = 0; bit < m_nStatusBits; bit++) statusBitsCount[bit] = 0;
    
    if (! zdcModules.isValid() ) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcModuleContainerKey);
       return StatusCode::SUCCESS;
    }

    for (const auto zdcMod : *zdcModules){
        int iside = (zdcMod->zdcSide() > 0)? 1 : 0;
    
        if (zdcMod->zdcType() == 0){
            // zdc
            zdc_filled = true;
            int imod = zdcMod->zdcModule();
            int status = zdcModuleStatusHandle(*zdcMod);
    
            for (int bit = 0; bit < m_nStatusBits; bit++){
                if (status & 1 << bit){
                    statusBitsCount[bit] += 1;
                }
            }

            if ((status & 1 << ZDCPulseAnalyzer::PulseBit) != 0){
                zdcModuleAmp = zdcModuleAmplitudeHandle(*zdcMod);
                zdcModuleTime = zdcModuleTimeHandle(*zdcMod);
                zdcModuleChisq = zdcModuleChisqHandle(*zdcMod);
                zdcModuleCalibAmp = zdcModuleCalibEnergyHandle(*zdcMod);
                zdcModuleCalibTime = zdcModuleCalibTimeHandle(*zdcMod);
                zdcEnergySumCurrentSide = (zdcMod->zdcSide() > 0)? 1. * zdcUncalibSumA : 1. * zdcUncalibSumC;
                zdcModuleFract = (zdcEnergySumCurrentSide == 0)? -1000. : zdcModuleAmp / zdcEnergySumCurrentSide;
                zdcModuleChisqOverAmp = zdcModuleChisq / zdcModuleAmp;
            
                fill(m_tools[m_ZDCModuleToolIndices[iside][imod]], zdcModuleAmp, zdcModuleFract, zdcEnergySumCurrentSide, zdcModuleTime, zdcModuleChisq, zdcModuleChisqOverAmp, zdcModuleCalibAmp, zdcModuleCalibTime);
            } 
        } else if (zdcMod->zdcType() == 1) {
            // this is the RPD

            // int ichannel = zdcMod->zdcChannel() - 1; // zero-based
            // // at runtime, the following 
            int status = RPDChannelStatusHandle(*zdcMod);
            if ((status & 1 << ZDCPulseAnalyzer::PulseBit) != 0){
                RPDChannelAmplitude = RPDChannelAmplitudeHandle(*zdcMod);
                RPDChannelAmplitudeCalib = RPDChannelAmplitudeCalibHandle(*zdcMod);
            // fill(m_tools[m_RPDChannelToolIndices[iside][ichannel]], rpdChannelAmplitude, rpdChannelMaxSample);
            }
        }
    }

    
    if (!zdc_filled){
        ATH_MSG_WARNING("No ZDC modules filled");
        return StatusCode::SUCCESS; 
    }
    
    auto statusBits = Monitored::Collection("statusBits", statusBitsCount);

// ______________________________________________________________________________
    // obtaining fCalEt on A,C side
// ______________________________________________________________________________

    SG::ReadHandle<xAOD::HIEventShapeContainer> eventShapes(m_HIEventShapeContainerKey, ctx);
    auto fcalEtA = Monitored::Scalar<float>("fcalEtA", -1000.0);
    auto fcalEtC = Monitored::Scalar<float>("fcalEtC", -1000.0);
    if (! eventShapes.isValid()) {
        // we often don't expect calorimeter info to be on (e.g, when using the ZDC calibration stream)
        // only print out warning if we expect the calorimeter info to be on
        // not returning since the ZDC side-sum & module info is more important and yet need to be filled
        if (m_CalInfoOn) ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_HIEventShapeContainerKey);
    }
    else{
        ATH_MSG_DEBUG("Able to retrieve container "<< m_HIEventShapeContainerKey);
        ATH_MSG_DEBUG("Used to obtain fCalEtA, fCalEtC");

        for (const auto eventShape : *eventShapes){
            int layer = eventShape->layer();
            float eta = eventShape->etaMin();
            float et = eventShape->et();
            if (layer == 21 || layer == 22 || layer == 23){
                if (eta > 0) fcalEtA += et;
                if (eta < 0) fcalEtC += et;
            }
        }
    }

// ______________________________________________________________________________
    // filling generic ZDC tool (within the same Monitored::Group, so that any possible correlation is recognized)
// ______________________________________________________________________________

    if (m_CalInfoOn){ // calorimeter information is turned on
        fill(zdcTool, lumiBlock, zdcEnergySumA, zdcEnergySumC, zdcUncalibSumA, zdcUncalibSumC, statusBits, fcalEtA, fcalEtC);
    } else{
        fill(zdcTool, lumiBlock, zdcEnergySumA, zdcEnergySumC, zdcUncalibSumA, zdcUncalibSumC, statusBits);
    }

    return StatusCode::SUCCESS;
}


StatusCode ZdcMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("calling the fillHistograms function");

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx);
    if (! eventInfo.isValid() ) {
        ATH_MSG_WARNING("cannot retrieve event info from evtStore()!");
        return StatusCode::SUCCESS;
    }
    
    unsigned int eventType = ZdcEventInfo::ZdcEventUnknown;
    unsigned int DAQMode = ZdcEventInfo::DAQModeUndef;

    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> eventTypeHandle(m_eventTypeKey,ctx);
    SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> DAQModeHandle(m_DAQModeKey,ctx);

    SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums(m_ZdcSumContainerKey, ctx);

    if (! zdcSums.isValid() ) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcSumContainerKey);
       return StatusCode::SUCCESS;
    }
    for (const auto& zdcSum : *zdcSums) { 
        if (zdcSum->zdcSide() == 0){
            if (!eventTypeHandle.isAvailable()){
                ATH_MSG_WARNING("The global sum entry in zdc sum container can be retrieved; but it does NOT have the variable eventType written as a decoration!");
                return StatusCode::SUCCESS;
            } 

            if (!DAQModeHandle.isAvailable()){
                ATH_MSG_WARNING("The global sum entry in zdc sum container can be retrieved; but it does NOT have the variable DAQMode written as a decoration!");
                return StatusCode::SUCCESS;
            }

            eventType = eventTypeHandle(*zdcSum);
            DAQMode = DAQModeHandle(*zdcSum);
        }
    }

    ATH_MSG_DEBUG("extracted eventType = " << eventType << " DAQMode = " << DAQMode);

    if (eventType == ZdcEventInfo::ZdcEventUnknown || DAQMode == ZdcEventInfo::DAQModeUndef){
        ATH_MSG_WARNING("The zdc sum container can be retrieved from the evtStore() but");
        ATH_MSG_WARNING("Either the event type or the DAQ mode is invalid");
        ATH_MSG_WARNING("Most likely, there is no global sum (side == 0) entry in the zdc sum container");
        return StatusCode::SUCCESS;
    }

    if (eventType == ZdcEventInfo::ZdcEventPhysics || eventType == ZdcEventInfo::ZdcSimulation){
        return fillPhysicsDataHistograms(ctx);
    }
    if (eventType == ZdcEventInfo::ZdcEventLED){
        return fillLEDHistograms(ctx);
    }
    
    ATH_MSG_WARNING("Event type is invalid"); // case where m_eventType >= ZdcEventInfo::numEventTypes
    return StatusCode::SUCCESS;
}

