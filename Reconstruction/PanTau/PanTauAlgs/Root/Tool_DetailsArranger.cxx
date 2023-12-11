/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTau/TauJet.h"
#include "xAODPFlow/PFOContainer.h"
#include "xAODParticleEvent/Particle.h"
#include "xAODParticleEvent/ParticleContainer.h"

#include "PanTauAlgs/Tool_DetailsArranger.h"
#include "PanTauAlgs/PanTauSeed.h"
#include "PanTauAlgs/HelperFunctions.h"


bool sortBDTscore(const ElementLink< xAOD::PFOContainer >& i, const ElementLink< xAOD::PFOContainer >& j){

  return ( i.cachedElement()->bdtPi0Score() > j.cachedElement()->bdtPi0Score() );
}


PanTau::Tool_DetailsArranger::Tool_DetailsArranger(const std::string& name) :
  asg::AsgTool(name),
  m_Tool_InformationStore("PanTau::Tool_InformationStore/Tool_InformationStore")
{
  declareProperty("Tool_InformationStore",            m_Tool_InformationStore,            "Tool handle to the information store tool");
  declareProperty("Tool_InformationStoreName",            m_Tool_InformationStoreName,            "Tool handle to the information store tool");
}


PanTau::Tool_DetailsArranger::~Tool_DetailsArranger() = default;


StatusCode PanTau::Tool_DetailsArranger::initialize() {

  m_init=true;

  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_InformationStore, m_Tool_InformationStoreName ) );
    
  ATH_CHECK( m_Tool_InformationStore->getInfo_Double("TauConstituents_Types_DeltaRCore",                      m_CoreCone) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecDouble("TauConstituents_BinEdges_Eta",                       m_EtaBinEdges) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecDouble("TauConstituents_Selection_Neutral_EtaBinned_EtCut",  m_EtaBinnedEtCuts) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_Sum",          m_varTypeName_Sum) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_Ratio",        m_varTypeName_Ratio) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_EtInRing",     m_varTypeName_EtInRing) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_Isolation",    m_varTypeName_Isolation) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_Num",          m_varTypeName_Num) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_Mean",         m_varTypeName_Mean) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_StdDev",       m_varTypeName_StdDev) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_HLV",          m_varTypeName_HLV) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_Angle",        m_varTypeName_Angle) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_DeltaR",       m_varTypeName_DeltaR) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_JetMoment",    m_varTypeName_JetMoment) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_Combined",     m_varTypeName_Combined) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_JetShape",     m_varTypeName_JetShape) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_ImpactParams", m_varTypeName_ImpactParams) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_Basic",        m_varTypeName_Basic) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_PID",          m_varTypeName_PID) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("FeatureExtractor_VarTypeName_varTypeName_Shots",        m_varTypeName_Shots) );
    
  return StatusCode::SUCCESS;
}


