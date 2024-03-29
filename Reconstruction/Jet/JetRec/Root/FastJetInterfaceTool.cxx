/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "fastjet/ClusterSequenceArea.hh"
#include "xAODEventInfo/EventInfo.h"


//#include "EventInfo/EventID.h"
#include "CxxUtils/crc64.h"

#include "JetRec/FastJetInterfaceTool.h"

using namespace FastJetInterface;

///////////////
// Constants //
///////////////

// process control
const std::string FastJetInterfaceTool::m_invalidKeyReference = "INVALID_KEY";
const unsigned int FastJetInterfaceTool::m_failedExecCtrMax = 10;

////////////////////////
// Constant Accessors //
////////////////////////

// jet algorithm dictionary
const algomap_t& FastJetInterfaceTool::getKnownAlgorithms()
{
  /*! @brief Map of jet algorithm keywords and @c fastjet tags */
  static const FastJetInterface::algomap_t knownAlgorithms = {
    // kt-style algorithms
    {"default",           fastjet::antikt_algorithm},
    {"kt",                fastjet::kt_algorithm},
    {"anti-kt",           fastjet::antikt_algorithm},
    {"cambridge",         fastjet::cambridge_algorithm},
    {"genkt",             fastjet::genkt_algorithm},
    {"passive cambridge", fastjet::cambridge_for_passive_algorithm},
    {"passive genkt",     fastjet::genkt_for_passive_algorithm},
    // add also ATLAS kt-style
    {"Kt",         fastjet::kt_algorithm},
    {"AntiKt",     fastjet::antikt_algorithm},
    {"CamKt",      fastjet::cambridge_algorithm},
    // Plugin algorithms
    {"CMSCone",    fastjet::plugin_algorithm},
    {"SISCone",    fastjet::plugin_algorithm},
  };
  return knownAlgorithms;
}

// jet cluster strategies dictionary
const strategymap_t& FastJetInterfaceTool::getKnownStrategies()
{
  /*! @brief Map of jet clustering strategy keywords and @c fastjet tags */
  static const FastJetInterface::strategymap_t knownStrategies = {
    {"default",         fastjet::Best},
    {"Best",            fastjet::Best},
    {"N2MinHeapTiled",  fastjet::N2MinHeapTiled},
    {"N2Tiled",         fastjet::N2Tiled},
    {"N2PoorTiled",     fastjet::N2PoorTiled},
    {"N2Plain",         fastjet::N2Plain},
    {"N3Dumb",          fastjet::N3Dumb},
    {"NlnN",            fastjet::NlnN},
    {"NlnN3pi",         fastjet::NlnN3pi},
    {"NlnN4pi",         fastjet::NlnN4pi},
    {"NlnNCam4pi",      fastjet::NlnNCam4pi},
    {"NlnNCam2pi2R",    fastjet::NlnNCam2pi2R},
    {"NlnNCam",         fastjet::NlnNCam},
    {"plugin_strategy", fastjet::plugin_strategy},
  };
  return knownStrategies;
}

// recombination schemes dictionary
const schememap_t& FastJetInterfaceTool::getKnownRecombinationSchemes()
{
  /*! @brief Map of recombination scheme keywords and @c fastjet tags */
  static const FastJetInterface::schememap_t knownRecombinationSchemes = {
    {"default", fastjet::E_scheme},
    {"E",       fastjet::E_scheme},
    {"pt",      fastjet::pt_scheme},
    {"pt2",     fastjet::pt2_scheme},
    {"Et",      fastjet::Et_scheme},
    {"Et2",     fastjet::Et2_scheme},
    {"BIpt",    fastjet::BIpt_scheme},
    {"BIpt2",   fastjet::BIpt2_scheme},
  };
  return knownRecombinationSchemes;
}

