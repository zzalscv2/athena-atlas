/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
// file:        TauPi0ScoreCalculator.cxx
// package:     Reconstruction/tauRec
// authors:     Benedict Winter, Will Davey
// date:        2012-10-09
//
//-----------------------------------------------------------------------------

#include <vector>

#include "tauRecTools/TauPi0ScoreCalculator.h"
#include "tauRecTools/HelperFunctions.h"
#include "xAODPFlow/PFO.h"

#include "MVAUtils/BDT.h"

using std::vector;
using std::string;

//-------------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------------

TauPi0ScoreCalculator::TauPi0ScoreCalculator( const string& name ) :
  TauRecToolBase(name),
  m_mvaBDT(nullptr),
  m_Abs_FIRST_ETA(0),
  m_SECOND_R(0),
  m_Abs_DELTA_THETA(0),
  m_CENTER_LAMBDA_helped(0),
  m_LONGITUDINAL(0),
  m_ENG_FRAC_EM(0),
  m_ENG_FRAC_CORE(0),
  m_log_SECOND_ENG_DENS(0),
  m_EcoreOverEEM1(0),
  m_NPosCells_EM1(0),
  m_NPosCells_EM2(0),
  m_firstEtaWRTCluster_EM1(0),
  m_secondEtaWRTCluster_EM2(0)
{
    declareProperty("BDTWeightFile",           m_weightfile);
}

//-------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------

TauPi0ScoreCalculator::~TauPi0ScoreCalculator() 
{
}


StatusCode TauPi0ScoreCalculator::initialize() 
{
  //---------------------------------------------------------------------
  // Create TMVA reader
  //---------------------------------------------------------------------
  m_availableVars.insert( std::make_pair("Pi0Cluster_Abs_FIRST_ETA"         ,&m_Abs_FIRST_ETA) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_SECOND_R"              ,&m_SECOND_R) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_Abs_DELTA_THETA"       ,&m_Abs_DELTA_THETA) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_CENTER_LAMBDA_helped"  ,&m_CENTER_LAMBDA_helped) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_LONGITUDINAL"          ,&m_LONGITUDINAL) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_ENG_FRAC_EM"           ,&m_ENG_FRAC_EM) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_ENG_FRAC_CORE"         ,&m_ENG_FRAC_CORE) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_log_SECOND_ENG_DENS"   ,&m_log_SECOND_ENG_DENS) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_EcoreOverEEM1"         ,&m_EcoreOverEEM1) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_NPosECells_EM1"        ,&m_NPosCells_EM1) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_NPosECells_EM2"        ,&m_NPosCells_EM2) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_AbsFirstEtaWRTClusterPosition_EM1" ,&m_firstEtaWRTCluster_EM1) );
  m_availableVars.insert( std::make_pair("Pi0Cluster_secondEtaWRTClusterPosition_EM2" ,&m_secondEtaWRTCluster_EM2) );

  std::string weightFile = find_file(m_weightfile);

  m_mvaBDT = tauRecTools::configureMVABDT(m_availableVars, weightFile);
  if(m_mvaBDT==nullptr) {
    ATH_MSG_FATAL("Couldn't configure MVA");
    return StatusCode::FAILURE;
  }
 
  return StatusCode::SUCCESS;
}

StatusCode TauPi0ScoreCalculator::finalize()
{
  return StatusCode::SUCCESS;
}


