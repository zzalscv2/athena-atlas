/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************

NAME:     eflowObject.cxx
PACKAGE:  offline/Reconstruction/eflowRec

AUTHORS:  D.R. Tovey
CREATED:  18th November, 2001

UPDATED:  14th March 2003 (P Loch) implement navigation system

UPDATED:   8th April 2004 (P Loch) implement new navigation scheme

********************************************************************/

#include "FourMom/P4EEtaPhiM.h"

#include "eflowEvent/eflowObject.h"
#include "eflowEvent/eflowCaloCluster.h"

#include "CaloEvent/CaloCluster.h"
#include "CaloEvent/CaloClusterContainer.h"

#include "Particle/TrackParticle.h"
#include "Particle/TrackParticleContainer.h"

#include "VxVertex/VxCandidate.h"

#include <cmath>
#include <vector>

#include "Navigation/INavigationToken.h"
#include "Navigation/NavigationToken.h"
#include "Navigation/NavigableVector.h"

////////////////////////////////////////////////////////////////////
// temporary for navigation !!
#include <any>

eflowObject::eflowObject()
  : P4EEtaPhiM(0.,0.,0.,0.),
    m_passedEOverPCheck (true), //by default true
    m_eflowObjectType (ChargedPion),
    m_recoStatus(),
    m_valid (true)
{
}

eflowObject::eflowObject(eflowObject* eflowObj) : P4EEtaPhiM(eflowObj->e(),eflowObj->eta(),eflowObj->phi(),eflowObj->m())
{
  this->initialize(eflowObj,true);
}

eflowObject::eflowObject(eflowObject* eflowObj, bool useClus) : P4EEtaPhiM(eflowObj->e(),eflowObj->eta(),eflowObj->phi(),eflowObj->m())
{
  this->initialize(eflowObj,useClus);
}


void eflowObject::initialize(eflowObject* eflowObj, bool useClus) 
{
  m_d0 = eflowObj->d0();
  m_z0 = eflowObj->z0();
  m_eflowType = eflowObj->eflowType();
  m_charge = eflowObj->charge();
  m_valid = eflowObj->isValid();
  m_passedEOverPCheck = eflowObj->getPassEOverPCheck();
  m_isSubtracted = eflowObj->getIsSubtracted();
  m_isDuplicated = eflowObj->getIsDuplicated();
  m_recoStatus = eflowObj->getCaloRecoStatus();
  m_nTrack = eflowObj->numTrack();
  m_nClus = eflowObj->numClus();
  m_pi0MVA = eflowObj->getPi0MVA();
  m_centerMag = eflowObj->getCenterMag();
  m_eflowObjectType = eflowObj->m_eflowObjectType;

  //add the conversion
  this->addConversion(eflowObj->m_convElementLink);
  //add the muon
  this->addMuon(eflowObj->m_muonElementLink);

  //add any tracks
  this->m_eflowTrack = eflowObj->m_eflowTrack;

  //*possibly* add some clusters
  if (useClus) this->m_eflowClus = eflowObj->m_eflowClus;


}


eflowObject::~eflowObject()
{
  for (CaloClusterContainer* c : m_eflowClusContainers)
    delete c;
}

bool eflowObject::checkParticleType(ParticleType particleType) const{
  return m_eflowObjectType == particleType;
}

const Analysis::Muon* eflowObject::muon() const         { 
    if (m_muonElementLink.isValid()) return *m_muonElementLink;
    else{
      const Analysis::Muon* muon(nullptr);
      return muon;
    }
  }


// new interface supports persistency
void eflowObject::addClus(const ElementLink<CaloClusterContainer>& clusElementLink)
{
  m_eflowClus.addElement(clusElementLink);
}

void eflowObject::addClus(const CaloCluster* clus)
{
  CaloClusterContainer* newContainer = new CaloClusterContainer(SG::VIEW_ELEMENTS);
  eflowCaloCluster* newClus = new eflowCaloCluster(clus);
  newContainer->push_back(newClus);
  m_eflowClus.addElement(newContainer,newClus);
  m_eflowClusContainers.push_back(newContainer);
}


void eflowObject::addTrack(const ElementLink<Rec::TrackParticleContainer>& trackElementLink)
{
  //const Rec::TrackParticleContainer* trackContainer = trackElementLink.getDataPtr();
  //const Rec::TrackParticle* track = *trackElementLink;
  //m_eflowTrack.addElement(trackContainer,track);
  m_eflowTrack.addElement(trackElementLink);
}

////////////////
// Navigation //
////////////////

void
eflowObject::fillToken(INavigationToken& thisToken,
		       const std::any& aRelation) const
{
  //---------------------------------------------------------------------------
  // eflowObject can honor several queries:
  //
  // - CaloCluster
  // - TrackParticle
  // - Muon
  // - Conversion
  //
  // There are two potential objects to forward the query to, CaloCluster and
  // the egamma object. 
  //---------------------------------------------------------------------------
  
  // parameter type checking
  double weight;
  try        { weight = std::any_cast<double>(aRelation); } 
  catch(...) { return; }

  //////////////////////////
  // Calorimeter Response //
  //////////////////////////

  if ( m_eflowClus.size() > 0 ) 
    {
      this->navigateClusters(m_eflowClus,thisToken,weight);
    }

  /////////////////
  // TrackParticle //
  /////////////////

  // called from within navigateClusters for performance optimization

  //////////////////
  // Muon //
  //////////////////

  // called from within navigateClusters for performance optimization

  ////////////////
  // Conversion //
  ////////////////

  // called from within navigateClusters for performance optimization

}

