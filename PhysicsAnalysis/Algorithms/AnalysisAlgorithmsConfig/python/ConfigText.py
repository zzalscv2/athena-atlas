# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# @author Joseph Lambert

# constant
ROOTNAME = 'root'

import yaml
import os
import inspect
from collections import namedtuple

from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

# class for config block information
node = namedtuple("ConfigBlock", ['alg', 'algName', 'options', 'defaults', 'subAlgs'])


def readYaml(yamlPath):
    """Loads YAML file into a dictionary"""
    if not os.path.isfile(yamlPath):
        raise ValueError(f"{yamlPath} is not a file.")
    with open(yamlPath, 'r') as f:
        textConfig = yaml.safe_load(f)
    return textConfig


def printYaml(d, sort=False, jsonFormat=False):
    """Prints a dictionary as YAML"""
    print(yaml.dump(d, default_flow_style=jsonFormat, sort_keys=sort))


def getDefaultArgs(func):
    """return dict(par, val) with all func parameters with defualt values"""
    signature = inspect.signature(func)
    return {
        k: v.default
        for k, v in signature.parameters.items()
        if v.default is not inspect.Parameter.empty
    }


def getFuncArgs(func):
    """return list of input parameters"""
    signature = inspect.signature(func)
    return list(signature.parameters.keys())


def getClassArgs(func):
    """return list of args used i=ton initialize class"""
    args = list(inspect.signature(func.__init__).parameters.keys())
    args.remove('self')
    return args



