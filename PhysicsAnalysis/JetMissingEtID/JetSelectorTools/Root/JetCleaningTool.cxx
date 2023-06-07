/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/******************************************************************************
Name:        JetCleaningTool

Author:      Zach Marshall
Created:     Feb 2014

Description: Class for selecting jets that pass some cleaning cuts
******************************************************************************/

#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"

// This class header and package headers
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetSelectorTools/Helpers.h"

// xAOD/ASG includes
#include "AsgMessaging/AsgMessaging.h"

// STL includes
#include <iostream>
#include <cmath>
#include <cfloat>
#include <stdexcept>
#include <utility>
// ROOT includes
#include "TEnv.h"

//=============================================================================
// Constructors
//=============================================================================
namespace JCT
{
class HotCell : public asg::AsgMessaging
{
    public:
        HotCell(const int layer, const float etaMin, const float etaMax, const float phiMin, const float phiMax);
        virtual ~HotCell() {}
        bool jetAffectedByHotCell(const xAOD::Jet& jet) const;
    private:
        HotCell();
        const int m_layer{-1};
        const float m_etaMin{10};
        const float m_etaMax{10};
        const float m_phiMin{10};
        const float m_phiMax{10};
        SG::AuxElement::ConstAccessor< std::vector<float> > m_ePerSamp{"EnergyPerSampling"};
};

HotCell::HotCell(const int layer, const float etaMin, const float etaMax, const float phiMin, const float phiMax)
    : asg::AsgMessaging("HotCell")
    , m_layer(layer)
    , m_etaMin(etaMin)
    , m_etaMax(etaMax)
    , m_phiMin(phiMin)
    , m_phiMax(phiMax) { }

HotCell::HotCell()
    : asg::AsgMessaging("HotCell"){}

bool HotCell::jetAffectedByHotCell(const xAOD::Jet& jet) const
{
    // First check if the jet points to the cell
    const float eta = jet.eta();
    const float phi = jet.phi();
    if ( (m_etaMin < eta && eta < m_etaMax) && (m_phiMin < phi && phi < m_phiMax) )
    {
        // It points to the cell, now check if the maximum layer is the hot cell or if at least 40% of the energy is the hot cell
        const int fmaxIndex = jet.getAttribute<int>(xAOD::JetAttribute::FracSamplingMaxIndex);
        float ePerSamp = 0;
        if( m_ePerSamp.isAvailable(jet) )
            ePerSamp = m_ePerSamp(jet).at(m_layer)/jet.e();
        else
            ATH_MSG_WARNING("Could not retrieve EnergyPerSampling from jet, cleaning performance may be reduced");

        if (fmaxIndex == m_layer || ePerSamp > 0.4)
            return true;
    }
    return false;
}

} // end JCT namespace



//=============================================================================
// Constructors
//=============================================================================
JetCleaningTool::JetCleaningTool(const std::string& name)
  : asg::AsgTool(name) {}
/** Cut-based constructor */
JetCleaningTool::JetCleaningTool(const CleaningLevel alevel, const bool doUgly)
  : JetCleaningTool( "JetCleaningTool_"+getCutName(alevel) )
{
  m_cutLevel=alevel;
  m_doUgly = doUgly;
}

/** Cut and string based constructor */
JetCleaningTool::JetCleaningTool(const std::string& name , const CleaningLevel alevel, const bool doUgly)
  : JetCleaningTool(name)
{
  m_cutLevel=alevel;
  m_doUgly = doUgly;
}


//=============================================================================
// Destructor  
//=============================================================================
JetCleaningTool::~JetCleaningTool() = default;

