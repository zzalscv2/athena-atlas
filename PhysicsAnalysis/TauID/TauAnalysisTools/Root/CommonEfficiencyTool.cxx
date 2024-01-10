/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Framework include(s):
#include "PathResolver/PathResolver.h"

// local include(s)
#include "TauAnalysisTools/CommonEfficiencyTool.h"
#include "TauAnalysisTools/TauEfficiencyCorrectionsTool.h"
#include "xAODTruth/TruthParticleContainer.h"

// ROOT include(s)
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TROOT.h"
#include "TClass.h"
#include <utility>

using namespace TauAnalysisTools;

/*
  This tool acts as a common tool to apply efficiency scale factors and
  uncertainties. By default, only nominal scale factors without systematic
  variations are applied. Unavailable systematic variations are ignored, meaning
  that the tool only returns the nominal value. In case the one available
  systematic is requested, the smeared scale factor is computed as:
    - sf = sf_nominal +/- n * uncertainty

  where n is in general 1 (representing a 1 sigma smearing), but can be any
  arbitrary value. In case multiple systematic variations are passed they are
  added in quadrature. Note that it's currently only supported if all are up or
  down systematics.

  The tool reads in root files including TH2 histograms which need to fullfil a
  predefined structure:

  scale factors:
    - sf_<workingpoint>_<prongness>p
  uncertainties:
    - <NP>_<up/down>_<workingpoint>_<prongness>p (for asymmetric uncertainties)
    - <NP>_<workingpoint>_<prongness>p (for symmetric uncertainties)

  where the <workingpoint> (e.g. loose/medium/tight) fields may be
  optional. <prongness> represents either 1 or 3, whereas 3 is currently used
  for multiprong in general. The <NP> fields are names for the type of nuisance
  parameter (e.g. STAT or SYST), note the tool decides whethe the NP is a
  recommended or only an available systematic based on the first character:
    - uppercase -> recommended
    - lowercase -> available
  This magic happens here:
    - CommonEfficiencyTool::generateSystematicSets()

  In addition the root input file can also contain objects of type TF1 that can
  be used to provide kind of unbinned scale factors or systematics. The major
  usecase for now is the high-pt uncertainty for the tau ID and tau
  reconstruction.

  The files may also include TNamed objects which is used to define how x and
  y-axes should be treated. By default the x-axis is given in units of tau-pT in
  GeV and the y-axis is given as tau-eta. If there is for example a TNamed
  object with name "Yaxis" and title "|eta|" the y-axis is treated in units of
  absolute tau eta. All this is done in:
    - void CommonEfficiencyTool::ReadInputs(TFile* fFile)

  Other tools for scale factors may build up on this tool and overwrite or add
  praticular functionality (one example is the TauEfficiencyTriggerTool).
*/

//______________________________________________________________________________
CommonEfficiencyTool::CommonEfficiencyTool(const std::string& sName)
  : asg::AsgTool( sName )
  , m_mSF(nullptr)
  , m_sSystematicSet(nullptr)
  , m_fX(&finalTauPt)
  , m_fY(&finalTauEta)
  , m_sSFHistName("sf")
  , m_bNoMultiprong(false)
  , m_eCheckTruth(TauAnalysisTools::Unknown)
  , m_bSFIsAvailable(false)
  , m_bSFIsAvailableChecked(false)
{
  declareProperty( "InputFilePath",       m_sInputFilePath       = "" );
  declareProperty( "VarName",             m_sVarName             = "" );
  declareProperty( "WP",                  m_sWP                  = "" );
  declareProperty( "UseHighPtUncert",     m_bUseHighPtUncert     = false );
  declareProperty( "SkipTruthMatchCheck", m_bSkipTruthMatchCheck = false );
  declareProperty( "JetIDLevel",          m_iJetIDLevel          = (int)JETIDNONE );
  declareProperty( "EleIDLevel",          m_iEleIDLevel          = (int)ELEIDNONE );
  declareProperty( "SplitMu",             m_bSplitMu             = false );
  declareProperty( "SplitMCCampaign",     m_bSplitMCCampaign     = false );
  declareProperty( "MCCampaign",          m_sMCCampaign          = "");
  declareProperty( "UseTauSubstructure",  m_bUseTauSubstructure  = false);
}

