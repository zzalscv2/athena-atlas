/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Framework include(s):
#include "PathResolver/PathResolver.h"

// local include(s)
#include "TauAnalysisTools/TauEfficiencyTriggerTool.h"
#include "TauAnalysisTools/TauEfficiencyCorrectionsTool.h"

// ROOT include(s)
#include "TFile.h"

using namespace TauAnalysisTools;

//______________________________________________________________________________
TauEfficiencyTriggerTool::TauEfficiencyTriggerTool(const std::string& sName)
  : CommonEfficiencyTool ( sName )
{
  m_mSystematics = {};
  declareProperty( "MinRunNumber", m_iMinRunNumber = 0 );
  declareProperty( "MaxRunNumber", m_iMaxRunNumber = 0 );
}

//______________________________________________________________________________
StatusCode TauEfficiencyTriggerTool::initialize()
{
  ATH_MSG_INFO( "Initializing TauEfficiencyTriggerTool" );
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

  std::vector<std::string> vInputFilePath;
  split(m_sInputFilePath,'/',vInputFilePath);
  m_sInputFileName = vInputFilePath.back();
  generateSystematicSets();

  // load empty systematic variation by default
  if (applySystematicVariation(CP::SystematicSet()) != StatusCode::SUCCESS )
    return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}

//______________________________________________________________________________
CP::CorrectionCode TauEfficiencyTriggerTool::getEfficiencyScaleFactor(const xAOD::TauJet& xTau,
    double& dEfficiencyScaleFactor, unsigned int /*iRunNumber*/, unsigned int /*iMu*/)
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

  // get prong extension for histogram name
  std::string sProng = ConvertProngToString(xTau.nTracks());

  // get standard scale factor
  CP::CorrectionCode tmpCorrectionCode;
  tmpCorrectionCode = getValue(m_sSFHistName+"_all_"+m_sWP+sProng,
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

    // dirty hack, replace urgently
    std::string sDirection = (syst.parameter() > 0) ? "_up" : "_down";

    // get uncertainty value
    double dUncertaintySyst = 0.;
    tmpCorrectionCode = getValue(it->second+sDirection+"_all_"+m_sWP+sProng,
                                 xTau,
                                 dUncertaintySyst);

    // return correction code if histogram is not available
    if (tmpCorrectionCode != CP::CorrectionCode::Ok)
      return tmpCorrectionCode;

    // needed for up/down decision
    dDirection = syst.parameter();

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

//______________________________________________________________________________
StatusCode TauEfficiencyTriggerTool::applySystematicVariation ( const CP::SystematicSet& sSystematicSet)
{
  // first check if we already know this systematic configuration
  auto itSystematicSet = m_mSystematicSets.find(sSystematicSet);
  if (itSystematicSet != m_mSystematicSets.end())
  {
    m_sSystematicSet = &itSystematicSet->first;
    return StatusCode::SUCCESS;
  }

  // sanity checks if systematic set is supported
  // double dDirection = 0;
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

    // if (sSyst.parameter() * dDirection < 0)
    // {
    //   ATH_MSG_ERROR("unsupported set of systematic variations, you should either use only \"UP\" or only \"DOWN\" systematics in one set!");
    //   ATH_MSG_ERROR("systematic set will not be applied");
    //   return StatusCode::FAILURE;
    // }
    // dDirection = sSyst.parameter();

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

//______________________________________________________________________________
bool TauEfficiencyTriggerTool::isSupportedRunNumber(int iRunNumber) const
{
  if ( m_iMinRunNumber != 0 and m_iMinRunNumber > iRunNumber)
    return false;
  if ( m_iMaxRunNumber != 0 and m_iMaxRunNumber < iRunNumber)
    return false;
  return true;
}