// SIS Cone split merge scale dictionary
const splitMergeScaleMap_t& FastJetInterfaceTool::getKnownSplitMergeScales()
{
#ifndef XAOD_STANDALONE
  /*! @brief Map of split merge scale keywords and @c fastjet tags */
  static const FastJetInterface::splitMergeScaleMap_t knownSplitMergeScales = {
    {"default", fastjet::SISConePlugin::SM_pttilde},
    {"pttilde", fastjet::SISConePlugin::SM_pttilde},
    {"PtTilde", fastjet::SISConePlugin::SM_pttilde},
    {"Pt",      fastjet::SISConePlugin::SM_pt},
    {"Et",      fastjet::SISConePlugin::SM_Et},
    {"mt",      fastjet::SISConePlugin::SM_mt},
  };
#else
  static const FastJetInterface::splitMergeScaleMap_t knownSplitMergeScales;
#endif
  return knownSplitMergeScales;
}

// jet area dictionary
const areamap_t& FastJetInterfaceTool::getKnownAreas()
{
  /*! @brief Map of jet area keywords and @c fastjet tags */
  static const FastJetInterface::areamap_t knownAreas = {
    {"default",    fastjet::voronoi_area},
    {"ActiveArea", fastjet::active_area},
    {"ActiveAreaExplicitGhost", fastjet::active_area_explicit_ghosts},
    {"PassiveArea", fastjet::passive_area},
    {"1GhostPassiveArea", fastjet::one_ghost_passive_area},
    {"VoronoiArea", fastjet::voronoi_area},
  };
  return knownAreas;
}


////////////////////////////////////
// Tool Constructors & Destructor //
////////////////////////////////////

FastJetInterfaceTool::FastJetInterfaceTool(const std::string& n)
  : asg::AsgTool(n)
    // jet clustering and jet definition --- ////////// jet clustering
  , m_clusterSequenceType("ClusterSequence") // <--{ not used right now!
  , m_jetAlgorithmType("anti-kt")            // 
  , m_clusterStrategyType("Best")            // 
  , m_recombinationSchemeType("E")           // 
  , m_CMS_seedThreshold(15.)                 //
  , m_SIS_overlapThreshold(0.75)             //
  , m_SIS_nPass(0)                           //
  , m_SIS_protojetPtMin(0.0)                 //
  , m_SIS_doCaching(false)                   //
  , m_SIS_splitMergeScale_STRING("PtTilde")  //
  , m_SIS_splitMergeStopScale(0.0)           //
  , m_radius(0.4)                            //
  , m_inclusive(true)                        //
  , m_exclusiveDcut(0.5)                     //
  , m_exclusiveNjets(3)                      //
  , m_pTmin(0.)                              //
    // jet areas --------------------------- ////////// jet areas
  , m_doJetArea(false)                       //
  , m_jetAreaDefinitionType("VoronoiArea")   //
    // Voronoi area specs                    // -- Voronoi
  , m_voronoiEffectiveRfact(1.0)             //
    // ghosted area specs                    // -- ghosted -- //
  , m_ghostedMaxRap(fastjet::gas::def_ghost_maxrap)           //
  , m_ghostedMinRap(-m_ghostedMaxRap)                         //
  , m_ghostedRepeat(fastjet::gas::def_repeat)                 //
  , m_ghostedArea(fastjet::gas::def_ghost_area)               //
  , m_ghostedGridScatter(fastjet::gas::def_grid_scatter)      //
  , m_ghostedKtScatter(fastjet::gas::def_pt_scatter)          //
  , m_ghostedMeanKt(fastjet::gas::def_mean_ghost_pt)          //
    // process control ----------------------///////////////////
  , m_failedExecCtr(0)
    // presets                               ////////// private data presets 
  , m_jetDefinition((fastjet::JetDefinition*)nullptr)
  , m_clusterSequence((fastjet::ClusterSequence*)nullptr)
  , m_areaDefinition((fastjet::AreaDefinition*)nullptr)
  , m_baseRNDSeed (0)
  , m_userRNDSeed (0)
{
    //declareInterface<IFastJetInterfaceTool>(this);
  // setup choices
  //  declareProperty("ClusterSequence",m_clusterSequenceType);
  declareProperty("Algorithm",m_jetAlgorithmType);
  // parameters for kt-style jets
  declareProperty("Strategy",m_clusterStrategyType);
  declareProperty("RecombScheme",m_recombinationSchemeType);
  // parameters for CMS cone jets
  declareProperty("CMS_SeedThreshold",m_CMS_seedThreshold);
  // parameters for SIS cone jets
  declareProperty("SIS_OverlapThreshold",m_SIS_overlapThreshold);
  declareProperty("SIS_NumPassMax",m_SIS_nPass);
  declareProperty("SIS_ProtojetPtMin",m_SIS_protojetPtMin);
  declareProperty("SIS_DoCaching",m_SIS_doCaching);
  declareProperty("SIS_SplitMergeScale",m_SIS_splitMergeScale_STRING);
  declareProperty("SIS_SplitMergeStopScale",m_SIS_splitMergeStopScale);
  // parameters for jet finders/jet definition
  declareProperty("Radius",m_radius);
  declareProperty("Inclusive",m_inclusive);
  declareProperty("InclusivePtMin",m_pTmin);
  declareProperty("ExclusiveDcut",m_exclusiveDcut);
  declareProperty("ExclusiveNjets",m_exclusiveNjets);
  // parameters for areas
  declareProperty("CalculateJetArea",m_doJetArea);
  declareProperty("JetAreaMethod",m_jetAreaDefinitionType);
  declareProperty("VoronoiEffectiveRfact",m_voronoiEffectiveRfact);
  declareProperty("GhostMaxRapidity",m_ghostedMaxRap);
  declareProperty("GhostMinRapidity",m_ghostedMinRap);
  declareProperty("GhostRepeats",m_ghostedRepeat);
  declareProperty("GhostAreaSize",m_ghostedArea);
  declareProperty("GhostGridScatter",m_ghostedGridScatter);
  declareProperty("GhostKtScatter",m_ghostedKtScatter);
  declareProperty("GhostMeanKt",m_ghostedMeanKt);

  declareProperty("UserRNDSeed", m_userRNDSeed = 0, "Added to the base RND seed for active areas. USE WITH CARE !");

  // default processor
  m_processor = &FastJetInterfaceTool::f_processWithoutArea;
  m_extractor = &FastJetInterfaceTool::f_extractInclusive;
  // other presets (will be set/overwritten in initialize)
  m_jetAlgorithm        = fastjet::antikt_algorithm;
  m_strategy            = fastjet::Best;
  m_areaType            = fastjet::voronoi_area;
  m_recombinationScheme = fastjet::E_scheme;
#ifndef XAOD_STANDALONE
  m_SIS_splitMergeScale = fastjet::SISConePlugin::SM_pttilde;
#endif

}