/*
  need to clear the map of histograms cause we have the ownership, not ROOT
*/
CommonEfficiencyTool::~CommonEfficiencyTool()
{
  if (m_mSF)
    for (auto mEntry : *m_mSF)
      delete std::get<0>(mEntry.second);
}

/*
  - Find the root files with scale factor inputs on cvmfs using PathResolver
    (more info here:
    https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/PathResolver)
  - Call further functions to process and define NP strings and so on
  - Configure to provide nominal scale factors by default
*/
StatusCode CommonEfficiencyTool::initialize()
{
  ATH_MSG_INFO( "Initializing CommonEfficiencyTool" );
  // only read in histograms once
  if (m_mSF==nullptr)
  {
    std::string sInputFilePath = PathResolverFindCalibFile(m_sInputFilePath);

    m_mSF = std::make_unique< tSFMAP >();
    std::unique_ptr< TFile > fSF( TFile::Open( sInputFilePath.c_str(), "READ" ) );
    if(!fSF)
    {
      ATH_MSG_FATAL("Could not open file " << sInputFilePath.c_str());
      return StatusCode::FAILURE;
    }
    ReadInputs(*fSF);
    fSF->Close();
  }

  // needed later on in generateSystematicSets(), maybe move it there
  std::vector<std::string> vInputFilePath;
  split(m_sInputFilePath,'/',vInputFilePath);
  m_sInputFileName = vInputFilePath.back();

  generateSystematicSets();

  if (!m_sWP.empty())
    m_sSFHistName = "sf_"+m_sWP;

  // load empty systematic variation by default
  if (applySystematicVariation(CP::SystematicSet()) != StatusCode::SUCCESS )
    return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}

/*
  Retrieve the scale factors and if requested the values for the NP's and add
  this stuff in quadrature. Finally return sf_nom +/- n*uncertainty
*/

//______________________________________________________________________________
CP::CorrectionCode CommonEfficiencyTool::getEfficiencyScaleFactor(const xAOD::TauJet& xTau,
    double& dEfficiencyScaleFactor, unsigned int iRunNumber, unsigned int iMu)
{
  // check which true state is requested
  if (!m_bSkipTruthMatchCheck and getTruthParticleType(xTau) != m_eCheckTruth)
  {
    dEfficiencyScaleFactor = 1.;
    return CP::CorrectionCode::Ok;
  }

  // check if 1 prong
  if (m_bNoMultiprong && xTau.nTracks() != 1)
  {
    dEfficiencyScaleFactor = 1.;
    return CP::CorrectionCode::Ok;
  }

  // get decay mode or prong extension for histogram name
  std::string sMode;
  if (m_bUseTauSubstructure)
  {
    int iDecayMode = -1;
    xTau.panTauDetail(xAOD::TauJetParameters::PanTau_DecayMode, iDecayMode);
    sMode = ConvertDecayModeToString(iDecayMode);
    if (sMode.empty())
    {
      ATH_MSG_WARNING("Found tau with unknown decay mode. Skip efficiency correction.");
      return CP::CorrectionCode::OutOfValidityRange;
    }
  }
  else
  {
    sMode = ConvertProngToString(xTau.nTracks());
  }

  std::string sMu = "";
  std::string sMCCampaign = "";

  if (m_bSplitMu) sMu = ConvertMuToString(iMu);
  if (m_bSplitMCCampaign) sMCCampaign = GetMcCampaignString(iRunNumber);
  std::string sHistName = m_sSFHistName + sMode + sMu + sMCCampaign;

  // get standard scale factor
  CP::CorrectionCode tmpCorrectionCode = getValue(sHistName,
                                                  xTau,
                                                  dEfficiencyScaleFactor);
  // return correction code if histogram is not available
  if (tmpCorrectionCode != CP::CorrectionCode::Ok)
    return tmpCorrectionCode;

  // skip further process if systematic set is empty
  if (m_sSystematicSet->empty())
    return CP::CorrectionCode::Ok;

  // get uncertainties summed in quadrature
  double dTotalSystematic2 = 0.;
  double dDirection = 0.;
  for (auto syst : *m_sSystematicSet)
  {
    // check if systematic is available
    auto it = m_mSystematicsHistNames.find(syst.basename());

    // get uncertainty value
    double dUncertaintySyst = 0.;

    // needed for up/down decision
    dDirection = syst.parameter();

    // build up histogram name
    sHistName = it->second;
    if (dDirection>0.)  sHistName+="_up";
    else                sHistName+="_down";
    if (!m_sWP.empty()) sHistName+="_"+m_sWP;
    sHistName += sMode + sMu + sMCCampaign;

    // filter unwanted combinations
    if( (sHistName.find("3P") != std::string::npos && sHistName.find("1p") != std::string::npos) ||
        (sHistName.find("1P") != std::string::npos && sHistName.find("3p") != std::string::npos)) 
        continue;

    // get the uncertainty from the histogram
    tmpCorrectionCode = getValue(sHistName,
                                 xTau,
                                 dUncertaintySyst);

    // return correction code if histogram is not available
    if (tmpCorrectionCode != CP::CorrectionCode::Ok)
      return tmpCorrectionCode;

    // scale uncertainty with direction, i.e. +/- n*sigma
    dUncertaintySyst *= dDirection;

    // square uncertainty and add to total uncertainty
    dTotalSystematic2 += dUncertaintySyst * dUncertaintySyst;
  }

  // now use dDirection to use up/down uncertainty
  dDirection = (dDirection > 0.) ? 1. : -1.;

  // finally apply uncertainty (eff * ( 1 +/- \sum  )
  dEfficiencyScaleFactor *= 1. + dDirection * std::sqrt(dTotalSystematic2);

  return CP::CorrectionCode::Ok;
}

