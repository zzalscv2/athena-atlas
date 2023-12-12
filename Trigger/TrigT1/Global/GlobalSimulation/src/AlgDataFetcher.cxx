//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "AlgDataFetcher.h"
#include "AlgData.h"
#include "Digraph.h"
#include "TrigConfData/L1Menu.h"
#include "dot.h"
#include "GraphAlgs.h"

#include "TrigConfData/L1TopoAlgorithm.h"  // config algs
#include "TrigConfData/L1Connector.h" // TriggerLine

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

namespace GlobalSim{

  void algGraphOut(const Digraph& G,
		     const std::string& name_stub) {
    dot(G, name_stub + ".dot");
    std::ofstream g_stream(name_stub+".txt", std::ios_base::out);
    g_stream << G << '\n';
  }

  AlgDataFetcher::AlgDataFetcher(bool do_eConn,
				 bool do_oConn,
				 const TrigConf::L1Menu* l1menu) {
    auto boards = getBoards(l1menu);
    std::vector<const TrigConf::L1Connector*> pConfConnectors;
    
    /*
     * obtain the conf connectors the L1 topo boards know about
     */

    for (const auto& pboard : boards) {
      auto pConnectors = conf_connectors_(l1menu, pboard);
      pConfConnectors.insert(pConfConnectors.end(),
			     pConnectors.begin(),
			     pConnectors.end());
    }

    
    auto econn_end =
      std::partition(pConfConnectors.begin(),
		     pConfConnectors.end(),
		     [](const auto& pcon) {
		       return pcon->connectorType() ==
		       TrigConf::L1Connector::ConnectorType::ELECTRICAL;});
    

    auto decisionAlgData = std::vector<ADP>();
    auto sortingAlgData = std::vector<ADP>();
    
    if (do_eConn) {
      decisionAlgData = uniqueConfDecisionAlgs(pConfConnectors.begin(),
					       econn_end,
					       l1menu);
      sortingAlgData = uniqueConfSortingAlgs(decisionAlgData,
					     l1menu);
    }

    auto countingAlgData = std::vector<ADP>();
    if (do_oConn){
      countingAlgData = uniqueConfCountingAlgs(econn_end,
					       pConfConnectors.end(),
					       l1menu);
    }
    // look for input ADPs from combined sorting and counting ADPS
    // to do duplicate removal correctly
    std::vector<ADP> sANDc = sortingAlgData;
    sANDc.insert(sANDc.end(),
		 sortingAlgData.cbegin(),
		 sortingAlgData.cend());

    sANDc.insert(sANDc.end(),
		 countingAlgData.cbegin(),
		 countingAlgData.cend());
		 

    auto inputAlgData = uniqueConfInputAlgs(sANDc);
    auto rootAlgData = uniqueConfRootAlg();
    
    m_algData = rootAlgData;

    m_algData.insert(m_algData.end(),
		     decisionAlgData.cbegin(),
		     decisionAlgData.cend());

    
    m_algData.insert(m_algData.end(),
		     sortingAlgData.cbegin(),
		     sortingAlgData.cend());
    
    m_algData.insert(m_algData.end(),
		     countingAlgData.cbegin(),
		     countingAlgData.cend());

    m_algData.insert(m_algData.end(),
		     inputAlgData.cbegin(),
		     inputAlgData.cend());


    //final uniqueness tests:
    std::sort(m_algData.begin(), m_algData.end(), [](const auto& l,
						     const auto& r) {
      if (l->m_className != r->m_className){
	return (l->m_className < r->m_className);
      }
      
      return l->m_algName < r->m_algName;});
    

    auto iter = std::unique(m_algData.begin(), m_algData.end(),
			    [](const auto& l, const auto& r) {
			      return l->m_className == r->m_className and
				l->m_algName == r->m_algName;});
    
    if (iter != m_algData.end()) {
      m_errMsgs.emplace_back("Duplicate AlgData");
    }
    
    //sort by description to aid human understanding
    auto part_iter = std::partition(m_algData.begin(), m_algData.end(),
				    [](const auto& ad) {
				      return ad->m_desc == AlgDataDesc::root;});
    
    part_iter = std::partition(part_iter, m_algData.end(),
			       [](const auto& ad) {
				 return ad->m_desc == AlgDataDesc::decision;});
    part_iter = std::partition(part_iter, m_algData.end(),
			       [](const auto& ad) {
				 return ad->m_desc == AlgDataDesc::sort;});
    
    part_iter = std::partition(part_iter, m_algData.end(),
			       [](const auto& ad) {
				 return ad->m_desc == AlgDataDesc::count;});
    
 
    std::size_t sn{0}; 
    for (const auto& adp : m_algData) {
      adp->m_sn = sn;
      m_sn2name[sn++] = adp->m_algName;
    }

    for (const auto& p : m_sn2name){m_name2sn[p.second] = p.first;}
    if (m_sn2name.size() != m_name2sn.size()){
      m_errMsgs.push_back("There are  duplicate AlgData names");
    }
    
    if (!isValid()) {return;}
    
    // invert name2sn
    for (const auto& p : m_name2sn) {
      m_sn2name.insert({p.second, p.first});
    }

    if (m_name2sn.size() != m_sn2name.size()) {
      m_errMsgs.emplace_back("AlgDataFetcher: error inverting name2sn");
      return;
    }
    
    for (const auto& adp : m_algData) {
      for (const auto& c: adp->m_childNames) {
	try{
	  adp->m_childSNs.push_back(m_name2sn[c]);
	} catch  (const std::exception& e) {
	  m_errMsgs.push_back("Unknown child name: " + c);
	}
      }
    }

    
  }
  