class TextConfig():
    """This class provides a configuration manager that is intended to allow the user to:
        - define and configure functions that return an algSequence(?) object
        - configure ConfigBlocks based on options stored in a YAML file.
    TODO:
        - check that there are no unused parameters in the configureation file.
        - allow order to be set when declaring a function
        - update block config to make getting information easier
    """
    def __init__(self, yamlPath=None):
        self._algs = {}
        self._order = {ROOTNAME: []}
        self._textConfig = {}
        if yamlPath is not None:
            self.loadConfig(yamlPath)


    def loadConfig(self, yamlPath):
        """read a YAML file in to a dictionary and return it"""
        if not os.path.isfile(yamlPath):
            raise ValueError(f"{yamlPath} is not a file")
        self._textConfig = readYaml(yamlPath)
        return 


    def printConfig(self, sort=False, jsonFormat=False):
        """Print YAML configuration file."""
        if self._textConfig is None:
            raise ValueError("No configuration has been loaded.")
        printYaml(self._textConfig, sort, jsonFormat)
        return


    def setConfig(self, config):
        """Print YAML configuration file."""
        if self._textConfig:
            # should this be an error?
            #raise ValueError("Configuration has already been loaded.")
            print("WARNING: Overwritting existing configuration.")
        # should check to make sure that config is type dict or OrderedDict
        self._textConfig = config
        return


    def addAlgConfigBlock(self, algName, alg, defaults=None, pos=None, superBlocks=None):
        """Add class to list of available algorithms"""
        if not callable(alg):
            raise ValueError(f"{algName} is not a callable.")
        if type(alg) == type(dict):
            opts = getClassArgs(alg)
        else:
            opts = getFuncArgs(alg)    

        if superBlocks is None:
            superBlocks = [ROOTNAME]
        elif type(superBlocks) != list:
            superBlocks = [superBlocks]

        # add new alg block to subAlgs dict of super block
        for block in superBlocks:
            if block not in self._order:
                self._order[block] = []
            order = self._order[block]
            
            if block == ROOTNAME:
                algs = self._algs
            else:
                if block not in self._algs:
                    raise ValueError(f"{block} not added")
                algs = self._algs[block].subAlgs

            if alg in algs:
                raise ValueError(f"{algName} has already been added.")

            # create node with alg information
            algs[algName] = node(
                alg=alg,
                algName=algName,
                options=opts,
                defaults=defaults,
                subAlgs={}
            )
            # insert into order (list)
            if pos is None:
                order.append(algName)
            elif pos in order:
                order.insert(order.index(pos), algName)
            else:
                raise ValueError(f"{pos} does not exit in already added config blocks")

        return


    def printAlgs(self, printOpts=False):
        """Prints algorithms exposed to configuration"""
        algs = self._algs
        for alg, algInfo in algs.items():
            algName = algInfo.alg.__name__
            algOptions = algInfo.options
            algDefaults = algInfo.defaults
            print(f"{alg} -> {algName}")
            if printOpts and algOptions:
                for opt in algOptions:
                    if algDefaults and opt in algDefaults:
                        print(f"    {opt}: {algDefaults[opt]}")
                    else:
                        print(f"    {opt}")
        return


    def configure(self):
        """Process YAML configuration file and confgure added algorithms."""
        def getOptions(configSeq):
            """get information on options for last block in sequence"""
            options = []
            for name, o in configSeq._blocks[-1].getOptions().items():
                val = getattr(configSeq._blocks[-1], name)
                valType = o.type
                valRequired = o.required 
                options.append({'name':name, 'defaultValue':val, 'type': valType, 
                                'required': valRequired})
            return options

        
        def setOptions(configSeq, options):
            """Set options for a ConfigBlock"""
            algOptions = getOptions(configSeq)
            for opt in algOptions:
                name = opt['name']
                valType = opt['type']
                valDefault = opt['defaultValue']
                valRequired = opt['required']

                if valRequired and name not in options:
                    raise ValueError(f'{name} is required but not included in config')

                if name in options:
                    opt_type = type(options[name])
                    # does not check type if expected type is None
                    if valType is not None and opt_type != valType:
                        raise ValueError(f'{name} should be of type {valType} not {opt_type}')
                    configSeq.setOptionValue (f'.{name}', options[name])
                    print(f"    {name}: {options[name]}")
                else:
                    # add default used to config
                    options[name] = valDefault
                    print(f"    {name}: {valDefault}")
            return algOptions


        def makeConfig(funcInfo, funcOptions):
            """
            Parameters
            ----------
            funcName: str
                name associated with the algorithm. This name must have been added to the 
                list of available algorithms
            funcOptions: dict
                dictionary containing options for the algorithm read from the YAML file

            Returns
            -------
                configSequence
            """
            configSeq = ConfigSequence()

            func = funcInfo.alg
            funcName = funcInfo.algName
            funcDefaults = getDefaultArgs(func)
            defaults = funcInfo.defaults

            args = {}
            # loop over all options for the function
            for arg in getFuncArgs(func):
                # supplied from config file
                if arg in funcOptions:
                    args[arg] = funcOptions[arg]
                # defaults set in function def
                elif arg in funcDefaults:
                    args[arg] = funcDefaults[arg]
                # defaults provided when func was added
                elif defaults is not None and arg in defaults:
                    args[arg] = defaults[arg]
                elif arg == 'seq':
                    # 'seq' should be first arg of func (not needed for class)
                    args[arg] = configSeq
                elif arg == 'kwargs':
                    # cannot handle arbitrary parameters
                    continue
                else:
                    raise ValueError(f"{arg} is requried for {funcName}")
            if type(func) == type(dict):
                configSeq.append(func(**args))
            else:
                func(**args)
            return configSeq, args.keys()
            

        def configureAlg(configSeq, block, blockConfig, containerName=None):
            if type(blockConfig) != list:
                blockConfig = [blockConfig]

            for options in blockConfig:
                # Special case: propogate containerName down to subAlgs
                if 'containerName' in options:
                    containerName = options['containerName']
                elif containerName is not None and 'containerName' not in options:
                    options['containerName'] = containerName
                # will check which options are associated alg and not options
                print(f"Configuring {block.algName}")
                seq, funcOpts = makeConfig(block, options)
                if not seq._blocks:
                    continue
                algOpts = setOptions(seq, options)
                configSeq += seq

                # check to see if there are unused parameters 
                algOpts = [i['name'] for i in algOpts]
                expectedOptions = set(funcOpts) 
                expectedOptions |= set(algOpts)
                expectedOptions |= set(block.subAlgs)

                difference = set(options.keys()) - expectedOptions
                if difference:
                    difference = "\n".join(difference)
                    raise ValueError(f"There are options set that are not used for "
                                     f"{block.algName}:\n{difference}\n"
                                     "Please check your configuration.")


                # check for sub-blocks and call this function recursively
                for alg in self._order.get(block.algName, []):
                    if alg in options:
                        subAlg = block.subAlgs[alg]
                        configureAlg(configSeq, subAlg, options[alg], containerName)
            return configSeq


        ### configure starts here ###
        # make sure all blocks in yaml file are added (otherwise they would be ignored)
        for blockName in self._textConfig:
            if blockName not in self._order[ROOTNAME]:
                raise ValueError(f"Unkown block {blockName} in yaml file")

        configSeq = ConfigSequence()
        for blockName in self._order[ROOTNAME]:
            # consistency check - should never happen
            if blockName not in self._algs:
                raise ValueError(f"{blockName} not added")
            # order only applies to root blocks
            if blockName in self._textConfig:
                blockConfig = self._textConfig[blockName]
                alg = self._algs[blockName]
                configureAlg(configSeq, alg, blockConfig)
            else:
                continue
        return configSeq



