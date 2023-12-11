/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/



#include "xAODJet/JetAccessorMap.h"
#include "xAODCaloEvent/CaloCluster.h"

#include "xAODPFlow/PFO.h"
#include "xAODPFlow/FlowElement.h"

#include "JetUtils/JetCaloCalculations.h"
#include "PFlowUtils/FEHelpers.h"
#include "xAODJet/JetAttributes.h"
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

namespace CaloConstitHelpers {
  
  using JetConstitIterator = xAOD::JetConstituentVector::iterator;
  

  ///****************************************************
  /// \class CaloClusterExtractor
  /// 
  /// Extract cluster moments when JetConstituents are CaloCluster
  struct CaloClusterExtractor : public CaloConstitExtractor {
    virtual ~CaloClusterExtractor()= default;
    virtual bool valid(JetConstitIterator & it ) const override {
      return (dynamic_cast<const xAOD::CaloCluster*>(it->rawConstituent())!=nullptr);
    }

    virtual double moment(JetConstitIterator & it, xAOD::CaloCluster::MomentType momentType) const override {
      double m = 0;
      static_cast<const xAOD::CaloCluster*>(it->rawConstituent())->retrieveMoment(momentType,m) ;
      return m;
    }

    virtual double time(JetConstitIterator & it) const override {
      return static_cast<const xAOD::CaloCluster*>(it->rawConstituent())->time();
    }

    virtual double energyHEC(JetConstitIterator & it) const override {
      const xAOD::CaloCluster* cl = static_cast<const xAOD::CaloCluster*>(it->rawConstituent());
      return cl->eSample( CaloSampling::HEC0) + cl->eSample( CaloSampling::HEC1) + 
        cl->eSample( CaloSampling::HEC2) + cl->eSample( CaloSampling::HEC3);
    }

  };

  ///****************************************************
  /// \class PFlowExtractor
  /// 
  /// Extract cluster moments when JetConstituents are PFO particles.
  struct PFOExtractor : public CaloConstitExtractor {
    virtual ~PFOExtractor()= default;
    virtual bool valid(JetConstitIterator & it ) const override {
      const xAOD::PFO* pfo = dynamic_cast<const xAOD::PFO*>(it->rawConstituent());
      if (pfo!=nullptr) return (!pfo->isCharged());
      return false;
    }

    virtual double moment(JetConstitIterator & it , xAOD::CaloCluster::MomentType momentType) const override {
      float m=0.;
      const xAOD::PFO* pfo = static_cast<const xAOD::PFO*>(it->rawConstituent()) ;
      pfo->getClusterMoment(m, momentType );
      return m;      
    }

    virtual double time(JetConstitIterator & it) const override {
      float t=0.;
      const xAOD::PFO* pfo = static_cast<const xAOD::PFO*>(it->rawConstituent()) ;
      pfo->attribute( xAOD::PFODetails::eflowRec_TIMING, t);
      return t;
    }        

    virtual double energyHEC(JetConstitIterator & it ) const override {
      float m=0.;
      const xAOD::PFO* pfo = static_cast<const xAOD::PFO*>(it->rawConstituent()) ;
      pfo->attribute( xAOD::PFODetails::eflowRec_LAYERENERGY_HEC, m);
      return m;
    }

  };

  ///****************************************************
  /// \class FlowElementExtractor
  /// 
  /// Extract cluster moments when JetConstituents are FlowElements.
  struct FlowElementExtractor : public CaloConstitExtractor {
    virtual ~FlowElementExtractor()= default;
    virtual bool valid(JetConstitIterator & it ) const override {
      const xAOD::FlowElement* fe = dynamic_cast<const xAOD::FlowElement*>(it->rawConstituent());
      if (fe != nullptr) return (!fe->isCharged());
      return false;
    }

    virtual double moment(JetConstitIterator & it , xAOD::CaloCluster::MomentType momentType) const override {
      float m=0.;
      const xAOD::FlowElement* fe = static_cast<const xAOD::FlowElement*>(it->rawConstituent());
      FEHelpers::getClusterMoment(*fe, momentType, m);
      return m;      
    }

