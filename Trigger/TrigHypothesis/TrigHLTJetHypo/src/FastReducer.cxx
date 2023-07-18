/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "./FastReducer.h"
#include "./GrouperByCapacityFactory.h"
#include "./ITrigJetHypoInfoCollector.h"
#include "./JetGroupProductFactory.h"

#include <map>
#include <algorithm>
#include <sstream>

class DepthComparison{
public:
  DepthComparison(const Tree& t) : m_tree(t){}
  bool operator () (const std::size_t& lhs, const std::size_t rhs){
    return m_tree.depth(rhs) > m_tree.depth(lhs);
  }
private:
  Tree m_tree;
};


struct IndexVecComp{
  bool operator () (const std::vector<std::size_t>& lhs,
		    const std::vector<std::size_t>& rhs){
    if(lhs.size() < rhs.size()){return true;}
    if(rhs.size() < rhs.size()){return false;}
    
    for(std::size_t i = 0; i < lhs.size(); ++i){
      if(lhs[i] > rhs[i]){return false;}
    }

    return true;
  }

};


FastReducer::FastReducer(const HypoJetVector& jv,
                         const ConditionPtrs& conditions,
			 const ConditionFilters& filters,
			 const ConditionFilterInds& filterInds,
                         const Tree& tree,
                         xAODJetCollector& jetCollector,
                         const Collector& collector):
  m_conditions(conditions),  m_conditionFilters(filters),
  m_conditionFilterInds(filterInds), m_tree(tree) {

  // create an empty vector of indices of satisfying jet groups
  // for each Condition.
  for(std::size_t i = 0; i < m_tree.size(); ++i){
    m_satisfiedBy.emplace(i, std::vector<std::size_t>{});
    m_testedBy.emplace(i, std::set<std::size_t>{});
    m_conditionMult.push_back(conditions[i]->multiplicity());
    m_conditionCap.push_back(conditions[i]->capacity());
    m_conditionClique.push_back(conditions[i]->clique());
  }


  if(!findInitialJetGroups(jv,
			   collector)){
    if(collector){
      collector->collect("FastReducer early return",
			 "from findInitialJetGroups");
      dumpDataStructures(collector);
    }
    return;  // m_pass retains initial value ie false 
  }

  
  if(!propagateJetGroups(collector)){
    // error propagating edges. e.g. unsatisfied condition
    if(collector){
      collector->collect("FastReducer early return",
			 "from propagateJetGroups");
      dumpDataStructures(collector);
    }
    return;  // early return, leave m_pass = false
  }

  m_pass = true;
  if(collector){
    collector->collect("FastReducer returning",
		       "from propagateJetGroups");
    dumpDataStructures(collector);
  }

  collectLeafJets(jetCollector, collector);

}

void FastReducer::collectLeafJets(xAODJetCollector& jetCollector,
				  const Collector& collector) const {

  // basic passing jet reporting

  // find the indices of the jets that make up the jet groups that pass
  // the root node.

  //.. obtain the passing jet groups for the root node...

  std::set<std::size_t> rootSatJetGroupInds(m_satisfiedBy.at(0).begin(),
					    m_satisfiedBy.at(0).end());

  // ...obtain the elemental jet group indicies...

  std::set<std::size_t> rootElSatJetGroupInds;
  for (const auto& ji : rootSatJetGroupInds) {
    rootElSatJetGroupInds.insert(m_jg2elemjgs.at(ji).begin(),
				 m_jg2elemjgs.at(ji).end());
  }

    

  // now do the same for the leaf nodes
  

  auto leaves = m_tree.leaves();
  // obtain the jet group indices for the jet groups satisfying the leaf conds
  for (const auto& ci : leaves) {  // for each leaf node...
    std::set<std::size_t> satJetGroupInds;
    
    // ... collect the (possibly compound) jet group indices...
    satJetGroupInds.insert(m_satisfiedBy.at(ci).cbegin(),
                           m_satisfiedBy.at(ci).cend());
    
    // ...obtain the corresponding elemental jet group indices...
    std::set<std::size_t> elSatJetGroupInds;
    for (const auto& ji : satJetGroupInds) {
      elSatJetGroupInds.insert(m_jg2elemjgs.at(ji).begin(),
			       m_jg2elemjgs.at(ji).end());
    }
    
    // .. get the leg label for the condition (possibly "")
    auto conditionLabel = (m_conditions.at(ci))->label();
    
    if (collector) {
      std::stringstream ss;
      ss <<  "elSatJettGroupInds.size() "
	 << conditionLabel
	 << ": "
	 << elSatJetGroupInds.size();
      collector->collect("FastReducer", ss.str());
    }

    // ... use the elemental jet group indices to recover the jets

    // if the leaf not jet is one of the jets that contributes to
    // passing root, store it in the collector, labelled by the leaf node
    // chainLegLabel
    
    auto end = rootElSatJetGroupInds.end();
    for(const auto& ji : elSatJetGroupInds) {
      
      if (rootElSatJetGroupInds.find(ji) != end){  /// jets by indices
	jetCollector.addJets(m_indJetGroup.at(ji).begin(), //jets py ptrs
			     m_indJetGroup.at(ji).end(),
			     conditionLabel);
      }
    }
  }
  if (collector) {
    collector->collect("FastReducer",
		       "collected " + std::to_string(jetCollector.size()));
  }
}