StatusCode PanTau::Tool_DetailsArranger::execute(PanTau::PanTauSeed* inSeed, xAOD::ParticleContainer& pi0Container, xAOD::PFOContainer& neutralPFOContainer) const {

  std::string inputAlg = inSeed->getNameInputAlgorithm();
    
  bool noAnyConstituents           = inSeed->isOfTechnicalQuality(PanTau::PanTauSeed::t_NoConstituentsAtAll);
  bool noSelConstituents           = inSeed->isOfTechnicalQuality(PanTau::PanTauSeed::t_NoSelectedConstituents);
  bool noValidInputTau             = inSeed->isOfTechnicalQuality(PanTau::PanTauSeed::t_NoValidInputTau);
  bool isBadSeed                   = (noAnyConstituents || noSelConstituents || noValidInputTau);
        
  //set the PFO link vector and pantau 4-vector to default values for every tau first (xAOD technicality)
  //if the tau is valid, overwrite with non-default values
  xAOD::TauJet* tauJet = inSeed->getTauJet();
    
  if (isBadSeed) {
    ATH_MSG_DEBUG("This seed is not useable for detail arranging (other than validity flag)");
    tauJet->setPanTauDetail(xAOD::TauJetParameters::PanTau_isPanTauCandidate, 0);
    tauJet->setPanTauDetail(xAOD::TauJetParameters::PanTau_DecayMode, xAOD::TauJetParameters::Mode_NotSet);
    return StatusCode::SUCCESS;
  }
    
  ATH_CHECK(arrangePFOLinks(inSeed, tauJet, pi0Container, neutralPFOContainer));

  //Basic variables
  addPanTauDetailToTauJet(inSeed, m_varTypeName_Basic + "_isPanTauCandidate",      xAOD::TauJetParameters::PanTau_isPanTauCandidate, PanTau::Tool_DetailsArranger::t_Int);
  addPanTauDetailToTauJet(inSeed, m_varTypeName_Basic + "_RecoMode_PanTau",        xAOD::TauJetParameters::PanTau_DecayMode, PanTau::Tool_DetailsArranger::t_Int);
  addPanTauDetailToTauJet(inSeed, m_varTypeName_Basic + "_BDTValue_1p0n_vs_1p1n",  xAOD::TauJetParameters::PanTau_BDTValue_1p0n_vs_1p1n, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, m_varTypeName_Basic + "_BDTValue_1p1n_vs_1pXn",  xAOD::TauJetParameters::PanTau_BDTValue_1p1n_vs_1pXn, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, m_varTypeName_Basic + "_BDTValue_3p0n_vs_3pXn",  xAOD::TauJetParameters::PanTau_BDTValue_3p0n_vs_3pXn, PanTau::Tool_DetailsArranger::t_Float);
    
  //Final 4-vector
  tauJet->setP4(xAOD::TauJetParameters::PanTauCellBased, inSeed->getFinalMomentum().Pt(), inSeed->getFinalMomentum().Eta(), inSeed->getFinalMomentum().Phi(), inSeed->getFinalMomentum().M());

  //BDT variables
  addPanTauDetailToTauJet(inSeed, "Basic_NNeutralConsts",
			  xAOD::TauJetParameters::PanTau_BDTVar_Basic_NNeutralConsts, PanTau::Tool_DetailsArranger::t_Int);
  addPanTauDetailToTauJet(inSeed, "Charged_JetMoment_EtDRxTotalEt",
			  xAOD::TauJetParameters::PanTau_BDTVar_Charged_JetMoment_EtDRxTotalEt, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, "Charged_StdDev_Et_WrtEtAllConsts",
			  xAOD::TauJetParameters::PanTau_BDTVar_Charged_StdDev_Et_WrtEtAllConsts, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, "Neutral_HLV_SumM",
			  xAOD::TauJetParameters::PanTau_BDTVar_Neutral_HLV_SumM, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, "Neutral_PID_BDTValues_BDTSort_1",
			  xAOD::TauJetParameters::PanTau_BDTVar_Neutral_PID_BDTValues_BDTSort_1, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, "Neutral_PID_BDTValues_BDTSort_2",
			  xAOD::TauJetParameters::PanTau_BDTVar_Neutral_PID_BDTValues_BDTSort_2, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, "Neutral_Ratio_1stBDTEtOverEtAllConsts",
			  xAOD::TauJetParameters::PanTau_BDTVar_Neutral_Ratio_1stBDTEtOverEtAllConsts, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, "Neutral_Ratio_EtOverEtAllConsts",
			  xAOD::TauJetParameters::PanTau_BDTVar_Neutral_Ratio_EtOverEtAllConsts, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, "Neutral_Shots_NPhotonsInSeed",
			  xAOD::TauJetParameters::PanTau_BDTVar_Neutral_Shots_NPhotonsInSeed, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, "Combined_DeltaR1stNeutralTo1stCharged",
			  xAOD::TauJetParameters::PanTau_BDTVar_Combined_DeltaR1stNeutralTo1stCharged, PanTau::Tool_DetailsArranger::t_Float);
  addPanTauDetailToTauJet(inSeed, "Charged_HLV_SumM",
			  xAOD::TauJetParameters::PanTau_BDTVar_Charged_HLV_SumM, PanTau::Tool_DetailsArranger::t_Float);
              
  return StatusCode::SUCCESS;
}