  const std::vector<ADP>& AlgDataFetcher::algData() const {
    return m_algData;
  }



  template<typename Iter>
  std::vector<ADP>
  AlgDataFetcher::uniqueConfDecisionAlgs(Iter begin,
					 Iter end,
					 const TrigConf::L1Menu* l1menu) const {

    /*
     *
     * A connector can have >1 trigger lines. A trigger line has a single algorithm
     * an algorithm can have > 1 trigger lines.
     *
     * here, we are not interested in relations with connectors. Rather we want to
     * know the algorithms and their trigger lines
     *
     * extract of a dump showing these points
     *
     *  connector name Topo3El tl name TOPO_HT190-jJ40s5pETA21 alg name HT190-jJ40s5pETA21
     * connector name Topo3El tl name TOPO_300INVM-jJ60s6-AjJ50s6 alg name jINVM_NFF
     * connector name Topo3El tl name TOPO_400INVM-jJ60s6-AjJ50s6 alg name jINVM_NFF
     * connector name Topo3El tl name TOPO_500INVM-jJ60s6-AjJ50s6 alg name jINVM_NFF
     * connector name Topo3El tl name TOPO_700INVM-jJ60s6-AjJ50s6 alg name jINVM_NFF
     * connector name Topo3El tl name TOPO_HT150-jJ50s5pETA31 alg name HT150-jJ50s5pETA31
     * ...
    */
    auto triggerLines = std::vector<TrigConf::TriggerLine>();
    for (auto iter = begin; iter != end; ++iter) {// config connectors
      auto l1conn = *iter;
      for( size_t fpga_ind : { 0, 1} ) {
	for( size_t clock_ind : { 0, 1} ) {
	  auto cTriggerLines = l1conn->triggerLines(fpga_ind, clock_ind);
	  triggerLines.insert(triggerLines.end(), cTriggerLines.cbegin(), cTriggerLines.cend());
	}
      }
    }
    return adpsFromTriggerLines(triggerLines, AlgDataDesc::decision, l1menu);
  }



std::vector<ADP>
  AlgDataFetcher::adpsFromTriggerLines(std::vector<TrigConf::TriggerLine>& triggerLines,
				       AlgDataDesc desc,
				       const TrigConf::L1Menu* l1menu
				       ) const {

  // Assume a TrigConf::Algorithm is uniquely identified by its name
 
  using TLMap = std::map<std::string, std::vector<TrigConf::TriggerLine>>;
  auto tlMap = TLMap();
  for (const auto& tl : triggerLines) {
    tlMap[l1menu->algorithmFromTriggerline(tl.name()).name()].push_back(tl);
  }

  auto result = std::vector<ADP>();
  for (const auto& p : tlMap) {
    const auto& alg =  l1menu->algorithmFromTriggerline(p.second[0].name());
    
    auto adp = std::make_shared<AlgData>();
    adp->m_desc = desc;
    adp->m_algName = alg.name();
    adp->m_className = alg.klass();
    adp->m_triggerLines = tlMap[alg.name()];
    adp->m_childNames = alg.inputs();
    adp->m_category = alg.category();
    result.push_back(adp);
  }
   
  return result;
}
 
  

  std::vector<ADP>
  AlgDataFetcher::uniqueConfSortingAlgs(const std::vector<ADP>& d_adps,
					const TrigConf::L1Menu* l1menu) const {
    
    auto sortNames = std::vector<std::pair<std::string,std::string>>();

    for (const auto& d_adp : d_adps) {
      for (const auto& sc_name : d_adp->m_childNames){
	sortNames.emplace_back(sc_name, d_adp->m_category);
      }	
    }

    std::sort(sortNames.begin(), sortNames.end());
    auto iter = std::unique(sortNames.begin(),
			    sortNames.end());
    
    sortNames.erase(iter, sortNames.end());

    auto sort_algs = std::vector<const TrigConf::L1TopoAlgorithm*>();
    sort_algs.reserve(sortNames.size());
    
    std::transform(sortNames.cbegin(),
		   sortNames.cend(),
		   std::back_inserter(sort_algs),
		   [&l1menu](const auto& s_name){
		     return &(l1menu->algorithm(s_name.first, s_name.second));});
    

    auto vdata = std::vector<ADP>();
    for (const auto& alg : sort_algs) {
      auto adata = std::make_shared<AlgData>();
      adata->m_desc = AlgDataDesc::sort;
      adata->m_algName = alg->name();
      adata->m_className = alg->klass();
      adata->m_childNames = alg->inputs();
      adata->m_category = alg->category();
      vdata.push_back(adata);
    }
    return vdata;
  }
  
