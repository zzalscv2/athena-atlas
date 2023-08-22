/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <sstream>
#include <memory>

#include "ZdcAnalysis/ZdcLEDAnalysisTool.h"
#include "xAODEventInfo/EventInfo.h"
#include <xAODForward/ZdcModuleAuxContainer.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace ZDC
{
  ZdcLEDAnalysisTool::ZdcLEDAnalysisTool(const std::string& name) :
    asg::AsgTool(name),
    m_name(name)
{
#ifndef XAOD_STANDALONE
  declareInterface<IZdcAnalysisTool>(this);
#endif

}

ZdcLEDAnalysisTool::~ZdcLEDAnalysisTool()
{
  ATH_MSG_DEBUG("Deleting ZdcLEDAnalysisTool named " << m_name);
}

StatusCode ZdcLEDAnalysisTool::initialize()
{
  // Use configuration to direct initialization. 
  //
  if (m_configuration == "ppPbPb2023") {
    initialize_ppPbPb2023();
  }
  else {
    ATH_MSG_ERROR("Unknown configuration: "  << m_configuration);
    return StatusCode::FAILURE;
  }

  // Check for valid configuration
  //
  if (m_daqMode >= numDAQModes) {
    ATH_MSG_ERROR("Invalid DAQ mode, mode = " <<  m_daqMode);
    return StatusCode::FAILURE;
  }

  if (m_LEDBCID.size() != NumLEDs ||
      (m_daqMode != Standalone && m_LEDCalreqIdx.size() != NumLEDs)) {
    ATH_MSG_ERROR("Invalid initialization of tool for configuration " <<  m_configuration);
    return StatusCode::FAILURE;
  }
  
  ATH_MSG_INFO("doZDC: " << (m_doZDC ? "true" : "false"));
  ATH_MSG_INFO("doRPD: " << (m_doRPD ? "true" : "false"));
  ATH_MSG_INFO("Configuration: " << m_configuration);
  ATH_MSG_DEBUG("AuxSuffix: " << m_auxSuffix);
  ATH_MSG_DEBUG("NumSamples: " << m_numSamples);
  ATH_MSG_DEBUG("Presample: " << m_preSample);
  ATH_MSG_DEBUG("DeltaTSample: " << m_deltaTSample);

  std::ostringstream BCIDList;
  BCIDList << "LED BCIDs:";

  for (unsigned int idxLED = 0; idxLED < NumLEDs; idxLED++) {
    BCIDList << m_LEDNames[idxLED] << " - BCID " << m_LEDBCID[idxLED];
    if (idxLED < NumLEDs - 1) BCIDList << ", ";
  }

  ATH_MSG_DEBUG(BCIDList.str());

  
  // If an aux suffix is provided, prepend it with "_" so we don't have to do so at each use
  //
  if (m_auxSuffix != "") m_auxSuffix = "_" + m_auxSuffix;

  // initialize eventInfo access
  //
  ATH_CHECK( m_eventInfoKey.initialize());
  
  if (m_writeAux && m_auxSuffix != "") {
    ATH_MSG_DEBUG("suffix string = " << m_auxSuffix);
  }

  // Initialize writeDecor handles
  //
    
  m_ZdcLEDPresampleADC = m_zdcModuleContainerName + ".Presample" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDPresampleADC.initialize());

  m_ZdcLEDADCSum = m_zdcModuleContainerName + ".ADCSum" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDADCSum.initialize());

  m_ZdcLEDMaxADC = m_zdcModuleContainerName + ".MaxADC" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDMaxADC.initialize());
  
  m_ZdcLEDMaxSample = m_zdcModuleContainerName + ".MaxSample" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDMaxSample.initialize());
  
  m_ZdcLEDAvgTime = m_zdcModuleContainerName + ".AvgTime" + m_auxSuffix;
  ATH_CHECK( m_ZdcLEDAvgTime.initialize());
      
  m_init = true;
  return StatusCode::SUCCESS;
}

void ZdcLEDAnalysisTool::initialize_ppPbPb2023()
{
  // Use the defaults for now except for sampleAnaStart values and the BCIDs
  //
  m_sampleAnaStartZDC = 5;
  m_sampleAnaStartRPD = 5;

  m_LEDBCID = {1, 1108, 1110};
  m_LEDCalreqIdx = {1, 2, 3};
}
  
