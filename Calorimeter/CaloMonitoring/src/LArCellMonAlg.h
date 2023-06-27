/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// based on LArCellMonTool by W.Lampl (Spring 2017: Major re-design, early 2020: migration to AthenaMT)

#ifndef CALOMONITORING_LARCELLMONALG_H
#define CALOMONITORING_LARCELLMONALG_H

#include "CaloMonAlgBase.h"

#include "LArRecConditions/LArBadChannelMask.h"
#include "LArRecConditions/LArBadChannelCont.h"

#include "LArIdentifier/LArOnlineID.h"
#include "Identifier/IdentifierHash.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloConditions/CaloNoise.h"

#include "TrigDecisionTool/TrigDecisionTool.h"

#include "LArCellBinning.h"
          
#include <vector>
#include <string>
#include <bitset>
#include <array>
#include <map>
#include <unordered_map>
 
namespace Trig {
  class ChainGroup;
}

class CaloCell;
class TileID;


class LArCellMonAlg : public CaloMonAlgBase {
 
 public:

  LArCellMonAlg(const std::string& name, ISvcLocator* pSvcLocator);
  ~LArCellMonAlg();
  
  virtual StatusCode initialize()  override final;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override final;
  

private:

  // Job properties
  SG::ReadHandleKey<CaloCellContainer>  m_cellContainerKey{this,"CaloCellContainer","AllCalo","SG key of the input cell container"};

