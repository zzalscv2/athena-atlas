/**
 *
 * @copyright Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 *
 * @file DiTauEfficiencyCorrectionsTool.cxx
 * @brief Class for ditau efficiency correction scale factors and uncertainties
 * @date 2021-02-18
 *
 */

// EDM include(s):
#include "PATInterfaces/SystematicRegistry.h"
#include "xAODEventInfo/EventInfo.h"

// Local include(s):
#include "TauAnalysisTools/DiTauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/SharedFilesVersion.h"


namespace TauAnalysisTools
{

//______________________________________________________________________________
DiTauEfficiencyCorrectionsTool::DiTauEfficiencyCorrectionsTool( const std::string& sName )
  : asg::AsgMetadataTool( sName )
  , m_vCommonEfficiencyTools()
  , m_bIsData(false)
  , m_bIsConfigured(false)
{
  declareProperty( "EfficiencyCorrectionTypes",    m_vEfficiencyCorrectionTypes    = {} );
  declareProperty( "InputFilePathJetIDHadTau",     m_sInputFilePathJetIDHadTau     = "" );
  declareProperty( "VarNameJetIDHadTau",           m_sVarNameJetIDHadTau           = "" );
  declareProperty( "RecommendationTag",            m_sRecommendationTag            = "2017-moriond" );
  declareProperty( "JetIDLevel",                   m_iJetIDLevel                   = (int)JETIDBDTTIGHT );
  declareProperty( "SkipTruthMatchCheck",          m_bSkipTruthMatchCheck          = false );
}


//______________________________________________________________________________
DiTauEfficiencyCorrectionsTool::~DiTauEfficiencyCorrectionsTool()
{
  for (auto tTool : m_vCommonEfficiencyTools)
    delete tTool;
}


//______________________________________________________________________________
StatusCode DiTauEfficiencyCorrectionsTool::initialize()
{
  // Greet the user:
  ATH_MSG_INFO( "Initializing DiTauEfficiencyCorrectionsTool" );

  if (m_bSkipTruthMatchCheck)
    ATH_MSG_WARNING("Truth match check will be skipped. This is ONLY FOR TESTING PURPOSE!");

  // configure default set of variations if not set by the constructor using TauSelectionTool or the user
  if ((m_sRecommendationTag== "2017-moriond") and m_vEfficiencyCorrectionTypes.empty())
    m_vEfficiencyCorrectionTypes = {SFJetIDHadTau
                                   };

  if (m_sRecommendationTag == "2017-moriond")
    ATH_CHECK(initializeTools_2017_moriond());
  else
  {
    ATH_MSG_FATAL("Unknown RecommendationTag: "<<m_sRecommendationTag);
    return StatusCode::FAILURE;
  }

  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
  {
    ATH_CHECK((**it).setProperty("OutputLevel", this->msg().level()));
    ATH_CHECK((**it).initialize());
  }

  // Add the affecting systematics to the global registry
  CP::SystematicRegistry& registry = CP::SystematicRegistry::getInstance();
  if (!registry.registerSystematics(*this))
  {
    ATH_MSG_ERROR ("Unable to register the systematics");
    return StatusCode::FAILURE;
  }

  printConfig();

  return StatusCode::SUCCESS;
}


//______________________________________________________________________________
StatusCode DiTauEfficiencyCorrectionsTool::beginEvent()
{
  if (!m_bIsConfigured)
  {
    const xAOD::EventInfo* xEventInfo = nullptr;
    ATH_CHECK(evtStore()->retrieve(xEventInfo,"EventInfo"));
    m_bIsData = !(xEventInfo->eventType( xAOD::EventInfo::IS_SIMULATION));
    m_bIsConfigured = true;
  }

  return StatusCode::SUCCESS;
}


//______________________________________________________________________________
void DiTauEfficiencyCorrectionsTool::printConfig() const
{
  ATH_MSG_DEBUG( "DiTauEfficiencyCorrectionsTool with name " << name() << " is configured as follows:" );
  for (auto iEfficiencyCorrectionType : m_vEfficiencyCorrectionTypes) {
    ATH_MSG_DEBUG( "  EfficiencyCorrectionTypes " << iEfficiencyCorrectionType );
  }
  ATH_MSG_DEBUG( "  InputFilePathJetIDHadTau " << m_sInputFilePathJetIDHadTau );
  ATH_MSG_DEBUG( "  VarNameJetIDHadTau " << m_sVarNameJetIDHadTau );
  ATH_MSG_DEBUG( "  RecommendationTag " << m_sRecommendationTag );
}

//______________________________________________________________________________
CP::CorrectionCode DiTauEfficiencyCorrectionsTool::getEfficiencyScaleFactor( const xAOD::DiTauJet& xDiTau,
    double& eff, unsigned int /*iRunNumber*/, unsigned int /*iMu*/ )
{
  eff = 1.;

  if (m_bIsData)
    return CP::CorrectionCode::Ok;

  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
  {
    double dToolEff = 1.;
    CP::CorrectionCode tmpCorrectionCode = (**it)->getEfficiencyScaleFactor(xDiTau, dToolEff);
    if (tmpCorrectionCode != CP::CorrectionCode::Ok)
      return tmpCorrectionCode;
    eff *= dToolEff;
  }
  return CP::CorrectionCode::Ok;
}

//______________________________________________________________________________
CP::CorrectionCode DiTauEfficiencyCorrectionsTool::applyEfficiencyScaleFactor( const xAOD::DiTauJet& xDiTau, 
  unsigned int iRunNumber, unsigned int iMu)
{
  if (m_bIsData)
    return CP::CorrectionCode::Ok;

  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
  {
    CP::CorrectionCode tmpCorrectionCode = (**it)->applyEfficiencyScaleFactor(xDiTau, iRunNumber, iMu);
    if (tmpCorrectionCode != CP::CorrectionCode::Ok)
    {
      return tmpCorrectionCode;
    }
  }
  return CP::CorrectionCode::Ok;
}

/// returns: whether this tool is affected by the given systematis
//______________________________________________________________________________
bool DiTauEfficiencyCorrectionsTool::isAffectedBySystematic( const CP::SystematicVariation& systematic ) const
{
  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
    if ((**it)->isAffectedBySystematic(systematic))
      return true;
  return false;
}

/// returns: the list of all systematics this tool can be affected by
//______________________________________________________________________________
CP::SystematicSet DiTauEfficiencyCorrectionsTool::affectingSystematics() const
{
  CP::SystematicSet sAffectingSystematics;
  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
    sAffectingSystematics.insert((**it)->affectingSystematics());
  return sAffectingSystematics;
}

/// returns: the list of all systematics this tool recommends to use
//______________________________________________________________________________
CP::SystematicSet DiTauEfficiencyCorrectionsTool::recommendedSystematics() const
{
  CP::SystematicSet sRecommendedSystematics;
  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
  {
    sRecommendedSystematics.insert((**it)->recommendedSystematics());
  }
  return sRecommendedSystematics;
}

//______________________________________________________________________________
StatusCode DiTauEfficiencyCorrectionsTool::applySystematicVariation ( const CP::SystematicSet& sSystematicSet)
{
  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
    if ((**it)->applySystematicVariation(sSystematicSet) == StatusCode::FAILURE)
    {
      return StatusCode::FAILURE;
    }
  return StatusCode::SUCCESS;
}

//=================================PRIVATE-PART=================================

//______________________________________________________________________________
StatusCode DiTauEfficiencyCorrectionsTool::initializeTools_2017_moriond()
{
  std::string sDirectory = "TauAnalysisTools/" + std::string(sSharedFilesVersion) + "/EfficiencyCorrections/";
  for (auto iEfficiencyCorrectionType : m_vEfficiencyCorrectionTypes)
  {
    if (iEfficiencyCorrectionType == SFJetIDHadTau)
    {
      // only set vars if they have been configured by the user
      if (m_sInputFilePathJetIDHadTau.empty()) m_sInputFilePathJetIDHadTau = sDirectory+"JetID_TrueHadDiTau_2017-fall.root";
      if (m_sVarNameJetIDHadTau.empty()) m_sVarNameJetIDHadTau = "DiTauScaleFactorJetIDHadTau";

      asg::AnaToolHandle<IDiTauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<IDiTauEfficiencyCorrectionsTool>("JetIDHadTauTool", this);
      m_vCommonEfficiencyTools.push_back(tTool);
      ATH_CHECK(ASG_MAKE_ANA_TOOL(*tTool, TauAnalysisTools::CommonDiTauEfficiencyTool));
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathJetIDHadTau));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameJetIDHadTau));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
      ATH_CHECK(tTool->setProperty("WP", ConvertJetIDToString(m_iJetIDLevel)));
    }
    else
    {
      ATH_MSG_WARNING("unsupported EfficiencyCorrectionsType with enum " << iEfficiencyCorrectionType);
    }
  }
  return StatusCode::SUCCESS;
}

//______________________________________________________________________________
std::string DiTauEfficiencyCorrectionsTool::ConvertJetIDToString(const int iLevel) const
{
  switch(iLevel)
  {
  case JETIDNONE:
    return "ditaureconstruction";
    break;
  case JETIDBDTVERYLOOSE:
    return "jetbdtsigveryloose";
    break;
  case JETIDBDTLOOSE:
    return "jetbdtsigloose";
    break;
  case JETIDBDTMEDIUM:
    return "jetbdtsigmedium";
    break;
  case JETIDBDTTIGHT:
    return "jetbdtsigtight";
    break;
  default:
    assert(false && "No valid ID level passed. Breaking up ...");
    break;
  }
  return "";
}


} // namespace TauAnalysisTools