/*
  Get scale factor from getEfficiencyScaleFactor and decorate it to the
  tau. Note that this can only be done if the variable name is not already used,
  e.g. if the variable was already decorated on a previous step (enured by the
  m_bSFIsAvailableChecked check).

  Technical note: cannot use `static SG::AuxElement::Decorator` as we will have
  multiple instances of this tool with different decoration names.
*/
//______________________________________________________________________________
CP::CorrectionCode CommonEfficiencyTool::applyEfficiencyScaleFactor(const xAOD::TauJet& xTau,
  unsigned int iRunNumber, unsigned int iMu)
{
  double dSf = 0.;

  if (!m_bSFIsAvailableChecked)
  {
    m_bSFIsAvailable = xTau.isAvailable< double >(m_sVarName);
    m_bSFIsAvailableChecked = true;
    if (m_bSFIsAvailable)
    {
      ATH_MSG_DEBUG(m_sVarName << " decoration is available on first tau processed, switched off applyEfficiencyScaleFactor for further taus.");
      ATH_MSG_DEBUG("If an application of efficiency scale factors needs to be redone, please pass a shallow copy of the original tau.");
    }
  }
  if (m_bSFIsAvailable)
    return CP::CorrectionCode::Ok;

  // retrieve scale factor
  CP::CorrectionCode tmpCorrectionCode = getEfficiencyScaleFactor(xTau, dSf, iRunNumber, iMu);
  // adding scale factor to tau as decoration
  xTau.auxdecor<double>(m_sVarName) = dSf;

  return tmpCorrectionCode;
}

/*
  standard check if a systematic is available
*/
//______________________________________________________________________________
bool CommonEfficiencyTool::isAffectedBySystematic( const CP::SystematicVariation& systematic ) const
{
  CP::SystematicSet sys = affectingSystematics();
  return sys.find(systematic) != sys.end();
}

/*
  standard way to return systematics that are available (including recommended
  systematics)
*/
//______________________________________________________________________________
CP::SystematicSet CommonEfficiencyTool::affectingSystematics() const
{
  return m_sAffectingSystematics;
}

/*
  standard way to return systematics that are recommended
*/
//______________________________________________________________________________
CP::SystematicSet CommonEfficiencyTool::recommendedSystematics() const
{
  return m_sRecommendedSystematics;
}