void PanTau::Tool_DetailsArranger::addPanTauDetailToTauJet(PanTauSeed* inSeed,
                                                           const std::string& featName,
                                                           xAOD::TauJetParameters::PanTauDetails detailEnum,
                                                           PanTauDetailsType detailType) const {

  bool isValid;
  const PanTau::TauFeature* features  = inSeed->getFeatures();
  std::string         fullFeatName    = inSeed->getNameInputAlgorithm() + "_" + featName;

  double theValue = features->value(fullFeatName, isValid);
  if (!isValid) {
    bool noAnyConstituents           = inSeed->isOfTechnicalQuality(PanTau::PanTauSeed::t_NoConstituentsAtAll);
    bool noSelConstituents           = inSeed->isOfTechnicalQuality(PanTau::PanTauSeed::t_NoSelectedConstituents);
    bool noValidInputTau             = inSeed->isOfTechnicalQuality(PanTau::PanTauSeed::t_NoValidInputTau);
    bool isBadSeed                   = (noAnyConstituents || noSelConstituents || noValidInputTau);

    if (!isBadSeed) {
      ATH_MSG_DEBUG("WARNING --- Problems getting value for feature " << fullFeatName << " from map! This should not happen for this seed!");
      ATH_MSG_DEBUG("WARNING --- Did the ModeDiscriminator set features used in BDTs that were not found to their default values?");
      ATH_MSG_DEBUG("NOTE    --- This can also happen for seeds with (for example) 0 neutrals when trying to get Neutral_SumM - check seed");
    }
    theValue = -1111.;
  }

  xAOD::TauJet* tauJet = inSeed->getTauJet();
  int     valueToAddInt   = -1;
  float   valueToAddFloat = -1.1;

  switch(detailType) {
  case PanTau::Tool_DetailsArranger::t_Int:
    valueToAddInt   = (int)theValue;
    tauJet->setPanTauDetail(detailEnum, valueToAddInt);
    break;
  case PanTau::Tool_DetailsArranger::t_Float:
    valueToAddFloat = (float)theValue;
    tauJet->setPanTauDetail(detailEnum, valueToAddFloat);
    break;
  default:
    ATH_MSG_WARNING("Unknown PanTauDetailType! Please use enum PanTauDetailsType from PanTauAlgs/Tool_DetailsArranger.h.");
    ATH_MSG_WARNING("!NOT! adding detail named " << featName);
    break;
  }

  }