//=============================================================================
// Initialize
//=============================================================================
StatusCode JetCleaningTool::initialize()
{
  if (UnknownCut==m_cutLevel){
    ATH_MSG_ERROR( "Tool initialized with unknown cleaning level." );
    return StatusCode::FAILURE;
  }

  if (m_cutName!="") m_cutLevel = getCutLevel( m_cutName );
  ATH_MSG_INFO( "Configured with cut level " << getCutName( m_cutLevel ) );
  std::string jetCleanDFName = "DFCommonJets_jetClean_"+getCutName(m_cutLevel);
  m_acc_jetClean = jetCleanDFName;
  m_jetCleanKey = m_jetContainerName + "." + getCutName(m_cutLevel);

  ATH_MSG_DEBUG( "Initialized decorator name: " << jetCleanDFName );

  m_accept.addCut( "Cleaning", "Cleaning of the jet" );
    
  // Read in the map of runNumbers to bad cells
  ATH_CHECK(readHotCells());
   
  ATH_CHECK(m_jetCleanKey.initialize(!m_jetContainerName.empty()));

  return StatusCode::SUCCESS;
}
//===============================================================
// Calculate the accept from the DFCommonJets_jetClean decorator
//===============================================================
asg::AcceptData JetCleaningTool::accept( const int isJetClean, const int fmaxIndex ) const
{                
    asg::AcceptData acceptData (&m_accept);
    acceptData.setCutResult( "Cleaning", false );

    //=============================================================
    //Run-II ugly cuts
    //=============================================================
    if(m_doUgly && fmaxIndex==17) return acceptData;

    //=============================================================
    //Loose/tight cleaning taken from decoration
    //=============================================================
    if(isJetClean==0) return acceptData;
    else{
        acceptData.setCutResult( "Cleaning", true );
        return acceptData;
    }

    // We should never arrive here!
    ATH_MSG_ERROR( "Unknown cut name: " << getCutName( m_cutLevel ) << " in JetCleaningTool" );
    return acceptData;

}
//===============================================================
// Calculate tight cleaning from loose decoration + variables
//===============================================================
asg::AcceptData JetCleaningTool::accept( const int isJetClean,
        const double sumpttrk, //in MeV, same as sumpttrk
        const double fmax,
        const double eta,
        const double pt,
        const int    fmaxIndex                        
        ) const
{                
    asg::AcceptData acceptData (&m_accept);
    acceptData.setCutResult( "Cleaning", false );
    const double chf=sumpttrk/pt;

    //=============================================================
    //Run-II ugly cuts
    //=============================================================
    if(m_doUgly && fmaxIndex==17) return acceptData;

    //=============================================================
    //Tight cleaning taken from decoration
    //=============================================================
    if(isJetClean==0) return acceptData;  //fails Loose cleaning
    else if (fmax<DBL_MIN) return acceptData;
        else if(std::fabs(eta)<2.4 && chf/fmax<0.1) return acceptData;
    else{
        acceptData.setCutResult( "Cleaning", true );
        return acceptData;
    }

    // We should never arrive here!
    ATH_MSG_ERROR( "Unknown cut name: " << getCutName( m_cutLevel ) << " in JetCleaningTool" );
    return acceptData;

}

//=============================================================================
// Calculate the actual accept of each cut individually.
//=============================================================================
asg::AcceptData JetCleaningTool::accept( const double emf,
                              const double hecf,
                              const double larq,
                              const double hecq,
                              //const double time,     //in ns
                              const double sumpttrk, //in MeV, same as sumpttrk
                              const double eta,      //emscale Eta  
                              const double pt,       //in MeV, same as sumpttrk
                              const double fmax,
                              const double negE ,     //in MeV
                              const double AverageLArQF,
                              const int    fmaxIndex
                              ) const
{
  asg::AcceptData acceptData (&m_accept);
  acceptData.setCutResult( "Cleaning", false );

  // -----------------------------------------------------------
  // Do the actual selection
  if (pt<DBL_MIN) return acceptData;
  const double chf=sumpttrk/pt;

  //=============================================================
  //Run-II ugly cuts
  //=============================================================
  if(m_doUgly && fmaxIndex==17) return acceptData;

  //=============================================================
  //Run-II very loose LLP cuts
  // From https://indico.cern.ch/event/642438/contributions/2704590/attachments/1514445/2362870/082117a_HCW_NCB_LLP.pdf
  //=============================================================
  if (VeryLooseBadLLP == m_cutLevel){
    if (fmax>0.80) return acceptData;
    if (emf>0.96) return acceptData;
    acceptData.setCutResult( "Cleaning", true );
    return acceptData;
  }

  //=============================================================
  //Run-II loose cuts
  //=============================================================
  //Non-collision background & cosmics
  const bool useLLP = (LooseBadLLP == m_cutLevel); // LLP cleaning cannot use emf...
  const bool isTrigger = (LooseBadTrigger == m_cutLevel); // trigger cannot use chf
  if (!useLLP) {
    if(!isTrigger && emf<0.05 && chf<0.05 && std::fabs(eta)<2)            return acceptData;
    if(emf<0.05 && std::fabs(eta)>=2)                       return acceptData;
  }
  if(fmax>0.99 && std::fabs(eta)<2)                       return acceptData;
  //HEC spike-- gone as of 2017! 
  if(hecf>0.5 && std::fabs(hecq)>0.5 && AverageLArQF/65535>0.8)                     return acceptData;
  //EM calo noise
  if(emf>0.95 && std::fabs(larq)>0.8 && std::fabs(eta)<2.8 && AverageLArQF/65535>0.8)    return acceptData;
  // LLP cleaning uses negative energy cut
  // (https://indico.cern.ch/event/472320/contribution/8/attachments/1220731/1784456/JetTriggerMeeting_20160102.pdf)
  if (useLLP && std::fabs(negE*0.001)>4 && fmax >0.85) return acceptData;
  
  if (LooseBad==m_cutLevel || LooseBadLLP==m_cutLevel || LooseBadTrigger==m_cutLevel){
    acceptData.setCutResult( "Cleaning", true );
    return acceptData;
  }

  //=============================================================
  //Run-II tight cuts
  //=============================================================
  // NCB monojet-style cut in central, EMF cut in forward
  if (fmax<DBL_MIN) return acceptData;
  if(std::fabs(eta)<2.4 && chf/fmax<0.1) return acceptData;
  //if(std::fabs(eta)>=2.4 && emf<0.1) return acceptData;
  if(TightBad==m_cutLevel){
    acceptData.setCutResult( "Cleaning", true );
    return acceptData;
  }

  // We should never arrive here!
  ATH_MSG_ERROR( "Unknown cut name: " << getCutName( m_cutLevel ) << " in JetCleaningTool" );
  return acceptData;
}


