/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcMonitoring/ZdcMonitorAlgorithm.h"
#include "ZdcAnalysis/ZDCPulseAnalyzer.h"

ZdcMonitorAlgorithm::ZdcMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
:AthMonitorAlgorithm(name,pSvcLocator){}


ZdcMonitorAlgorithm::~ZdcMonitorAlgorithm() {}


StatusCode ZdcMonitorAlgorithm::initialize() {

    using namespace Monitored;
    ATH_CHECK( m_EventInfoKey.initialize() );
    ATH_CHECK( m_ZdcSumContainerKey.initialize() );
    ATH_CHECK( m_ZdcModuleContainerKey.initialize() );
    ATH_CHECK( m_HIEventShapeContainerKey.initialize() );
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
    ATH_CHECK( m_RPDChannelMaxSampleKey.initialize() );

    
    // m_ZDCSumToolIndices = buildToolMap<int>(m_tools,"ZdcMonitor",2);
    m_ZDCModuleToolIndices = buildToolMap<std::vector<int>>(m_tools,"ZdcModuleMonitor",m_nSides,m_nModules);
    m_RPDChannelToolIndices = buildToolMap<std::vector<int>>(m_tools,"RPDMonitor",m_nSides,m_nChannels);

    //---------------------------------------------------
    // initialize superclass

    return AthMonitorAlgorithm::initialize();
    //---------------------------------------------------
    
}


StatusCode ZdcMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
    auto zdcTool = getGroup("genZdcMonTool"); // get the tool for easier group filling

// ______________________________________________________________________________
    // declaring & obtaining event-level information of interest (lumiblock)
// ______________________________________________________________________________
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx);
    auto lumiBlock = Monitored::Scalar<uint32_t>("lumiBlock", eventInfo->lumiBlock());
    if (! eventInfo.isValid() ) {
        ATH_MSG_WARNING("cannot retrieve event info from evtStore()!");
        return StatusCode::SUCCESS;
    }
    else{
        lumiBlock = eventInfo->lumiBlock();
    }
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
        for (const auto& zdcSum : *zdcSums) { // side 0: C; side 1: A
            

            if (zdcSum->zdcSide() > 0){
                zdcEnergySumA = ZdcSumCalibEnergyHandle(*zdcSum);
                zdcAvgTimeA = ZdcSumAverageTimeHandle(*zdcSum);
                zdcUncalibSumA = ZdcSumUncalibSumHandle(*zdcSum);
            } else{
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
    // SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> rpdChannelAmplitudeHandle(m_RPDChannelAmplitudeKey, ctx);
    // SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> rpdChannelMaxSampleHandle(m_RPDChannelMaxSampleKey, ctx);

    bool zdc_filled = false;

    auto zdcModuleAmp = Monitored::Scalar<float>("zdcModuleAmp", -1000.0);
    auto zdcModuleAmpFract = Monitored::Scalar<float>("zdcModuleAmpFract", -1000.0);
    auto zdcUncalibSumCurrentSide = Monitored::Scalar<float>("zdcUncalibSumCurrentSide", -1000.0);
    auto zdcModuleTime = Monitored::Scalar<float>("zdcModuleTime", -1000.0);
    auto zdcModuleChisq = Monitored::Scalar<float>("zdcModuleChisq", -1000.0);
    auto zdcModuleChisqOverAmp = Monitored::Scalar<float>("zdcModuleChisqOverAmp", -1000.0);
    auto zdcModuleCalibAmp = Monitored::Scalar<float>("zdcModuleCalibAmp", -1000.0);
    auto zdcModuleCalibTime = Monitored::Scalar<float>("zdcModuleCalibTime", -1000.0);

    auto rpdChannelAmplitude = Monitored::Scalar<float>("RPDChannelAmplitude", -1000.0);
    auto rpdChannelMaxSample = Monitored::Scalar<unsigned int>("RPDChannelMaxSample", -1000);

    std::array<float, m_nStatusBits> statusBitsCount;
    for (int bit = 0; bit < m_nStatusBits; bit++) statusBitsCount[bit] = 0;
    
    if (! zdcModules.isValid() ) {
       ATH_MSG_WARNING("evtStore() does not contain Collection with name "<< m_ZdcModuleContainerKey);
       return StatusCode::SUCCESS;
    }
    else{

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
                    // zdcModuleAmpFract = (zdcMod->zdcSide() > 0)? zdcModuleAmp / zdcUncalibSumA : zdcModuleAmp / zdcUncalibSumC;
                    // zdcUncalibSumCurrentSide = (zdcMod->zdcSide() > 0)? 1. * zdcUncalibSumA : 1. * zdcUncalibSumC;
                    zdcModuleAmp = zdcModuleAmplitudeHandle(*zdcMod);
                    zdcModuleTime = zdcModuleTimeHandle(*zdcMod);
                    zdcModuleChisq = zdcModuleChisqHandle(*zdcMod);
                    zdcModuleCalibAmp = zdcModuleCalibEnergyHandle(*zdcMod);
                    zdcModuleCalibTime = zdcModuleCalibTimeHandle(*zdcMod);
                    zdcModuleAmpFract = (zdcMod->zdcSide() > 0)? zdcModuleAmp / zdcUncalibSumA : zdcModuleAmp / zdcUncalibSumC;
                    zdcUncalibSumCurrentSide = (zdcMod->zdcSide() > 0)? 1. * zdcUncalibSumA : 1. * zdcUncalibSumC;
                    zdcModuleChisqOverAmp = zdcModuleChisq / zdcModuleAmp;
                
                    fill(m_tools[m_ZDCModuleToolIndices[iside][imod]], zdcModuleAmp, zdcModuleAmpFract, zdcUncalibSumCurrentSide, zdcModuleTime, zdcModuleChisq, zdcModuleChisqOverAmp, zdcModuleCalibAmp, zdcModuleCalibTime);
                } 
            } else if (zdcMod->zdcType() == 1) {
                // this is the RPD
                int ichannel = zdcMod->zdcChannel() - 1; // zero-based
                // // at runtime, the following 
                // rpdChannelAmplitude = rpdChannelAmplitudeHandle(*zdcMod);
                // rpdChannelMaxSample = rpdChannelMaxSampleHandle(*zdcMod);
                fill(m_tools[m_RPDChannelToolIndices[iside][ichannel]], rpdChannelAmplitude, rpdChannelMaxSample);
            }
        }

    
        if (!zdc_filled){
            ATH_MSG_WARNING("No ZDC modules filled");
            return StatusCode::SUCCESS; 
        }
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