  Gaudi::Property<std::string> m_MonGroupName  {this, "MonGroupName", "LArCellMonGroup"};
  Gaudi::Property<std::string> m_MonGroupNamePerJob  {this, "MonGroupName_perJob"};
  Gaudi::Property<std::string> m_groupnameTotalOccupancyEtaPhi  {this, "MonGroupName_OccupancyEtaPhi"};
  Gaudi::Property<std::string> m_groupnamePercentageOccupancyEtaPhi  {this, "MonGroupName_PercentageOccupancyEtaPhi"};
  Gaudi::Property<std::string> m_groupnameOccupancyEta  {this, "MonGroupName_OccupancyEta"};
  Gaudi::Property<std::string> m_groupnameOccupancyPhi  {this, "MonGroupName_OccupancyPhi"};
  Gaudi::Property<std::string> m_groupnameTotEnergyEtaPhi  {this, "MonGroupName_TotEnergyEtaPhi"};
  Gaudi::Property<std::string> m_groupnameTotQualityEtaPhi  {this, "MonGroupName_AvgQualityEtaPhi"};
  Gaudi::Property<std::string> m_groupnameFractionOverQthEtaPhi  {this, "MonGroupName_FractionOverQthEtaPhi"};
  Gaudi::Property<std::string> m_groupnameTotTimeEtaPhi  {this, "MonGroupName_AvgTimeEtaPhi"};
  Gaudi::Property<std::string> m_groupnameFractionPastTthEtaPhi  {this, "MonGroupName_FractionPastTthEtaPhi"};

  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this,"CaloNoiseKey","totalNoise","SG Key of CaloNoise data object"};

  //Tool maps, for thr. histograms
  std::map<std::string,std::map<std::string,int>> m_toolmapAll;

  //Histogram path in root file:

  // Thresholds for time and Time vs Energy plots:
  // Energy thresholds hardcoded following official timing analysis. See for example:
  // https://indico.cern.ch/event/522351/
  FloatArrayProperty m_eCutForTiming {this, "EcutForTiming",
  //EMBPNS=0, EMB1NS, EMB2NS, EMB3NS, HEC0NS, HEC1NS, HEC2NS, HEC3NS,EMECPNS,EMEC1NS,EMEC2NS,EMEC3NS,FCAL1NS,FCAL2NS,FCAL3NS
    {1000.,    1000.,  3000.,  1500.,  3500.,  3500.,  3500.,  3500., 1500.,  3000.,  3000.,  2000.,  10000., 10000., 10000.}
  };

  StringArrayProperty m_layerNames{this, "LayerNames", {"EMBPA", "EMBPC", "EMB1A", "EMB1C", "EMB2A", "EMB2C", "EMB3A", "EMB3C",
				   	    "HEC0A", "HEC0C", "HEC1A", "HEC1C", "HEC2A", "HEC2C", "HEC3A", "HEC3C",
					    "EMECPA", "EMECPC", "EMEC1A", "EMEC1C", "EMEC2A", "EMEC2C", "EMEC3A", "EMEC3C", 
					    "FCAL1A", "FCAL1C", "FCAL2A", "FCAL2C", "FCAL3A", "FCAL3C"},
                                                       "Names of individual layers to monitor"};
  IntegerArrayProperty  m_layerNcells{this,"LayerNcells",{ 3905, 3905, 29376, 29376, 14595, 14595, 6912, 6912,
                                                       768, 768, 736, 736, 672, 672, 640, 640,
                                                       768, 768, 14272, 14272, 11712, 11712, 5120, 5120,
                                                       1008, 1008, 500, 500, 254, 254},
                                                       "Number of expected cells per layer"};

  StringArrayProperty m_partitionNames{this, "PartitionNames", {"EMBA","EMBC","EMECA","EMECC","HECA","HECC","FCALA","FCALC"}};  

  // Trigger Awareness:
  //Enumerations of possible threshold directions and trigger types:
  enum Direction{OVER,UNDER,NONE,BOTH}; 
  enum TriggerType{RNDM,CALO,MINBIAS,MET,MISC,NOTA,MAXTRIGTYPE};

  BooleanProperty m_useTrigger{this, "useTrigger", true};
  StringProperty   m_triggerNames[NOTA];

  BooleanProperty m_ignoreKnownBadChannels{this, "MaskBadChannels", false, "Do not fill histograms with values from known bad channels"};
  BooleanProperty m_maskNoCondChannels{this, "MaskNoCondChannels", false, "Do not fill histograms with values from cells reco'ed w/o conditions database"};

  BooleanArrayProperty m_doBeamBackgroundRemovalProp{this, "DoBeamBackgroundRemoval"}; 
  BooleanProperty m_doKnownBadChannelsVsEtaPhi{this, "doKnownBadChannelsVsEtaPhi", true};
  BooleanProperty m_doDatabaseNoiseVsEtaPhi{this, "doDatabaseNoiseVsEtaPhi", true};
  // Switch on/off additional (non thresholded) histograms:
  BooleanProperty m_doEnergyVsTime{this, "doEnergyVsTime", true};
  BooleanProperty m_doUnnormalized1DEnergy{this, "doUnnormalized1DEnergy", false};
    
  //threshold for sporadic plots
  BooleanProperty m_sporadic_switch{this, "Sporadic_switch", false};
  FloatProperty m_threshold_em_S0S1{this, "Threshold_EM_S0S1", 5000.};
  FloatProperty m_threshold_HECFCALEMS2S3{this, "Threshold_HECFCALEMS2S3", 15000.};
  UnsignedIntegerProperty m_sporadicPlotLimit{this, "NsporadicPlotLimit", 300};
  UnsignedIntegerProperty m_sporadic_protc{this, "Sporadic_protection", 4000};
  UnsignedIntegerProperty m_minSporadicNoiseEventsPerCell{this, "minSporadicEvents", 10};

  // Switch on/off thresholded histograms for each threshold:
  StringArrayProperty  m_thresholdNameProp{this, "ThresholdType", {},"Vector of threshold names"};
  StringArrayProperty  m_thresholdColumnType{this, "ThresholdColumnType",{}};
  StringArrayProperty  m_thresholdDirectionProp{this, "ThresholdDirection", {}, "Vector of threshold directions"};
  StringArrayProperty  m_triggersToIncludeProp{this, "TriggersToInclude", {}};
  StringArrayProperty  m_triggersToExcludeProp{this, "TriggersToExclude", {}};
  StringArrayProperty  m_thresholdTitleTemplates{this, "ThresholdTitleTemplates", {}};
  FloatArrayProperty   m_defaultThresholds{this, "DefaultThresholds", {}};
  BooleanArrayProperty m_inSigNoise{this, "ThresholdinSigNoise", {}};

  FloatArrayProperty   m_timeThresholdProp{this, "TimeThreshold"};
  FloatArrayProperty   m_qualityFactorThresholdProp{this, "QualityFactorThreshold",{}};
  //string arrays for histogram filling
  StringArrayProperty  m_doEtaPhiTotalOccupancyNames{this, "DoEtaPhiTotalOccupancyNames", {}};
  StringArrayProperty  m_doEtaPhiPercentageOccupancyNames{this, "DoEtaPhiPercentageOccupancyNames", {}};
  StringArrayProperty  m_doEtaOccupancyNames{this, "DoEtaOccupancyNames", {}};
  StringArrayProperty  m_doPhiOccupancyNames{this, "DoPhiOccupancyNames", {}};
  StringArrayProperty  m_doEtaPhiTotEnergyNames{this, "DoEtaPhiTotEnergyNames", {}};
  StringArrayProperty  m_doEtaPhiAvgQualityNames{this, "DoEtaPhiAvgQualityNames", {},"Turns on 'totQuality' and total 'Occupancy' plots. The ratio will be computed at post-processing stage"};
  StringArrayProperty  m_doEtaPhiFractionOverQthNames{this, "DoEtaPhiFractionOverQthNames", {}};
  StringArrayProperty  m_doEtaPhiAvgTimeNames{this, "DoEtaPhiAvgTimeNames", {},"Turns on 'totTime' and total 'Occupancy' plots. The ratio will be computed at post-processing stage"};
  StringArrayProperty  m_doEtaPhiFractionPastTthNames{this, "DoEtaPhiFractionPastTthNames", {}};

  


  FloatArrayProperty   m_thresholdsProp[MAXLYRNS];

  //Enumerate partitions
  enum PartitionEnum{EMBA,EMBC,EMECA,EMECC,HECA,HECC,FCALA,FCALC,MAXPARTITIONS};
  

  //Private methods: Initialization and job-option interpretation
  StatusCode initThresh();
  void setLArCellBinning();
  static bool isThrListed(const std::vector<std::string>& vec, const std::string& s) ;
  //void resetInternals();

  static std::string strToLower(const std::string& input) ;

  //The threshold-related variables and histograms are grouped in the following struct
  //one instance per threshold-type
  
  struct threshold_t {
    //Configuration variables
    std::string m_threshName;                        /// Name of this threshold
    std::string m_threshTitleTemplate;               /// Histogram title template
    std::array<float,MAXLYRNS> m_threshValue{{}};    /// The actual threshold (per layer)
    std::array<std::string,MAXLYRNS> m_threshTitles; /// Part of the histogram title (containing the threshold value, so per-layer)
    size_t m_thrIndex;                                  ///for filling in the eventCount plot 

    bool m_inSigNoise=false;                         /// Absolute threshold or in sigma noise?
    Direction m_threshDirection=OVER;
    bool m_doBeamBackgroundRemoval=false; 
    float m_qualityFactorThreshold=4000;
    float m_timeThreshold=4.;

    //Variables related to trigger-filtering
    std::bitset<MAXTRIGTYPE> m_triggersToInclude;
    std::bitset<MAXTRIGTYPE> m_triggersToExclude;
    bool m_threshTriggerDecision=true;

    //Switch histograms on/off
    bool m_doPercentageOccupancy=false;
    bool m_doEtaPhiOccupancy=false;
    bool m_doEtaOccupancy=false;
    bool m_doPhiOccupancy=false;

    bool m_doEtaPhiTotalEnergy=false;
    //    bool m_doEtaPhiAverageEnergy=false;

    bool m_doEtaPhiAverageQuality=false;
    bool m_doEtaPhiFractionOverQth=false;
    
    bool m_doEtaPhiAverageTime=false;
    bool m_doEtaPhiFractionPastTth=false;

  };


  
 //Mapping of layers to the partition the layer belongs to
  const std::array<PartitionEnum,MAXLAYER> m_layerEnumtoPartitionEnum{{
      EMBA, EMBC,  EMBA,  EMBC,  EMBA,  EMBC,  EMBA,  EMBC,
      HECA, HECC,  HECA,  HECC,  HECA,  HECC,  HECA,   HECC,
      EMECA, EMECC, EMECA, EMECC, EMECA, EMECC, EMECA, EMECC,
      FCALA, FCALC, FCALA, FCALC, FCALA, FCALC
       }};

  //Private methods: Histogram filling
  StatusCode createPerJobHistograms(const CaloCellContainer* cellcont, const CaloNoise *noisep) const;
  void checkTriggerAndBeamBackground(bool passBeamBackgroundRemoval, std::vector<threshold_t> &thresholds) const;
  void sporadicNoiseCandidate(const CaloCell* cell, const LArCellMonAlg::LayerEnum iLyr,const float threshold, const LArOnOffIdMapping* cabling) const;
 
  // bad channel mask  
  LArBadChannelMask m_bcMask;
  Gaudi::Property<std::vector<std::string> > m_problemsToMask{this,"ProblemsToMask",{}, "Bad-Channel categories to mask"};
 
  std::vector<threshold_t> m_thresholds;

  // Identifer helpers and such

  const LArOnlineID* m_LArOnlineIDHelper;

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArBadChannelCont> m_BCKey {this, "BadChanKey", "LArBadChannel", "SG key for LArBadChan object"};
  
  std::array<CaloMonitoring::LArCellBinning,MAXLAYER> m_binning;

  //Sporadic monitoring related variables and structs

};

#endif 

//Histograms dropped during the redesign in spring 2017:
// * All inverse-masking
// * Cell Occupancy Eta/Phi Vs LumiBlock
// * DBNoiseNormalizedEnergy per layer 
// * doEtaPhiEnergyRMS per layer and threshold
// * DoEtaPhiRMSvsDBnoise per layer and threshold