FastJetInterfaceTool::~FastJetInterfaceTool()
{ 
  if(m_jetDefinition)  delete m_jetDefinition;
  if(m_areaDefinition) delete m_areaDefinition;
  if(m_clusterSequence) delete m_clusterSequence; 

}

/////////////////////////
// Tool Initialization //
/////////////////////////

StatusCode FastJetInterfaceTool::initialize()
{
  ATH_MSG_DEBUG( "  initialize " );

  // check requested jet algorithm
  ATH_MSG_INFO("Algorithm type \042" << m_jetAlgorithmType << "\042");
  if ( !this->checkConfig(m_jetAlgorithmType,m_jetAlgorithm) )
    {
      msg(MSG::ERROR) << "FastJet configuration error: "
		      << "Unknown jet algorithm \042"
		      << m_jetAlgorithmType << "\042 requested"
		      << endmsg;
      return StatusCode::FAILURE;
    }
  // check requested recombination scheme
  ATH_MSG_INFO("Recombination type \042" << m_recombinationSchemeType << "\042");
  if ( !this->checkConfig(m_recombinationSchemeType,m_recombinationScheme) )
    {
      msg(MSG::ERROR) << "FastJet configuration error: "
		      << "Unknown recombination scheme \042"
		      << m_recombinationScheme << "\042 requested"
		      << endmsg;
      return StatusCode::FAILURE;
    }
  // check requested strategy
  ATH_MSG_INFO("Cluster strategy type \042" << m_clusterStrategyType << "\042");
  if ( !checkConfig(m_clusterStrategyType,m_strategy) )
    {
      msg(MSG::ERROR) << "FastJet configuration error: " 
		      << "Unknown clustering strategy \042"
		      << m_clusterStrategyType << "\042 requested" << endmsg;
      return StatusCode::FAILURE;
    }
  
  // determine the type of jet algorithm we have (kt-style vs plugins)
  enum jetAlgTypeEnum { unknown_type, Kt_type, CMS_type, SIS_type } jetAlgType = Kt_type;
  if (m_jetAlgorithm == fastjet::plugin_algorithm)
  {
    if (!m_jetAlgorithmType.compare("CMSCone"))
      jetAlgType = CMS_type; 
    else if(!m_jetAlgorithmType.compare("SISCone"))
      jetAlgType = SIS_type;
    else
      jetAlgType = unknown_type;
  }
  if (jetAlgType == unknown_type)
  {
    msg(MSG::ERROR) << "FastJet configuration error: "
                    << "Unknown jet algorithm plugin \042"
                    << m_jetAlgorithmType << "\042 requested" << endmsg;
    return StatusCode::FAILURE;
  }

#ifndef XAOD_STANDALONE

  // define jets depending on the jet type
  if (jetAlgType == CMS_type)
    m_jetDefinition = new fastjet::JetDefinition(new fastjet::CMSIterativeConePlugin(m_radius,m_CMS_seedThreshold));
  else if (jetAlgType == SIS_type)
    m_jetDefinition = new fastjet::JetDefinition(new fastjet::SISConePlugin(m_radius,
                                                                            m_SIS_overlapThreshold,
                                                                            m_SIS_nPass,
                                                                            m_SIS_protojetPtMin,
                                                                            m_SIS_doCaching,
                                                                            m_SIS_splitMergeScale,
                                                                            m_SIS_splitMergeStopScale
                                                                            ));
  else
    m_jetDefinition = new fastjet::JetDefinition(m_jetAlgorithm,
    					         m_radius,
    					         m_recombinationScheme,
    					         m_strategy);
#else 
  // no athena error on SIS or CMS
  if ( (jetAlgType == CMS_type) || (jetAlgType == SIS_type)){
    ATH_MSG_ERROR("CMS and SIScone plugin not supporeted outside athena yet");
    return StatusCode::FAILURE;
  }else
    m_jetDefinition = new fastjet::JetDefinition(m_jetAlgorithm,
    					         m_radius,
    					         m_recombinationScheme,
    					         m_strategy);
#endif    


  // without area calculation
  if ( !m_doJetArea )
    {
      m_processor = &FastJetInterfaceTool::f_processWithoutArea;
    }
  // with area calculation
  else
    {
      ATH_MSG_INFO("Jet area type \042" << m_jetAreaDefinitionType << "\042");
      if ( this->configJetAreas().isFailure() )
	{
	  msg(MSG::ERROR) << "FastJet configuration error: "
			  << "Clustering with areas requested but area type"
			  << "\042" << m_jetAreaDefinitionType 
			  << "\042 is not known" << endmsg;
	  return StatusCode::FAILURE;
	}
      m_processor = &FastJetInterfaceTool::f_processWithArea;
    }

  // configure inclusive/exclusive jet extractor
  if ( m_inclusive )
    {
      m_extractor = &FastJetInterfaceTool::f_extractInclusive;
    } // inclusive jet finder
  else
    {
      // Priority:
      //  
      // if !inclusive: 
      //     m_exclusiveDcut > 0  -> yes
      //     m_exclusiveDcut <= 0 -> m_exclusiveNjets >= 0 -> yes
      //                             m_exclusiveNjets < 0 -> error
      if ( m_exclusiveDcut > 0. )
	{
	  m_extractor = &FastJetInterfaceTool::f_extractExclDcut;
	}
      else
	{
	  if ( m_exclusiveNjets >= 0 ) 
	    {
	      m_extractor = &FastJetInterfaceTool::f_extractExclNjets;
	    } 
	  else
	    {
	      msg(MSG::ERROR) << "Invalid configuration for exclusive "
			      << "jet finder, one of ExclusiveDcut("
			      << m_exclusiveDcut
			      << ") or ExclusiveNjets("
			      << m_exclusiveNjets
			      << ") properties should be >= 0."
			      << endmsg;
	      return StatusCode::FAILURE;
	    } // invalid configuration of exclusive jet finder
	} // exclusive jet finder using Njet cut
    } // exclusive jet finder using Dcut

  // print configuration
  msg(MSG::INFO) << "JetAlgorithm ................ \042" 
		 << m_jetAlgorithmType << "\042" << endmsg;
  msg(MSG::INFO) << "Radius ...................... " << m_radius << endmsg;
  if (jetAlgType == Kt_type)
  {
    msg(MSG::INFO) << "Recombination scheme ........ \042"
		   << m_recombinationSchemeType << "\042" << endmsg;
    msg(MSG::INFO) << "Strategy .................... \042"
		   << m_clusterStrategyType << "\042" << endmsg;
  }
  else if (jetAlgType == CMS_type)
    msg(MSG::INFO) << "CMS seed threshold .......... "
                   << m_CMS_seedThreshold << endmsg;
  else if (jetAlgType == SIS_type)
  {
    msg(MSG::INFO) << "SIS overlap threshold ....... "
                   << m_SIS_overlapThreshold << endmsg;
    msg(MSG::INFO) << "SIS max num pass ............ "
                   << m_SIS_nPass << endmsg;
    msg(MSG::INFO) << "SIS protojet Pt min ......... "
                   << m_SIS_protojetPtMin << endmsg;
    msg(MSG::INFO) << "SIS do caching .............. "
                   << (m_SIS_doCaching ? std::string("yes") : std::string("no")) << endmsg;
    msg(MSG::INFO) << "SIS split/merge scale ....... \042"
                   << m_SIS_splitMergeScale_STRING << "\042" << endmsg;
    msg(MSG::INFO) << "SIS split/merge stop scale .. "
                   << m_SIS_splitMergeStopScale << endmsg;
  }
  if ( m_doJetArea )
    {
      msg(MSG::INFO) << "Calculate jet areas ......... yes" << endmsg;
      msg(MSG::INFO) << "Area type ................... \042" 
		     << m_jetAreaDefinitionType << "\042" << endmsg;
    }



  return StatusCode::SUCCESS;
}