void JetCleaningTool::missingVariable(const std::string& varName) const
{
    ATH_MSG_FATAL(Form("JetCleaningTool failed to retrieve a required variable - please confirm that the xAOD::Jet being passed contains the variable named %s",varName.c_str()));
    throw std::runtime_error(Form("JetCleaningTool failed to retrieve a required variable - please confirm that the xAOD::Jet being passed contains the variable named %s",varName.c_str()));
}

asg::AcceptData JetCleaningTool::accept( const xAOD::Jet& jet) const
{
  std::vector<float> sumPtTrkvec;
  jet.getAttribute( xAOD::JetAttribute::SumPtTrkPt500, sumPtTrkvec );
  double sumpttrk = 0;
  if( ! sumPtTrkvec.empty() ) sumpttrk = sumPtTrkvec[0];
  // fmax index is not necessarily required
  // This is only used if doUgly is set
  // Handle it gracefully if the variable is not present but doUgly is false
  int FracSamplingMaxIndex = -1;
  if (!jet.getAttribute(xAOD::JetAttribute::FracSamplingMaxIndex,FracSamplingMaxIndex) && m_doUgly)
      missingVariable("FracSamplingMaxIndex"); 
  // get tight cleaning variables
  float FracSamplingMax = 0;
  if (!jet.getAttribute(xAOD::JetAttribute::FracSamplingMax,FracSamplingMax))
      missingVariable("FracSamplingMax");
 
  //start jet cleaning 
  int isJetClean = 0; 
  if( m_useDecorations && m_acc_jetClean.isAvailable(jet) ) { //decoration is already available for all jets 
          isJetClean = m_acc_jetClean(jet);
          return accept (isJetClean, FracSamplingMaxIndex);
  }
  else{   //running over AOD, need to use all variables
      ATH_MSG_DEBUG("DFCommon jet cleaning variable not available ... Using jet cleaning tool");
      // Get all of the required variables
      // Do it this way so we can gracefully handle missing variables (rather than segfaults)
      
      float EMFrac = 0;
      if (!jet.getAttribute(xAOD::JetAttribute::EMFrac,EMFrac))
          missingVariable("EMFrac");
      
      float HECFrac = 0;
      if (!jet.getAttribute(xAOD::JetAttribute::HECFrac,HECFrac))
          missingVariable("HECFrac");
      
      float LArQuality = 0;
      if (!jet.getAttribute(xAOD::JetAttribute::LArQuality,LArQuality))
          missingVariable("LArQuality");
    
     
      float HECQuality = 0;
      if (!jet.getAttribute(xAOD::JetAttribute::HECQuality,HECQuality))
          missingVariable("HECQuality");

     
      float NegativeE = 0;
      if (!jet.getAttribute(xAOD::JetAttribute::NegativeE,NegativeE))
          missingVariable("NegativeE");

     

      float AverageLArQF = 0;
      if (!jet.getAttribute(xAOD::JetAttribute::AverageLArQF,AverageLArQF))
          missingVariable("AverageLArQF");

      return accept (EMFrac,
              HECFrac,
              LArQuality,
              HECQuality,
              sumpttrk,
              jet.eta(),
              jet.pt(),
              FracSamplingMax,
              NegativeE,
              AverageLArQF,
              FracSamplingMaxIndex);}
}

StatusCode JetCleaningTool::decorate(const xAOD::JetContainer &jets) const
{
    ATH_MSG_DEBUG(" Decorating jets with jet cleaning decoration : " << m_jetCleanKey.key());

    SG::WriteDecorHandle<xAOD::JetContainer, bool> cleanHandle(m_jetCleanKey);

    for (const xAOD::Jet *jet : jets) {
        cleanHandle(*jet) = accept(*jet).getCutResult("Cleaning");
    }

    return StatusCode::SUCCESS;

}

