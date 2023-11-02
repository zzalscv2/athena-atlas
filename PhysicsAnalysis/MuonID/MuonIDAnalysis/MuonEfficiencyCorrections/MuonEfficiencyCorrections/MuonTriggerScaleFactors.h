/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MUONTRIGGERSCALEFACTORS_H_
#define MUONTRIGGERSCALEFACTORS_H_

#include "MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "PATInterfaces/ISystematicsTool.h"
#include "PATInterfaces/SystematicRegistry.h"
#include <stdexcept>
#include <iostream>
#include <string>
#include <sstream>
#include <memory>

#include "TDirectory.h"

#include <unordered_map>

class TH1;

namespace CP {
    typedef std::shared_ptr<TH1> TH1_Ptr;

    class MuonTriggerScaleFactors: virtual public CP::IMuonTriggerScaleFactors, public asg::AsgTool {
            ASG_TOOL_CLASS( MuonTriggerScaleFactors, CP::IMuonTriggerScaleFactors )

            public:

            MuonTriggerScaleFactors(const std::string& name);

            virtual ~MuonTriggerScaleFactors();

            virtual StatusCode initialize(void);
            
            // for single lepton triggers
            virtual CorrectionCode getTriggerScaleFactor(const xAOD::MuonContainer& mucont, Double_t& triggersf, const std::string& trigger) const;

            virtual CorrectionCode getTriggerScaleFactor(const xAOD::Muon& muon, Double_t& triggersf, const std::string& trigger) const;

            virtual CorrectionCode getTriggerEfficiency(const xAOD::Muon& mu, Double_t& efficiency, const std::string& trigger, Bool_t dataType) const;

            virtual bool isAffectedBySystematic(const CP::SystematicVariation& systematic) const;

            virtual CP::SystematicSet affectingSystematics() const;

            virtual CP::SystematicSet recommendedSystematics() const;

            virtual StatusCode applySystematicVariation(const CP::SystematicSet& systConfig);

            virtual int getBinNumber(const xAOD::Muon& muon, const std::string& trigger) const;

            virtual int getReplica_index(const std::string& sysBaseName, const std::string& trigStr) const;
      
            /// Returns whether the trigger is supported by the tool or not. The decision depends on the present (random)RunNumber 
            virtual bool isTriggerSupported(const std::string& trigger) const;
        private:

            virtual CorrectionCode getMuonEfficiency(Double_t& eff, const TrigMuonEff::Configuration& configuration, const xAOD::Muon& muon, const std::string& trigger, const std::string& systematic) const;

            virtual CorrectionCode GetTriggerSF_dimu(Double_t& TriggerSF, TrigMuonEff::Configuration& configuration, const xAOD::MuonContainer& mucont, const std::string& trigger) const;

            virtual CorrectionCode GetTriggerSF(Double_t& TriggerSF, TrigMuonEff::Configuration& configuration, const xAOD::MuonContainer& mucont, const std::string& trigger) const;

            virtual CorrectionCode GetTriggerSF(Double_t& TriggerSF, TrigMuonEff::Configuration& configuration, const xAOD::Muon& muon, const std::string& trigger) const;

            virtual CorrectionCode getDimuonEfficiency(Double_t& eff, const TrigMuonEff::Configuration& configuration, const xAOD::MuonContainer& mucont, const std::string& chain, const std::string& systematic) const;

            const CP::SystematicSet& appliedSystematics() const {
                return *m_appliedSystematics;
            }

            StatusCode registerSystematics();
            
            StatusCode LoadTriggerMap(unsigned int year);

            //This function is needed during initialization to parse the histograms to the cache 
            unsigned int encodeHistoName(const std::string &period, const std::string& Trigger, bool isData, const std::string& Systematic, bool isBarrel = true) const;
            //This function is the equivalent during run time to retrieve the histograms from the cache
            unsigned int encodeHistoName(const std::string& Trigger, const TrigMuonEff::Configuration& configuration, const std::string& Systematic, bool isBarrel = true) const;
        protected:
            virtual std::shared_ptr<TH1> getEfficiencyHistogram(unsigned int year, const std::string& period, const std::string& trigger, bool isData, const std::string& Systematic, bool isBarrel = true) const;
            virtual std::shared_ptr<TH1> getEfficiencyHistogram(const std::string& trigger, bool isData, const std::string& Systematic, bool isBarrel = true) const;
        private:
            typedef std::pair<unsigned int, std::string> YearPeriod;
            typedef std::pair<YearPeriod, unsigned int> EffiHistoIdent;
            typedef std::map<EffiHistoIdent, TH1_Ptr> EfficiencyMap;

            CorrectionCode getThreshold(Int_t& threshold, const std::string& trigger) const;

            std::string getTriggerCorrespondingToDimuonTrigger(const std::string& trigger) const;
        protected:      

            std::string getDataPeriod() const;
            unsigned int getRunNumber() const;     
            unsigned int getYear(unsigned int run) const;
            std::string getDataPeriod(unsigned int run) const;
            std::string getDataPeriod(unsigned int runNumber, unsigned int year) const;
       private:
            SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfoContName", "EventInfo", "event info key"};

            TDirectory* getTemporaryDirectory(void) const;

            //Generate replicas of h for Toys with each bin of h varied with Gaussian distribution
            //with mean from bin content and sigma from bin error
            std::vector<TH1_Ptr> generateReplicas(TH1_Ptr h, int nrep, int seed) const;

            std::unordered_map<CP::SystematicSet, CP::SystematicSet> m_systFilter;

            CP::SystematicSet* m_appliedSystematics;
            std::string m_fileName;
            EfficiencyMap m_efficiencyMap;
            std::map<EffiHistoIdent, std::vector<TH1_Ptr> > m_efficiencyMapReplicaArray;

            std::string m_muonquality;

            // subfolder to load from the calibration db
            std::string m_calibration_version;
            std::string m_custom_dir;
            std::string m_binning;
            bool m_allowZeroSF;
            bool m_experimental;
      int m_forceYear;
      std::string m_forcePeriod;
            //Variables for toy replicas setup
            std::vector<std::string> m_replicaTriggerList;
            std::set<std::string> m_replicaSet; //set of triggers for replicas, for fast searching
            int m_nReplicas;
            int m_ReplicaRandomSeed ;

    };

}

#endif
