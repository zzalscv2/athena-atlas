/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "TrigJetHypoToolConfig_fastreduction.h"

#include "GaudiKernel/StatusCode.h"

#include "./RepeatedCondition.h"
#include "./FastReductionMatcher.h"
#include "./Tree.h"
#include "./ConditionsDefs.h"

#include "TrigCompositeUtils/TrigCompositeUtils.h"

using TrigCompositeUtils::DecisionID;
using TrigCompositeUtils::Decision;
using TrigCompositeUtils::DecisionContainer;

TrigJetHypoToolConfig_fastreduction::TrigJetHypoToolConfig_fastreduction(const std::string& type,
                                                 const std::string& name,
                                                 const IInterface* parent) :
  base_class(type, name, parent){

}


TrigJetHypoToolConfig_fastreduction::~TrigJetHypoToolConfig_fastreduction(){
}

StatusCode TrigJetHypoToolConfig_fastreduction::initialize() {
  ATH_MSG_DEBUG("initialising " << name());

  if(m_conditionMakers.size() != m_treeVec.size()){
    ATH_MSG_ERROR("No. of conditions mismatch with tree vector size");
    return StatusCode::FAILURE;
  }
  
  if(m_conditionMakers.size() < 2){ // first  node is root, need more
    ATH_MSG_ERROR("No. of conditions " +
		  std::to_string( m_conditionMakers.size()) + 
		  " require at least 2" );
    return StatusCode::FAILURE;
  }

  if(m_filterMakerInds.size() != m_conditionMakers.size()){
    // need an index for each condition
    ATH_MSG_ERROR("No. of conditions " +
		  std::to_string( m_conditionMakers.size()) +
		  " no. of filter inds " +
		  std::to_string( m_filterMakerInds.size()) +
		  " must be equal" );
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}


ConditionPtrs
TrigJetHypoToolConfig_fastreduction::getRepeatedConditions() const {

  ConditionPtrs conditions;

  // collect the Conditions objects from the various sources

  for(const auto& cm : m_conditionMakers){
    conditions.push_back(cm->getRepeatedCondition());
  }
      
  return conditions;
}

/*
std::vector<std::unique_ptr<ConditionFilter>>
TrigJetHypoToolConfig_fastreduction::getConditionFilters() const {

  auto filters = std::vector<std::unique_ptr<ConditionFilter>>();
  
  for(const auto& cm : m_filtConditionMakers){

    ConditionPtrs filterConditions;  // will contain a single Condition
    ConditionPtr repeatedCondition = cm->getRepeatedCondition();

    // repeatedPtr is a nullptr is there are no contained conditions.
    // this means the filter should act as pass-through.
    if (repeatedCondition) {
      filterConditions.push_back(std::move(repeatedCondition));
    }
    
    auto cf = std::make_unique<ConditionFilter>(filterConditions);
    filters.push_back(std::move(cf));
  }
  
  return filters;
}
*/

std::vector<FilterPtr>
TrigJetHypoToolConfig_fastreduction::getFilters() const {

  auto filters = std::vector<FilterPtr>();
  filters.reserve(m_filterMakers.size());
  
  for(const auto& filterMaker : m_filterMakers) {
    filters.push_back(filterMaker->getHypoJetVectorFilter());
  }

  return filters;
}


// following function not used for treeless hypos
std::size_t
TrigJetHypoToolConfig_fastreduction::requiresNJets() const {
  return 0;
}


std::unique_ptr<IJetsMatcher>
TrigJetHypoToolConfig_fastreduction::getMatcher () const {

  auto matcher =  std::unique_ptr<IJetsMatcher>(nullptr);

  auto repeatedConds = getRepeatedConditions();
 
  if(repeatedConds.empty()){return matcher;}

  auto conditions = std::move(repeatedConds);
  auto filters = getFilters();

  auto fpm = new FastReductionMatcher(conditions,
				      filters,
				      m_filterMakerInds,
				      Tree(m_treeVec));
  matcher.reset(fpm);
  return matcher;
}

StatusCode TrigJetHypoToolConfig_fastreduction::checkVals() const {
  return StatusCode::SUCCESS;
}