bool FastReducer::findInitialJetGroups(const HypoJetVector& jv,
				       const Collector& collector) {
  

  /*
    Will now test the incoming jets against the leaf conditions.
  */

  auto leaves = m_tree.leaves();

  // if a jet group satisfies a condition, note the fact,
  // and store it by index

  for(const auto& leaf: leaves){

    auto filtered_jets = jv;
    auto& filter_ind = m_conditionFilterInds[leaf];
    if (filter_ind != -1) {
      const auto& filter = m_conditionFilters[filter_ind];
      filtered_jets = filter->filter(filtered_jets, collector);
    }
    
    auto iters = std::make_pair(filtered_jets.begin(),
				filtered_jets.end());

    recordFiltering(leaf, jv.size(), filtered_jets.size(), collector);

    auto grouper = grouperByCapacityFactory(m_conditions[leaf]->capacity(),
					    iters.first,
					    iters.second);

    while(true){
      auto jg = grouper->next();  // obtain a vector of jet ptrs
      if (jg.empty()) {break;}
      
      auto jg_ind = m_jgRegister.record(jg);  // obtain an int index for the jg
      m_testedBy[leaf].insert(jg_ind);

      if (m_conditions[leaf]->isSatisfied(jg, collector)){

	if(collector){recordJetGroup(jg_ind, jg, collector);}

	// note the index if the jet group if it passes a Condition
	m_satisfiedBy[leaf].push_back(jg_ind);

	// keep track of the indices for the individual jets that
	// make up the passing jet group.
	//
	// m_jg2elemjgs[idx] = {idx1, idx2, idx3}
	// where idx is the index of the jet group, and {} is a
	// vector containing the individual jet group indices of
	// the jet group with index idx.
	//
	// if the jet group contains a single jet, this reduces to 
	// m_jg2elemjgs[idx] = {idx}
	
	std::vector<std::size_t>& elem_indices =  m_jg2elemjgs[jg_ind];
	if (elem_indices.empty()) {  // first time jg_ind is seen
	  if (jg.size() > 1) {
	    elem_indices.reserve(jg.size());
	    for (const auto& hj : jg){

	      // deal with a jet member of the jet group
	      auto single_jet_group = std::vector{hj};
	      auto single_jet_index = m_jgRegister.record(single_jet_group);
	      auto& single_jet_elems = m_jg2elemjgs[single_jet_index];
	      if (single_jet_elems.empty()) {
		single_jet_elems.push_back(single_jet_index);
		m_indJetGroup.emplace(single_jet_index, single_jet_group);
	      }

	      // add the index of the individual jet to m_jg2elemjgs[jg_ind]
	      elem_indices.push_back(single_jet_index);
	    }
	  } else {
	    // one jet in the jet group: its index maps onto a vector containing
	    // only its index.
	    elem_indices.push_back(jg_ind);
	  }
	}

	// update jet gtoup index to jet group map.
	m_indJetGroup.emplace(jg_ind, jg);
      }
    }
  }
  
  if(collector){
    for(const auto& p : m_indJetGroup){
      recordJetGroup(p.first, p.second, collector);
    }
  }
  
  // check all leaf conditions are satisfied
  for (const auto& i : leaves) {
    if (!capacitySatisfied(i, collector)) {
      return false;
    }
  }

  return true;
  
}
  