////////////////////
// Tool Execution //
////////////////////

StatusCode FastJetInterfaceTool::execute(const fjetlist_t& inJets,
					 fjetlist_t& outJets)
{
  // clean up
  ATH_MSG_DEBUG( "  FastJetInterfaceTool::execute  injet size = "<< inJets.size() );

  if ( m_clusterSequence != nullptr ) 
    {
      delete m_clusterSequence;
      m_clusterSequence = (fastjet::ClusterSequence*)nullptr;
    }
  
  if ( ((this->*m_processor)(inJets,outJets)).isFailure() )
    {
      ++m_failedExecCtr;
      if ( m_failedExecCtr < m_failedExecCtrMax )
	{
	  msg(MSG::WARNING) << "Cannot allocate new fastjet::ClusterSequence,"
			    << " no jets found (message " 
			    << m_failedExecCtr << " of "
			    << m_failedExecCtrMax << ")" << endmsg;
	}
      else if ( m_failedExecCtr == m_failedExecCtrMax )
	{
	  msg(MSG::WARNING) << "Cannot allocate new fastjet::ClusterSequence,"
			    << " no jets found (message " 
			    << m_failedExecCtr << " of "
			    << m_failedExecCtrMax << ") [last message]" 
			    << endmsg;
	}
      outJets.clear();
    }
  return StatusCode::SUCCESS;
}