/*
  Configure the tool to use a systematic variation for further usage, until the
  tool is reconfigured with this function. The passed systematic set is checked
  for sanity:
    - unsupported systematics are skipped
    - only combinations of up or down supported systematics is allowed
    - don't mix recommended systematics with other available systematics, cause
      sometimes recommended are a quadratic sum of the other variations,
      e.g. TOTAL=(SYST^2 + STAT^2)^0.5
*/
//______________________________________________________________________________
StatusCode CommonEfficiencyTool::applySystematicVariation ( const CP::SystematicSet& sSystematicSet)
{

  // first check if we already know this systematic configuration
  auto itSystematicSet = m_mSystematicSets.find(sSystematicSet);
  if (itSystematicSet != m_mSystematicSets.end())
  {
    m_sSystematicSet = &itSystematicSet->first;
    return StatusCode::SUCCESS;
  }

  // sanity checks if systematic set is supported
  double dDirection = 0.;
  CP::SystematicSet sSystematicSetAvailable;
  for (auto sSyst : sSystematicSet)
  {
    // check if systematic is available
    auto it = m_mSystematicsHistNames.find(sSyst.basename());
    if (it == m_mSystematicsHistNames.end())
    {
      ATH_MSG_VERBOSE("unsupported systematic variation: "<< sSyst.basename()<<"; skipping this one");
      continue;
    }


    if (sSyst.parameter() * dDirection < 0)
    {
      ATH_MSG_ERROR("unsupported set of systematic variations, you should either use only \"UP\" or only \"DOWN\" systematics in one set!");
      ATH_MSG_ERROR("systematic set will not be applied");
      return StatusCode::FAILURE;
    }
    dDirection = sSyst.parameter();

    if ((m_sRecommendedSystematics.find(sSyst.basename()) != m_sRecommendedSystematics.end()) and sSystematicSet.size() > 1)
    {
      ATH_MSG_ERROR("unsupported set of systematic variations, you should not combine \"TAUS_{TRUE|FAKE}_EFF_*_TOTAL\" with other systematic variations!");
      ATH_MSG_ERROR("systematic set will not be applied");
      return StatusCode::FAILURE;
    }

    // finally add the systematic to the set of systematics to process
    sSystematicSetAvailable.insert(sSyst);
  }

  // store this calibration for future use, and make it current
  m_sSystematicSet = &m_mSystematicSets.insert(std::pair<CP::SystematicSet,std::string>(sSystematicSetAvailable, sSystematicSet.name())).first->first;

  return StatusCode::SUCCESS;
}

//=================================PRIVATE-PART=================================
/*
  prongness converter, note that it returns "_3p" for all values, except
  fProngness==1, i.e. for 0, 2, 3, 4, 5...
 */
//______________________________________________________________________________
std::string CommonEfficiencyTool::ConvertProngToString(const int fProngness) const
{
  std::string prong = "";
  if (fProngness == 0)
    ATH_MSG_DEBUG("passed tau with 0 tracks, which is not supported, taking multiprong SF for now");
  fProngness == 1 ? prong = "_1p" : prong = "_3p";
  return prong;
}

/*
  mu converter, returns "_highMu" for average number of vertices higher than 35 and
  "_lowMu" for everything below
*/
//______________________________________________________________________________
std::string CommonEfficiencyTool::ConvertMuToString(const int iMu) const
{
  if (iMu > 35 )
    return "_highMu";

  return "_lowMu";
}

/*
  run number converter, first checks if m_sMCCampaign is set. If yes, use it. 
  If not, use random run number to determine MC campaign 
*/
//______________________________________________________________________________
std::string CommonEfficiencyTool::GetMcCampaignString(const int iRunNumber) const
{
  if (m_sMCCampaign == "MC16a" || m_sMCCampaign == "MC16d")
    return std::string("_")+m_sMCCampaign;
  // FIXME?
  else if (m_sMCCampaign == "MC16e")
    return "_MC16d"; // MC16e recommendations not available yet, use MC16d instead
  else if (m_sMCCampaign != "")
    ATH_MSG_WARNING("unsupported mc campaign: " << m_sMCCampaign);

  // FIXME?
  if (iRunNumber > 324320 )
    return "_MC16d";

  return "_MC16a";
}