StatusCode PanTau::Tool_DetailsArranger::arrangePFOLinks(PanTau::PanTauSeed* inSeed, xAOD::TauJet* tauJet, xAOD::ParticleContainer& pi0Container, xAOD::PFOContainer& neutralPFOContainer) const {

  std::string inputAlg = inSeed->getNameInputAlgorithm();
   
  //get the PFO links
  std::vector< ElementLink< xAOD::PFOContainer > > chrgPFOLinks       = tauJet->protoChargedPFOLinks();
  std::vector< ElementLink< xAOD::PFOContainer > > neutralPFOLinks    = tauJet->protoNeutralPFOLinks();
  std::vector< ElementLink< xAOD::PFOContainer > > pi0PFOLinks        = CollectConstituentsAsPFOLinks( inSeed, tauJet->protoNeutralPFOLinks(), PanTau::TauConstituent::t_Pi0Neut );
  std::vector< ElementLink< xAOD::PFOContainer > > preSelected_neutralPFOLinks    = CollectConstituentsAsPFOLinks( inSeed, tauJet->protoNeutralPFOLinks(), PanTau::TauConstituent::t_Neutral );

  //clear the default links, just to be safe
  tauJet->clearChargedPFOLinks();
  tauJet->clearNeutralPFOLinks();
  tauJet->clearPi0PFOLinks();
    
  std::vector< ElementLink< xAOD::PFOContainer > > preLinkPi0PFOLinks = tauJet->pi0PFOLinks();

  // sort PFO links according to pi0 BDT score:
  std::sort (pi0PFOLinks.begin(), pi0PFOLinks.end(), sortBDTscore);
  std::sort (preSelected_neutralPFOLinks.begin(), preSelected_neutralPFOLinks.end(), sortBDTscore);
  std::sort (neutralPFOLinks.begin(), neutralPFOLinks.end(), sortBDTscore);

  // set the masses of all neutrals *and pi0 neutrals* to 0:
  SetNeutralConstituentVectorMasses(neutralPFOLinks, neutralPFOContainer, 0.);
    
  // arrange charged & neutral PFOs: they are not changed -> copy from cellbased
  tauJet->setChargedPFOLinks(chrgPFOLinks);
  tauJet->setNeutralPFOLinks(neutralPFOLinks);
    
  tauJet->setDetail(xAOD::TauJetParameters::nCharged, (int)chrgPFOLinks.size());

  //arrange pi0 pfos: depends on decay mode classification
  int decayModeProto = inSeed->getDecayModeBySubAlg();
  int decayModeFinal = inSeed->getDecayModeByPanTau();
    
  if(decayModeFinal == xAOD::TauJetParameters::Mode_Other) {
    tauJet->setPi0PFOLinks(pi0PFOLinks);
    SetHLVTau(inSeed, tauJet, inputAlg, m_varTypeName_Basic);

    return StatusCode::SUCCESS;
  }
    
  //if pantau sets the same decay mode as the substructure algorithm, just copy the links
  if(decayModeProto == decayModeFinal) {
      
    if( decayModeFinal == xAOD::TauJetParameters::Mode_3pXn && pi0PFOLinks.size() > 1 ){

      // assign all neutrals but only one pi0 neutral to Pantau:
      preLinkPi0PFOLinks.push_back( pi0PFOLinks.at(0) );
	
      // set all masses correctly:
      SetNeutralConstituentVectorMasses(preLinkPi0PFOLinks, neutralPFOContainer, MASS_PI0);
	
    } else {

      // assign the same constituents to Pantau:
      preLinkPi0PFOLinks=pi0PFOLinks;
	
      // set all masses correctly:
      SetNeutralConstituentVectorMasses(preLinkPi0PFOLinks, neutralPFOContainer, MASS_PI0);
    }

  } else {

    if( decayModeFinal == xAOD::TauJetParameters::Mode_1p1n && decayModeProto == xAOD::TauJetParameters::Mode_1p0n ){

      // add the highest BDT-score neutral from the sub-alg:
      if(!preSelected_neutralPFOLinks.empty()) preLinkPi0PFOLinks.push_back( preSelected_neutralPFOLinks.at(0) );
      else ATH_MSG_WARNING("No neutral PFO Links although there should be!!");

      // set the mass:
      SetNeutralConstituentVectorMasses(preLinkPi0PFOLinks, neutralPFOContainer, MASS_PI0);

    } else if( decayModeFinal == xAOD::TauJetParameters::Mode_1p0n && decayModeProto == xAOD::TauJetParameters::Mode_1p1n ){

      // do nothing (leave the pi0 neutrals list empty)

    } else if( decayModeFinal == xAOD::TauJetParameters::Mode_1pXn && decayModeProto == xAOD::TauJetParameters::Mode_1p1n ){


      if( pi0PFOLinks.size() == 1 && HasMultPi0sInOneCluster(pi0PFOLinks.at(0).cachedElement(), decayModeProto, inputAlg) ){ 
	  
	// assign twice the pi0 mass to the one pi0 PFO:
	SetNeutralConstituentVectorMasses(pi0PFOLinks, neutralPFOContainer, 2*MASS_PI0);
	  
	// assign the same constituents to Pantau:
	preLinkPi0PFOLinks=pi0PFOLinks;
	  
      } else {
	  
	// copy all (really only one) pi0s from the sub-alg and add
	// the highest BDT-score neutral:
	preLinkPi0PFOLinks=pi0PFOLinks;
	if(!preSelected_neutralPFOLinks.empty()) preLinkPi0PFOLinks.push_back( preSelected_neutralPFOLinks.at(0) );
	else ATH_MSG_WARNING("No neutral PFO Links although there should be!!");
	  
	// set the mass:
	SetNeutralConstituentVectorMasses(preLinkPi0PFOLinks, neutralPFOContainer, MASS_PI0);
      }

    } else if( decayModeFinal == xAOD::TauJetParameters::Mode_1p1n && decayModeProto == xAOD::TauJetParameters::Mode_1pXn ){

      // copy all (two) pi0s from the sub-alg:
      preLinkPi0PFOLinks.push_back( pi0PFOLinks.at(0) );
      preLinkPi0PFOLinks.push_back( pi0PFOLinks.at(1) );

      // set both pi0neutrals to mass 0 (photon mass):
      SetNeutralConstituentVectorMasses(preLinkPi0PFOLinks, neutralPFOContainer, 0.);

    } else if( decayModeFinal == xAOD::TauJetParameters::Mode_3pXn && decayModeProto == xAOD::TauJetParameters::Mode_3p0n ){

      // add the highest BDT-score neutral from the sub-alg:
      if(!preSelected_neutralPFOLinks.empty()) preLinkPi0PFOLinks.push_back( preSelected_neutralPFOLinks.at(0) );
      else ATH_MSG_WARNING("No neutral PFO Links although there should be!!");
	
      // set the mass:
      SetNeutralConstituentVectorMasses(preLinkPi0PFOLinks, neutralPFOContainer, MASS_PI0);

    } else if( decayModeFinal == xAOD::TauJetParameters::Mode_3p0n && decayModeProto == xAOD::TauJetParameters::Mode_3pXn ){

      // do nothing (leave the pi0 neutrals list empty)	
    }
  }

  tauJet->setPi0PFOLinks(preLinkPi0PFOLinks);

  SetHLVTau(inSeed, tauJet, inputAlg, m_varTypeName_Basic);

  std::vector< ElementLink< xAOD::PFOContainer > > finalChrgPFOLinks       = tauJet->chargedPFOLinks();
  std::vector< ElementLink< xAOD::PFOContainer > > finalPi0PFOLinks        = tauJet->pi0PFOLinks();
  std::vector< ElementLink< xAOD::PFOContainer > > finalNeutralPFOLinks    = tauJet->neutralPFOLinks(); 

  // vector of 4-vectors of actual pi0s and a vector with pointers to PFOs:
  std::vector< TLorentzVector > vec_pi04vec;
  std::vector< std::vector< ElementLink<xAOD::PFOContainer> > > vec_pi0pfos;
  createPi0Vectors(tauJet, vec_pi04vec, vec_pi0pfos);

  for(unsigned int itlv=0; itlv!=vec_pi04vec.size(); ++itlv) {      
    xAOD::Particle* p = new xAOD::Particle();
    pi0Container.push_back(p);
    p->setPxPyPzE(vec_pi04vec.at(itlv).Px(), vec_pi04vec.at(itlv).Py(), vec_pi04vec.at(itlv).Pz(), vec_pi04vec.at(itlv).E());
    std::vector< ElementLink< xAOD::PFOContainer > > pfo_link_vector;
    for( uint ipfo = 0; ipfo != vec_pi0pfos.at(itlv).size(); ++ipfo) {
      pfo_link_vector.push_back(vec_pi0pfos.at(itlv).at(ipfo));
    }

    static const SG::AuxElement::Accessor<std::vector< ElementLink< xAOD::PFOContainer > > > accPi0PFOLinks("pi0PFOLinks");
    accPi0PFOLinks(*p) = pfo_link_vector;

    ElementLink< xAOD::IParticleContainer > linkToPi0;
    linkToPi0.toContainedElement(pi0Container, dynamic_cast<xAOD::IParticle*> (p));

    tauJet->addPi0Link(linkToPi0);
  }
    
  return StatusCode::SUCCESS;
}