    virtual double time(JetConstitIterator & it) const override {
      const xAOD::FlowElement* fe = static_cast<const xAOD::FlowElement*>(it->rawConstituent());
      const static SG::AuxElement::ConstAccessor<float> accTiming("TIMING");

      float timing = -1;

      if(accTiming.isAvailable(*fe)){
	timing = accTiming(*fe);
      }
      else{
	if(fe->otherObjects().size() == 1 && fe->otherObject(0)){
	  const auto* neutralObject = (fe->otherObject(0));
	  const xAOD::CaloCluster* cluster = nullptr;

	  if(neutralObject->type() == xAOD::Type::CaloCluster){
            cluster = dynamic_cast<const xAOD::CaloCluster*> (neutralObject);
          }
          //If we have a PFO (in case of fe being a UFO), we need to get the associated cluster first
          else {
            const xAOD::FlowElement* pfo = dynamic_cast<const xAOD::FlowElement*>(neutralObject);
            if(!pfo->otherObjects().empty() && pfo->otherObject(0) && pfo->otherObject(0)->type() == xAOD::Type::CaloCluster){
              cluster = dynamic_cast<const xAOD::CaloCluster*> (pfo->otherObject(0));
            }
          }

	  if(cluster){
	    timing = cluster->time();
	  }
	}
      }
      return timing;
    }        

    virtual double energyHEC(JetConstitIterator & it ) const override {
      const xAOD::FlowElement* fe = static_cast<const xAOD::FlowElement*>(it->rawConstituent());

      // Add up the four individual HEC layers
      const static SG::AuxElement::ConstAccessor<float> accHEC0("LAYERENERGY_HEC0");
      const static SG::AuxElement::ConstAccessor<float> accHEC1("LAYERENERGY_HEC1");
      const static SG::AuxElement::ConstAccessor<float> accHEC2("LAYERENERGY_HEC2");
      const static SG::AuxElement::ConstAccessor<float> accHEC3("LAYERENERGY_HEC3");

      float sum_HEC = 0.0;

      // The variables are available for PFOs but not UFOs
      if(accHEC0.isAvailable(*fe) && accHEC1.isAvailable(*fe) && accHEC2.isAvailable(*fe) && accHEC3.isAvailable(*fe)){
	sum_HEC = accHEC0(*fe) + accHEC1(*fe) + accHEC2(*fe) + accHEC3(*fe);
      }
      else{
	for (size_t n = 0; n < fe->otherObjects().size(); ++n) {
	  if(! fe->otherObject(n)) continue;
	  const auto* neutralObject = (fe->otherObject(n));

	  const xAOD::CaloCluster* cluster = nullptr;

	  //If we have a cluster, we can directly access the calorimeter information
	  if(neutralObject->type() == xAOD::Type::CaloCluster){
	    cluster = dynamic_cast<const xAOD::CaloCluster*> (neutralObject);
	  }
	  //If we have a PFO (in case of fe being a UFO), we need to get the associated cluster first
	  else {
	    const xAOD::FlowElement* pfo = dynamic_cast<const xAOD::FlowElement*>(neutralObject);
	    if(!pfo->otherObjects().empty() && pfo->otherObject(0) && pfo->otherObject(0)->type() == xAOD::Type::CaloCluster){
	      cluster = dynamic_cast<const xAOD::CaloCluster*> (pfo->otherObject(0));
	    }
	  }
	  if(cluster){
	    sum_HEC += cluster->eSample( CaloSampling::HEC0 ) + cluster->eSample( CaloSampling::HEC1 ) + cluster->eSample( CaloSampling::HEC2 ) + cluster->eSample( CaloSampling::HEC3 );
	  }
	}
      }
      return sum_HEC;
    }

  };


  /// returns a pointer to a CaloConstitExtractor for a given jet. Do not delete the pointer !
  /// WARNING : not entirely safe. Assumes that all jet constituents have the same type as 1st constit.
  const CaloConstitExtractor* extractorForJet(const xAOD::Jet* jet){
    static const CaloConstitExtractor nullEx;
    static const CaloClusterExtractor clusteEx;
    static const PFOExtractor pfoEx;
    static const FlowElementExtractor feEx;

    if(jet->numConstituents() == 0 ) return &nullEx;    

    // WARNING not entirely safe : assume all constits are of same type

    switch(jet->rawConstituent(0)->type() ) {
    case xAOD::Type::CaloCluster: 
      return &clusteEx;
    case xAOD::Type::ParticleFlow:
      return &pfoEx;
    case xAOD::Type::FlowElement:
      return &feEx;
    default:
      break;
    }
    return &nullEx;
  }

}



// Implementation of JetCaloCalculator framwework
namespace jet {
  