/*
  decay mode converter
*/
//______________________________________________________________________________
std::string CommonEfficiencyTool::ConvertDecayModeToString(const int iDecayMode) const
{
  switch(iDecayMode)
  {
    case xAOD::TauJetParameters::DecayMode::Mode_1p0n:
      return "_r1p0n";
    case xAOD::TauJetParameters::DecayMode::Mode_1p1n:
      return "_r1p1n";
    case xAOD::TauJetParameters::DecayMode::Mode_1pXn:
      return "_r1pXn";
    case xAOD::TauJetParameters::DecayMode::Mode_3p0n:
      return "_r3p0n";
    case xAOD::TauJetParameters::DecayMode::Mode_3pXn:
      return "_r3pXn";
    default:
      return "";
  }
}

/*
  Read in a root file and store all objects to a map of this type:
  std::map<std::string, tTupleObjectFunc > (see header) It's basically a map of
  the histogram name and a function pointer based on the TObject type (TH1F,
  TH1D, TF1). This is resolved in the function:
  - CommonEfficiencyTool::addHistogramToSFMap
  Further this function figures out the axis definition (see description on the
  top)
*/
//______________________________________________________________________________
void CommonEfficiencyTool::ReadInputs(const TFile& fFile)
{
  m_mSF->clear();

  // initialize function pointer
  m_fX = &finalTauPt;
  m_fY = &finalTauEta;

  TKey *kKey;
  TIter itNext(fFile.GetListOfKeys());
  while ((kKey = (TKey*)itNext()))
  {
    // parse file content for objects of type TNamed, check their title for
    // known strings and reset funtion pointer
    std::string sKeyName = kKey->GetName();
    if (sKeyName == "Xaxis")
    {
      TNamed* tObj = (TNamed*)kKey->ReadObj();
      std::string sTitle = tObj->GetTitle();
      delete tObj;
      if (sTitle == "P" || sTitle == "PFinalCalib")
      {
        m_fX = &finalTauP;
        ATH_MSG_DEBUG("using full momentum for x-axis");
      }
      if (sTitle == "TruthDecayMode")
      {
        m_fX = &truthDecayMode;
        ATH_MSG_DEBUG("using truth decay mode for x-axis");
      }
      if (sTitle == "truth pt")
      {
        m_fX = &truthTauPt;
        ATH_MSG_DEBUG("using truth pT for x-axis");
      }
      if (sTitle == "|eta|")
      {
        m_fX = &finalTauAbsEta;
        ATH_MSG_DEBUG("using absolute tau eta for x-axis");
      }

      continue;
    }
    else if (sKeyName == "Yaxis")
    {
      TNamed* tObj = (TNamed*)kKey->ReadObj();
      std::string sTitle = tObj->GetTitle();
      delete tObj;
      if (sTitle == "track-eta")
      {
        m_fY = &tauLeadTrackEta;
        ATH_MSG_DEBUG("using leading track eta for y-axis");
      }
      else if (sTitle == "|eta|")
      {
        m_fY = &finalTauAbsEta;
        ATH_MSG_DEBUG("using absolute tau eta for y-axis");
      }
      else if (sTitle == "mu")
      {
	m_fY = [this](const xAOD::TauJet&) -> double {
          const xAOD::EventInfo* xEventInfo = nullptr;
          if (evtStore()->retrieve(xEventInfo,"EventInfo").isFailure()) {
            return 0;
          }
          if (xEventInfo->runNumber()==284500)
          {
            return xEventInfo->averageInteractionsPerCrossing();
          }
          else if (xEventInfo->runNumber()==300000 || xEventInfo->runNumber()==310000)
          {
            return xEventInfo->actualInteractionsPerCrossing();
          }
          return 0;
        };
	ATH_MSG_DEBUG("using average mu for y-axis");
      }
      else if (sTitle == "truth |eta|")
      {
        m_fY = &truthTauAbsEta;
        ATH_MSG_DEBUG("using absolute truth tau eta for y-axis");
      }
      continue;
    }

    std::vector<std::string> vSplitName = {};
    split(sKeyName,'_',vSplitName);
    if (vSplitName[0] == "sf")
    {
      addHistogramToSFMap(kKey, sKeyName);
    }
    else
    {
      // std::string sDirection = vSplitName[1];
      if (sKeyName.find("_up_") != std::string::npos or sKeyName.find("_down_") != std::string::npos)
        addHistogramToSFMap(kKey, sKeyName);
      else
      {
        size_t iPos = sKeyName.find('_');
        addHistogramToSFMap(kKey, sKeyName.substr(0,iPos)+"_up"+sKeyName.substr(iPos));
        addHistogramToSFMap(kKey, sKeyName.substr(0,iPos)+"_down"+sKeyName.substr(iPos));
      }
    }
  }
  ATH_MSG_INFO("data loaded from " << fFile.GetName());
}

