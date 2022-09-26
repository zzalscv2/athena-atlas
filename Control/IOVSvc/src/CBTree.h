/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IOVSVC_CBTREE_H
#define IOVSVC_CBTREE_H

/*****************************************************************************
 *
 *  CBTree.h
 *  IOVSvc
 *
 *  Author: Charles Leggett
 *
 *  Callback function trigger tree
 *
 *****************************************************************************/

#include "CBNode.h"

#include "AthenaKernel/IOVSvcDefs.h"

#include <set>
#include <string>

namespace SG {
  class DataProxy;
}

class CBTree {
public:

  typedef std::set<CBNode*, CBNode::nodeOrder> nodeSet;
  typedef IOVSvcCallBackFcn BFCN;

  CBTree();
  ~CBTree();

  CBTree (const CBTree&) = delete;
  CBTree& operator= (const CBTree&) = delete;

  CBNode* addNode(const std::string& name, CBNode* parent);
  CBNode* addNode(const SG::DataProxy* proxy, const std::string& name);
  CBNode* addNode(BFCN* fcn, const CallBackID& cb, 
                  const SG::DataProxy* parent_proxy);
  CBNode* addNode(BFCN* fcn, const CallBackID& cb, BFCN* parent_fcn);

  CBNode* replaceProxy(const SG::DataProxy* pOld, const SG::DataProxy* pNew);

  void connectNode(CBNode* node, CBNode* parent);
  void connectNode(const std::string& name, CBNode* parent);

  CBNode* findNode(const std::string& name);
  CBNode* findNode(const std::string& name, CBNode* start);

  CBNode* findNode(const SG::DataProxy* proxy);
  CBNode* findNode(const SG::DataProxy* proxy, CBNode* start);

  CBNode* findNode(BFCN* fcn);
  CBNode* findNode(BFCN* fcn, CBNode* start);

  bool delNode(const SG::DataProxy* prx);

  void printTree() const;
  void printTree( const CBNode* start ) const;

  int maxLevel() const;
  void adjustLevels( CBNode* start );

  void listNodes() const;
  void listNodes( const int& level, 
                  nodeSet::const_iterator& start, 
                  nodeSet::const_iterator& end ) const;

  void cascadeTrigger(const bool b, CBNode* start);
  void cascadeTrigger(const bool b, BFCN* fcn);
  void cascadeTrigger(const bool b, const SG::DataProxy* proxy);

  void clearTrigger() const;

  void cascadeFlag(const bool b, CBNode* node) const;
  void clearFlag() const;

  void traverse( void (*pF) (const CBNode*) ) const;
  void traverse( const CBNode*, void (*pF) (const CBNode*) ) const;
  void traverseR( const CBNode*, void (*pF) (const CBNode*) ) const;

  const CBNode* traverse( const CBNode* (*pF) (const CBNode*) ) const;
  const CBNode* traverse( const CBNode*, const CBNode* (*pF) (const CBNode*) ) const;
  const CBNode* traverseR( const CBNode*, const CBNode* (*pF) (const CBNode*) ) const;

  void traverse( void (*pF) (const CBNode*, const CBNode*) ) const;
  void traverse( const CBNode*, const CBNode*, void (*pF) (const CBNode*, const CBNode*) ) const;
  void traverseR( const CBNode*, const CBNode*, void (*pF) (const CBNode*, const CBNode*) ) const;

private:
  
  static void _printTree( const CBNode*, const CBNode* );

  CBNode* m_root;
  nodeSet m_allNodes;

};

#endif