  std::vector<ADP>
  AlgDataFetcher::uniqueConfInputAlgs(const std::vector<ADP>& adps) const {

    //Sorting and Counting Algs each have a single child Input Alg
  

    auto childNames = std::vector<std::string>();
    for (const auto& adp : adps) {
      if (adp->m_childNames.size() != 1) {
	throw std::runtime_error("AlgDataFetcher: exactly 1 child node required");
      }
      childNames.push_back(adp->m_childNames[0]);
    }

    std::sort(childNames.begin(), childNames.end());
    auto iter = std::unique(childNames.begin(), childNames.end());
    childNames.erase(iter, childNames.end());

    auto vdata = std::vector<ADP>();
    for (const auto& cn : childNames) {
      auto adata = std::make_shared<AlgData>();
      adata->m_algName = cn;
      adata->m_desc = AlgDataDesc::input;
      vdata.push_back(adata);
    }
    return vdata;
  }

  std::vector<ADP>
  AlgDataFetcher::uniqueConfRootAlg() const {
    
    //root node has sn = 0. For now, no children
    
    
     auto vdata = std::vector<ADP>{std::make_shared<AlgData>()};
     auto adata = vdata.back();
     adata->m_algName = "root";
     adata->m_desc = AlgDataDesc::root;

     return vdata;
  }
  

  std::vector<const TrigConf::L1Connector*>
   AlgDataFetcher::conf_connectors_(const TrigConf::L1Menu* menu,
					 const TrigConf::L1Board* board) const {
    
    std::vector<const TrigConf::L1Connector*> connectors;
    const auto& connectorNames = board->connectorNames();
    std::transform(connectorNames.cbegin(),
		   connectorNames.cend(),
		   std::back_inserter(connectors),
		   [&menu](const auto& name) {
		     return &(menu->connector(name));});		     
    return connectors;
  }



   template<typename Iter>
   std::vector<ADP>
   AlgDataFetcher::uniqueConfCountingAlgs(Iter begin,
					  Iter end,
					  const TrigConf::L1Menu* l1menu) const {
     /*
       Configure optical connectors - multiplicity algorithms
       In multiplicity boards all trigger lines are copied into the
       L1Menu::Connector 4 times (fpga 0,1 and clock 0,1)
       Take (fpga, clock) = (0, 0)
     */
 

     auto triggerLines = std::vector<TrigConf::TriggerLine>();
     for (auto iter = begin; iter != end; ++iter) {// config connectors
       auto l1conn = *iter;
       const auto& cTriggerLines = l1conn->triggerLines(0, 0);

       triggerLines.insert(triggerLines.end(),
			   cTriggerLines.cbegin(),
			   cTriggerLines.cend());
       triggerLines.push_back(cTriggerLines[0]);
     }

     return adpsFromTriggerLines(triggerLines, AlgDataDesc::count, l1menu);

   }

 

  std::string AlgDataFetcher::toString() const {
    auto ss  = std::stringstream();
    ss << "AlgDataFetcher: no of  AlgData "
       << m_algData.size()
       << '\n';

    return ss.str();
  }

 
  std::vector<const TrigConf::L1Board*>
  AlgDataFetcher::getBoards(const TrigConf::L1Menu* l1menu) const {
    std::vector<const TrigConf::L1Board*> boards;
    for (const std::string & boardName : l1menu->boardNames() ){
      
      auto & l1board = l1menu->board(boardName);
      
      if (l1board.type() != "TOPO") continue;
      if (l1board.legacy()) continue;
      boards.push_back(&l1board); // l1board is a reference
    }

    return boards;
  }

  std::vector<ADP> AlgDataFetcher::execOrderedAlgData() const {

    //add 1 to gs as vertex ids start at 0.
    auto gv =
      (*max_element(m_algData.cbegin(),
			   m_algData.end(),
			   [](const auto& l, const auto& r) {
			     return l->m_sn < r->m_sn;}))->m_sn + 1;

    Digraph G(gv);  // start at 0
    for (const auto& a : m_algData) {
      for (const auto& c : a->m_childSNs) {
	G.addEdge(a->m_sn, c);
      }
    }

    algGraphOut(G, "order_graph");
    
    auto graph_topo  = Topological(G);
  
    if (!graph_topo.isDag()) {
      throw std::runtime_error("Algorithm graph is not a directed acyclic graph");
    }

    auto node_order = graph_topo.order();

    auto adSz =  m_algData.size();
    auto result = std::vector<ADP>(adSz);
    for (std::size_t i = 0; i != adSz; ++i) {
      result[i]  = m_algData[node_order[i]];
    }

    return result;
  }
  

}