/*
  Create the tuple objects for the map
*/
//______________________________________________________________________________
void CommonEfficiencyTool::addHistogramToSFMap(TKey* kKey, const std::string& sKeyName)
{
  // handling for the 3 different input types TH1F/TH1D/TF1, function pointer
  // handle the access methods for the final scale factor retrieval
  TClass *cClass = gROOT->GetClass(kKey->GetClassName());
  if (cClass->InheritsFrom("TH2"))
  {
    TH1* oObject = (TH1*)kKey->ReadObj();
    oObject->SetDirectory(0);
    (*m_mSF)[sKeyName] = tTupleObjectFunc(oObject,&getValueTH2);
    ATH_MSG_DEBUG("added histogram with name "<<sKeyName);
  }
  else if (cClass->InheritsFrom("TH3"))
  {
    TH1* oObject = (TH1*)kKey->ReadObj();
    oObject->SetDirectory(0);
    (*m_mSF)[sKeyName] = tTupleObjectFunc(oObject,&getValueTH3);
    ATH_MSG_DEBUG("added histogram with name "<<sKeyName);
  }else if (cClass->InheritsFrom("TH1"))
  {
    TH1* oObject = (TH1*)kKey->ReadObj();
    oObject->SetDirectory(0);
    (*m_mSF)[sKeyName] = tTupleObjectFunc(oObject,&getValueTH1);
    ATH_MSG_DEBUG("added histogram with name "<<sKeyName);
  }
  else if (cClass->InheritsFrom("TF1"))
  {
    TObject* oObject = kKey->ReadObj();
    (*m_mSF)[sKeyName] = tTupleObjectFunc(oObject,&getValueTF1);
    ATH_MSG_DEBUG("added function with name "<<sKeyName);
  }
  else
  {
    ATH_MSG_DEBUG("ignored object with name "<<sKeyName);
  }
}