def addDefaultAlgs(config, dataType, isPhyslite, noPhysliteBroken, noSystematics):
    """add algorithms and options"""

    # CommonServices
    from AsgAnalysisAlgorithms.AsgAnalysisConfig import makeCommonServicesConfig
    def CommonServices(seq, noSystematics):
        makeCommonServicesConfig(seq)
        seq.setOptionValue ('.runSystematics', not noSystematics)
        return seq
    config.addAlgConfigBlock(algName="CommonServices", alg=CommonServices,
        defaults={'noSystematics': noSystematics})

    # pileup
    from AsgAnalysisAlgorithms.AsgAnalysisAlgorithmsTest import pileupConfigFiles
    from AsgAnalysisAlgorithms.AsgAnalysisConfig import makePileupReweightingConfig
    def makePileupConfig(seq, dataType, isPhyslite):
        if isPhyslite :
            return
        campaign, files, prwfiles, lumicalcfiles = None, None, None, None
        useDefaultConfig = False
        # need to allow for conversion of dataType from string to enum for the call to pileupConfigFiles
        prwfiles, lumicalcfiles = pileupConfigFiles(  {'data': DataType.Data, 'mc': DataType.FullSim, 'afii': DataType.FastSim}.get(dataType, dataType) )

        makePileupReweightingConfig(seq)
        seq.setOptionValue ('.campaign', campaign, noneAction='ignore')
        seq.setOptionValue ('.files', files, noneAction='ignore')
        seq.setOptionValue ('.useDefaultConfig', useDefaultConfig)
        seq.setOptionValue ('.userPileupConfigs', prwfiles, noneAction='ignore')
        seq.setOptionValue ('.userLumicalcFiles', lumicalcfiles, noneAction='ignore')
        return seq
    config.addAlgConfigBlock(algName="PileupReweighting", alg=makePileupConfig,
        defaults={'dataType':dataType,'isPhyslite':isPhyslite})

    # event cleaning
    from AsgAnalysisAlgorithms.EventCleaningConfig import EventCleaningBlock
    config.addAlgConfigBlock(algName="EventCleaning", alg=EventCleaningBlock)

    # jets
    from JetAnalysisAlgorithms.JetAnalysisConfig import makeJetAnalysisConfig
    config.addAlgConfigBlock(algName="Jets", alg=makeJetAnalysisConfig)
    from JetAnalysisAlgorithms.JetJvtAnalysisConfig import JetJvtAnalysisConfig
    config.addAlgConfigBlock(algName="JVT", alg=JetJvtAnalysisConfig,
        superBlocks="Jets")
    from FTagAnalysisAlgorithms.FTagAnalysisConfig import makeFTagAnalysisConfig
    config.addAlgConfigBlock(algName="FlavourTagging", alg=makeFTagAnalysisConfig,
        defaults={'selectionName': ''},
        superBlocks="Jets")

    # electrons
    from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import ElectronCalibrationConfig 
    config.addAlgConfigBlock(algName="Electrons", alg=ElectronCalibrationConfig)
    from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import ElectronWorkingPointConfig
    config.addAlgConfigBlock(algName="WorkingPoint", alg=ElectronWorkingPointConfig,
        superBlocks="Electrons")

    # photons
    from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import PhotonCalibrationConfig
    config.addAlgConfigBlock(algName="Photons", alg=PhotonCalibrationConfig)
    from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import PhotonWorkingPointConfig
    config.addAlgConfigBlock(algName="WorkingPoint", alg=PhotonWorkingPointConfig,
        superBlocks="Photons")

    # muons
    from MuonAnalysisAlgorithms.MuonAnalysisConfig import MuonCalibrationConfig
    config.addAlgConfigBlock(algName="Muons", alg=MuonCalibrationConfig)
    from MuonAnalysisAlgorithms.MuonAnalysisConfig import MuonWorkingPointConfig
    config.addAlgConfigBlock(algName="WorkingPoint", alg=MuonWorkingPointConfig,
        superBlocks="Muons")

    # tauJets
    from TauAnalysisAlgorithms.TauAnalysisConfig import TauCalibrationConfig
    config.addAlgConfigBlock(algName="TauJets", alg=TauCalibrationConfig)
    from TauAnalysisAlgorithms.TauAnalysisConfig import TauWorkingPointConfig
    config.addAlgConfigBlock(algName="WorkingPoint", alg=TauWorkingPointConfig,
        superBlocks="TauJets")

    # generator level analysis
    from AsgAnalysisAlgorithms.AsgAnalysisConfig import makeGeneratorAnalysisConfig 
    config.addAlgConfigBlock(algName="GeneratorLevelAnalysis", alg=makeGeneratorAnalysisConfig)

    # pT/Eta Selection
    from AsgAnalysisAlgorithms.AsgAnalysisConfig import makePtEtaSelectionConfig
    config.addAlgConfigBlock(algName="PtEtaSelection", alg=makePtEtaSelectionConfig,
        defaults={'selectionName': ''},
        superBlocks=["Jets", "Electrons", "Photons", "Muons", "TauJets"])

    # met
    from MetAnalysisAlgorithms.MetAnalysisConfig import MetAnalysisConfig
    config.addAlgConfigBlock(algName="MissingET", alg=MetAnalysisConfig)

    # overlap removal
    from AsgAnalysisAlgorithms.OverlapAnalysisConfig import OverlapAnalysisConfig
    config.addAlgConfigBlock(algName="OverlapRemoval", alg=OverlapAnalysisConfig,
        defaults={'configName': 'OverlapRemoval'})

    # object-based cutflow
    from AsgAnalysisAlgorithms.AsgAnalysisConfig import ObjectCutFlowBlock
    config.addAlgConfigBlock(algName='ObjectCutFlow', alg=ObjectCutFlowBlock)

    # thinning
    from AsgAnalysisAlgorithms.AsgAnalysisConfig import OutputThinningBlock
    config.addAlgConfigBlock(algName="Thinning", alg=OutputThinningBlock,
        defaults={'configName': 'Thinning'})

    # trigger
    from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import TriggerAnalysisBlock
    config.addAlgConfigBlock(algName="Trigger", alg=TriggerAnalysisBlock,
        defaults={'configName': 'Trigger'})

    # event selection
    from EventSelectionAlgorithms.EventSelectionConfig import EventSelectionConfig, EventSelectionMergerConfig
    config.addAlgConfigBlock(algName='EventSelection', alg=EventSelectionConfig)
    config.addAlgConfigBlock(algName='EventSelectionMerger', alg=EventSelectionMergerConfig)

    # event-based cutflow
    from AsgAnalysisAlgorithms.AsgAnalysisConfig import EventCutFlowBlock
    config.addAlgConfigBlock(algName='EventCutFlow', alg=EventCutFlowBlock,
        defaults={'containerName': 'EventInfo', 'selectionName': ''})

    # bootstraps
    from AsgAnalysisAlgorithms.BootstrapGeneratorConfig import BootstrapGeneratorConfig
    config.addAlgConfigBlock(algName='Bootstraps', alg=BootstrapGeneratorConfig)

    # output
    from AsgAnalysisAlgorithms.OutputAnalysisConfig import OutputAnalysisConfig
    config.addAlgConfigBlock(algName="Output", alg=OutputAnalysisConfig,
        defaults={'configName': 'Output'})

    return


