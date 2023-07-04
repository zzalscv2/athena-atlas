# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig
from TriggerMenuMT.HLT.Config.ControlFlow.MenuComponentsNaming import CFNaming
from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFTools import (NoHypoToolCreated, 
                                                             algColor, 
                                                             isHypoBase, 
                                                             isComboHypoAlg, 
                                                             isInputMakerBase, 
                                                             isHypoAlg)
from AthenaCommon.CFElements import parOR, seqAND, compName, getProp, hasProp, findAlgorithmByPredicate
from AthenaCommon.Configurable import Configurable, ConfigurableCABehavior
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from DecisionHandling.DecisionHandlingConfig import ComboHypoCfg
import GaudiConfig2
from GaudiKernel.DataHandle import DataHandle
from HLTSeeding.HLTSeedingConfig import mapThresholdToL1DecisionCollection
from TrigCompositeUtils.TrigCompositeUtils import legName
from AthenaConfiguration.ComponentAccumulator import appendCAtoAthena, conf2toConfigurable
from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching

from collections.abc import MutableSequence
import collections.abc
import inspect
import re

from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )

# Pool of mutable ComboHypo instances (as opposed to immutable cache of RecoFragmentsPool)
_ComboHypoPool = dict()


class Node(object):
    """base class representing one Alg + inputs + outputs, to be used to Draw dot diagrams and connect objects"""
    def __init__(self, Alg):
        self.name = ("%sNode")%( Alg.getName() )
        self.Alg=Alg
        self.inputs=[]
        self.outputs=[]

    def addOutput(self, name):
        self.outputs.append(str(name))

    def addInput(self, name):
        self.inputs.append(str(name)) 

    def getOutputList(self):
        return self.outputs

    def getInputList(self):
        return self.inputs

    def __repr__(self):
        return "Node::%s  [%s] -> [%s]"%(self.Alg.getName(), ' '.join(map(str, self.getInputList())), ' '.join(map(str, self.getOutputList())))


class AlgNode(Node):
    """Node class that connects inputs and outputs to basic alg. properties """
    def __init__(self, Alg, inputProp, outputProp):
        Node.__init__(self, Alg)
        self.outputProp = outputProp
        self.inputProp = inputProp

    def addDefaultOutput(self):
        if self.outputProp != '':
            self.addOutput(("%s_%s"%(self.Alg.getName(),self.outputProp)))

    def setPar(self, propname, value):
        cval = getProp( self.Alg, propname)
        if isinstance(cval, MutableSequence):
            cval.append(value)
            return setattr(self.Alg, propname, cval)
        else:
            return setattr(self.Alg, propname, value)

    def resetPar(self, prop):
        cval = getProp(self.Alg, prop)
        if isinstance(cval, MutableSequence):
            return setattr(self.Alg, prop, [])
        else:
            return setattr(self.Alg, prop, "")

    def getPar(self, prop):
        return getProp(self.Alg, prop)

    def resetOutput(self):
        self.resetPar(self.outputProp)

    def resetInput(self):
        self.resetPar(self.inputProp)

    def addOutput(self, name):
        outputs = self.readOutputList()
        if name in outputs:
            log.debug("Output DH not added in %s: %s already set!", self.Alg.getName(), name)
        else:
            if self.outputProp != '':
                self.setPar(self.outputProp, name)
            else:
                log.debug("no outputProp set for output of %s", self.Alg.getName())
        Node.addOutput(self, name)

    def readOutputList(self):
        cval = self.getPar(self.outputProp)
        return (cval if isinstance(cval, MutableSequence) else
                ([str(cval)] if cval else []))

    def addInput(self, name):
        inputs = self.readInputList()        
        if name in inputs:
            log.debug("Input DH not added in %s: %s already set!", self.Alg.getName(), name)
        else:
            if self.inputProp != '':
                self.setPar(self.inputProp, name)
            else:
                log.debug("no InputProp set for input of %s", self.Alg.getName())
        Node.addInput(self, name)
        return len(self.readInputList())

    def readInputList(self):
        cval = self.getPar(self.inputProp)
        return (cval if isinstance(cval, MutableSequence) else
                ([str(cval)] if cval else []))

    def __repr__(self):
        return "Alg::%s  [%s] -> [%s]"%(self.Alg.getName(), ' '.join(map(str, self.getInputList())), ' '.join(map(str, self.getOutputList())))


class HypoToolConf(object):
    """ Class to group info on hypotools for ChainDict"""
    def __init__(self, hypoToolGen):
        # Check if the generator function takes flags:
        self.hasFlags = 'flags' in inspect.signature(hypoToolGen).parameters
        self.hypoToolGen = hypoToolGen
        self.name=hypoToolGen.__name__

    def setConf( self, chainDict):
        if type(chainDict) is not dict:
            raise RuntimeError("Configuring hypo with %s, not good anymore, use chainDict" % str(chainDict) )
        self.chainDict = chainDict

    def create(self, flags):
        """creates instance of the hypo tool"""
        if self.hasFlags:
            return self.hypoToolGen( flags, self.chainDict )
        else:
            return self.hypoToolGen( self.chainDict )

    def confAndCreate(self, flags, chainDict):
        """sets the configuration and creates instance of the hypo tool"""
        self.setConf(chainDict)
        return self.create(flags)


class HypoAlgNode(AlgNode):
    """Node for HypoAlgs"""
    initialOutput= 'StoreGateSvc+UNSPECIFIED_OUTPUT'
    def __init__(self, Alg):
        assert isHypoBase(Alg), "Error in creating HypoAlgNode from Alg "  + compName(Alg)
        AlgNode.__init__(self, Alg, 'HypoInputDecisions', 'HypoOutputDecisions')
        self.previous=[]

    def addOutput(self, name):
        outputs = self.readOutputList()
        if name in outputs:
            log.debug("Output DH not added in %s: %s already set!", self.name, name)
        elif self.initialOutput in outputs:
            AlgNode.addOutput(self, name)
        else:
            log.error("Hypo %s has already %s as configured output: you may want to duplicate the Hypo!",
                      self.name, outputs[0])


    def addHypoTool (self, flags, hypoToolConf):
        from TriggerMenuMT.HLT.Config.GenerateMenuMT_newJO import isCAMenu 
        log.debug("Adding HypoTool %s for chain %s to %s", hypoToolConf.name, hypoToolConf.chainDict['chainName'], self.Alg.getName())        
        try:
            result = hypoToolConf.create(flags)
            if isinstance(result, ComponentAccumulator):
                tool = result.popPrivateTools()
                assert not isinstance(tool, list), "Can not handle list of tools"
                if not isCAMenu():
                    # do not do this in CA, use unconverted tool
                    tool = conf2toConfigurable(tool)
                    self.Alg.HypoTools = self.Alg.HypoTools + [tool]  # see ATEAM-773
                else:
                    self.Alg.HypoTools.append(tool)
                return result
            else:
                self.Alg.HypoTools = self.Alg.HypoTools + [result]  # see ATEAM-773
            if isCAMenu():
                assert isinstance(self.Alg.HypoTools[-1], GaudiConfig2._configurables.Configurable), "The Hypo Tool for {} is not Configurable2".format(hypoToolConf.chainDict['chainName'])

        except NoHypoToolCreated as e:
            log.debug("%s returned empty tool: %s", hypoToolConf.name, e)
        return None
    
    def setPreviousDecision(self,prev):
        self.previous.append(prev)
        return self.addInput(prev)

    def resetDecisions(self):
        self.previous = []
        self.resetOutput()
        self.resetInput()

    def __repr__(self):
        return "HypoAlg::%s  [%s] -> [%s], previous = [%s], HypoTools=[%s]" % \
            (compName(self.Alg),' '.join(map(str, self.getInputList())),
             ' '.join(map(str, self.getOutputList())),
             ' '.join(map(str, self.previous)),
             ' '.join([t.getName() for t in self.Alg.HypoTools]))