/*
  This function parses the names of the obejects from the input file and
  generates the systematic sets and defines which ones are recommended or only
  available. It also checks, based on the root file name, on which tau it needs
  to be applied, e.g. only on reco taus coming from true taus or on those faked
  by true electrons...

  Examples:
  filename: Reco_TrueHadTau_2016-ichep.root -> apply only to true taus
  histname: sf_1p -> nominal 1p scale factor
  histname: TOTAL_3p -> "total" 3p NP, recommended
  histname: afii_1p -> "total" 3p NP, not recommended, but available
*/
//______________________________________________________________________________
void CommonEfficiencyTool::generateSystematicSets()
{
  // creation of basic string for all NPs, e.g. "TAUS_TRUEHADTAU_EFF_RECO_"
  std::vector<std::string> vSplitInputFilePath = {};
  split(m_sInputFileName,'_',vSplitInputFilePath);
  std::string sEfficiencyType = vSplitInputFilePath.at(0);
  std::string sTruthType = vSplitInputFilePath.at(1);
  std::transform(sEfficiencyType.begin(), sEfficiencyType.end(), sEfficiencyType.begin(), toupper);
  std::transform(sTruthType.begin(), sTruthType.end(), sTruthType.begin(), toupper);
  std::string sSystematicBaseString = "TAUS_"+sTruthType+"_EFF_"+sEfficiencyType+"_";

  // set truth type to check for in truth matching
  if (sTruthType=="TRUEHADTAU") m_eCheckTruth = TauAnalysisTools::TruthHadronicTau;
  else if (sTruthType=="TRUEELECTRON") m_eCheckTruth = TauAnalysisTools::TruthElectron;
  else if (sTruthType=="TRUEMUON") m_eCheckTruth = TauAnalysisTools::TruthMuon;
  else if (sTruthType=="TRUEJET") m_eCheckTruth = TauAnalysisTools::TruthJet;
  else if (sTruthType=="TRUEHADDITAU") m_eCheckTruth = TauAnalysisTools::TruthHadronicDiTau;
  // 3p eVeto, still need this to be measurable in T&P
  if (sEfficiencyType=="ELERNN" || sEfficiencyType=="ELEOLR") m_bNoMultiprong = true;

  for (auto mSF : *m_mSF)
  {
    // parse for nuisance parameter in histogram name
    std::vector<std::string> vSplitNP = {};
    split(mSF.first,'_',vSplitNP);
    std::string sNP = vSplitNP.at(0);
    std::string sNPUppercase = vSplitNP.at(0);

    // skip nominal scale factors
    if (sNP == "sf") continue;

    // skip if 3p histogram to avoid duplications (TODO: come up with a better solution)
    //if (mSF.first.find("_3p") != std::string::npos) continue;

    // test if NP starts with a capital letter indicating that this should be recommended
    bool bIsRecommended = false;
    if (isupper(sNP.at(0)) || isupper(sNP.at(1)))
      bIsRecommended = true;

    // make sNP uppercase and build final NP entry name
    std::transform(sNPUppercase.begin(), sNPUppercase.end(), sNPUppercase.begin(), toupper);
    std::string sSystematicString = sSystematicBaseString+sNPUppercase;
  
    // add all found systematics to the AffectingSystematics
    m_sAffectingSystematics.insert(CP::SystematicVariation (sSystematicString, 1));
    m_sAffectingSystematics.insert(CP::SystematicVariation (sSystematicString, -1));
    // only add found uppercase systematics to the RecommendedSystematics
    if (bIsRecommended)
    {
      m_sRecommendedSystematics.insert(CP::SystematicVariation (sSystematicString, 1));
      m_sRecommendedSystematics.insert(CP::SystematicVariation (sSystematicString, -1));
    }

    ATH_MSG_DEBUG("connected base name " << sNP << " with systematic " <<sSystematicString);
    m_mSystematicsHistNames.insert({sSystematicString,sNP});
  }
}

/*
  return value from the tuple map object based on the pt/eta values (or the
  corresponding value in case of configuration)
*/
//______________________________________________________________________________
CP::CorrectionCode CommonEfficiencyTool::getValue(const std::string& sHistName,
    const xAOD::TauJet& xTau,
    double& dEfficiencyScaleFactor) const
{
  const tSFMAP& mSF = *m_mSF;
  auto it = mSF.find (sHistName);
  if (it == mSF.end())
  {
    ATH_MSG_ERROR("Object with name "<<sHistName<<" was not found in input file.");
    ATH_MSG_DEBUG("Content of input file");
    for (auto eEntry : mSF)
      ATH_MSG_DEBUG("  Entry: "<<eEntry.first);
    return CP::CorrectionCode::Error;
  }

  // get a tuple (TObject*,functionPointer) from the scale factor map
  tTupleObjectFunc tTuple = it->second;

  // get pt and eta (for x and y axis respectively)
  double dPt = m_fX(xTau);
  double dEta = m_fY(xTau);

  double dVars[2] = {dPt, dEta};

  // finally obtain efficiency scale factor from TH1F/TH1D/TF1, by calling the
  // function pointer stored in the tuple from the scale factor map
  return  (std::get<1>(tTuple))(std::get<0>(tTuple), dEfficiencyScaleFactor, dVars);
}

/*
  find the particular value in TH1 depending on pt (or the
  corresponding value in case of configuration)
  Note: In case values are outside of bin ranges, the closest bin value is used
*/
//______________________________________________________________________________
CP::CorrectionCode CommonEfficiencyTool::getValueTH1(const TObject* oObject,
    double& dEfficiencyScaleFactor, double dVars[])
{
  double dPt = dVars[0];

  const TH1* hHist = dynamic_cast<const TH1*>(oObject);

  if (!hHist)
  {
    // ATH_MSG_ERROR("Problem with casting TObject of type "<<oObject->ClassName()<<" to TH2F");
    return CP::CorrectionCode::Error;
  }

  // protect values from underflow bins
  dPt = std::max(dPt,hHist->GetXaxis()->GetXmin());
  // protect values from overflow bins (times .999 to keep it inside last bin)
  dPt = std::min(dPt,hHist->GetXaxis()->GetXmax() * .999);

  // get bin from TH2 depending on x and y values; finally set the scale factor
  int iBin = hHist->FindFixBin(dPt);
  dEfficiencyScaleFactor = hHist->GetBinContent(iBin);
  return CP::CorrectionCode::Ok;
}

