///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// CaloClusterMatchLinkAlg.cxx 
// Implementation file for class CaloClusterMatchLinkAlg
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// CaloClusterMatching includes
#include "CaloClusterMatchLinkAlg.h"

// STL includes

// FrameWork includes
#include "GaudiKernel/Property.h"
#include "CaloClusterMatching/TopoClusterMap.h"
#include "CaloClusterMatching/ICaloClusterMatchingTool.h"


namespace ClusterMatching {
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 

  // Constructors
  ////////////////
  CaloClusterMatchLinkAlg::CaloClusterMatchLinkAlg( const std::string& name, 
						    ISvcLocator* pSvcLocator ) : 
    ::AthReentrantAlgorithm( name, pSvcLocator ),
    m_clusterMatch("ClusterMatching::CaloClusterMatchingTool/CaloClusterMatch")
  {
    //
    // Property declaration
    // 
    //declareProperty( "Property", m_nProperty );

    declareProperty( "ClusterMatchTool",    m_clusterMatch               );
    declareProperty( "UseLeadCellEtaPhi",   m_useLeadCellEtaPhi=false    );
    declareProperty( "ClusterSortMethod",   m_clusterSortMethod=MatchedE );

  }

  // Destructor
  ///////////////
  CaloClusterMatchLinkAlg::~CaloClusterMatchLinkAlg()
  {}

  // Athena Algorithm's Hooks
  ////////////////////////////
  StatusCode CaloClusterMatchLinkAlg::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");

    ATH_CHECK( m_clusterKey.initialize() );

    ATH_CHECK( m_clusterMatch.retrieve() );

    if(m_clusterSortMethod<0 || m_clusterSortMethod>ClusterMatching::MatchedEFrac) {
      ATH_MSG_ERROR("Invalid configuration of cluster sortin method");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode CaloClusterMatchLinkAlg::finalize()
  {
    ATH_MSG_INFO ("Finalizing " << name() << "...");

    return StatusCode::SUCCESS;
  }

  StatusCode CaloClusterMatchLinkAlg::execute_r(const EventContext& ctx) const
  {  
    ATH_MSG_DEBUG ("Executing " << name() << "...");

    SG::ReadHandle<xAOD::CaloClusterContainer> clustersToDecorate (m_clusterKey, ctx);

    TopoClusterMap tcmap;
    ATH_CHECK( m_clusterMatch->fillClusterMap(ctx,tcmap) );

    bool (*gtrthan)(const tcmatch_pair& a, const tcmatch_pair& b)=0;
    switch(m_clusterSortMethod) {
    case ClusterE: gtrthan = ClusterMatching::gtrClusterE;
      break;
    case MatchedE: gtrthan = ClusterMatching::gtrMatchedE;
      break;
    case MatchedEFrac: gtrthan = ClusterMatching::gtrMatchedEFrac;
      break;
    default:
      ATH_MSG_WARNING("Invalid cluster sorting method configured!");
      return StatusCode::SUCCESS;
    }

    for(const auto& cl : *clustersToDecorate) {
      ATH_CHECK( m_clusterMatch->linkMatchedClusters(*cl, tcmap, m_useLeadCellEtaPhi, gtrthan) );
    }

    return StatusCode::SUCCESS;
  }

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 


}