class InputMakerNode(AlgNode):
    def __init__(self, Alg):
        assert isInputMakerBase(Alg), "Error in creating InputMakerNode from Alg "  + compName(Alg)
        AlgNode.__init__(self,  Alg, 'InputMakerInputDecisions', 'InputMakerOutputDecisions')
        self.resetInput()
        self.resetOutput() ## why do we need this in CA mode??
        input_maker_output = CFNaming.inputMakerOutName(compName(self.Alg))
        self.addOutput(input_maker_output)


class ComboMaker(AlgNode):
    def __init__(self, name, comboHypoCfg):
        self.prop1 = "MultiplicitiesMap"
        self.prop2 = "LegToInputCollectionMap"
        self.comboHypoCfg = comboHypoCfg
        Alg = self.create( name )
        log.debug("ComboMaker init: Alg %s", name)
        AlgNode.__init__(self,  Alg, 'HypoInputDecisions', 'HypoOutputDecisions')
        self.resetInput()
        self.resetOutput() ## why do we need this in CA mode??
        # reset the chains, why do we need to do it?
        setattr(self.Alg, self.prop1, {})
        setattr(self.Alg, self.prop2, {})

    def create (self, name):
        log.debug("ComboMaker.create %s",name)
        return self.comboHypoCfg(name=name)        

    """
    AlgNode automatically de-duplicates input ReadHandles upon repeated calls to addInput.
    Node instead stores all the inputs, even if repeated (self.inputs)
    This function maps from the raw number of times that addInput was called to the de-duplicated index of the handle.
    E.g. a step processing chains such as HLT_e5_mu6 would return [0,1]
    E.g. a step processing chains such as HLT_e5_e6 would return [0,0]
    E.g. a step processing chains such as HLT_e5_mu6_mu7 would return [0,1,1]
    These data are needed to configure the step's ComboHypo
    """
    def mapRawInputsToInputsIndex(self):
        mapping = []        
        theInputs = self.readInputList() #only unique inputs    
        for rawInput in self.inputs: # all inputs
            mapping.append( theInputs.index(rawInput) )
        return mapping

    """ overwrite AlgNode::addInput with Node::addInput"""
    def addInput(self, name):        
        return AlgNode.addInput(self, name)

    def addChain(self, chainDict):        
        chainName = chainDict['chainName']
        chainMult = chainDict['chainMultiplicities']        
        legsToInputCollections = self.mapRawInputsToInputsIndex()                
        if len(chainMult) != len(legsToInputCollections):
            log.error("ComboMaker for Alg:{} with addChain for:{} Chain multiplicity:{} Per leg input collection index:{}."
                .format(compName(self.Alg), chainName, tuple(chainMult), tuple(legsToInputCollections)))
            log.error("The size of the multiplicies vector must be the same size as the per leg input collection vector.")
            log.error("The ComboHypo needs to know which input DecisionContainers contain the DecisionObjects to be used for each leg.")
            log.error("Check why ComboMaker.addInput(...) was not called exactly once per leg.")
            raise Exception("[createDataFlow] Error in ComboMaker.addChain. Cannot proceed.")

        cval1 = getProp(self.Alg, self.prop1)  # check necessary to see if chain was added already?
        cval2 = getProp(self.Alg, self.prop2)          
        if type(cval1) is dict or isinstance(cval1, GaudiConfig2.semantics._DictHelper):
            if chainName in cval1.keys():
                log.error("ERROR in configuration: ComboAlg %s has already been configured for chain %s", compName(self.Alg), chainName)
                raise Exception("[createDataFlow] Error in ComboMaker.addChain. Cannot proceed.")
            else:
                cval1[chainName] = chainMult
                cval2[chainName] = legsToInputCollections
        else:
            cval1 = {chainName : chainMult}
            cval2 = {chainName : legsToInputCollections} 

        setattr(self.Alg, self.prop1, cval1)
        setattr(self.Alg, self.prop2, cval2)
        

    def getChains(self):
        cval = getProp(self.Alg, self.prop1)        
        return cval.keys()


    def createComboHypoTools(self, flags, chainDict, comboToolConfs):
         """Created the ComboHypoTools"""
         if not len(comboToolConfs):
             return
         confs = [ HypoToolConf( tool ) for tool in comboToolConfs ]
         log.debug("ComboMaker.createComboHypoTools for chain %s, Alg %s with %d tools", chainDict["chainName"],self.Alg.getName(), len(comboToolConfs))        
         for conf in confs:
             log.debug("ComboMaker.createComboHypoTools adding %s", conf)
             tools = self.Alg.ComboHypoTools
             self.Alg.ComboHypoTools = tools + [ conf.confAndCreate( flags, chainDict ) ]
 

##########################################################
# Now sequences and chains
##########################################################


             
class EmptyMenuSequence(object):
    """ Class to emulate reco sequences with no Hypo"""
    """ By construction it has no Hypo;"""
    
    def __init__(self, the_name):
        self._name = the_name
        Maker = CompFactory.InputMakerForRoI("IM"+the_name)
        # isEmptyStep causes the IM to try at runtime to merge by feature by default (i.e for empty steps appended after a leg has finised).
        # But if this failes then it will merge by initial ROI instead (i.e. for empy steps prepended before a leg has started)
        Maker.isEmptyStep = True 
        Maker.RoIsLink = 'initialRoI' #(this is the default property, just making it explicit)
        self._maker       = InputMakerNode( Alg = Maker )
        self._seed        = ''
        self._sequence    = Node( Alg = seqAND(the_name, [Maker]))
        log.debug("Made EmptySequence %s",the_name)

    @property
    def sequence(self):
        return self._sequence

    @property
    def seed(self):
        return self._seed

    @property
    def name(self):
        return self._name

    def getOutputList(self):
        return self._maker.readOutputList() # Only one since it's merged

    def connectToFilter(self, outfilter):
        """ Connect filter to the InputMaker"""
        self._maker.addInput(outfilter)

    def createHypoTools(self, chainDict):
        log.debug("This sequence is empty. No Hypo to configure")
    
    def getHypoToolConf(self):
        return None

    def addToSequencer(self, recoSeq_list, hypo_list):
        recoSeq_list.add(self.sequence.Alg)                    
        
    def buildDFDot(self, cfseq_algs, all_hypos, last_step_hypo_nodes, file):
        cfseq_algs.append(self._maker)
        cfseq_algs.append(self.sequence )
        file.write("    %s[fillcolor=%s]\n"%(self._maker.Alg.getName(), algColor(self._maker.Alg)))
        file.write("    %s[fillcolor=%s]\n"%(self.sequence.Alg.getName(), algColor(self.sequence.Alg)))
        return cfseq_algs, all_hypos, last_step_hypo_nodes

    def setSeed( self, seed ):
        self._seed = seed

    def __repr__(self):
        return "MenuSequence::%s \n Hypo::%s \n Maker::%s \n Sequence::%s \n HypoTool::%s\n"\
            %(self.name, "Empty", self._maker.Alg.getName(), self.sequence.Alg.getName(), "None")