//////////////////////////////////////////////
// New Navigation: Temporary Implementation //
//////////////////////////////////////////////

// navigate CaloClusters
void eflowObject::navigateClusters(const cluster_type& theClusters,
			      INavigationToken&   thisToken,
			      double              weight) const
{
  // navigation with weights
  NavigationToken<CaloCluster,double>* weightedToken = 
    dynamic_cast< NavigationToken<CaloCluster,double>* >(&thisToken);
  NavigationToken<CaloCluster>* simpleToken = 
    weightedToken == nullptr
    ? dynamic_cast< NavigationToken<CaloCluster>* >(&thisToken)
    : nullptr;

  // query can not be honored, check on other types within eflowObject
  bool isHonored = weightedToken != nullptr || simpleToken != nullptr;
  if ( ! isHonored )
    { 
      if ( m_eflowTrack.size() > 0 )
	{ 
	  isHonored = this->navigateTrackParticles(thisToken,weight);
	}
    }
  if ( ! isHonored )
    {
      isHonored = this->navigateMuons(thisToken,weight);
    }
  if ( ! isHonored )
    {
      isHonored = this->navigateConversions(thisToken,weight);
    }

  // forward query
  if ( ! isHonored )
    {
      for (const CaloCluster* c : theClusters)
	{
	  c->fillToken(thisToken,weight);
	}
    }

  // fill token
  else
    {
      if ( weightedToken != nullptr ) 
	{
	  this->
	    toToken< cluster_type, NavigationToken<CaloCluster,double> >
	    (theClusters,weightedToken,weight);
	}
      else if (simpleToken != nullptr)
	{
	  this->
	    toToken< cluster_type, NavigationToken<CaloCluster> >
	    (theClusters,simpleToken);
	}
    }
}

// navigate TrackParticles
bool
eflowObject::navigateTrackParticles(INavigationToken& thisToken, 
				  double weight) const
{
  NavigationToken<Rec::TrackParticle,double>* weightedToken = 
    dynamic_cast< NavigationToken<Rec::TrackParticle,double>* >(&thisToken);
  NavigationToken<Rec::TrackParticle>* simpleToken =
    weightedToken == nullptr 
    ? dynamic_cast< NavigationToken<Rec::TrackParticle>* >(&thisToken)
    : nullptr;
  bool isHonored = weightedToken != nullptr || simpleToken != nullptr;
  
  if ( isHonored ) 
    {
      if ( weightedToken != nullptr )
	{
	  this->toToken< eflowTrack_type, NavigationToken<Rec::TrackParticle,double> >
	    (m_eflowTrack,weightedToken,weight);
	}
      else
	{
	  this->toToken< eflowTrack_type, NavigationToken<Rec::TrackParticle> >
	    (m_eflowTrack,simpleToken);
	}
    }

  return isHonored;
}

// navigate Muons
bool
eflowObject::navigateMuons(INavigationToken& thisToken,
			   double weight) const
{
  NavigationToken<Analysis::Muon,double>* weightedToken = 
    dynamic_cast< NavigationToken<Analysis::Muon,double>* >(&thisToken);
  NavigationToken<Analysis::Muon>* simpleToken =
    weightedToken == nullptr
    ? dynamic_cast< NavigationToken<Analysis::Muon>* >(&thisToken)
    : nullptr;
  // honored
  bool isHonored = weightedToken != nullptr || simpleToken != nullptr;

  if ( isHonored )
    {
      if ( weightedToken != nullptr )
	{
	  weightedToken->setObject(*m_muonElementLink,weight);
	}
      else
	{
	  simpleToken->setObject(*m_muonElementLink);
	}
    }

  return isHonored;
}

// navigate conversions
bool
eflowObject::navigateConversions(INavigationToken& thisToken,
				 double weight) const
{
  NavigationToken<Trk::VxCandidate,double>* weightedToken =
    dynamic_cast< NavigationToken<Trk::VxCandidate,double>* >(&thisToken);
  NavigationToken<Trk::VxCandidate>* simpleToken =
    weightedToken == nullptr
    ? dynamic_cast< NavigationToken<Trk::VxCandidate>* >(&thisToken)
    : nullptr;

  bool isHonored = weightedToken != nullptr || simpleToken != nullptr;
  
  if ( isHonored )
    {
      if ( weightedToken != nullptr )
	{
	  weightedToken->setObject(*m_convElementLink,weight);
	}
      else
	{
	  simpleToken->setObject(*m_convElementLink);
	}
    }

  return isHonored;
}

template <typename CONT, typename TOKEN>
void 
eflowObject::toToken(const CONT&  theData,
		     TOKEN*       theToken,
		     double        weight) const
{
  for (const auto* p : theData)
    {
      theToken->setObject(p,weight);
    }
}

template <typename CONT, typename TOKEN>
void 
eflowObject::toToken(const CONT&  theData,
		     TOKEN*       theToken) const
{
  for (const auto* p : theData)
    {
      theToken->setObject(p);
    }
}

const ElementLink<Analysis::MuonContainer>& eflowObject::muonLink() const { return m_muonElementLink;}

const ElementLink<VxContainer>& eflowObject::conversionLink() const { return m_convElementLink;}
		     
	  