// Calculate final 4-vector:
void PanTau::Tool_DetailsArranger::SetHLVTau( PanTau::PanTauSeed* inSeed, xAOD::TauJet* tauJet, const std::string& inputAlg, const std::string& varTypeName_Basic) {

  std::vector< ElementLink< xAOD::PFOContainer > > finalChrgPFOLinks       = tauJet->chargedPFOLinks();
  std::vector< ElementLink< xAOD::PFOContainer > > finalPi0PFOLinks        = tauJet->pi0PFOLinks();
  
  unsigned int NCharged    = finalChrgPFOLinks.size();
  unsigned int NPi0Neut    = finalPi0PFOLinks.size();

  TLorentzVector tlv_PanTau_Final;
  for(unsigned int iPFO=0; iPFO<NCharged; iPFO++) {
    const xAOD::PFO* pfo = finalChrgPFOLinks.at(iPFO).cachedElement();
    tlv_PanTau_Final += pfo->p4();
  }
  for(unsigned int iPFO=0; iPFO<NPi0Neut; iPFO++) {
    const xAOD::PFO* pfo = finalPi0PFOLinks.at(iPFO).cachedElement();
    tlv_PanTau_Final += pfo->p4();
  }

  inSeed->setFinalMomentum(tlv_PanTau_Final);

  PanTau::TauFeature* featureMap = inSeed->getFeatures();
  featureMap->addFeature(inputAlg + "_" + varTypeName_Basic + "_FinalMomentumCore_pt", tlv_PanTau_Final.Pt() );
  featureMap->addFeature(inputAlg + "_" + varTypeName_Basic + "_FinalMomentumCore_eta", tlv_PanTau_Final.Eta() );
  featureMap->addFeature(inputAlg + "_" + varTypeName_Basic + "_FinalMomentumCore_phi", tlv_PanTau_Final.Phi() );
  featureMap->addFeature(inputAlg + "_" + varTypeName_Basic + "_FinalMomentumCore_m", tlv_PanTau_Final.M() );
}