class MenuSequence(object):
    """ Class to group reco sequences with the Hypo"""
    """ By construction it has one Hypo Only; behaviour changed to support muFastOvlpRmSequence() which has two, but this will change"""

    def __init__(self, flags, Sequence, Maker,  Hypo, HypoToolGen, IsProbe=False):
        assert compName(Maker).startswith("IM"), "The input maker {} name needs to start with letter: IM".format(compName(Maker))        
        # For probe legs we need to substitute the inputmaker and hypo alg
        # so we will use temp variables for both
        if IsProbe: 
            #CA based config should have probe IM set up already, so use existing IM/Hypo if it's already probe
            if 'probe' in Maker.getName():
                _Maker=Maker
                _Hypo = Hypo
            else:
                _Hypo = RecoFragmentsPool.retrieve(MenuSequence.getProbeHypo,flags,basehypo=Hypo)
                # Reset this so that HypoAlgNode.addOutput will actually do something
                _Hypo.HypoOutputDecisions = "StoreGateSvc+UNSPECIFIED_OUTPUT"
                _Maker = RecoFragmentsPool.retrieve(MenuSequence.getProbeInputMaker,flags,baseIM=Maker)

        else: # For regular legs, just use the provided components
            _Hypo = Hypo
            _Maker = Maker
            _Sequence = Sequence
        self._maker = InputMakerNode( Alg = _Maker )
        self._seed =''
        input_maker_output= self.maker.readOutputList()[0] # only one since it's merged

        # Connect InputMaker output to ROBPrefetchingAlg(s) if there are any (except for probe seq which is handled later)
        if ROBPrefetching.StepRoI in flags.Trigger.ROBPrefetchingOptions:
            seqChildren = Sequence.getChildren() if hasattr(Sequence,'getChildren') else Sequence.Members
            for child in seqChildren:
                if hasProp(child,'ROBPrefetchingInputDecisions') and input_maker_output not in child.ROBPrefetchingInputDecisions and not input_maker_output.endswith('_probe'):
                    locked = bool(child.isLocked()) if hasattr(child,'isLocked') else False
                    if locked:
                        child.unlock()
                    child.ROBPrefetchingInputDecisions += [input_maker_output]
                    if locked:
                        child.lock()

        self._name = CFNaming.menuSequenceName(compName(_Hypo))
        self._hypoToolConf = HypoToolConf( HypoToolGen )
        Hypo.RuntimeValidation = flags.Trigger.doRuntimeNaviVal
        self._hypo = HypoAlgNode( Alg = _Hypo )
        hypo_output = CFNaming.hypoAlgOutName(compName(_Hypo))
        self._hypo.addOutput(hypo_output)
        self._hypo.setPreviousDecision( input_maker_output )

        #probe legs in lagacy config need the IM cloned
        #CA based chains should use SelectionCA which does the probe leg changes directly, so skip if already a probe sequence
        if IsProbe and 'probe' not in Sequence.getName():
            def getProbeSequence(baseSeq,probeIM):
                # Add IM and sequence contents to duplicated sequence
                probeSeq = baseSeq.clone(baseSeq.getName()+"_probe")
                probeSeq += probeIM                
                if isinstance(probeIM,CompFactory.EventViewCreatorAlgorithm):
                    for child in baseSeq.getChildren()[1:]:
                        probeChild = child.clone(child.getName()+"_probe")
                        if hasProp(child,'ROBPrefetchingInputDecisions') and (ROBPrefetching.StepRoI in flags.Trigger.ROBPrefetchingOptions):
                            # child is a ROB prefetching alg, map the probe IM decisions
                            probeChild.ROBPrefetchingInputDecisions = [str(probeIM.InputMakerOutputDecisions)]
                        elif probeIM.ViewNodeName == child.getName():
                            # child is the view alg sequence, map it to the probe sequence
                            probeIM.ViewNodeName = probeChild.getName()
                            for viewalg in child.getChildren():
                                probeChild += viewalg
                        probeSeq += probeChild
                return probeSeq
            # Make sure nothing was lost
            _Sequence = getProbeSequence(baseSeq=Sequence,probeIM=_Maker)
            assert len(_Sequence.getChildren()) == len(Sequence.getChildren()), f'Different number of children in sequence {_Sequence.getName()} vs {Sequence.getName()} ({len(_Sequence.getChildren())} vs {len(Sequence.getChildren())})'
        else:
            _Sequence = Sequence

        self._sequence = Node( Alg=_Sequence)

        log.debug("connecting InputMaker and HypoAlg, adding: \n\
        InputMaker::%s.output=%s",\
                        compName(self.maker.Alg), input_maker_output)   
        log.debug("HypoAlg::%s.HypoInputDecisions=%s, \n \
        HypoAlg::%s.HypoOutputDecisions=%s",\
                      compName(self.hypo.Alg), self.hypo.readInputList()[0], compName(self.hypo.Alg), self.hypo.readOutputList()[0])

    @property
    def seed(self):
        return self._seed

    @property
    def name(self):
        return self._name

    @property
    def sequence(self):
        return self._sequence

    @property
    def maker(self):
        return self._maker

    @property
    def hypo(self):
        return self._hypo

    @staticmethod
    def getProbeHypo(flags,basehypo):
        '''Clone hypo & input maker'''
        probehypo = basehypo.clone(basehypo.getName()+"_probe")
        for p,v in basehypo.getValuedProperties().items():
            setattr(probehypo,p,getattr(basehypo,p))
        return probehypo

    @staticmethod
    def getProbeInputMaker(flags,baseIM):
        probeIM = baseIM.clone(baseIM.getName()+"_probe")
        for p,v in baseIM.getValuedProperties().items():
            setattr(probeIM,p,getattr(baseIM,p))

        if isinstance(probeIM,CompFactory.EventViewCreatorAlgorithm):
            assert(baseIM.Views)
            probeIM.Views = baseIM.Views.Path + "_probe"
            probeIM.RoITool = baseIM.RoITool
            probeIM.InputCachedViews = baseIM.InputMakerOutputDecisions
            def updateHandle(baseTool, probeTool, handleName):
                if hasattr(baseTool, handleName) and getattr(baseTool, handleName).Path!="StoreGateSvc+":
                    setattr(probeTool, handleName, getattr(baseTool, handleName).Path + "_probe")
            updateHandle(baseIM.RoITool, probeIM.RoITool, "RoisWriteHandleKey")
            if hasattr(baseIM.RoITool, "RoiCreator"):
                updateHandle(baseIM.RoITool, probeIM.RoITool, "ExtraPrefetchRoIsKey")
                updateHandle(baseIM.RoITool.RoiCreator, probeIM.RoITool.RoiCreator, "RoisWriteHandleKey")
        else:
            raise TypeError(f"Probe leg input maker may not be of type '{baseIM.__class__}'.")

        # Reset this initially to avoid interference
        # with the original
        probeIM.InputMakerInputDecisions = []
        return probeIM

    def getOutputList(self):
        outputlist = []     
        outputlist.append(self._hypo.readOutputList()[0])
        return outputlist

    
    def connectToFilter(self, outfilter):
        """ Connect filter to the InputMaker"""
        log.debug("connecting %s to inputs of %s", outfilter,compName(self._maker.Alg))        
        self._maker.addInput(outfilter)        
    
  
    def createHypoTools(self, flags, chainDict):
        if type(self._hypoToolConf) is list:
            log.warning ("This sequence %s has %d multiple HypoTools ",self.sequence.name, len(self.hypoToolConf))
            for hypo, hypoToolConf in zip(self._hypo, self._hypoToolConf):
                hypoToolConf.setConf( chainDict )
                hypo.addHypoTool(flags, self._hypoToolConf)
        else:
            self._hypoToolConf.setConf( chainDict )
            self._hypo.addHypoTool(flags, self._hypoToolConf) #this creates the HypoTools
            
    def getHypoToolConf(self) :
        return self._hypoToolConf

    def getHypoToolMap(self,chainDict) :
        self._hypoToolConf.setConf( chainDict )
        return (self._hypo.Alg.getName(), self._hypoToolConf)

    def addToSequencer(self, recoSeq_list, hypo_list):
        recoSeq_list.add(self.sequence.Alg)
        hypo_list.add(self._hypo.Alg)
            
    def buildDFDot(self, cfseq_algs, all_hypos, last_step_hypo_nodes, file):
        cfseq_algs.append(self._maker)
        cfseq_algs.append(self.sequence )
        file.write("    %s[fillcolor=%s]\n"%(self._maker.Alg.getName(), algColor(self._maker.Alg)))
        file.write("    %s[fillcolor=%s]\n"%(self.sequence.Alg.getName(), algColor(self.sequence.Alg)))    
        cfseq_algs.append(self._hypo)
        file.write("    %s[color=%s]\n"%(self._hypo.Alg.getName(), algColor(self._hypo.Alg)))
        all_hypos.append(self._hypo)
        return cfseq_algs, all_hypos, last_step_hypo_nodes

    def setSeed( self, seed ):
        self._seed = seed

    def __repr__(self):    
        hyponame = self._hypo.Alg.getName()
        hypotool = self._hypoToolConf.name
        return "MenuSequence::%s \n Hypo::%s \n Maker::%s \n Sequence::%s \n HypoTool::%s\n"\
          %(self.name, hyponame, self._maker.Alg.getName(), self.sequence.Alg.getName(), hypotool)