/*
  find the particular value in TH2 depending on pt and eta (or the
  corresponding value in case of configuration)
  Note: In case values are outside of bin ranges, the closest bin value is used
*/
//______________________________________________________________________________
CP::CorrectionCode CommonEfficiencyTool::getValueTH2(const TObject* oObject,
    double& dEfficiencyScaleFactor, double dVars[])
{
  double dPt = dVars[0];
  double dEta = dVars[1];

  const TH2* hHist = dynamic_cast<const TH2*>(oObject);

  if (!hHist)
  {
    // ATH_MSG_ERROR("Problem with casting TObject of type "<<oObject->ClassName()<<" to TH2F");
    return CP::CorrectionCode::Error;
  }

  // protect values from underflow bins
  dPt = std::max(dPt,hHist->GetXaxis()->GetXmin());
  dEta = std::max(dEta,hHist->GetYaxis()->GetXmin());
  // protect values from overflow bins (times .999 to keep it inside last bin)
  dPt = std::min(dPt,hHist->GetXaxis()->GetXmax() * .999);
  dEta = std::min(dEta,hHist->GetYaxis()->GetXmax() * .999);

  // get bin from TH2 depending on x and y values; finally set the scale factor
  int iBin = hHist->FindFixBin(dPt,dEta);
  dEfficiencyScaleFactor = hHist->GetBinContent(iBin);
  return CP::CorrectionCode::Ok;
}

/*
  find the particular value in TH3 depending on x, y, z
  Note: In case values are outside of bin ranges, the closest bin value is used
*/
//______________________________________________________________________________
CP::CorrectionCode CommonEfficiencyTool::getValueTH3(const TObject* oObject,
    double& dEfficiencyScaleFactor, double dVars[])
{
  double dX = dVars[0];
  double dY = dVars[1];
  double dZ = dVars[2];

  const TH3* hHist = dynamic_cast<const TH3*>(oObject);

  if (!hHist)
  {
    // ATH_MSG_ERROR("Problem with casting TObject of type "<<oObject->ClassName()<<" to TH2D");
    return CP::CorrectionCode::Error;
  }

  // protect values from underflow bins
  dX = std::max(dX,hHist->GetXaxis()->GetXmin());
  dY = std::max(dY,hHist->GetYaxis()->GetXmin());
  dZ = std::max(dZ,hHist->GetZaxis()->GetXmin());
  // protect values from overflow bins (times .999 to keep it inside last bin)
  dX = std::min(dX,hHist->GetXaxis()->GetXmax() * .999);
  dY = std::min(dY,hHist->GetYaxis()->GetXmax() * .999);
  dZ = std::min(dZ,hHist->GetZaxis()->GetXmax() * .999);

  // get bin from TH2 depending on x and y values; finally set the scale factor
  int iBin = hHist->FindFixBin(dX,dY,dZ);
  dEfficiencyScaleFactor = hHist->GetBinContent(iBin);
  return CP::CorrectionCode::Ok;
}

/*
  Find the particular value in TF1 depending on pt and eta (or the corresponding
  value in case of configuration)
*/
//______________________________________________________________________________
CP::CorrectionCode CommonEfficiencyTool::getValueTF1(const TObject* oObject,
    double& dEfficiencyScaleFactor, double dVars[])
{
  double dPt = dVars[0];
  double dEta = dVars[1];

  const TF1* fFunc = static_cast<const TF1*>(oObject);

  if (!fFunc)
  {
    // ATH_MSG_ERROR("Problem with casting TObject of type "<<oObject->ClassName()<<" to TF1");
    return CP::CorrectionCode::Error;
  }

  // evaluate TFunction and set scale factor
  dEfficiencyScaleFactor = fFunc->Eval(dPt, dEta);
  return CP::CorrectionCode::Ok;
}
