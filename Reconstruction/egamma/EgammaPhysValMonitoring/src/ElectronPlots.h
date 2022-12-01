/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAPHYSVALMONITORING_ELECTRONPLOTS_H
#define EGAMMAPHYSVALMONITORING_ELECTRONPLOTS_H

#include "GaudiKernel/ToolHandle.h"

#include "TrkValHistUtils/PlotBase.h"
#include "TrkValHistUtils/ParamPlots.h"
#include "KinematicsPlots.h"
#include "ShowerShapesPlots.h"
#include "IsolationPlots.h"
#include "TrackPlots.h"
#include "TrkValHistUtils/EfficiencyPlots.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

#include "xAODEgamma/Electron.h"
#include "xAODTruth/TruthParticle.h"

namespace Egamma{

class ElectronPlots:public PlotBase {
    public:
      ElectronPlots(PlotBase* pParent, const std::string& sDir, const std::string& sParticleType);
      void fill(const xAOD::Electron& electron, const xAOD::EventInfo& eventInfo, bool isPrompt);
     // Reco only information

      Egamma::KinematicsPlots     m_oKinAllRecoPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesAllRecoPlots;
      Egamma::IsolationPlots      m_oIsolationAllRecoPlots;
      Egamma::TrackPlots          m_oTrackAllRecoPlots;
    
      Egamma::KinematicsPlots     m_oKinIsoRecoPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesIsoRecoPlots;
      Egamma::IsolationPlots      m_oIsolationIsoRecoPlots;
      Egamma::TrackPlots          m_oTrackIsoRecoPlots;
  
      Egamma::KinematicsPlots     m_oKinIsoLHLoosePlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesIsoLHLoosePlots;
      Egamma::IsolationPlots      m_oIsolationIsoLHLoosePlots;
      Egamma::TrackPlots          m_oTrackIsoLHLoosePlots;
    
      Egamma::KinematicsPlots     m_oKinIsoLHMediumPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesIsoLHMediumPlots;
      Egamma::IsolationPlots      m_oIsolationIsoLHMediumPlots;
      Egamma::TrackPlots          m_oTrackIsoLHMediumPlots;
    
      Egamma::KinematicsPlots     m_oKinIsoLHTightPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesIsoLHTightPlots;
      Egamma::IsolationPlots      m_oIsolationIsoLHTightPlots;
      Egamma::TrackPlots          m_oTrackIsoLHTightPlots;

      Egamma::KinematicsPlots     m_oKinPromptRecoPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesPromptRecoPlots;
      Egamma::IsolationPlots      m_oIsolationPromptRecoPlots;
      Egamma::TrackPlots          m_oTrackPromptRecoPlots;
    
      Egamma::KinematicsPlots     m_oKinPromptLHLoosePlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesPromptLHLoosePlots;
      Egamma::IsolationPlots      m_oIsolationPromptLHLoosePlots;
      Egamma::TrackPlots          m_oTrackPromptLHLoosePlots;
    
      Egamma::KinematicsPlots     m_oKinPromptLHMediumPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesPromptLHMediumPlots;
      Egamma::IsolationPlots      m_oIsolationPromptLHMediumPlots;
      Egamma::TrackPlots          m_oTrackPromptLHMediumPlots;
    
      Egamma::KinematicsPlots     m_oKinPromptLHTightPlots;
      Egamma::ShowerShapesPlots   m_oShowerShapesPromptLHTightPlots;
      Egamma::IsolationPlots      m_oIsolationPromptLHTightPlots;
      Egamma::TrackPlots          m_oTrackPromptLHTightPlots;
      
      TH1* nParticles;
      TH1* nParticles_weighted;
      TH1* nTypeParticles;
      std::string m_sParticleType;

    private:
      virtual void initializePlots();
      
      bool Match(const xAOD::Egamma& particle);

};

}

#endif