class MenuSequenceCA(MenuSequence):
    ''' MenuSequence with Component Accumulator '''

    def __init__(self, flags, selectionCA, HypoToolGen, isProbe=False, globalRecoCA=None ):
        self.ca = selectionCA
        self._globalCA = globalRecoCA
        allAlgs = self.ca.getEventAlgos()
        inputMaker = [ a for a in allAlgs if isInputMakerBase(a)]
        assert len(inputMaker) == 1, "Wrong number of input makers in the component accumulator {}".format(len(inputMaker))
        inputMaker = inputMaker[0]        
        hypoAlg = [ a for a in allAlgs if isHypoAlg(a)]
        assert len(hypoAlg) == 1, "Wrong number of hypo algs in the component accumulator {}".format(len(hypoAlg))
        hypoAlg = hypoAlg[0]         
        MenuSequence.__init__(self, flags, self.ca.topSequence(), inputMaker,  hypoAlg, HypoToolGen, IsProbe=isProbe)

    @property
    def sequence(self):
        makerAlg = self.ca.getEventAlgo(self._maker.Alg.getName())
        self._maker.Alg = makerAlg
        return self._sequence

    @property
    def maker(self):
        makerAlg = self.ca.getEventAlgo(self._maker.Alg.getName())
        self._maker.Alg = makerAlg
        return self._maker

    @property
    def hypo(self):
        hypoAlg = self.ca.getEventAlgo(self._hypo.Alg.getName())
        self._hypo.Alg = hypoAlg
        return self._hypo

    @property
    def globalRecoCA(self):
        return self._globalCA

    def __del__(self):
        self.ca.wasMerged()
        if self._globalCA:
            self._globalCA.wasMerged()

class EmptyMenuSequenceCA(EmptyMenuSequence):
    def __init__(self, the_name):
        self.ca=ComponentAccumulator()
        EmptyMenuSequence.__init__(self,the_name )
        self.ca.addEventAlgo(self._maker.Alg)

    @property
    def maker(self):
        makerAlg = self.ca.getEventAlgo(self._maker.Alg.getName())
        self._maker.Alg = makerAlg
        return self._maker

    @property
    def globalRecoCA(self):
        return None

    def __del__(self):
        self.ca.wasMerged()

