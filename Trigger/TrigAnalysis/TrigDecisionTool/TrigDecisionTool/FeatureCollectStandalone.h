// -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGER_DECISION_TOOL_FeatureCollectStandalone_H
#define TRIGGER_DECISION_TOOL_FeatureCollectStandalone_H


/**********************************************************************************
 * @Project:
 * @Package: TrigDecision
 *
 *
 * @author Nicolas Berger  <Nicolas.Berger@cern.ch>  - LAPP Annecy
 * @author Tomasz Bold     <tomasz.bold@cern.ch>     - UC Irvine, AGH-UST Krakow
 * @author Lukas Heinrich  <lukas.heinrich@cern.ch>  - NYU
 *
 ***********************************************************************************/

#include <string>
#include <set>
#include "boost/foreach.hpp"
#include "boost/type_traits/is_same.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/lexical_cast.hpp"

#include "TrigDecisionTool/Conditions.h"
#include "TrigDecisionTool/TDTUtilities.h"

#include "TrigDecisionTool/Feature.h"

#include "TrigSteeringEvent/TrigPassBits.h"
#include "TrigSteeringEvent/TrigPassBitsCollection.h"

#include "TrigDecisionTool/TypelessFeature.h"
#include "TrigDecisionTool/EventPtrDef.h"

#include "TrigNavStructure/TrigNavStructure.h"

#include "AsgTools/AsgToolsConf.h"
#include "TrigNavStructure/TypedHolder.h"

namespace HLT{
  class TrigNavStructure;
}



namespace Trig {

  /**
   * This is the implementation of the Feature collection for ASG
   **/

  namespace FeatureAccessImpl{
 
    HLT::TriggerElement::FeatureAccessHelper getTypelessBits(const HLT::TriggerElement* te, const HLT::TrigNavStructure* navigation);

    void typelessCollect(const HLT::TriggerElement* te, HLT::class_id_type clid, std::vector<Trig::TypelessFeature>& data, const std::string& label, unsigned int condition, HLT::TrigNavStructure* navigation);
    

    template<typename REQUESTED,typename STORED, typename CONTAINER>
    std::vector<Trig::Feature<REQUESTED> > typedGet(const std::vector<TypelessFeature>& features, HLT::TrigNavStructure* navigation, EventPtr_t store,const std::string& container_name = ClassID_traits<CONTAINER>::typeName());


    //////////////////////////////////////////////////////////
    
    //do not implement
    template<typename STORED, typename REQUESTED>
    void insert_element(std::vector<Trig::Feature<REQUESTED> >&, Trig::TypelessFeature&, const STORED*);
    //do implement
    template<typename REQUESTED>
    void insert_element(std::vector<Trig::Feature<REQUESTED> >& result, Trig::TypelessFeature& feature, const REQUESTED* element){
      result.push_back(Trig::Feature<REQUESTED>(feature,element));
    }


    template<typename STORED>
    void insert_flatten_if(std::vector<Trig::Feature<STORED> >& result, const Trig::TypelessFeature& feature, const std::shared_ptr<const STORED>& newfeature){
      result.push_back(Trig::Feature<STORED>(feature,newfeature));
    }

    template<typename REQUESTED, typename STORED>
    void insert_flatten_if(std::vector<Trig::Feature<REQUESTED> >& result,  const Trig::TypelessFeature& feature, const std::shared_ptr<const STORED>& newfeature){
      for(auto element : *newfeature){
	result.push_back(Trig::Feature<REQUESTED>(feature,element));
      }
    }


    template<typename STORED>
    std::shared_ptr<const STORED> filter_if(boost::mpl::bool_<true> is_same, std::shared_ptr<const STORED>& original,const TrigPassBits* bits){ 
      (void)is_same;


      if(original->size() != bits->size()){
	std::cerr << "WARNING: bits size and owned size" << std::endl;
	return original;
      }
      STORED* f = new STORED;
      f->clear(SG::VIEW_ELEMENTS);
  
      for(auto obj : *original){
	if(HLT::isPassing(bits,obj,original.get())){
	  f->push_back(const_cast<typename STORED::value_type>(obj));
	}
      }
      //manage the memory
      std::shared_ptr<const STORED> filtered(f);
      return filtered;
    }

    template<typename STORED>
    std::shared_ptr<const STORED> filter_if(boost::mpl::bool_<false> is_same, std::shared_ptr<const STORED>& original,const TrigPassBits* bits){
      //nothing
      (void)is_same;
      return original;
    }

    template<typename REQUESTED,typename STORED, typename CONTAINER>
    std::vector<Trig::Feature<REQUESTED> > typedGet(const std::vector<TypelessFeature>& features, HLT::TrigNavStructure* navigation, EventPtr_t store, const std::string& container_name) {
      std::vector<Trig::Feature<REQUESTED> > typedvec;

      for(auto feature : features){
	auto typelessholder = navigation->getHolder(feature.accessHelper());
	if(!typelessholder){
	  std::cerr << "ERROR, holder not present" << std::endl;
	}
	HLT::TypedHolder<STORED,CONTAINER> typedholder(*typelessholder,store,container_name);
	const STORED* dest = 0;

	StatusCode sc = typedholder.get(dest,feature.accessHelper().getIndex());
	if(sc.isFailure()){
	  std::cerr << "TrigDecisionTool WARNING: Feature access failed for feature: " << feature.accessHelper() << std::endl;
	  continue;
	}

	if(boost::is_same<STORED,CONTAINER>::value){
	  //take ownership because typedholder.get created VIEW container with 'new'
	  std::shared_ptr<const STORED> owned(dest);
      
	  auto passbitsFeatureAccessHelper = FeatureAccessImpl::getTypelessBits(feature.te(),navigation);
	  if(passbitsFeatureAccessHelper.valid()){
#ifndef __CLING__
	    auto passbitsHolder = navigation->getHolder(passbitsFeatureAccessHelper);
	    HLT::TypedHolder<TrigPassBits,TrigPassBitsCollection> typedBits(*passbitsHolder,store);
	    const TrigPassBits* bits = 0;
	    if(typedBits.get(bits,passbitsFeatureAccessHelper.getIndex()).isFailure()){
	      std::cerr << "WARNING: couldn't get passbits from Holder" << std::endl;
	    }
	    owned = filter_if(boost::is_same<STORED,CONTAINER>(),owned,bits);
#endif
	  }
	  //if REQUESTED != STORED we assume REQUESTED is element type
	  //and we flatten the vector
	  insert_flatten_if(typedvec,feature,owned);
	}
	else{
	  //storegate owns the pointer
	  insert_element(typedvec,feature,dest);
	}
      }
      return typedvec;
    }

  } // EOF namespace FeatureAccessImpl

} // EOF namespace Trig
#endif