///////////////////////////
// Configuration Helpers //
///////////////////////////

bool FastJetInterfaceTool::checkConfig(const std::string& key,
				       fastjet::JetAlgorithm& fjalg)
{
  const algomap_t& map = this->getKnownAlgorithms();
  return chkConfig(key,fjalg,map);
}

bool FastJetInterfaceTool::checkConfig(const std::string& key,
				       fastjet::Strategy& fjstr)
{
  const strategymap_t& map = this->getKnownStrategies();
  return chkConfig(key,fjstr,map);
}

bool FastJetInterfaceTool::checkConfig(const std::string& key,
				       fastjet::RecombinationScheme& fjrs)
{
  const schememap_t& map = this->getKnownRecombinationSchemes();
  return chkConfig(key,fjrs,map);
}

#ifndef XAOD_STANDALONE
bool FastJetInterfaceTool::checkConfig(const std::string& key,
                                       fastjet::SISConePlugin::SplitMergeScale& fjsms)
{
  const splitMergeScaleMap_t& map = this->getKnownSplitMergeScales();
  return chkConfig(key,fjsms,map);
}
#endif

bool FastJetInterfaceTool::checkConfig(const std::string& key,
				       fastjet::AreaType& fjart)
{
  const areamap_t& map = this->getKnownAreas();
  return chkConfig(key,fjart,map);
}