class Chain(object):
    """Basic class to define the trigger menu """
    __slots__ ='name','steps','nSteps','alignmentGroups','vseeds','L1decisions', 'topoMap'
    def __init__(self, name, ChainSteps, L1Thresholds, nSteps = [], alignmentGroups = [], topoMap=None):
        """
        Construct the Chain from the steps
        Out of all arguments the ChainSteps & L1Thresholds are most relevant, the chain name is used in debug messages
        """
        self.name   = name
        self.steps  = ChainSteps
        self.nSteps = nSteps
        self.alignmentGroups = alignmentGroups
        self.vseeds = L1Thresholds

        # The chain holds a map of topo ComboHypoTool configurators
        # This is needed to allow placement of the ComboHypoTool in the right position
        # for multi-leg chains (defaults to last step)
        # Format is {"[step name]" : ([topo config function], [topo descriptor string]), ...}
        # Here, the topo descriptor string would usually be the chain name expression that
        # configures the topo
        self.topoMap = {}
        if topoMap:
            self.topoMap.update(topoMap)

        # L1decisions are used to set the seed type (EM, MU,JET), removing the actual threshold
        # in practice it is the HLTSeeding Decision output
        self.L1decisions = [ mapThresholdToL1DecisionCollection(stri) for stri in L1Thresholds]
        self.setSeedsToSequences()
        log.debug("[Chain.__init__] Made Chain %s with seeds: %s ", name, self.L1decisions)

    def append_bjet_steps(self,new_steps):
        assert len(self.nSteps) == 1, "[Chain.append_bjet_steps] appending already-merged step lists - chain object will be broken. This should only be used to append Bjets to jets!"
        self.steps = self.steps + new_steps
        self.nSteps = [len(self.steps)]

    def append_step_to_jet(self,new_steps):
        assert len(self.nSteps) == 1, "[Chain.append_step_to_jet] appending already-merged step lists - chain object will be broken. This is used either for appending Beamspot algorithms to jets!"
        self.steps = self.steps + new_steps
        self.nSteps = [len(self.steps)]


    def numberAllSteps(self):
        if len(self.steps)==0:
            return
        else:
            for stepID,step in enumerate(self.steps):
                step_name = step.name
                if re.search('^Step[0-9]_',step_name):
                    step_name = step_name[6:]
                elif re.search('^Step[0-9]{2}_', step_name):
                    step_name = step_name[7:]   
                step.name = 'Step%d_'%(stepID+1)+step_name
        return


    def insertEmptySteps(self, empty_step_name, n_new_steps, start_position):
        #start position indexed from 0. if start position is 3 and length is 2, it works like:
        # [old1,old2,old3,old4,old5,old6] ==> [old1,old2,old3,empty1,empty2,old4,old5,old6]

        if len(self.steps) == 0 :
            log.error("I can't insert empty steps because the chain doesn't have any steps yet!")

        if len(self.steps) < start_position :
            log.error("I can't insert empty steps at step %d because the chain doesn't have that many steps!", start_position)

        
        chain_steps_pre_split = self.steps[:start_position]
        chain_steps_post_split = self.steps[start_position:]

        next_step_name = ''
        prev_step_name = ''
        # copy the same dictionary as the last step, which else?
        prev_chain_dict = []
        if start_position == 0:
            next_step_name = chain_steps_post_split[0].name
            if re.search('^Step[0-9]_',next_step_name):
                next_step_name = next_step_name[6:]
            elif re.search('^Step[0-9]{2}_', next_step_name):
                next_step_name = next_step_name[7:]

            prev_step_name = 'empty_'+str(len(self.L1decisions))+'L1in'
            prev_chain_dict = chain_steps_post_split[0].stepDicts
        else:
            if len(chain_steps_post_split) == 0:
                log.error("Adding empty steps to the end of a chain (%s)- why would you do this?",self.name)
            else:
                prev_step_name = chain_steps_pre_split[-1].name
                next_step_name = chain_steps_post_split[0].name
            prev_chain_dict = chain_steps_pre_split[-1].stepDicts


        steps_to_add = []
        for stepID in range(1,n_new_steps+1):
            new_step_name =  prev_step_name+'_'+empty_step_name+'%d_'%stepID+next_step_name

            log.debug("Configuring empty step %s", new_step_name)
            steps_to_add += [ChainStep(new_step_name, [], [], chainDicts=prev_chain_dict, comboHypoCfg=ComboHypoCfg)]
        
        self.steps = chain_steps_pre_split + steps_to_add + chain_steps_post_split

        return

    def checkMultiplicity(self):
        if len(self.steps) == 0:
            return 0
        mult=[sum(step.multiplicity) for step in self.steps] # on mult per step
        not_empty_mult = [m for m in mult if m!=0]
        if len(not_empty_mult) == 0: #empty chain?
            log.error("checkMultiplicity: Chain %s has all steps with multiplicity =0: what to do?", self.name)
            return 0
        if not_empty_mult.count(not_empty_mult[0]) != len(not_empty_mult):
            log.error("checkMultiplicity: Chain %s has steps with differnt multiplicities: %s", self.name, ' '.join(mult))
            return 0

        if not_empty_mult[0] != len(self.vseeds):
            log.error("checkMultiplicity: Chain %s has %d multiplicity per step, and %d L1Decisions", self.name, mult, len(self.vseeds))
            return 0
        return not_empty_mult[0]



    def setSeedsToSequences(self):
        """ Set the L1 seeds (L1Decisions) to the menu sequences """
        if len(self.steps) == 0:
            return

        for step in self.steps:
            step.setSeedsToSequences()
    
    def createHypoTools(self, flags):
        """ This is extrapolating the hypotool configuration from the chain name"""
        log.debug("createHypoTools for chain %s", self.name)        
        
        for step in self.steps:
            if step.combo is None:
                continue
            log.debug("createHypoTools for Step %s", step.name)
            log.debug('%s in new hypo tool creation method, step mult= %d', self.name, sum(step.multiplicity))
            log.debug("N(seq)=%d, N(chainDicts)=%d", len(step.sequences), len(step.stepDicts))
            for seq, onePartChainDict in zip(step.sequences, step.stepDicts):
                log.debug('    seq: %s, onePartChainDict:', seq.name)
                log.debug('    %s', onePartChainDict)
                seq.createHypoTools( flags, onePartChainDict )
            
            step.createComboHypoTools(flags, self.name)

    # Receives a pair with the topo config function and an identifier string,
    # optionally also a target step name
    # The string is needed to rename the step after addition of the ComboHypoTool
    def addTopo(self,topoPair,step="last"):
        stepname = "last step" if step=="last" else step.name
        log.debug("Adding topo configurator %s for %s to %s", topoPair[0].__qualname__, topoPair[1], "step " + stepname)
        self.topoMap[step] = topoPair

    def __repr__(self):
        return "-*- Chain %s -*- \n + Seeds: %s, Steps: %s, AlignmentGroups: %s \n + Steps: \n %s \n"%(\
                    self.name, ' '.join(map(str, self.L1decisions)), self.nSteps, self.alignmentGroups, '\n '.join(map(str, self.steps)))



