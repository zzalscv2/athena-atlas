/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCLEDMONITORALGORITHM_H
#define ZDCLEDMONITORALGORITHM_H

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

class ZdcLEDMonitorAlgorithm : public AthMonitorAlgorithm {
public:
    ZdcLEDMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~ZdcLEDMonitorAlgorithm();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
    StatusCode fillLEDHistograms(unsigned int DAQMode, const EventContext& ctx ) const;

private:
    Gaudi::Property<std::string> m_zdcModuleContainerName {this, "ZdcModuleContainerName", "ZdcModules", "Location of ZDC processed data"};
    Gaudi::Property<std::string> m_zdcSumContainerName {this, "ZdcSumContainerName", "ZdcSums", "Location of ZDC processed sums"};
    Gaudi::Property<std::string> m_auxSuffix{this, "AuxSuffix", "", "Append this tag onto end of AuxData"};

    Gaudi::Property<std::string> m_CalReq0{this, "CalReq0", "CALREQ_0"};
    Gaudi::Property<std::string> m_CalReq1{this, "CalReq1", "CALREQ_1"};
    Gaudi::Property<std::string> m_CalReq2{this, "CalReq2", "CALREQ_2"};

    static const int m_nSides = 2;
    static const int m_nModules = 4;
    static const int m_nChannels = 16;
    const std::vector<std::string> m_LEDNames = {"Blue1", "Green", "Blue2"};

    // the i-th element (or (i,j)-th element for 2D vector) here gives the index of the generic monitoring tool (GMT)
    // in the array of all GMT's --> allows faster tool retrieving and hence faster histogram filling
    std::vector<std::vector<std::vector<int>>> m_ZDCModuleLEDToolIndices;
    std::vector<std::vector<std::vector<int>>> m_RPDChannelLEDToolIndices;

    //---------------------------------------------------
    
    // owner, name (allows us to modify the key in python configuration), key
    SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumContainerKey {this, "ZdcSumContainerKey", "ZdcSums"};
    SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleContainerKey {this, "ZdcModuleContainerKey", "ZdcModules"};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_eventTypeKey {this, "ZdcEventTypeKey", m_zdcSumContainerName + ".EventType" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_DAQModeKey {this, "ZdcDAQModeKey", m_zdcSumContainerName + ".DAQMode" + m_auxSuffix};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_robBCIDKey {this, "ROBBCIDKey", m_zdcSumContainerName + ".rodBCID" + m_auxSuffix, "BCID from LUCROD ROB headers"};


    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDTypeKey{this, "ZdcLEDTypeKey", m_zdcSumContainerName + ".LEDType" + m_auxSuffix}; // recorded in the global sum
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDPresampleADCKey{this, "ZdcLEDPresampleADCKey", m_zdcModuleContainerName + ".Presample" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDADCSumKey{this, "ZdcLEDADCSumKey", m_zdcModuleContainerName + ".ADCSum" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDMaxADCKey{this, "ZdcLEDMaxADCKey", m_zdcModuleContainerName + ".MaxADC" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDMaxSampleKey{this, "ZdcLEDMaxSampleKey", m_zdcModuleContainerName + ".MaxSample" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_LEDAvgTimeKey{this, "ZdcLEDAvgTimeKey", m_zdcModuleContainerName + ".AvgTime" + m_auxSuffix};

};
#endif