bool FastReducer::propagateJetGroups(const Collector& collector){
  
  
  // construct jet groups according from jet groups that pass child
  // conditions.
  // This method controls which nodes to process.
  // It checks whether all sibling nodes are processed.
  // if so, processing of the parent is delegated to propagate_()
  
  //The parent of the next condition to be processed
  // is found, and from the parent the condition's siblings are found,
  
  typedef std::priority_queue<std::size_t,
			      std::vector<std::size_t>,
			      DepthComparison> DepthQueue;

  auto comparator = DepthComparison(m_tree);
  DepthQueue to_process(comparator); // conditions to be processed.
  
  // keep track if a condition's sibling has been processed.
  std::vector<bool> checked(m_conditions.size(), false);

  // initialise the queue with satisfied leaf conditions indices.
  for(const auto& item : m_satisfiedBy){
    if(!(item.second.empty())){
      to_process.push(item.first);
    }
  }
  
  while(!to_process.empty()){

    auto k = to_process.top();
    to_process.pop();

    if(checked[k]){
      continue;
    }
    
    if(k == 0){
      // have propagated to the root node
      if(m_satisfiedBy.at(0).empty()){
	if(collector){
	  collector->collect("FastReducer",
			     "Condition node 0 fails");
	}
	return false;
      } else {
	return true; // event passes
      }
    }

    auto siblings = m_tree.siblings(k);
    for(const auto& s : siblings){
      checked[s] = true;
    }
     
    // check if combinations of groups satisfying children satisfy their parent
    if(!propagate_(k,
		   siblings,
		   collector)){
      return false;
    }
    
    std::size_t par =  m_tree.parent(k);
    to_process.push(par);
  }
  return true;
}


bool FastReducer::propagate_(std::size_t child,
			     const std::vector<std::size_t>& siblings,
			     const Collector& collector){

  // all combinations of the jet groups passing the sibings are
  // constructed. One by one these combinations are tested for
  // parent satisfaction.


  // Edges are contructed between satisfying jet groups and the parent.
  // if any such edge is constructed, the calling rroutine is notified so it
  // can scheduling processing the parent as a child.
        
  std::size_t par =  m_tree.parent(child);

  // child == 0  do not attempt to process parent of node.
  if(child == 0){return true;}

      
  // calculate the external product of the jet groups
  // eg if condition c1 is satisfied by jg11 and jg12, while its only
  // sibling c2 is satisfied by jg21, the external jet groups are
  // jg11jg21, jg12jg21. Each of these  are flattened.

  auto jg_product = makeJetGroupProduct(siblings,
					m_satisfiedBy,
					m_conditionMult,
					m_conditionCap,
					m_conditionClique,
					m_jg2elemjgs,
					m_conditions[par]->capacity(),
					m_tree.is_simple(),
					collector);
   
  // obtain the next product of jet groups passing siblings
  auto jg_indices = jg_product->next(collector);

  // step through the jet groups found by combining ghe child groups
  // check ecach combination to see if it satisfies the parent. If so
  // add an edge from the contributing children, and from the new jet group to the parent.

  while (!jg_indices.empty()){  // empty jg_inidices: end of iteration
    if (!std::is_sorted(jg_indices.begin(), jg_indices.end())) {
	throw std::runtime_error("Jet hypo unsorted jet group");
    }

    for (const auto& ind : jg_indices) {
      if (m_jg2elemjgs.at(ind).size() != 1) {
	throw std::runtime_error("Jet hypo jet group with non-elementary index");
      }
    }
      
    
    // use the elemental jet group indices to form a vector of jet pointers
    HypoJetVector jg;
    for(const auto& i : jg_indices){
      const auto& jetGroup = m_indJetGroup.at(i);
      jg.insert(jg.end(), jetGroup.begin(), jetGroup.end());
    }
    
    // obtain an index for the new jet group.
    auto cur_jg = m_jgRegister.record(jg);

    // keep track of which jet groups a Condition sees.
    if(m_testedBy[par].find(cur_jg) != m_testedBy[par].end()){
      jg_indices = jg_product->next(collector);
      continue;
    }
    
    m_testedBy[par].insert(cur_jg);

    // check if parent is satisfied by a jet group (vector of jet ptrs)
    if (m_conditions[par]->isSatisfied(jg, collector)){// par is a tree ind.

      // get an index for this vector of elementary jet group indices
      m_satisfiedBy[par].push_back(cur_jg);

      m_jg2elemjgs[cur_jg] = jg_indices;
      if(collector){recordJetGroup(cur_jg, jg, collector);}
    }
    
    jg_indices = jg_product->next(collector);
  }

  // check if enough jet groups pass the parent condition
  bool par_satisfied =
    m_conditions[par]->multiplicitySatisfied(m_satisfiedBy[par].size(),
					     collector);
  if(collector and !par_satisfied){
    collector->collect("FastReducer",
		       "Condition node " + std::to_string(par) +
		       " unsatisfied");
  }
  
  return par_satisfied;
}