  JetCaloCalculator::JetCaloCalculator(xAOD::JetAttribute::AttributeID id) : 
    m_name(xAOD::JetAttributeAccessor::name(id)), 
    m_id(id)
  {
    //    std::cout<< "  JetCaloCalculatorBase" << " id "<< id <<std::endl;
  }

  double JetCaloCalculator::operator()(const xAOD::Jet* jet, xAOD::JetConstitScale constitscale){

    if (! setupJet( jet ) ) return 0;
    size_t nConstit = jet->numConstituents();
    if( nConstit == 0) return true;

    // retrieve the cluster moment extractor for this jet.
    m_constitExtractor = CaloConstitHelpers::extractorForJet(jet);
      
    
    xAOD::JetConstituentVector constits = jet->getConstituents();
    // Use the constituent iterator
    // IMPORTANT : use UncalibratedJetConstituent.
    // By default the calculators will just use the kinematic from the iterator they are
    // given. This allows to choose what scale we give them.
    // Here we use UncalibratedJetConstituent as default, because most (or all) calo quantity to be calculated
    // are based on EM scale cluster moments.
    xAOD::JetConstituentVector::iterator it  = constits.begin(constitscale );
    xAOD::JetConstituentVector::iterator itE = constits.end(constitscale);
    
    for(; it != itE; ++it)
      {         
        if( m_constitExtractor->valid(it) )
          processConstituent(it); // (no support for weight now)          
      }
    
    return jetCalculation();
  
  }



  // ********************************************************
  // JetCaloCalculations methods
  // ********************************************************
  void JetCaloCalculations::addCalculator(JetCaloCalculator *c){ 
    m_calculators.push_back(c); 
  }

  // ********************************************************
  bool JetCaloCalculations::setupEvent(){
    for(size_t i=0;i < m_calculators.size();i++) m_calculators[i]->setupEvent();
    return true;
  }


  // ********************************************************
  std::vector<double> JetCaloCalculations::process(const xAOD::Jet* jet) const {
    size_t nConstit = jet->numConstituents();
    std::vector<double> results;
    results.reserve(m_calculators.size());

    if( nConstit == 0) {
      results.resize(m_calculators.size(),0);
      return results;
    }

    // Clone calculators : we are in a const method, we can't call the 
    // non const setupJet() and  processConstituent() methods of our calculators.
    std::vector<JetCaloCalculator*> clonedCalc;
    clonedCalc.reserve(m_calculators.size());
    for( const JetCaloCalculator *calc : m_calculators) clonedCalc.push_back( calc->clone() );  
 
    // prepare each calculator
    for(JetCaloCalculator* calc: clonedCalc) {
      calc->setupJet(jet);
    }
   
    // retrieve the cluster moment extractor for this jet.
    const CaloConstitHelpers::CaloConstitExtractor * extractor = CaloConstitHelpers::extractorForJet(jet);
    // assign the extractor to the calculators :
    for( JetCaloCalculator* calc : clonedCalc) calc->setExtractor(extractor);

    xAOD::JetConstituentVector constits = jet->getConstituents();
    // Use the constituent iterator at UNCALIBRATED scale 
    xAOD::JetConstituentVector::iterator it  = constits.begin(xAOD::UncalibratedJetConstituent );
    xAOD::JetConstituentVector::iterator itE = constits.end(xAOD::UncalibratedJetConstituent);
                  
    // loop over constituents
    for(; it != itE; ++it)
      {
        //std::cout << " processing constit  ---- "<< (*it)->pt() << " v= "<< extractor->valid(it) << std::endl;
        
        if( ! extractor->valid(it) ) continue; // this can happen for charged pflow objects
        // loop over calculators for this constituent.
        for(JetCaloCalculator* calc : clonedCalc) {
          calc->processConstituent(it);
        }
      } // constituents loop
  
    // copy results & clear the cloned calc
    for(JetCaloCalculator* calc: clonedCalc) {
      results.push_back( calc->jetCalculation() );
      delete calc;
    }
    
    return results;
  }

  
  // std::vector<double> JetCaloCalculations::calculations(){

  //   std::vector<double> v(m_calculators.size());
  //   for(size_t i=0;i < m_calculators.size();i++) {
  //     v[i] =  m_calculators[i]->jetCalculation() ;
  //   }
  //   return v;
  // }

  void JetCaloCalculations::clear(){
    if(m_owncalculators) for(size_t i=0;i < m_calculators.size();i++) delete m_calculators[i];
    m_calculators.clear();
  }
}