StatusCode TauPi0ScoreCalculator::executePi0nPFO(xAOD::TauJet& pTau, xAOD::PFOContainer& neutralPFOContainer) 
{
    //---------------------------------------------------------------------
    // only run on 1-5 prong taus 
    //---------------------------------------------------------------------
    if (pTau.nTracks() == 0 || pTau.nTracks() >5 ) {
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("ScoreCalculator: new tau. \tpt = " << pTau.pt() << "\teta = " << pTau.eta() << "\tphi = " << pTau.phi() << "\tnprongs = " << pTau.nTracks());

    //---------------------------------------------------------------------
    // retrieve neutral PFOs from tau, calculate BDT scores and store them in PFO
    //---------------------------------------------------------------------
    for( auto neutralPFO : neutralPFOContainer )
    {
      float BDTScore = calculateScore(neutralPFO);
      neutralPFO->setBDTPi0Score((float) BDTScore);
    }

    ATH_MSG_DEBUG("End of TauPi0ScoreCalculator::execute");

    return StatusCode::SUCCESS;
}


float TauPi0ScoreCalculator::calculateScore(const xAOD::PFO* neutralPFO)
{
    m_Abs_FIRST_ETA=0.;
    m_SECOND_R=0.;
    m_Abs_DELTA_THETA=0.;
    m_CENTER_LAMBDA_helped=0.;
    m_LONGITUDINAL=0.;
    m_ENG_FRAC_EM=0.;
    m_ENG_FRAC_CORE=0.;
    m_log_SECOND_ENG_DENS=0.;
    m_EcoreOverEEM1=0.;
    // Need to convert int variables to floats after retrieving them
    int NPosCells_EM1=0;
    int NPosCells_EM2=0;
    m_firstEtaWRTCluster_EM1=0.;
    m_secondEtaWRTCluster_EM2=0.;

    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_FIRST_ETA,m_Abs_FIRST_ETA) == false)
        ATH_MSG_WARNING("Can't find FIRST_ETA. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_SECOND_R,m_SECOND_R) == false)
        ATH_MSG_WARNING("Can't find SECOND_R. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_DELTA_THETA,m_Abs_DELTA_THETA) == false)
        ATH_MSG_WARNING("Can't find DELTA_THETA. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_CENTER_LAMBDA,m_CENTER_LAMBDA_helped) == false) 
        ATH_MSG_WARNING("Can't find CENTER_LAMBDA. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_LONGITUDINAL,m_LONGITUDINAL) == false) 
        ATH_MSG_WARNING("Can't find LONGITUDINAL. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_ENG_FRAC_EM,m_ENG_FRAC_EM) == false) 
        ATH_MSG_WARNING("Can't find ENG_FRAC_EM. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_ENG_FRAC_CORE,m_ENG_FRAC_CORE) == false) 
        ATH_MSG_WARNING("Can't find ENG_FRAC_CORE. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_SECOND_ENG_DENS,m_log_SECOND_ENG_DENS) == false) 
        ATH_MSG_WARNING("Can't find SECOND_ENG_DENS. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_EM1CoreFrac,m_EcoreOverEEM1) == false) 
        ATH_MSG_WARNING("Can't find EM1CoreFrac. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_NPosECells_EM1,NPosCells_EM1) == false) 
        ATH_MSG_WARNING("Can't find NPosECells_EM1. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_NPosECells_EM2,NPosCells_EM2) == false) 
        ATH_MSG_WARNING("Can't find NPosECells_EM2. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_firstEtaWRTClusterPosition_EM1,m_firstEtaWRTCluster_EM1) == false) 
        ATH_MSG_WARNING("Can't find firstEtaWRTClusterPosition_EM1. Set it to 0.");
    if(neutralPFO->attribute(xAOD::PFODetails::PFOAttributes::cellBased_secondEtaWRTClusterPosition_EM2,m_secondEtaWRTCluster_EM2) == false) 
        ATH_MSG_WARNING("Can't find secondEtaWRTClusterPosition_EM2. Set it to 0.");
    // Apply variable transformations
    m_Abs_FIRST_ETA   = fabs(m_Abs_FIRST_ETA);
    m_Abs_DELTA_THETA = fabs(m_Abs_DELTA_THETA);
    m_CENTER_LAMBDA_helped = fmin(m_CENTER_LAMBDA_helped, 1000.);
    if(m_log_SECOND_ENG_DENS==0.) m_log_SECOND_ENG_DENS=-50.;
    else m_log_SECOND_ENG_DENS = log(m_log_SECOND_ENG_DENS);
    // Convert ints to floats so they can be read by the TMVA reader
    m_NPosCells_EM1 = (float) NPosCells_EM1;
    m_NPosCells_EM2 = (float) NPosCells_EM2;
    m_firstEtaWRTCluster_EM1 = fabs(m_firstEtaWRTCluster_EM1);

    // Calculate BDT score
    float BDTScore = m_mvaBDT->GetGradBoostMVA(m_mvaBDT->GetPointers());
     
    return BDTScore;
}