/** Hot cell checks */
bool JetCleaningTool::containsHotCells( const xAOD::Jet& jet, const unsigned int runNumber) const
{
    // Check if the runNumber contains bad cells
    std::unordered_map<unsigned int, std::vector<std::unique_ptr<JCT::HotCell>>>::const_iterator hotCells = m_hotCellsMap.find(runNumber);
    if (hotCells != m_hotCellsMap.end())
    {
        // The run contains hot cells
        // Check if the jet is affected by one of the hot cells
        for (const std::unique_ptr<JCT::HotCell>& cell : hotCells->second)
            if (cell->jetAffectedByHotCell(jet))
                return true;
    }
    return false;
}

/** Helpers for cut names */
JetCleaningTool::CleaningLevel JetCleaningTool::getCutLevel( const std::string s ) const
{
  if (s=="VeryLooseBadLLP") return VeryLooseBadLLP;
  if (s=="LooseBad")     return LooseBad;
  if (s=="LooseBadLLP")     return LooseBadLLP;
  if (s=="LooseBadTrigger")     return LooseBadTrigger;
  if (s=="TightBad")     return TightBad;
  ATH_MSG_ERROR( "Unknown cut level requested: " << s );
  return UnknownCut;  
}

std::string JetCleaningTool::getCutName( const CleaningLevel c) const
{
  if (c==VeryLooseBadLLP) return "VeryLooseBadLLP";
  if (c==LooseBad)     return "LooseBad";
  if (c==LooseBadLLP)     return "LooseBadLLP";
  if (c==LooseBadTrigger)     return "LooseBadTrigger";
  if (c==TightBad)     return "TightBad";
  return "UnknownCut";
}


/** Hot cells reading helper */
StatusCode JetCleaningTool::readHotCells()
{
    const std::string& file_path{m_hotCellsFile};
    if (file_path.empty()) return StatusCode::SUCCESS;
    
    // Ensure that the file exists
    if ( !JCT::utils::fileExists(file_path) )
    {
        ATH_MSG_ERROR("Failed to find hot cells file: " << m_hotCellsFile);
        return StatusCode::FAILURE;
    }

    // Now parse the file
    TEnv readCells;
    if (readCells.ReadFile(file_path.c_str(),kEnvGlobal))
    {
        ATH_MSG_ERROR("Cannot read hot cells file: " << m_hotCellsFile);
        return StatusCode::FAILURE;
    }

    // Get the list of run numbers
    const TString runNumbersString = readCells.GetValue("RunNumbers","");
    if (runNumbersString=="")
    {
        ATH_MSG_ERROR("No RunNumbers field was specified in the hot cells file: " << m_hotCellsFile);
        return StatusCode::FAILURE;
    }

    // Convert into a vector
    std::vector<unsigned int> runNumbers = JCT::utils::vectorize<unsigned int>(runNumbersString,", ");
    if (!runNumbers.size())
    {
        ATH_MSG_ERROR("RunNumbers field specified, but value is empty or not unsigned ints for hot cells file: " << m_hotCellsFile);
        return StatusCode::FAILURE;
    }

    // Loop over the run numbers
    for (unsigned int run : runNumbers){
        std::vector<std::unique_ptr<JCT::HotCell>>& cellVec = m_hotCellsMap[run];

        // The number of hot cells should be below 100 for a given run...
        for (size_t iCell = 0; iCell < 100; ++iCell)
        {
            const TString baseName = Form("Run%u.Cell%zu.",run,iCell);

            // Read the cell info
            const int   layer  = readCells.GetValue(baseName+"Layer", -1  );
            const float minEta = readCells.GetValue(baseName+"EtaMin",-10.);
            const float maxEta = readCells.GetValue(baseName+"EtaMax", 10.);
            const float minPhi = readCells.GetValue(baseName+"PhiMin",-10.);
            const float maxPhi = readCells.GetValue(baseName+"PhiMax", 10.);

            // Check that the input makes sense
            if (layer < 0 && minEta < -5 && maxEta > 5 && minPhi < -5 && maxPhi > 5)
                continue;
            if (layer < 0 || minEta < -5 || maxEta > 5 || minPhi < -5 || maxPhi > 5)
            {
                ATH_MSG_ERROR("Partially specified cell - please check the file: " << m_hotCellsFile);
                ATH_MSG_ERROR(Form("Got Layer=%d, EtaMin=%f, EtaMax=%f, PhiMin=%f, PhiMax=%f",layer,minEta,maxEta,minPhi,maxPhi));
                return StatusCode::FAILURE;
            }
            cellVec.emplace_back(std::make_unique<JCT::HotCell>(layer,minEta,maxEta,minPhi,maxPhi));
        }
        
        // Ensure we found the expected run
        if (cellVec.empty())
        {
            ATH_MSG_ERROR("Specified that Run# " << run << " contains hot cells, but did not find any corresponding cells in the file: " << m_hotCellsFile);
            return StatusCode::FAILURE;
        }
    }
    
    // Done
    return StatusCode::SUCCESS;
}