bool PanTau::Tool_DetailsArranger::HasMultPi0sInOneCluster(const xAOD::PFO* pfo, int decayModeProto, const std::string& inputAlg) const {

  // this is only relevant for reco 1p1n modes, hence restrict the output to these modes

  int nPi0sPerCluster = 1;

  if (inputAlg != "CellBased" ) return (nPi0sPerCluster > 1);

  // cell-based sets this to 1pXn however below this function is
  // called with the decayModeProto as evaluated by Pantau!
  if (decayModeProto != xAOD::TauJetParameters::Mode_1p1n ) return (nPi0sPerCluster > 1);
                
  if( !pfo->attribute(xAOD::PFODetails::nPi0Proto, nPi0sPerCluster) ) {
    ATH_MSG_WARNING("Could not retrieve nPi0Proto. Will set it to 1.");
    nPi0sPerCluster = 1;
  }

  return (nPi0sPerCluster > 1);
}


void PanTau::Tool_DetailsArranger::SetNeutralConstituentMass(xAOD::PFO* neutral_pfo, double mass) {

  TLorentzVector momentum; 
  PanTau::SetP4EEtaPhiM( momentum, neutral_pfo->e(), neutral_pfo->eta(), neutral_pfo->phi(), mass);
  neutral_pfo->setP4(momentum.Pt(), neutral_pfo->eta(), neutral_pfo->phi(), mass);
}


void PanTau::Tool_DetailsArranger::SetNeutralConstituentVectorMasses(const std::vector< ElementLink<xAOD::PFOContainer> >& neutralPFOLinks, xAOD::PFOContainer& neutralPFOContainer, double mass) {

  for (const auto& link : neutralPFOLinks) {
    size_t index = link.index();
    xAOD::PFO* curNeutralPFO = neutralPFOContainer.at(index);
    SetNeutralConstituentMass(curNeutralPFO, mass);
  }
    
  }


std::vector< ElementLink< xAOD::PFOContainer > > PanTau::Tool_DetailsArranger::CollectConstituentsAsPFOLinks( PanTau::PanTauSeed* inSeed,
													      const std::vector< ElementLink< xAOD::PFOContainer > >& cellbased_neutralPFOLinks,
													      PanTau::TauConstituent::Type type ) const {
  // collect element links from tau constituents in the Pantau
  // seed of type "type". cellbased_neutralPFOLinks is only used
  // to obtain the ElementLinks.

  std::vector< ElementLink< xAOD::PFOContainer > > new_links;

  unsigned int nConstsOfType=0;
  bool foundIt=false;
  std::vector<PanTau::TauConstituent*> tauConstituents=inSeed->getConstituentsOfType(type,foundIt);

  if( (type != PanTau::TauConstituent::t_Neutral && type != PanTau::TauConstituent::t_Pi0Neut) || !foundIt){
    ATH_MSG_WARNING("CollectConstituentsAsPFOLinks: Function was called with type = " << type << " , however it was only designed for types t_Pi0Neut and t_Neutral! Returning...");
    return new_links;
  }

  for(unsigned int iConst=0; iConst<tauConstituents.size(); iConst++) {
    bool isOfType = tauConstituents[iConst]->isOfType(type);
    if(!isOfType) continue;

    // if the requested type is t_Neutral then exclude any t_Pi0Neut
    // from the list (note: tau constituents that are t_Pi0Neut are
    // also t_Neutral at the same time):
    if(type==PanTau::TauConstituent::t_Neutral && tauConstituents[iConst]->isOfType(PanTau::TauConstituent::t_Pi0Neut) ) continue;
    ++nConstsOfType;

    for(unsigned int iPFO=0; iPFO<cellbased_neutralPFOLinks.size(); iPFO++) {
      const xAOD::PFO* pfo = cellbased_neutralPFOLinks.at(iPFO).cachedElement();

      if( tauConstituents[iConst]->getPFO() != pfo ) continue;

      new_links.push_back( cellbased_neutralPFOLinks.at(iPFO) );
    }
  }

  if( nConstsOfType != new_links.size() ){
    ATH_MSG_WARNING("CollectConstituentsAsPFOLinks: Couldn't find PFOLinks " << new_links.size() << " for all tau constituents (" << tauConstituents.size() << ")!");
  }

  return new_links;
}