StatusCode ZdcLEDAnalysisTool::recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer&)
{
  if (!m_init) {
    ATH_MSG_WARNING("Tool not initialized!");
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
  if (!eventInfo.isValid()) return StatusCode::FAILURE;

  //
  // only do something on calibration events
  //
  if (m_daqMode == CombinedPhysics && !eventInfo->eventType(xAOD::EventInfo::IS_CALIBRATION)) return StatusCode::SUCCESS;
  
  if (moduleContainer.size()==0) return StatusCode::SUCCESS; // if no modules, do nothing

  // Determine the LED type
  //
  unsigned int bcid = eventInfo->bcid();
  unsigned int evtLEDType = LEDNone;
  //
  // We can only use BCID to identify LED type
  //
  for (unsigned int idxLED = 0; idxLED < NumLEDs; idxLED++) {
    //
    //  Does the BCID match one of those associated with the LEDs?
    //
    if (m_LEDBCID[idxLED] == bcid) {
      if (m_daqMode != Standalone) {
	//
	// Also check the calreq trigger (to be implemented)
	//
	if (false) continue;
      }
      
      evtLEDType = idxLED;
      break;
    }
  }

  if (evtLEDType == LEDNone) {
    //
    // Thie BCID does not appear to be associated with one of the LEDs, print warning and quit processing
    //
    ATH_MSG_WARNING("Unexpected BCID found in data: bcid = " << bcid  << m_configuration);
    return StatusCode::SUCCESS;
  }

  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> moduleLEDPresampleADCHandle(m_ZdcLEDPresampleADC);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> moduleLEDADCSumHandle(m_ZdcLEDADCSum);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> moduleLEDMaxADCHandle(m_ZdcLEDMaxADC);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> moduleLEDMaxSampleHandle(m_ZdcLEDMaxSample);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,float> moduleLEDAvgTimeHandle(m_ZdcLEDAvgTime);

  ATH_MSG_DEBUG("Starting event processing for LED " << m_LEDNames[evtLEDType]);

  for (const auto zdcModule : moduleContainer)
  {
    ZDCLEDModuleResults results;
    if (zdcModule->zdcType() == 0) {
      results = processZDCModule(*zdcModule);
    }
    else if (zdcModule->zdcType() == 1 && zdcModule->zdcModule() == 4) {
      results = processRPDModule(*zdcModule);
    }

    ATH_MSG_DEBUG("Writing aux decors to module with side, module, channel = " << zdcModule->zdcSide() << ", " << zdcModule->zdcModule()
		<< ", " << zdcModule->zdcChannel());

    moduleLEDPresampleADCHandle(*zdcModule) = results.getPresampleADC();
    moduleLEDADCSumHandle(*zdcModule) = results.getADCSum();
    moduleLEDMaxADCHandle(*zdcModule) = results.getMaxADC();
    moduleLEDMaxSampleHandle(*zdcModule) = results.getMaxSample();
    moduleLEDAvgTimeHandle(*zdcModule) = results.getAvgTime();
  } 

  ATH_MSG_DEBUG("Finishing event processing");


  return StatusCode::SUCCESS;
}

StatusCode ZdcLEDAnalysisTool::reprocessZdc()
{
  if (!m_init) {
    ATH_MSG_WARNING("Tool not initialized!");
    return StatusCode::FAILURE;
  }
  
  ATH_MSG_DEBUG ("Trying to retrieve " << m_zdcModuleContainerName);
  
  m_zdcModules = 0;
  ATH_CHECK(evtStore()->retrieve(m_zdcModules, m_zdcModuleContainerName));
  
  
  ATH_CHECK(recoZdcModules(*m_zdcModules, *m_zdcSums));
  
  return StatusCode::SUCCESS;
}

ZDCLEDModuleResults ZdcLEDAnalysisTool::processZDCModule(const xAOD::ZdcModule& module)
{
  ATH_MSG_DEBUG("Processing ZDC side, channel = " << module.zdcSide() << ", " << module.zdcModule());
  bool doLG = false;
  
  std::vector<uint16_t> HGSamples = module.auxdata<std::vector<uint16_t> >("g1data");
  std::vector<uint16_t> LGSamples = module.auxdata<std::vector<uint16_t> >("g0data");

  std::vector<uint16_t>::const_iterator maxIter = std::max_element(HGSamples.begin(), HGSamples.end());
  if (maxIter != HGSamples.end()) {
    if (*maxIter > m_HGADCOverflow) doLG = true;
  }

  if (doLG) {
    return processModuleData(LGSamples, m_sampleAnaStartZDC, m_sampleAnaEndZDC, m_ZdcLowGainScale);
  }
  else {
    return processModuleData(HGSamples, m_sampleAnaStartZDC, m_sampleAnaEndZDC, 1);
  }
}

ZDCLEDModuleResults ZdcLEDAnalysisTool::processRPDModule(const xAOD::ZdcModule& module)
{
  ATH_MSG_DEBUG("Processing RPD side, channel = " << module.zdcSide() << ", " << module.zdcChannel());
  return processModuleData(module.auxdata<std::vector<uint16_t> >("g0data"), m_sampleAnaStartRPD, m_sampleAnaEndRPD, 1);
}

ZDCLEDModuleResults ZdcLEDAnalysisTool::processModuleData(const std::vector<unsigned short>& data,
							  unsigned int startSample, unsigned int endSample, float gainScale)
{

  unsigned int ADCSum = 0;
  int maxADCsub = 0;
  unsigned int maxSample = 0;
  float avgTime = 0;

  if (startSample > m_numSamples || endSample > m_numSamples) {
    ATH_MSG_ERROR("Start or end sample number greater than number of samples");
    return ZDCLEDModuleResults();
  }

  int preFADC = data[m_preSample];
  
  for (unsigned int sample = startSample; sample <= endSample; sample++) {
    int FADCsub = data[sample] - preFADC;
    float time = (sample + 0.5)*m_deltaTSample;
    ADCSum += FADCsub;
    if (FADCsub > maxADCsub) {
      maxADCsub = FADCsub;
      maxSample = sample;
    }

    avgTime += time*FADCsub;
  }

  avgTime /= ADCSum;

  return ZDCLEDModuleResults(preFADC, ADCSum*gainScale, maxADCsub*gainScale, maxSample, avgTime);
}
 

} // namespace ZDC

