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
    StatusCode fillPhysicsDataHistograms( const EventContext& ctx ) const;

private:
    // see the standalone version of the Gaudi::Property class (a wrapper in AsgTools) at
    // athena/Control/AthToolSupport/AsgTools/AsgTools/PropertyWrapper.h
    // input to constructor: owner, name, value, title = "" (by default)
    Gaudi::Property<bool> m_CalInfoOn {this,"CalInfoOn",false};

    Gaudi::Property<std::string> m_zdcModuleContainerName {this, "ZdcModuleContainerName", "ZdcModules", "Location of ZDC processed data"};
    Gaudi::Property<std::string> m_zdcSumContainerName {this, "ZdcSumContainerName", "ZdcSums", "Location of ZDC processed sums"};
    Gaudi::Property<std::string> m_auxSuffix{this, "AuxSuffix", "", "Append this tag onto end of AuxData"};

    // single side triggers - less error-prone if defined as separate properties then in a vector (where order would be crucial)
    Gaudi::Property<std::string> m_triggerSideA{this, "triggerSideA", "L1_ZDC_A", "Trigger on side A, needed for 1N-peak monitoring on side C"};
    Gaudi::Property<std::string> m_triggerSideC{this, "triggerSideC", "L1_ZDC_C", "Trigger on side C, needed for 1N-peak monitoring on side A"};


    static const int m_nSides = 2;
    static const int m_nModules = 4;
    static const int m_nChannels = 16;
    static const int m_nZdcStatusBits = 18; // ignoring the last one
    static const int m_nRpdStatusBits = 3; // ignoring the last one
    static const int m_nRpdCentroidStatusBits = 17; // ignoring the last one

    // the i-th element (or (i,j)-th element for 2D vector) here gives the index of the generic monitoring tool (GMT)
    // in the array of all GMT's --> allows faster tool retrieving and hence faster histogram filling
    std::vector<int> m_ZDCSideToolIndices;
    std::vector<std::vector<int>> m_ZDCModuleToolIndices;
    std::vector<std::vector<int>> m_RPDChannelToolIndices;

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
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcSumModuleMaskKey {this, "ZdcSumModuleMaskKey", m_zdcSumContainerName + ".ModuleMask" + m_auxSuffix};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleStatusKey {this, "ZdcModuleStatusKey", m_zdcModuleContainerName + ".Status" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleAmplitudeKey {this, "ZdcModuleAmplitudeKey", m_zdcModuleContainerName + ".Amplitude" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleTimeKey {this, "ZdcModuleTimeKey", m_zdcModuleContainerName + ".Time" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleChisqKey {this, "ZdcModuleChisqKey", m_zdcModuleContainerName + ".Chisq" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleCalibEnergyKey {this, "ZdcModuleCalibEnergyKey", m_zdcModuleContainerName + ".CalibEnergy" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcModuleCalibTimeKey {this, "ZdcModuleCalibTimeKey", m_zdcModuleContainerName + ".CalibTime" + m_auxSuffix};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeKey {this, "RPDChannelAmplitudeKey", m_zdcModuleContainerName + ".RPDChannelAmplitude" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeCalibKey {this, "RPDChannelAmplitudeCalibKey", m_zdcModuleContainerName + ".RPDChannelAmplitudeCalib" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelMaxADCKey {this, "RPDChannelMaxADCKey", m_zdcModuleContainerName + ".RPDChannelMaxADC" + m_auxSuffix};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelStatusKey {this, "RPDChannelStatusKey", m_zdcModuleContainerName + ".RPDChannelStatus" + m_auxSuffix};
    
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDrowKey { // needed since the subtracted amplitudes for each side are written in a nrow * ncol matrix and saved in zdcSums
        this, "rowKey", m_zdcModuleContainerName + ".row" + m_auxSuffix, // need to convert channel to row & column to plot the subtracted amplitudes using rpdChannelMonToolArr (reading in the row and column for each RPD channel, instead of using an analytical expression, allows flexibility for future different RPD geometry / run conditions)
        "Row index of RPD channel"
    };
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDcolKey { // needed since the subtracted amplitudes for each side are written in a nrow * ncol matrix and saved in zdcSums
        this, "colKey", m_zdcModuleContainerName + ".col" + m_auxSuffix, // need to convert channel to row & column to plot the subtracted amplitudes using rpdChannelMonToolArr (reading in the row and column for each RPD channel, instead of using an analytical expression, allows flexibility for future different RPD geometry / run conditions)
        "Column index of RPD channel"
    };

    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelPileupFitParamsKey{
        this, "RpdChannelPileupFitParams", m_zdcModuleContainerName+".RPDChannelPileupFitParams"+m_auxSuffix, 
        "RPD Channel Pileup Fit Parameters: exp([0] + [1]*sample)"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelPileupFracKey{
        this, "RPDChannelPileupFrac", m_zdcModuleContainerName+".RPDChannelPileupFrac"+m_auxSuffix, 
        "RPD Channel Pileup as Fraction of Sum"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDsubAmpKey {
        this, "rpdSubAmpKey", m_zdcSumContainerName + ".RpdSubAmp" + m_auxSuffix,
        "Subtracted RPD amplitudes, index row then column"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDsubAmpSumKey {
        this, "rpdSubAmpSumKey", m_zdcSumContainerName + ".RpdSubAmpSum" + m_auxSuffix,
        "Sum of subtracted RPD amplitudes"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDxCentroidKey {
        this, "xCentroidKey", m_zdcSumContainerName + ".xCentroid" + m_auxSuffix, 
        "X position of centroid in beamline coordinates (after geometry corrections)"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDyCentroidKey {
        this, "yCentroidKey", m_zdcSumContainerName + ".yCentroid" + m_auxSuffix, 
        "Y position of centroid in beamline coordinates (after geometry corrections)"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDxDetCentroidUnsubKey {
        this, "xDetCentroidUnsubKey", m_zdcSumContainerName + ".xDetCentroidUnsub" + m_auxSuffix, 
        "X position of centroid in RPD detector coordinates (before geometry corrections), calculated with unsubtracted amplitudes"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDyDetCentroidUnsubKey {
        this, "yDetCentroidUnsubKey", m_zdcSumContainerName + ".yDetCentroidUnsub" + m_auxSuffix, 
        "Y position of centroid in RPD detector coordinates (before geometry corrections), calculated with unsubtracted amplitudes"};
    
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDreactionPlaneAngleKey {
        this, "reactionPlaneAngleKey", m_zdcSumContainerName + ".reactionPlaneAngle" + m_auxSuffix, 
        "Reaction plane angle"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDcosDeltaReactionPlaneAngleKey {
        this, "cosDeltaReactionPlaneAngleKey", m_zdcSumContainerName + ".cosDeltaReactionPlaneAngle" + m_auxSuffix, 
        "Cosine of the difference between the reaction plane angles of the two sides"};
    SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDcentroidStatusKey {
        this, "centroidStatusKey", m_zdcSumContainerName + ".centroidStatus" + m_auxSuffix, 
        "Centriod calculation status word"};
    //---------------------------------------------------

};
#endif