# next:  can we remove multiplicity array, if it can be retrieved from the ChainDict?
class ChainStep(object):
    """Class to describe one step of a chain; if multiplicity is greater than 1, the step is combo/combined.  Set one multiplicity value per sequence"""
    def __init__(self, name,  Sequences = [], multiplicity = [1], chainDicts = [], comboHypoCfg = ComboHypoCfg, comboToolConfs = [], isEmpty = False, createsGhostLegs = False):
        # TODO: remove parameter multiplicity, since this must be extracted from the ChainDict ATR-23928
        # include cases of empty steps with multiplicity = [] or multiplicity=[0,0,0///]
        if sum(multiplicity) == 0:
            multiplicity = []
        else:
            log.debug("chain %s, step %s: len=%d multiplicty=%d", chainDicts[0]['chainName'], name, len(chainDicts), len(multiplicity))
            # sanity check on inputs, excluding empty steps
            if len(chainDicts) != len(multiplicity):
                log.error("[ChainStep] Sequences: %s",Sequences)
                log.error("[ChainStep] chainDicts: %s",chainDicts)
                log.error("[ChainStep] multiplicity: %s",multiplicity)
                raise RuntimeError("[ChainStep] Tried to configure a ChainStep %s with %i multiplicity and %i dictionaries. These lists must have the same size" % (name, len(multiplicity), len(chainDicts)) )
            
            if len(Sequences) != len(multiplicity) and 'Jet' not in chainDicts[0]['signatures']:
                log.error("[ChainStep] Sequences: %s",Sequences)
                log.error("[ChainStep] multiplicities: %s",multiplicity)
                raise RuntimeError("Tried to configure a ChainStep %s with %i Sequences and %i multiplicities. These lists must have the same size" % (name, len(Sequences), len(multiplicity)) )
 
        self.name      = name
        self.sequences = Sequences
        self.onlyJets  = False
        sig_set = None
        if len(chainDicts) > 0  and 'signature' in chainDicts[0]: 
            leg_signatures = [step['signature'] for step in chainDicts if step['signature'] != 'Bjet']
            if (len(multiplicity) > 0 and leg_signatures.count('Jet') == 1) and (len(set(leg_signatures)) > 1 and chainDicts[0]['signatures'].count('Jet') > 1) and (len(leg_signatures) != 2 or leg_signatures.count('MET') == 0):
                index_jetLeg = leg_signatures.index('Jet')
                multiplicity[index_jetLeg:index_jetLeg] = [1] * (len(chainDicts[0]['chainMultiplicities']) - len(multiplicity))
            sig_set = set([step['signature'] for step in chainDicts])
            if len(sig_set) == 1 and ('Jet' in sig_set or 'Bjet' in sig_set):
                self.onlyJets = True
            if len(sig_set) == 2 and ('Jet' in sig_set and 'Bjet' in sig_set):
                self.onlyJets = True

        self.multiplicity = multiplicity
        self.comboHypoCfg = comboHypoCfg
        self.comboToolConfs = list(comboToolConfs)
        self.stepDicts = chainDicts # one dict per leg
        self.isEmpty = (sum(multiplicity) == 0 or isEmpty)
        if not self.isEmpty:
            #self.relabelLegIdsForJets()
            self.setChainPartIndices()
        self.legIds = self.getLegIds() if len(multiplicity) > 1 else [0]
        self.makeCombo()

    def relabelLegIdsForJets(self):

        has_jets = False
        leg_counter = []    

        for step_dict in self.stepDicts:
            if 'Jet' in step_dict['signatures'] or 'Bjet' in step_dict['signatures']:
                has_jets = True
                leg_counter += [len(step_dict['chainParts'])]
            elif len(step_dict['chainParts']) > 1:
                log.error("[relabelLegIdsForJets] this should only happen for jet chains, but the signatures are %s",step_dict['signatures'])
                raise Exception("[relabelLegIdsForJets] leg labelling is probably wrong...")
            else:
                leg_counter +=[1]

        self.onlyJets = False
        if len(leg_counter) == len(self.multiplicity):
            self.onlyJets = True
 
        log.debug("[relabelLegIdsForJets] leg_counter: %s , onlyjets: %s, multiplicity: %s...",leg_counter, self.onlyJets, self.multiplicity) 

        if not has_jets or len(leg_counter) == len(self.multiplicity): #also don't relabel only jets since no offset needed
            return

        if len(leg_counter) == 1 or (len(set(leg_counter)) == 1 and leg_counter[0] == 1):
            #all legs are already length 1, or there's only one jet blocks nothing to do
            return
        elif len(set(leg_counter[:-1])) == 1 and leg_counter[0] == 1:
            #it's the last leg that's not length one, so we don't need to relabel any end legs
            return
        else:
            nLegs = 0
            for i,nLegParts in enumerate(leg_counter):
                oldLegName = self.stepDicts[i]['chainName']
                if re.search('^leg[0-9]{3}_',oldLegName):
                    oldLegName = oldLegName[7:]
                else:
                    log.error("[relabelLegIdsForJets] you told me to relabel the legs for %s",self.stepDicts)
                    raise Exception("[relabelLegIdsForJets] you told me to relabel the legs but this leg doesn't have a legXXX_ name!")
                self.stepDicts[i]['chainName'] = legName(oldLegName,nLegs)
                nLegs += nLegParts
        return
    
    #Heather updated for full jet chain dicts
    def setChainPartIndices(self):
    
        leg_counter = 0
        lists_of_chainPartNames = []
        for step_dict in self.stepDicts:
            if len(lists_of_chainPartNames) == 0:
                lists_of_chainPartNames += [[cp['chainPartName'] for cp in step_dict['chainParts']]]
            else:
                new_list_of_chainPartNames = [cp['chainPartName'] for cp in step_dict['chainParts']]
                if new_list_of_chainPartNames == lists_of_chainPartNames[-1]:
                    leg_counter -= len(new_list_of_chainPartNames)
            for chainPart in step_dict['chainParts']:
                chainPart['chainPartIndex'] =  leg_counter
                leg_counter += 1
        return

    def getLegIds(self):
        leg_ids = []
        for istep,step_dict in enumerate(self.stepDicts):
            if step_dict['chainName'][0:3] != 'leg':
                if self.onlyJets:
                    leg_ids += [istep]
                else:
                    log.error("[getLegIds] chain %s has multiplicities %s but no legs? ",step_dict['chainName'], self.multiplicity)
                    raise Exception("[getLegIds] cannot extract leg IDs, exiting.")
            else:
                leg_ids += [int(step_dict['chainName'][3:6])]
        return leg_ids

    def addComboHypoTools(self, tool):
        #this function does not add tools, it just adds tool. do not pass it a list!
        self.comboToolConfs.append(tool)

    def makeCombo(self):
        self.combo = None 
        if self.isEmpty or self.comboHypoCfg is None:
            return        
        comboName = CFNaming.comboHypoName(self.name)
        key = hash((comboName, self.comboHypoCfg))
        if key not in _ComboHypoPool:            
            _ComboHypoPool[key] = createComboAlg(None, name=comboName, comboHypoCfg=self.comboHypoCfg)
        self.combo = _ComboHypoPool[key]
        

    def createComboHypoTools(self, flags, chainName):
        chainDict = HLTMenuConfig.getChainDictFromChainName(chainName)
        self.combo.createComboHypoTools(flags, chainDict, self.comboToolConfs)
            
    def getChainLegs(self):
        """ This is extrapolating the chain legs from the step dictionaries"""       
        legs = [part['chainName'] for part in self.stepDicts]
        return legs

    def getChainNames(self):  
        if self.combo is not None:   
            return list(self.combo.getChains())
        return self.getChainLegs()

    def setSeedsToSequences(self):
        for seed, seq in zip( [d["chainParts"][0]["L1threshold"] for d in self.stepDicts], self.sequences):
            l1Collection = mapThresholdToL1DecisionCollection(seed)
            seq.setSeed( l1Collection )
            log.debug( "setSeedsToSequences: ChainStep %s adding seed %s to sequence %s", self.name, l1Collection, seq.name )

    def __repr__(self):
        if len(self.sequences) == 0:
            return "--- ChainStep %s ---\n is Empty, ChainDict = %s "%(self.name,  ' '.join(map(str, [dic['chainName'] for dic in self.stepDicts])) )
        
        repr_string= "--- ChainStep %s ---\n , multiplicity = %s  ChainDict = %s \n + MenuSequences = %s "%\
          (self.name,  ' '.join(map(str,[mult for mult in self.multiplicity])),
             ' '.join(map(str, [dic['chainName'] for dic in self.stepDicts])),
             ' '.join(map(str, [seq.name for seq in self.sequences]) ))
        if self.combo is not None:
            repr_string += "\n + ComboHypo = %s" %(compName(self.combo.Alg))
            if len(self.comboToolConfs)>0:
                repr_string +=",  ComboHypoTools = %s" %(' '.join(map(str, [tool.__name__ for tool in self.comboToolConfs]))) 
        repr_string += "\n"       
        return repr_string


def createComboAlg(dummyFlags, name, comboHypoCfg):
    # remove StepXXX_ from the name
    if re.search('^Step[0-9]_',name):
        name = name[6:]
    elif re.search('^Step[0-9]{2}_', name):
        name = name[7:]
    return ComboMaker(name, comboHypoCfg)



class InEventRecoCA( ComponentAccumulator ):
    """ Class to handle in-event reco """
    def __init__(self, name, inputMaker=None, **inputMakerArgs):
        super( InEventRecoCA, self ).__init__()
        self.name = name
        self.recoSeq = None

        if inputMaker:
            assert len(inputMakerArgs) == 0, "No support for explicitly passed input maker and and input maker arguments at the same time" 
            self.inputMakerAlg = inputMaker
        else:
            assert 'name' not in inputMakerArgs, "The name of input maker is predefined by the name of sequence"
            args = {'name': "IM"+name,
                    'RoIsLink' : 'initialRoI',
                    'RoIs' : f'{name}RoIs',
                    'RoITool': CompFactory.ViewCreatorInitialROITool(),
                    'mergeUsingFeature': False}
            args.update(**inputMakerArgs)
            self.inputMakerAlg = CompFactory.InputMakerForRoI(**args)
                
    def addRecoSequence(self):
        if self.recoSeq is None:
            self.recoSeq = parOR( self.name )
            self.addSequence( self.recoSeq )

    def mergeReco( self, ca ):
        """ Merged CA moving reconstruction algorithms into the right sequence """ 
        self.addRecoSequence()       
        return self.merge( ca, sequenceName=self.recoSeq.name )

    def addRecoAlgo( self, algo ):
        """ Place algorithm in the correct reconstruction sequence """
        self.addRecoSequence()  
        return self.addEventAlgo( algo, sequenceName=self.recoSeq.name )

    def inputMaker( self ):
        return self.inputMakerAlg