//______________________________________________________________________________
void PanTau::Tool_DetailsArranger::createPi0Vectors(xAOD::TauJet* tauJet, std::vector<TLorentzVector>& vPi0s, std::vector< std::vector< ElementLink< xAOD::PFOContainer > > > &vec_pi0pfos) 
{
  // reset the pi0s
  vPi0s.clear();
  vec_pi0pfos.clear();

  // Since the PFO links as they come out of reconstruction, only correspond to
  // calorimeter clusters, whereas we want the consts_pi0 vectors to correspond
  // to real pi0s, we need to be careful to collect the PFOs correctly to pi0s
  // for the cases where number of pi0s does not match to the decay mode:
  size_t iNumPi0PFO = tauJet->nPi0PFOs();

  int iDecayMode = -1;
  tauJet->panTauDetail(xAOD::TauJetParameters::PanTau_DecayMode, iDecayMode);

  if (iDecayMode == xAOD::TauJetParameters::Mode_1p1n && iNumPi0PFO > 1) {

    // TODO: find out if the pi0 mass is defined elsewhere in atlas code!
    // float fMassPi0 = 134.98;
    float fMassPi0Squared = MASS_PI0*MASS_PI0;
    
    // combine both photons (with 0 mass that is already set) to one pi0 vector:
    const xAOD::PFO* xPfo1 = tauJet->pi0PFO(0);
    const xAOD::PFO* xPfo2 = tauJet->pi0PFO(1);
    vPi0s.push_back(xPfo1->p4() + xPfo2->p4());
    
    // re-set the mass to one pi0:
    double dNewMomentum = std::sqrt(vPi0s[0].E() * vPi0s[0].E() - fMassPi0Squared);
    vPi0s[0].SetPxPyPzE(vPi0s[0].Vect().Unit().Px() * dNewMomentum,
			vPi0s[0].Vect().Unit().Py() * dNewMomentum,
			vPi0s[0].Vect().Unit().Pz() * dNewMomentum,
			vPi0s[0].E());

    std::vector< ElementLink<xAOD::PFOContainer> > pfovec;
    pfovec.push_back(tauJet->pi0PFOLinks()[0]);
    pfovec.push_back(tauJet->pi0PFOLinks()[1]);
    vec_pi0pfos.push_back( pfovec );

  } else if (iDecayMode == xAOD::TauJetParameters::DecayMode::Mode_1pXn && iNumPi0PFO == 1){

    // make a single pi0 from a PFO that contains two pi0s:
    const xAOD::PFO* xPfo = tauJet->pi0PFO(0);
    // add the 2-pi0 vector preliminarily to the pi0vector:
    vPi0s.push_back(xPfo->p4());
    
    // re-set the mass back to one pi0:
    double dNewMomentum = std::sqrt( (vPi0s[0].E()/2.)*(vPi0s[0].E()/2.) - MASS_PI0*MASS_PI0 );
    vPi0s[0].SetVectM(vPi0s[0].Vect().Unit() * dNewMomentum, MASS_PI0 );
    
    // create another pi0 from the same vector:
    vPi0s.push_back(vPi0s[0]);

    std::vector< ElementLink<xAOD::PFOContainer> > pfovec;
    pfovec.push_back(tauJet->pi0PFOLinks()[0]);
    vec_pi0pfos.push_back( pfovec );
    vec_pi0pfos.push_back( pfovec );//fix rare crash?

  }  else {
    // if it's not any of the special cases above then just collect the PFOs:
    for (size_t iPFO = 0; iPFO < iNumPi0PFO; iPFO++){
      vPi0s.push_back(tauJet->pi0PFO(iPFO)->p4());
      std::vector< ElementLink<xAOD::PFOContainer> > pfovec;
      pfovec.push_back(tauJet->pi0PFOLinks()[iPFO]);
      vec_pi0pfos.push_back( pfovec );
    }
  }

}
