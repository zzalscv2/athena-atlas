/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "PATInterfaces/SystematicRegistry.h"
#include "xAODMetaData/FileMetaData.h"

// Local include(s):
#include "TauAnalysisTools/TauSmearingTool.h"
#include "TauAnalysisTools/SharedFilesVersion.h"

#include <algorithm>

namespace TauAnalysisTools
{

TauSmearingTool::TauSmearingTool( const std::string& sName )
  : asg::AsgMetadataTool( sName )
  , m_tCommonSmearingTool(sName+"_CommonSmearingTool", this)
{
  declareProperty( "InputFilePath",           m_sInputFilePath = "" );
  declareProperty( "RecommendationTag",       m_sRecommendationTag = "2022-prerec" );
  declareProperty( "Campaign",                m_sCampaign = "mc21" );
  declareProperty( "Generator",               m_sGenerator = "PoPy" );  
  declareProperty( "SkipTruthMatchCheck",     m_bSkipTruthMatchCheck = false );
  declareProperty( "ApplyFading",             m_bApplyFading = true );
  declareProperty( "ApplyMVATESQualityCheck", m_bApplyMVATESQualityCheck = false );
  declareProperty( "ApplyInsituCorrection",   m_bApplyInsituCorrection = true );
  declareProperty( "isAFII",	              m_sAFII = false );
}

TauSmearingTool::~TauSmearingTool()
{
}

StatusCode TauSmearingTool::initialize()
{
  ATH_MSG_INFO( "Initializing TauSmearingTool" );

  if (m_bSkipTruthMatchCheck)
    ATH_MSG_WARNING("Truth match check will be skipped. This is ONLY FOR TESTING PURPOSE!");

  if (m_sInputFilePath.empty()) {
    std::string sDirectory = "TauAnalysisTools/" + std::string(sSharedFilesVersion) + "/Smearing/";
    if (m_sRecommendationTag == "2019-summer") {
      if (m_sAFII) m_sInputFilePath = sDirectory+"TES_TrueHadTau_2019-summer_AFII.root";
      else m_sInputFilePath = sDirectory+"TES_TrueHadTau_2019-summer.root";
    } else if (m_sRecommendationTag == "2022-prerec") {

      if (m_sCampaign!="mc21" && m_sCampaign!="mc20"){
        ATH_MSG_ERROR("unknown campaign (mc20|mc21):" << m_sCampaign);
        return StatusCode::FAILURE;
      }

      if (m_sGenerator!="PoPy" && m_sCampaign!="Sherpa"){
        ATH_MSG_ERROR("unknown generator tag (PoPy|Sherpa):" << m_sCampaign);
        return StatusCode::FAILURE;
      }

      if (m_sGenerator == "PoPy" && m_sCampaign=="mc20") m_sInputFilePath = sDirectory+"TES_TrueHadTau_PoPy8_mc20-prerec_v2.root";
      if (m_sCampaign=="mc21") {
        m_sInputFilePath = sDirectory+"TES_TrueHadTau_PoPy8_mc21-prerec_v2.root";
        if (m_sGenerator=="Sherpa")ATH_MSG_WARNING("No Sherpa mc21 recommendations available yet, using PoPy8!");
      }
      if (m_sGenerator == "Sherpa" && m_sCampaign=="mc20") m_sInputFilePath = sDirectory+"TES_TrueHadTau_Sherpa2211-prerec_v2.root";
    }
    else {
      ATH_MSG_ERROR("unknown recommendation tag " << m_sRecommendationTag);
      return StatusCode::FAILURE;
    }
  }
  ATH_CHECK(ASG_MAKE_ANA_TOOL(m_tCommonSmearingTool, TauAnalysisTools::CommonSmearingTool));
  ATH_CHECK(m_tCommonSmearingTool.setProperty("InputFilePath", m_sInputFilePath));
  ATH_CHECK(m_tCommonSmearingTool.setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
  ATH_CHECK(m_tCommonSmearingTool.setProperty("ApplyFading", m_bApplyFading));
  ATH_CHECK(m_tCommonSmearingTool.setProperty("ApplyMVATESQualityCheck", m_bApplyMVATESQualityCheck));
  ATH_CHECK(m_tCommonSmearingTool.setProperty("ApplyInsituCorrection", m_bApplyInsituCorrection));
  ATH_CHECK(m_tCommonSmearingTool.setProperty("OutputLevel", this->msg().level()));
  ATH_CHECK(m_tCommonSmearingTool.initialize());

  // Add the affecting systematics to the global registry
  CP::SystematicRegistry& registry = CP::SystematicRegistry::getInstance();
  if (!registry.registerSystematics(*this)) {
    ATH_MSG_ERROR ("Unable to register the systematics");
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}

// auto detection of simulation flavour, used to cross check configuration of tool
//______________________________________________________________________________
StatusCode TauSmearingTool::beginInputFile()
{
  if (inputMetaStore()->contains<xAOD::FileMetaData>("FileMetaData")) {
    const xAOD::FileMetaData* fmd = nullptr;
    ATH_CHECK( inputMetaStore()->retrieve( fmd, "FileMetaData" ) );
    std::string simType("");
    bool result = fmd->value( xAOD::FileMetaData::simFlavour , simType );
    // if no result -> no simFlavor metadata, so must be data
    if(result)  std::transform(simType.begin(), simType.end(), simType.begin(), ::toupper);

    if (simType.find("ATLFASTII")!=std::string::npos && !m_sAFII)
      ATH_MSG_WARNING("Input file is fast simulation but you are _not_ using AFII corrections and uncertainties, you should set \"isAFII\" to \"true\"");
    else if (simType.find("FULLG4")!=std::string::npos && m_sAFII)
      ATH_MSG_WARNING("Input file is full simulation but you are using AFII corrections and uncertainties, you should set \"isAFII\" to \"false\"");
  }

  return StatusCode::SUCCESS;
}

CP::CorrectionCode TauSmearingTool::applyCorrection( xAOD::TauJet& xTau ) const
{
  return m_tCommonSmearingTool->applyCorrection(xTau);
}

CP::CorrectionCode TauSmearingTool::correctedCopy( const xAOD::TauJet& input, xAOD::TauJet*& output ) const
{
  return m_tCommonSmearingTool->correctedCopy(input, output);
}

/// returns: whether this tool is affected by the given systematis
bool TauSmearingTool::isAffectedBySystematic( const CP::SystematicVariation& systematic ) const
{
  return m_tCommonSmearingTool->isAffectedBySystematic( systematic );
}

/// returns: the list of all systematics this tool can be affected by
CP::SystematicSet TauSmearingTool::affectingSystematics() const
{
  return m_tCommonSmearingTool->affectingSystematics();
}

/// returns: the list of all systematics this tool recommends to use
CP::SystematicSet TauSmearingTool::recommendedSystematics() const
{
  return m_tCommonSmearingTool->recommendedSystematics();
}
  
StatusCode TauSmearingTool::applySystematicVariation ( const CP::SystematicSet& sSystematicSet )
{
  return m_tCommonSmearingTool->applySystematicVariation( sSystematicSet );
}


} // namespace TauAnalysisTools