class InViewRecoCA(ComponentAccumulator):
    """ Class to handle in-view reco, sets up the View maker if not provided and exposes InputMaker so that more inputs to it can be added in the process of assembling the menu """
    def __init__(self, name, viewMaker=None, isProbe=False, **viewMakerArgs):
        super( InViewRecoCA, self ).__init__()
        self.name = name +"_probe" if isProbe else name
        def updateHandle(baseTool, probeTool, handleName):
            if hasattr(baseTool, handleName) and getattr(baseTool, handleName).Path!="StoreGateSvc+":
                setattr(probeTool, handleName, getattr(probeTool, handleName).Path + "_probe")

        if len(viewMakerArgs) != 0:
            assert viewMaker is None, "No support for explicitly passed view maker and args for EventViewCreatorAlgorithm" 

        if viewMaker:
            assert len(viewMakerArgs) == 0, "No support for explicitly passed view maker and args for EventViewCreatorAlgorithm" 
            if isProbe:
                self.viewMakerAlg = viewMaker.__class__(viewMaker.getName()+'_probe', **viewMaker._properties)
                self.viewMakerAlg.Views = viewMaker.Views+'_probe'
                roiTool = self.viewMakerAlg.RoITool.__class.__(self.viewMakerAlg.RoITool.getName()+'_probe', **self.viewMakerAlg.RoITool._properties)
                updateHandle(viewMakerArgs['RoITool'], roiTool, "RoisWriteHandleKey")
                if hasattr(viewMakerArgs['RoITool'], "RoiCreator"):
                    updateHandle(viewMakerArgs['RoITool'], roiTool, "ExtraPrefetchRoIsKey")
                    updateHandle(viewMakerArgs['RoITool'].RoiCreator, roiTool.RoiCreator, "RoisWriteHandleKey")

                self.viewMakerAlg.RoITool = roiTool
            else:
                self.viewMakerAlg = viewMaker
        else:
            assert 'name' not in viewMakerArgs, "The name of view maker is predefined by the name of sequence"
            assert 'Views' not in viewMakerArgs, "The Views is predefined by the name of sequence"
            assert 'ViewsNodeName' not in viewMakerArgs, "The ViewsNodeName is predefined by the name of sequence"
            if 'RoITool' in viewMakerArgs:
                roiTool = viewMakerArgs['RoITool']
            else:
                roiTool = CompFactory.ViewCreatorInitialROITool()


            args = {'name': f'IM_{self.name}', 
                    'ViewFallThrough'   : True,
                    'RoIsLink'          : 'initialRoI',
                    'RoITool'           : roiTool,
                    'InViewRoIs'        : f'{name}RoIs',
                    'Views'             : f'{name}Views'+'_probe' if isProbe else f'{name}Views',
                    'ViewNodeName'      : f'{name}InViews'+'_probe' if isProbe else f'{name}InViews', 
                    'RequireParentView' : False,
                    'mergeUsingFeature' : False }
            args.update(**viewMakerArgs)
            self.viewMakerAlg = CompFactory.EventViewCreatorAlgorithm(**args)
            if isProbe:
                updateHandle(args['RoITool'], roiTool, "RoisWriteHandleKey")
                if hasattr(args['RoITool'], "RoiCreator"):
                    updateHandle(args['RoITool'], roiTool, "ExtraPrefetchRoIsKey")
                    updateHandle(args['RoITool'].RoiCreator, roiTool.RoiCreator, "RoisWriteHandleKey")
        self.viewsSeq = parOR( self.viewMakerAlg.ViewNodeName )
        self.addSequence( self.viewsSeq )

    def mergeReco( self, ca ):
        """ Merge CA moving reconstruction algorithms into the right sequence """
        return self.merge( ca, sequenceName=self.viewsSeq.name )


    def addRecoAlgo( self, algo ):
        """ Place algorithm in the correct reconstruction sequence """
        return self.addEventAlgo( algo, sequenceName=self.viewsSeq.name )


    def inputMaker( self ):
        return self.viewMakerAlg

class SelectionCA(ComponentAccumulator):
    """ CA component for MenuSequenceCA sequence """
    def __init__(self, name, isProbe=False):
        self.name = name+"_probe" if isProbe else name        
        self.isProbe=isProbe
        super( SelectionCA, self ).__init__()   

        self.stepViewSequence = seqAND(self.name)
        

    def mergeReco(self, recoCA, robPrefetchCA=None, upSequenceCA=None):        
        ''' upSequenceCA is the user CA to run before the recoCA'''
        ca=ComponentAccumulator()
        ca.addSequence(self.stepViewSequence)
        if upSequenceCA:
            ca.merge(upSequenceCA, sequenceName=self.stepViewSequence.name)
        ca.addEventAlgo(recoCA.inputMaker(), sequenceName=self.stepViewSequence.name)
        if robPrefetchCA:
            ca.merge(robPrefetchCA, self.stepViewSequence.name)
        ca.merge(recoCA, sequenceName=self.stepViewSequence.name)
        self.merge(ca)        
        

    def mergeHypo(self, other):
        """To be used when the hypo alg configuration comes with auxiliary tools/services"""
        self.merge(other, sequenceName=self.stepViewSequence.name)

    def addHypoAlgo(self, algo):
        """To be used when the hypo alg configuration does not require auxiliary tools/services"""        
        if self.isProbe:
            newname = algo.getName()+'_probe'
            algo.name=newname
           #algo.name(algo.getName()+'_probe')
        self.addEventAlgo(algo, sequenceName=self.stepViewSequence.name)

    def hypo(self):
        """Access hypo algo (or throws)"""
        h = findAlgorithmByPredicate(self.stepViewSequence, lambda alg: "HypoInputDecisions" in alg._descriptors ) # can't use isHypo
        assert h is not None, "No hypo in SeelectionCA {}".format(self.name)
        return h

    def inputMaker(self):
        """Access Input Maker (or throws)"""
        im = findAlgorithmByPredicate(self.stepViewSequence, lambda alg: "InputMakerInputDecisions" in alg._descriptors )
        assert im is not None, "No input maker in SeelectionCA {}".format(self.name)
        return im

    def topSequence(self):
        return self.stepViewSequence

# mainline/rec-ex-common and CA based JO compatibility layer (basically converters)
def algorithmCAToGlobalWrapper(gen, flags, *args, **kwargs):
    """ Merges CA with athena for all components except the algorithms. Those are converted to Run2 objects and returned.
        If CA contains more than one algorithm, a list is returned, else a single algorithm is returned.
    """
    with ConfigurableCABehavior():
        ca = gen(flags, *args, **kwargs)
        assert isinstance(ca, ComponentAccumulator), "Function provided does not generate ComponentAccumulator"
    return extractAlgorithmsAndAppendCA(ca)

