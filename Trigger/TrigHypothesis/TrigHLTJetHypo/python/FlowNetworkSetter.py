"""Instantiates TrigJetHypoToolConfig_flownetwork AlgTool 
from a hypo tree."""

# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
from TrigHLTJetHypo.TrigHLTJetHypoConf import (
    TrigJetConditionConfig_dijet_mass,
    TrigJetConditionConfig_dijet_deta,
    TrigJetConditionConfig_dijet_dphi,
    TrigJetConditionConfig_eta,
    TrigJetConditionConfig_et,
    TrigJetConditionConfig_acceptAll,
    TrigJetHypoToolConfig_flownetwork,
    TrigJetHypoToolHelperMT,
    TrigJetHypoToolConfig_flownetwork,
    )

from node import rotate

from collections import defaultdict

class FlowNetworkSetter(object):
    """Visitor to set instantiated AlgTools to a jet hypo tree"""
    
    def __init__(self, name):

        self.name = name
        # for simple, use TrigJetConditionConfig_etaet. Needs to be
        # completed because simple can conain any single jet condition
        self.tool_factories = {
            'et': [TrigJetConditionConfig_et, 0], 
            'eta': [TrigJetConditionConfig_eta, 0],
            'dijet_mass': [TrigJetConditionConfig_dijet_mass, 0],
            'dijet_dphi': [TrigJetConditionConfig_dijet_deta, 0],
            'dijet_deta': [TrigJetConditionConfig_dijet_dphi, 0],
            'partgen': [TrigJetConditionConfig_acceptAll, 0],
            }

        self.nodeToParent=defaultdict(list)
        self.nodeToConditionTool=defaultdict(list)

        self.mod_router = {
            'not': self.not_supported_yet,
            'and': self.illegal,
            'agree': self.not_supported_yet,
            'or': self.not_supported_yet,
            'simple': self._mod_leaf,
            'etaet': self._mod_leaf,
            'simplepartition': self.not_supported_yet,
            'combgen': self.illegal,
            'partgen': self._mod_partition,
            'dijet': self._mod_leaf,
        }

        # map conaining parent child ids for the node
        self.treeMap = {}

        # map containing the a list of Condition factory AlgTools for scenario
        self.conditionMakers = {}

    def not_supported_yet(self, node):
        raise NotImplementedError(
            'FlowNetworkSetter: scenarion not yet supported: ' + node.scenario)

    
    def illegal(self, node):
        raise RuntimeErrorError(
            'FlowNetworkSetter: illegal scenario: %s' + node.scenario)

    def mod_logical_binary(self, node):
        """Set the HypoConfigTool instance the 'and' and 'or' scenarios
        these take two predicates"""
        
        scen = node.scenario
        klass = self.tool_factories[scen][0]
        sn = self.tool_factories[scen][1]
        name = '%s_%d' % (scen, sn)
        self.tool_factories[scen][1] += 1

        tool = klass(name=name)

        # print 'ToolSetter, setting lhs ', node.children[0].tool
        tool.lhs = node.children[0].tool
        tool.rhs = node.children[1].tool

        tool.node_id = node.node_id
        tool.parent_id = node.parent_id

        node.tool = tool


    def mod_logical_unary(self, node):
        """Set the HypoConfigTool instance for the 'not' scenario
        this takes a single predicate"""
        
        scen = node.scenario
        klass = self.tool_factories[scen][0]
        sn = self.tool_factories[scen][1]
        name = '%s_%d' % (scen, sn)
        self.tool_factories[scen][1] += 1

        tool = klass(name=name)

        # print 'ToolSetter, setting lhs ', node.children[0].tool
        tool.hypoTool = node.children[0].tool

        tool.node_id = node.node_id
        tool.parent_id = node.parent_id

        node.tool = tool


    def mod_combgen(self, node):
        """Set the HypoConfigTool instance for the 'not' scenario
        this takes a single predicate"""
        
        scen = node.scenario
        klass = self.tool_factories[scen][0]
        sn = self.tool_factories[scen][1]
        name = '%s_%d' % (scen, sn)
        self.tool_factories[scen][1] += 1

        config_tool = klass(name=name+'_config')
        config_tool.children = [child.tool for child in node.children]
        [setattr(config_tool, k, v) for k, v in node.conf_attrs.items()]

        helper_tool = CombinationsHelperTool(name=name+'_helper')
        helper_tool.HypoConfigurer = config_tool

        helper_tool.node_id = node.node_id
        helper_tool.parent_id = node.parent_id

        node.tool = helper_tool

    def mod_simple(self, node):
        """Set the TrigJetHypoToolHelperMT instance in a hypo tree node.
        This Algtool will have a TrigJetHypoToolConfig_flownetwork
        instance as an attribute, which it will use to configure itself.
        """

        scen = node.scenario
        klass = self.tool_factories[scen][0]
        sn = self.tool_factories[scen][1]
        name = '%s_%d' % (scen, sn)
        
        self.tool_factories[scen][1] += 1

        config_tool = TrigJetHypoToolConfig_flownetwork
        [setattr(config_tool, k, v) for k, v in node.conf_attrs.items()]
        
        helper_tool = TrigJetHypoToolHelperMT(name=name+'_helper')
        helper_tool.HypoConfigurer = config_tool
        helper_tool.node_id = node.node_id
        helper_tool.parent_id = node.parent_id

        node.tool = helper_tool
 
    def _mod_partition(self, node):
        
        scen = node.scenario
        klass = self.tool_factories[scen][0]
        sn = self.tool_factories[scen][1]
        name = '%s_%d' % (scen, sn)
        
        self.tool_factories[scen][1] += 1
        config_tool = klass(name=name)

        self.treeMap[node.node_id] = node.parent_id
        self.conditionMakers[node.node_id].append(config_tool)
        
        
    def _mod_leaf(self, node):
        """For a leaf node, fill in 
        1. map conaining parent child ids for the node
        2. map conainting the AlgTool used to instanitiate the node Conditions
        """
        print 'FlowNetworkSetter processing node with scenario', node.scenario

        self.treeMap[node.node_id] = node.parent_id

        scen = node.scenario

        for k, v in node.conf_attrs.items():
      
            klass = self.tool_factories[scen][0]
            sn = self.tool_factories[scen][1]

            name = '%s_%d' % (scen, sn)
            config_tool = klass(name=name)
            setattr(config_tool, k, v)
            self.conditionMakers[node.node_id].append(config_tool)
            self.tool_factories[scen][1] += 1

        # klass = self.tool_factories[scen][0]
        # sn = self.tool_factories[scen][1]
        # name = '%s_%d' % (scen, sn)
        
        # self.tool_factories[scen][1] += 1

        # config_tool = klass(name=name)
        # [setattr(config_tool, k, v) for k, v in node.conf_attrs.items()]
        # self.conditionMakers[node.node_id] = config_tool
        
        for cn in node.children:
             self.mod_router[cn.scenario](cn)

    def report(self):
        wid = max(len(k) for k in self.tool_factories.keys())
        rep = '\n%s: ' % self.__class__.__name__

        rep += '\n'.join(
            ['%s: %d' % (k.ljust(wid), v[1])
             for k, v in self.tool_factories.items()])

        return rep


    def mod(self, node):
        """Entry mode for this module. Uses a (usaully compound) hypo tree node.
        - The Tools set in the tree nodes are TrigJetConditionConfig_XXX
          objects. These are factories for Condition objects
        - The tree use used to 
          = find the tree vector (node-parent relationsships)
          = find the node - Condition factory relationships
          = instantiate and return a TrigJetHypoToolConfig_flownetwork instance.
        """

        print 'FlowNetworkSetter start rotation'
        new_node = rotate(node)
        print 'FlowNetworkSetter rotation complete'
        new_node.set_ids(0, 0)

        # navigate the tree filling in node-parent and node- Condtion factory
        # relations
        print 'FlowNetworkSetter - node.scenario', new_node.scenario
        self.mod_router[new_node.scenario](new_node)

        
        print 'FlowNetworkSetter result:'
        print new_node

        config_tool = TrigJetHypoToolConfig_flownetwork()

        print 'treeMap: ', self.treeMap
        treeVec = [0 for i in range(len(self.treeMap))]
        for n, p in self.treeMap.items():
            treeVec[n] = p

        config_tool.treeVector = treeVec
           
        conditionMakers = [None for i in range(len(self.treeMap))]
        for n, p in self.conditionMakers.items():
            conditionMakers[n] = p

        
        print conditionMakers
        assert False

        assert None not in conditionMakers 

        config_tool.conditionMakers = conditionMakers
            
        self.tool = TrigJetHypoToolHelperMT(
            name='TrigJetHypoToolConfig_flownetwor_helper')
    
        self.tool.HypoConfigurer = config_tool


        # # convert the maps to  lists
        # print treeMap
        # treeVector = [0 for i in range(len(treeMap))]
        # for i,p in treeMap.items():
        #     treeVector[i] = p
        # 
        # conditionMakersVector =  [0 for i in range(len(conditionMakersDict))]
        # for i,p in conditionMakersDict.items():
        #     conditionMakersVector[i] = p

        #  # instantiate and initilise the TrigJetHypoToolConfig_flownetwork obj.
        # # The helper tool will ask the config tool for various compnents.


        #   name = '%s_%d' % (node.scenario, node.node_id)
        # print name

        #  config_tool = TrigJetHypoToolConfig_flownetwork(name + '_config')
        
        #  config_tool.treeVector = treeVector
        # config_tool.conditionMakers = conditionMakersVector
        
        #  # instantantiate and initialise the helper tool
        # helper_tool =  TrigJetHypoToolHelperMT(name+'_helper')
        # helper_tool.HypoConfigurer = config_tool
        # helper_tool.node_id = node.node_id
        # helper_tool.parent_id = node.parent_id
        # helper_tool.debug=True
        
        #  # overwrite the top node Condition Factory tool
        # # with the TrigJetHypoToolHelperMT intance
        # node.tool = helper_tool 

    def __str__(self):
        s =  '%s: treeMap: %s, conditionMakers [%d]: ' % (
            self.__class__.__name__,
            str(self.treeMap),
            len(self.conditionMakers))

        if self.conditionMakers:
            s += '\n'
            for cm in self.conditionMakers.values():
                s +=  cm.__class__.__name__ + '\n'

        return s 