const std::string& FastJetInterfaceTool::configName(fastjet::JetAlgorithm fjalg)
{
  const algomap_t& map = this->getKnownAlgorithms();
  return cfgName(fjalg,map,m_invalidKeyReference);
}

const std::string& FastJetInterfaceTool::configName(fastjet::Strategy fjstr)
{
  const strategymap_t& map = this->getKnownStrategies();
  return cfgName(fjstr,map,m_invalidKeyReference);
}

const std::string& 
FastJetInterfaceTool::configName(fastjet::RecombinationScheme fjrs)
{
  const schememap_t& map = this->getKnownRecombinationSchemes();
  return cfgName(fjrs,map,m_invalidKeyReference);
}

#ifndef XAOD_STANDALONE
const std::string&
FastJetInterfaceTool::configName(fastjet::SISConePlugin::SplitMergeScale fjsms)
{
  const splitMergeScaleMap_t& map = this->getKnownSplitMergeScales();
  return cfgName(fjsms,map,m_invalidKeyReference);
}
#endif
const std::string& FastJetInterfaceTool::configName(fastjet::AreaType fjart)
{
  const areamap_t& map = this->getKnownAreas();
  return cfgName(fjart,map,m_invalidKeyReference);
}

///////////////////////
// Processor Helpers //
///////////////////////

StatusCode FastJetInterfaceTool::f_processWithoutArea(const fjetlist_t& inJets,
						      fjetlist_t& outJets)
{
  ATH_MSG_DEBUG( "  processing without area " );
  m_clusterSequence = new fastjet::ClusterSequence(inJets,*m_jetDefinition);
  ATH_MSG_DEBUG( "  processed without area " );
  return m_clusterSequence != nullptr 
    ? (this->*m_extractor)(outJets) : StatusCode::FAILURE;
}

StatusCode FastJetInterfaceTool::f_processWithArea(const fjetlist_t& inJets,
						   fjetlist_t& outJets)
{

  updateRandomSeeds(); // make sure rnd seeds depend on event & run numbers
  m_clusterSequence = (fastjet::ClusterSequence*)
    new fastjet::ClusterSequenceArea(inJets,*m_jetDefinition,*m_areaDefinition);

  return m_clusterSequence != nullptr 
    ? (this->*m_extractor)(outJets) : StatusCode::FAILURE; 
}

