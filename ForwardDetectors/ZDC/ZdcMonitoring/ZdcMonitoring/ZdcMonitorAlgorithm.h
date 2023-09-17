/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCMONITORALGORITHM_H
#define ZDCMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "TRandom3.h"
#include "array"

//---------------------------------------------------
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODHIEvent/HIEventShapeContainer.h"
//---------------------------------------------------
#include "ZdcUtils/ZdcEventInfo.h"

class ZdcMonitorAlgorithm : public AthMonitorAlgorithm {
public:
    ZdcMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~ZdcMonitorAlgorithm();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
    StatusCode fillLEDHistograms( const EventContext& ctx ) const;
    StatusCode fillPhysicsDataHistograms( const EventContext& ctx ) const;

private:
    // see the standalone version of the Gaudi::Property class (a wrapper in AsgTools) at
    // athena/Control/AthToolSupport/AsgTools/AsgTools/PropertyWrapper.h
    // input to constructor: owner, name, value, title = "" (by default)
    Gaudi::Property<bool> m_isLED {this,"isLED",false};
    Gaudi::Property<bool> m_CalInfoOn {this,"CalInfoOn",false};

    Gaudi::Property<std::string> m_zdcModuleContainerName {this, "ZdcModuleContainerName", "ZdcModules", "Location of ZDC processed data"};
    Gaudi::Property<std::string> m_zdcSumContainerName {this, "ZdcSumContainerName", "ZdcSums", "Location of ZDC processed sums"};
    Gaudi::Property<std::string> m_auxSuffix{this, "AuxSuffix", "", "Append this tag onto end of AuxData"};

    static const int m_nSides = 2;
    static const int m_nModules = 4;
    static const int m_nChannels = 16;
    static const int m_nStatusBits = 18; // ignoring the last one
    const std::vector<std::string> m_LEDNames = {"Blue1", "Green", "Blue2"};

    // the i-th element (or (i,j)-th element for 2D vector) here gives the index of the generic monitoring tool (GMT)
    // in the array of all GMT's --> allows faster tool retrieving and hence faster histogram filling
    std::vector<std::vector<int>> m_ZDCModuleToolIndices;
    std::vector<std::vector<int>> m_RPDChannelToolIndices;
    std::vector<std::vector<std::vector<int>>> m_ZDCModuleLEDToolIndices;
    std::vector<std::vector<std::vector<int>>> m_RPDChannelLEDToolIndices;

    //---------------------------------------------------
    
    // owner, name (allows us to modify the key in python configuration), key
    SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumContainerKey {this, "ZdcSumContainerKey", "ZdcSums"};
    SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleContainerKey {this, "ZdcModuleContainerKey", "ZdcModules"};
    SG::ReadHandleKey<xAOD::HIEventShapeContainer> m_HIEventShapeContainerKey {this, "HIEventShapeContainerKey", "HIEventShape"};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_eventTypeKey {this, "ZdcEventTypeKey", m_zdcSumContainerName + ".EventType" + m_auxSuffix};
    // SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcBCIDKey {this, "ZdcBCIDKey", m_zdcSumContainerName + ".BCID" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_DAQModeKey {this, "ZdcDAQModeKey", m_zdcSumContainerName + ".DAQMode" + m_auxSuffix};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumCalibEnergyKey {this, "ZdcSumCalibEnergyKey", m_zdcSumContainerName + ".CalibEnergy" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumAverageTimeKey {this, "ZdcSumAverageTimeKey", m_zdcSumContainerName + ".AverageTime" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumUncalibSumKey {this, "ZdcSumUncalibSumKey", m_zdcSumContainerName + ".UncalibSum" + m_auxSuffix};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleStatusKey {this, "ZdcModuleStatusKey", m_zdcModuleContainerName + ".Status" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleAmplitudeKey {this, "ZdcModuleAmplitudeKey", m_zdcModuleContainerName + ".Amplitude" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleTimeKey {this, "ZdcModuleTimeKey", m_zdcModuleContainerName + ".Time" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleChisqKey {this, "ZdcModuleChisqKey", m_zdcModuleContainerName + ".Chisq" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleCalibEnergyKey {this, "ZdcModuleCalibEnergyKey", m_zdcModuleContainerName + ".CalibEnergy" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleCalibTimeKey {this, "ZdcModuleCalibTimeKey", m_zdcModuleContainerName + ".CalibTime" + m_auxSuffix};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeKey {this, "RPDChannelAmplitudeKey", m_zdcModuleContainerName + ".RPDChannelAmplitude" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeCalibKey {this, "RPDChannelAmplitudeCalibKey", m_zdcModuleContainerName + ".RPDChannelAmplitudeCalib" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelStatusKey {this, "RPDChannelStatusKey", m_zdcModuleContainerName + ".RPDChannelStatus" + m_auxSuffix};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDTypeKey{this, "ZdcLEDTypeKey", m_zdcSumContainerName + ".LEDType" + m_auxSuffix}; // recorded in the global sum
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDPresampleADCKey{this, "ZdcLEDPresampleADCKey", m_zdcModuleContainerName + ".Presample" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDADCSumKey{this, "ZdcLEDADCSumKey", m_zdcModuleContainerName + ".ADCSum" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDMaxADCKey{this, "ZdcLEDMaxADCKey", m_zdcModuleContainerName + ".MaxADC" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDMaxSampleKey{this, "ZdcLEDMaxSampleKey", m_zdcModuleContainerName + ".MaxSample" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDAvgTimeKey{this, "ZdcLEDAvgTimeKey", m_zdcModuleContainerName + ".AvgTime" + m_auxSuffix};

    //---------------------------------------------------

};
#endif