def printOptions(configSeq):
    """
    Prints options and their values for each config block in a config sequence
    """
    for config in configSeq:
        print(config)
        try:
            options = [(opt, getattr(config, opt)) for opt in config._options]
            for k, v in options:
                print(f"    {k}: {v}")
        except Exception as e:
            print(e)

def makeSequence(configPath, dataType, algSeq,
                 isPhyslite=False, noPhysliteBroken=False, noSystematics=False):
    """
    """
    print(os.getcwd())

    from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator

    config = TextConfig(configPath)

    print(">>> Configuration file read in:")
    config.printConfig()

    print(">>> Adding default algorithms")
    addDefaultAlgs(config, dataType, isPhyslite, noPhysliteBroken, noSystematics)
    config.printAlgs(printOpts=True)

    print(">>> Configuring algorithms based on YAML file")
    configSeq = config.configure()

    # defaults are added to config as algs are configured
    print(">>> Configuration used:")
    config.printConfig()

    print(">>> ConfigBlocks and their configuration")
    printOptions(configSeq)

    # compile
    configAccumulator = ConfigAccumulator(dataType, algSeq, isPhyslite=isPhyslite)
    configSeq.fullConfigure(configAccumulator)

    from AnaAlgorithm.DualUseConfig import isAthena, useComponentAccumulator
    if isAthena and useComponentAccumulator:
        return configAccumulator.CA
    else:
        return None