def extractAlgorithmsAndAppendCA(ca: ComponentAccumulator) -> list[Configurable]:
    """Extract and return the algorithms from a component accumulator"""
    algorithms = ca.getEventAlgos()
    ca._algorithms.clear()
    ca._allSequences.clear()
    appendCAtoAthena(ca)
    return list(map(conf2toConfigurable, algorithms))


def menuSequenceCAToGlobalWrapper(gen, flags, *args, **kwargs):
    """
    Generates & converts MenuSequenceCA into the MenuSequence, in addition appending aux stuff to global configuration
    """
    with ConfigurableCABehavior():
        msca = gen(flags, *args, **kwargs)
        assert isinstance(msca, MenuSequenceCA), "Function provided to menuSequenceCAToGlobalWrapper does not generate MenuSequenceCA"
    return appendMenuSequenceCAToAthena(msca, flags)

def appendMenuSequenceCAToAthena(msca, flags):
    """
    Converts MenuSequenceCA into the MenuSequence, in addition appending aux stuff to global configuration.
    For use when MSCA generator function returns a tuple instead of bare MSCA
    """
    from AthenaCommon.AlgSequence import AthSequencer
    from AthenaCommon.CFElements import compName, isSequence
    hypo = conf2toConfigurable(msca.hypo.Alg)
    maker = conf2toConfigurable(msca.maker.Alg)

    def _convertSeq(s):
        sname = compName(s)
        old = AthSequencer( sname )
        if s.ModeOR: #this seems stupid way to do it but we avoid setting this property if is == default here, this is streamlining comparisons
            old.ModeOR = True
        if s.Sequential:
            old.Sequential = True
        old.StopOverride = s.StopOverride 
        for member in s.Members:
            if isSequence(member):
                old += _convertSeq(member)
            else:
                if member != msca.hypo.Alg: # removed hypo, as MenuSequence assembles it later
                    old += conf2toConfigurable(member)
        return old

    sequence = _convertSeq(msca.ca.topSequence())
    msca.ca._algorithms = {}
    msca.ca._sequence = None
    msca.ca._allSequences = []
    appendCAtoAthena(msca.ca)
    if msca.globalRecoCA:
        appendCAtoAthena(msca.globalRecoCA)

    return MenuSequence(flags,
                        Sequence   = sequence,
                        Maker       = maker,
                        Hypo        = hypo,
                        HypoToolGen = msca._hypoToolConf.hypoToolGen)


def lockConfigurable(conf):
    # Need to recurse through a few possibilities to ensure the
    # locking block only receives Configurables
    if isinstance(conf,Node): # Send along the alg from the node
        lockConfigurable(conf.Alg)
    elif isinstance(conf, collections.abc.Sequence): # Iterate over children
        for i in list(conf):
            if isinstance(i,Configurable):
                lockConfigurable(i)
    # Skip all other types
    if not isinstance(conf,Configurable):
        return

    log.debug('RecoFragmentsPool: Trying to lock %s',compName(conf))

    # Don't attempt this on components that the menu needs to configure
    skiplock = False
    if isInputMakerBase(conf) or isHypoAlg(conf) or isComboHypoAlg(conf):
        skiplock=True
    if skiplock:
        return

    try:
        # lock item if possible
        # except if it is a ComboHypo, as the menu needs to modify this
        # -- should not be configured in signature code
        log.debug("RecoFragmentsPool: Attempting to lock %s of type %s",compName(conf),conf.__class__)
        conf.lock()
    except AttributeError:
        log.info(f"RecoFragmentsPool: Unable to lock returned fragment {compName(conf)} of type {conf.__class__}")
    pass

class RecoFragmentsPool(object):
    """
    This used to be a custom cache implementation for the creator function,
    but now it is a fairly simple wrapper to apply @AccumulatorCache to creator.
    All methods are static, this class holds no state (instead AccumulatorCache does).
    """

    # Set RecoFragmentsPool.DebugCaching=True in top-level JO to enable debug messages.
    # This is very useful to study cache misses and find which ones can be avoided.
    DebugCaching = False

    @staticmethod
    def cacheableArgsDict(kwargsDict):
        """
        Sort the kwargs dictionary and replace all DataHandle objects in dict values with
        their string representation.

        For the purpose of RecoFragmentsPool we can assume DataHandle objects in kwargs
        are immutable and thus hashable by their string representation.
        """
        kwargsCopy = dict()
        for k,v in sorted(kwargsDict.items()):
            if (isinstance(v,DataHandle)):
                kwargsCopy[k] = str(v)
            else:
                kwargsCopy[k] = v
        return kwargsCopy

    @staticmethod
    def recursiveToTuple(obj):
        """Convert unhashable dict or list to a hashable tuple"""
        if isinstance(obj,list):
            new_list = [RecoFragmentsPool.recursiveToTuple(x) for x in obj]
            return tuple(new_list)
        if isinstance(obj,dict):
            new_dict = dict([(k,RecoFragmentsPool.recursiveToTuple(v)) for k,v in obj.items()])
            # Workaround for odd case where a passed Configurable is compared by pointer value
            conf_dict = {}
            for k,v in new_dict.items():
                if hasattr(v,'getFullName'):
                    conf_dict[k+'.__fullname__'] = v.getFullName()
            new_dict.update(conf_dict)
            return tuple(new_dict.items())
        # neither list or dict
        return obj

    @staticmethod
    def recursiveFromTuple(obj):
        """Reverse operation to recursiveToTuple"""
        if not isinstance(obj,tuple):
            return obj
        if all([isinstance(x,tuple) and len(x)==2 for x in obj]):
            # tuple consisting only of 2-element tuples is most likely a dict
            # Ignore the [something]__name__ keys used for workaround above
            return dict([(k,RecoFragmentsPool.recursiveFromTuple(v)) for k,v in obj if not k.endswith('.__fullname__')])
        # all other cases is a list
        return list([RecoFragmentsPool.recursiveFromTuple(x) for x in obj])

    @staticmethod
    def retrieve(creator, flags, **kwargs):
        if not isinstance(flags, AthConfigFlags):
            raise TypeError("RecoFragmentsPool: First argument for creator function passed to retrieve() must be of type AthConfigFlags but is %s" % type(flags))
        kwargs2 = RecoFragmentsPool.cacheableArgsDict(kwargs)
        return RecoFragmentsPool.retrieve_cacheable(creator, flags, RecoFragmentsPool.recursiveToTuple(kwargs2))

    @staticmethod
    @AccumulatorCache(maxSize=1024)
    def retrieve_cacheable(creator, flags, kwargsTuple):
        kwargs = RecoFragmentsPool.recursiveFromTuple(kwargsTuple)
        if RecoFragmentsPool.DebugCaching:
            fName = 'RecoFragmentsPool.retrieve_cacheable'
            log.debug('%s: creator = %s hash = %s', fName, creator, hash(creator))
            log.debug('%s: flags = %s hash = %s', fName, flags, flags.athHash())
            log.debug('%s: kwargs = %s hash = %s', fName, kwargs, hash(kwargsTuple))
        recoFragment = creator( flags, **kwargs )
        try:
            lockConfigurable(recoFragment)
        except Exception as e:
            log.error("RecoFragmentsPool: Failed to lock %s with exception %s", recoFragment, e)
            raise e
        return recoFragment