StatusCode FastJetInterfaceTool::f_extractInclusive(fjetlist_t& outJets)
{
  outJets = m_clusterSequence->inclusive_jets(m_pTmin);
  ATH_MSG_DEBUG( "  extract inclusive outjets N= "<< outJets.size() );
  return !outJets.empty() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

StatusCode FastJetInterfaceTool::f_extractExclNjets(fjetlist_t& outJets)
{
  outJets = m_clusterSequence->exclusive_jets(m_exclusiveNjets);
  ATH_MSG_DEBUG( "  extract exclusiveNjets outjets N= "<< outJets.size() );

  return !outJets.empty() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

StatusCode FastJetInterfaceTool::f_extractExclDcut(fjetlist_t& outJets)
{
  outJets = m_clusterSequence->exclusive_jets(m_exclusiveDcut);
  ATH_MSG_DEBUG( "  extract exclusiveDcut outjets N= "<< outJets.size() );
  return ! outJets.empty() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

fastjet::ClusterSequence* FastJetInterfaceTool::clusterSequence() 
{ return m_clusterSequence; }

const fastjet::ClusterSequence* FastJetInterfaceTool::clusterSequence() const
{ return m_clusterSequence; }

const fastjet::JetDefinition* FastJetInterfaceTool::getJetDefinition() const
{ return m_jetDefinition; }

const fastjet::AreaDefinition* FastJetInterfaceTool::getAreaDefinition() const
{ return m_areaDefinition; }

const std::string FastJetInterfaceTool::getAreaDefinitionType() const
{ return m_jetAreaDefinitionType; }

StatusCode FastJetInterfaceTool::configJetAreas()
{
  // check area type
  const areamap_t& map = getKnownAreas();
  m_areaType = (*(map.find("default"))).second;
  if ( !checkConfig(m_jetAreaDefinitionType,m_areaType) )
    { return StatusCode::FAILURE; }
  // area definition according to type
  switch ( m_areaType )
    {
      // Voronoi
    case fastjet::voronoi_area:
      m_areaDefinition = 
	new 
	fastjet::AreaDefinition(fastjet::VoronoiAreaSpec(m_voronoiEffectiveRfact));
      break;
      // Ghosted
    case fastjet::active_area:
    case fastjet::active_area_explicit_ghosts:
    case fastjet::one_ghost_passive_area:
    case fastjet::passive_area:
      {
        fastjet::GhostedAreaSpec Garea_spec(m_ghostedMinRap,
                                            m_ghostedMaxRap,
                                            m_ghostedRepeat,
                                            m_ghostedArea,
                                            m_ghostedGridScatter,
                                            m_ghostedKtScatter,
                                            m_ghostedMeanKt);

        // create a seed from the jet alg and the seed
        m_baseRNDSeed = CxxUtils::crc64 ("FastJet");
        m_baseRNDSeed = CxxUtils::crc64addint (m_baseRNDSeed, (uint64_t) m_jetAlgorithm);
        m_baseRNDSeed = CxxUtils::crc64addint (m_baseRNDSeed, uint64_t (m_radius*10) );
        m_baseRNDSeed += m_userRNDSeed;
        // seed will be modified and reset in the Garea_spec for each event (see updateRandomSeeds() ).

        ATH_MSG_DEBUG(" Base  seed = "<< m_baseRNDSeed<< "  user seed ="<< m_userRNDSeed );

        m_areaDefinition = new fastjet::AreaDefinition(Garea_spec,                    
                                                       m_areaType);
        break;
      }
      // others
    default:
      m_areaDefinition = new fastjet::AreaDefinition(m_areaType);
      break;
    } // switch through area options
      
  // configure processor
  return StatusCode::SUCCESS;
}

 ////////////////////////////////////////////////////////////////////////////////
 // update seeds. Call this function ONLY if using a ClusterSequenceArea
 ////////////////////////////////////////////////////////////////////////////////
void FastJetInterfaceTool::updateRandomSeeds()
{


  std::vector<int> seed;
  
  fastjet::GhostedAreaSpec & Garea_spec = m_areaDefinition->ghost_spec() ;
  Garea_spec.get_random_status (seed);
  
  unsigned int evnum = 0;
  unsigned int runnum = 0;
  
  const xAOD::EventInfo* ei = nullptr;
  if (evtStore()->retrieve (ei,"EventInfo").isSuccess()){
    evnum  = ei->eventNumber();
    runnum = ei->runNumber();
  }

  // Use both event and run nunmber to reset the random seeds
  uint64_t crc = CxxUtils::crc64addint (m_baseRNDSeed, evnum);
  crc = CxxUtils::crc64addint (crc, runnum);
  
  // Tacitly assuming that there are no consistency requirements
  // on the RNG state (true for the current RNG that fastjet uses).
  // Use fixed data sizes for crossplatform reproducibility.
  for (size_t i = 0; i < seed.size(); i++) {
    seed[i] = static_cast<int> (static_cast<uint32_t> (crc));
    crc = (crc >> 32) ^ crc;
  }
    
  
  Garea_spec.set_random_status (seed);
  seed.clear();
  Garea_spec.get_random_status (seed);
  ATH_MSG_DEBUG("area def "<< m_areaDefinition<< "   ghost spec= "<< &Garea_spec << " crc= "<< crc<< "  seed0="<< seed[0] << " seed1="<<seed[1]);

}
