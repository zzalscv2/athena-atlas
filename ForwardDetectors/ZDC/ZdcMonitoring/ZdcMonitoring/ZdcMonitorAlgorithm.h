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

class ZdcMonitorAlgorithm : public AthMonitorAlgorithm {
public:
    ZdcMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~ZdcMonitorAlgorithm();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
private:
    std::string m_auxSuffix;

    // see the standalone version of the Gaudi::Property class (a wrapper in AsgTools) at
    // athena/Control/AthToolSupport/AsgTools/AsgTools/PropertyWrapper.h
    // input to constructor: owner, name, value, title = "" (by default)
    Gaudi::Property<bool> m_CalInfoOn {this,"CalInfoOn",false};

    static const int m_nSides = 2;
    static const int m_nModules = 4;
    static const int m_nChannels = 16;
    static const int m_nStatusBits = 18; // ignoring the last one

    // the i-th element (or (i,j)-th element for 2D vector) here gives the index of the generic monitoring tool (GMT)
    // in the array of all GMT's --> allows faster tool retrieving and hence faster histogram filling
    // std::vector<int> m_ZDCSumToolIndices;
    std::vector<std::vector<int>> m_ZDCModuleToolIndices;
    std::vector<std::vector<int>> m_RPDChannelToolIndices;

    //---------------------------------------------------
    
    // owner, name (allows us to modify the key in python configuration), key
    SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumContainerKey {this, "ZdcSumContainerKey", "ZdcSums"};
    SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleContainerKey {this, "ZdcModuleContainerKey", "ZdcModules"};
    SG::ReadHandleKey<xAOD::HIEventShapeContainer> m_HIEventShapeContainerKey {this, "HIEventShapeContainerKey", "HIEventShape"};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumCalibEnergyKey {this, "ZdcSumCalibEnergyKey", "ZdcSums.CalibEnergy"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumAverageTimeKey {this, "ZdcSumAverageTimeKey", "ZdcSums.AverageTime"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumUncalibSumKey {this, "ZdcSumUncalibSumKey", "ZdcSums.UncalibSum"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleStatusKey {this, "ZdcModuleStatusKey", "ZdcModules.Status"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleAmplitudeKey {this, "ZdcModuleAmplitudeKey", "ZdcModules.Amplitude"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleTimeKey {this, "ZdcModuleTimeKey", "ZdcModules.Time"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleChisqKey {this, "ZdcModuleChisqKey", "ZdcModules.Chisq"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleCalibEnergyKey {this, "ZdcModuleCalibEnergyKey", "ZdcModules.CalibEnergy"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleCalibTimeKey {this, "ZdcModuleCalibTimeKey", "ZdcModules.CalibTime"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeKey {this, "RPDChannelAmplitudeKey", "ZdcModules.RPDChannelAmplitude"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelMaxSampleKey {this, "RPDChannelMaxSampleKey", "ZdcModules.RPDChannelMaxSample"};

    //---------------------------------------------------

};
#endif