std::string FastReducer::toString() const {
  std::stringstream ss;
  ss << "FastReducer:\n";
  ss << "  treeVector: " << m_tree << '\n';;
  ss << "FastReducer Conditions ["
     << m_conditions.size() << "]: \n";

  std::size_t count{0u};
  for(const auto& c : m_conditions){
    auto sc = std::to_string(count++);
    sc.insert(sc.begin(), 3-sc.length(), ' ');
    ss << sc <<": "<< c->toString() + '\n';
  }

  return ss.str();
}


std::string FastReducer::dataStructuresToStr() const {

   std::stringstream ss;
  ss << "FastReducer data structure dumps\nindToJetGroup:\n";
  for(const auto& pair : m_indJetGroup){
    ss << pair.first << " [";
    for(const auto& j : pair.second){
      ss << static_cast<const void*>(j.get()) << " ";
    }
    ss << "]\n";
  }

  ss << "satisfiedBy: \n";
  for(const auto& pair : m_satisfiedBy){
    ss << pair.first << " [";
    for(const auto& i : pair.second){
      ss << i << " ";
    }
    ss << "]\n";
  }

  
  ss << "testedBy: \n";
  for(const auto& pair : m_testedBy){
    ss << pair.first << " [";
    for(const auto& i : pair.second){
      ss << i << " ";
    }
    ss << "]\n";
  }
  
  ss << "jg to elemental jgs: \n";
  
  for(const auto& pair : m_jg2elemjgs){
    ss << pair.first << " [";
    for(const auto& i : pair.second){
      ss << i << " ";
    }
    ss << "]\n";
  }
  ss <<'\n';

  return ss.str();
 }

 void  FastReducer::dumpDataStructures(const Collector& collector) const {
   
   if(!collector){return;}
   
   collector->collect("FastReducer",
		      dataStructuresToStr());
 }


void FastReducer::recordJetGroup(std::size_t ind,
				 const HypoJetVector& jg,
				 const Collector& collector) const {
  
  std::stringstream ss0;
  ss0  << "FastReducer jet group "
       << ind << " [" << jg.size() <<"]:";
  
  std::stringstream ss1;
  for(auto ip : jg){
    const void* address = static_cast<const void*>(ip.get());
    ss1  << "\n "  << address << " eta " << ip->eta()
	 << " e " << ip->e()
	 << " et " << ip->et();
  }
  ss1 << '\n';
  collector->collect(ss0.str(), ss1.str());
}

void FastReducer::recordFiltering(std::size_t leaf_ind,
				  std::size_t n_injets,
				  int n_filteredjets,
				  const Collector& collector) const {

  if(!collector) {return;}
  
  std::stringstream ss0;
  ss0  << "FastReducer filtering Condition index: "  << leaf_ind;
  
  std::stringstream ss1;
  ss1  << "n jets. in: " << n_injets << " filtered: " << n_filteredjets << '\n';
  
  collector->collect(ss0.str(), ss1.str());
}

bool FastReducer::pass() const { return m_pass; }


bool FastReducer::capacitySatisfied(std::size_t ind,
				    const Collector& collector) const {
  // Check that the number of satisfying jet groups is sufficient to
  // satisfy the capacity of the Condition. Uses include handling
  // of Conditions which represent multiple identical conditions.
  
  auto jgMult = m_satisfiedBy.at(ind).size();
  auto capSat = m_conditions.at(ind)->multiplicitySatisfied(jgMult, collector);
  if (!capSat and collector) {
    collector->collect("FastReduce", "Condition " + std::to_string(ind)
		       + " unsatisfied multiplicity, aborting"); 
  }
  
  return capSat;
}
