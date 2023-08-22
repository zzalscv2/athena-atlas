/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigCompositeUtils/NavGraph.h"
#include "CxxUtils/sgkey_t.h"

#ifndef XAOD_STANDALONE // Athena or AthAnalysis
#include "AthenaKernel/CLIDRegistry.h"
#endif

namespace TrigCompositeUtils {

  NavGraphNode::NavGraphNode(const Decision* me) : m_decisionObject(me), 
    m_filteredSeeds(), m_filteredChildren(), m_keepFlag(false)
  {
  }


  bool NavGraphNode::addIfNotDuplicate(std::vector<NavGraphNode*>& container, NavGraphNode* toAdd) {
    std::vector<NavGraphNode*>::iterator it = std::find(container.begin(), container.end(), toAdd);
    if (it == container.end()) {
      container.push_back(toAdd);
      return true;
    }
    return false;
  }


  bool NavGraphNode::linksTo(NavGraphNode* to) {
    addIfNotDuplicate(to->m_filteredChildren, this);
    return addIfNotDuplicate(m_filteredSeeds, to); // Return TRUE if a new edge is added
  }


  void NavGraphNode::dropLinks(NavGraphNode* node) {
    m_filteredChildren.erase(std::remove(m_filteredChildren.begin(), m_filteredChildren.end(), node), m_filteredChildren.end());
    m_filteredSeeds.erase(std::remove(m_filteredSeeds.begin(), m_filteredSeeds.end(), node), m_filteredSeeds.end());
  }


  const Decision* NavGraphNode::node() const {
    return m_decisionObject;
  }


  const std::vector<NavGraphNode*>& NavGraphNode::seeds() const {
    return m_filteredSeeds;
  }

  const std::vector<NavGraphNode*>& NavGraphNode::children() const {
    return m_filteredChildren;
  }


  void NavGraphNode::keep() {
    m_keepFlag = true;
  }


  void NavGraphNode::resetKeep() {
    m_keepFlag = false;
  }

 
  bool NavGraphNode::getKeep() const {
    return m_keepFlag;
  }


  // #################################################


  NavGraph::NavGraph() : m_nodes(), m_finalNodes(), m_edges(0) {
  }


  void NavGraph::addNode(const Decision* node, const EventContext& ctx, const Decision* comingFrom) {

    const ElementLink<DecisionContainer> nodeEL = decisionToElementLink(node, ctx);
    auto nodePairIt = m_nodes.insert( std::make_pair(nodeEL, NavGraphNode(node)) );
    NavGraphNode& nodeObj = nodePairIt.first->second;
    
    if (comingFrom == nullptr) { // Not coming from anywhere - hence a final node.
      m_finalNodes.push_back( &nodeObj );
    } else {
      const ElementLink<DecisionContainer> comingFromEL = decisionToElementLink(comingFrom, ctx);
      auto comingFromPairIt = m_nodes.insert( std::make_pair(comingFromEL, NavGraphNode(comingFrom)) );
      NavGraphNode& comingFromNodeObj = comingFromPairIt.first->second;
      const bool newEdge = comingFromNodeObj.linksTo( &nodeObj );
      if (newEdge) {
        ++m_edges;
      }
    }
  }


  std::vector<NavGraphNode*> NavGraph::finalNodes() const {
    return m_finalNodes;
  }

  std::vector<NavGraphNode*> NavGraph::allNodes() {
    std::vector<NavGraphNode*> returnVec;
    returnVec.reserve(m_nodes.size());
    for (auto& entry : m_nodes) {
      NavGraphNode& n = entry.second;
      returnVec.push_back( &n );
    }
    return returnVec;
  }


  size_t NavGraph::nodes() const {
    return m_nodes.size();
  }


  size_t NavGraph::edges() const {
    return m_edges;
  }

  std::vector<const Decision*> NavGraph::thin() {
    std::vector<const Decision*> returnVec;
    std::map<const ElementLink<TrigCompositeUtils::DecisionContainer>, NavGraphNode>::iterator it;
    for (it = m_nodes.begin(); it != m_nodes.end(); /*noop*/) {
      if (it->second.getKeep()) {
        it->second.resetKeep();
        ++it;
      } else {
        returnVec.push_back(*(it->first)); // Dereferencing ElementLink to element
        rewireNodeForRemoval(it->second);
        it = m_nodes.erase(it);
      }
    }
    return returnVec;
  }

  void NavGraph::rewireNodeForRemoval(NavGraphNode& toBeDeleted) {
    const std::vector<NavGraphNode*> myParents = toBeDeleted.seeds();
    const std::vector<NavGraphNode*> myChildren = toBeDeleted.children();

    // Perform the (potentially) many-to-many re-linking required to remove toBeDeleted from the graph
    for (NavGraphNode* child : myChildren) {
      for (NavGraphNode* parent : myParents) {
        bool newEdge = child->linksTo(parent);
        if (newEdge) {
          ++m_edges;
        }
      }
    }

    // Remove the edges connecting to toBeDeleted from all of its children and all of parents
    for (NavGraphNode* child : myChildren) {
      child->dropLinks(&toBeDeleted);
      --m_edges;
    }
    for (NavGraphNode* parent : myParents) {
      parent->dropLinks(&toBeDeleted);
      --m_edges;
    }
  }


  void NavGraph::printAllPaths(MsgStream& log, MSG::Level msgLevel) const {
    for (const NavGraphNode* finalNode : m_finalNodes) {
      recursivePrintNavPath(*finalNode, 0, log, msgLevel);
    }
  }


  void NavGraph::recursivePrintNavPath(const NavGraphNode& nav, size_t level, MsgStream& log, MSG::Level msgLevel) const {
    const Decision* node = nav.node();
    const ElementLink<DecisionContainer> nodeEL = decisionToElementLink( node );
    std::stringstream ss;
    for (size_t i = 0; i < level; ++i) {
      ss << "  ";
    }

    ss << "|-> " << nodeEL.dataID() << " #" << nodeEL.index() << " Name(" << node->name() << ") Passing(" << node->decisions().size() << ")";
    if (nav.getKeep()) ss << " [KEEP]";
    if (node->hasObjectLink(featureString())) {
      SG::sgkey_t key;
      uint32_t clid;
      uint16_t index;
      node->typelessGetObjectLink(featureString(), key, clid, index);
#ifndef XAOD_STANDALONE // Athena or AthAnalysis
      ss << " Feature(#" << index << ", " << CLIDRegistry::CLIDToTypeinfo(clid)->name() << ", " << key << ")"; 
#else 
      ss << " Feature(#" << index << ", " << key << ")";
#endif
    }
    if (node->hasObjectLink(viewString())) ss << " [View]";
    if (node->hasObjectLink(roiString())) ss << " [RoI]";
    if (node->hasObjectLink(initialRoIString())) ss << " [InitialRoI]";
    log << msgLevel << ss.str() << endmsg;
    for (const NavGraphNode* seed : nav.seeds()) {
      recursivePrintNavPath(*seed, level + 1, log, msgLevel);
    }
  }

}
