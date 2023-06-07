# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

import re
from importlib import import_module
from collections import defaultdict as ddict
from itertools import chain

from AthenaCommon.Logging import logging

from .Base.L1MenuFlags import L1MenuFlags
from .Base.Limits import Limits
from .Base.Boards import BoardType
from .Base.L1Menu import L1Menu
from .Base.Thresholds import TopoThreshold
from .Base.TopoAlgorithms import AlgCategory
from .Base.L1Menu2JSON import L1MenuJSONConverter
from .Config.TriggerTypeDef import TT
from .Config.TopoAlgoDefMultiplicity import TopoAlgoDefMultiplicity 
from .Config.ItemDef import ItemDef

"""
L1MenuConfig is responsible for building the L1 Menu

Building a menu happens in two stages

1) Configuration objects are defined in the directory TriggerMenuMT/Config/ in *Def.py files
  * Items are definded in Config/ItemDef.py
  * Topo algorithms are define in files TopoAlgoDef*.py, with separate files for
    - multiplicity counting algorithms on the Run 3 topo boards 
    - topological selection algorithms on the Run 3 topo boards
    - topological selection algorithms on the MUCTPI board
    - topological selection algorithms on the Run 2 topo boards (legacy boards)
  * Thresholds are defined in files ThresholdDef.py
"""

log = logging.getLogger(__name__)

class L1MenuConfig(object):

    def __init__(self, menuName, inputFile = None, do_alfa = False ):

        L1MenuFlags.MenuSetup = menuName

        self.menuFullName    = L1MenuFlags.MenuSetup()
        self.menuFilesToLoad = self._menuToLoad()
        self.menuName        = self.menuFilesToLoad[0]
        self.inputFile       = inputFile
        self.l1menuFromFile  = (self.inputFile is not None)
        self.generated       = False

        # all registered items
        self.registeredItems = {}
        
        # all registered thresholds
        self._registeredThresholds = {}
        self._registeredThresholdsStats = { "muon": 0, "calo": 0, "nim": 0, "legacy": 0, 
                                         AlgCategory.TOPO : 0, AlgCategory.MUCTPI : 0, AlgCategory.LEGACY : 0 }

        # all registered topo algos
        self._registeredTopoAlgos = {}
        for cat in AlgCategory.getAllCategories():
            self._registeredTopoAlgos[cat] = {}

        # menu
        L1MenuFlags.CTPVersion = 4 # this needs to be done here already, since L1Menu depends on it during init
        self.l1menu = L1Menu(self.menuName, do_alfa)
        self.l1menu.setBunchGroupSplitting() # store bunchgroups separate from other item inputs

        if not self._checkMenuExistence():
            log.error("Generating L1 menu %s is not possible", self.menuName)
        else:
            log.info("=== Generating L1 menu %s ===", self.menuName)
            self._generate()

    def _generate(self):

        log.info("=== Reading definition of algorithms, thresholds, and items ===")

        self._registerDefinedConfigurationObjects() # defines and registers all items, thresholds, ...

        log.info("=== Reading definition of menu ===")

        # Build menu
        self._importMenuDefinition() # puts thresholds and items names for the menu into the LVL1Flags

        self._extendInformation()

        self._generateTopoMenu()

        self._generateMenu()

        self.generated = True
        

    def thresholdExists(self,thrName):
        return thrName in self._registeredThresholds

    def getDefinedThreshold(self, name):
        try:
            return self._registeredThresholds[name]
        except KeyError as ex:
            if name.startswith("R2TOPO"):
                log.error("No legacy topo output %s is registered. Make sure it is defined in L1/Config/TopoAlgoDefLegacy.py", name)
            elif name.startswith("TOPO"):
                log.error("No topo output %s is registered. Make sure it is defined in L1/Config/TopoAlgoDef.py", name)
            else:
                isLegacy = any(filter(lambda x: name.startswith(x), ["EM", "TAU", "J", "XE", "TE", "XS"]))
                log.error("No threshold %s is registered. Make sure it is defined in L1/Config/ThresholdDef%s.py", name,"Legacy" if isLegacy else "")
            raise ex

    def getDefinedThresholds(self):
        return self._registeredThresholds.values()



    ###
    # registration of available components
    ###

    ## topo algorithms
    def registerTopoAlgo(self, algo):
        """ Add a L1Topo algo to the set of algos which are registered for further use"""

        if self.currentAlgoDef == AlgCategory.MULTI:
            algo.name = "Mult_" + algo.name

        if algo.name in self._registeredTopoAlgos[self.currentAlgoDef]:
            raise RuntimeError('%s algo %s is already registered as such' % (self.currentAlgoDef.desc, algo.name))
        self._registeredTopoAlgos[self.currentAlgoDef][algo.name] = algo
        log.debug("Added in the %s type the algo: %s", self.currentAlgoDef.desc, algo.name)

        return algo



    def registerThreshold(self, thr):
        """
        Add externally created L1 threshold to the list of available thresholds.
        """
        if thr.run!=0:
            self._addThrToRegistry(thr)
        else:
            raise RuntimeError("For threshold %s the run (=2 or 3) was not set!" % thr.name)
        return thr

    def _addThrToRegistry(self, thr):
        if self.thresholdExists(thr.name):
            raise RuntimeError("LVL1 threshold of name '%s' already defined, need to abort" % thr.name)

        self._registeredThresholds[thr.name] = thr

        # collect stats
        isTopo = isinstance(thr, TopoThreshold)
        if isTopo:
            self._registeredThresholdsStats[thr.algCategory] += 1
            return

        isNIM = thr.ttype in ['NIM','CALREQ','MBTSSI', 'MBTS', 'LUCID', 'BCM', 'BCMCMB', 'ZDC', 'ALFA', 'BPTX', 'ZB']
        if thr.isLegacy():
            self._registeredThresholdsStats["legacy"] += 1
        elif isNIM:
            self._registeredThresholdsStats["nim"] += 1
        elif thr.ttype == 'MU':
            self._registeredThresholdsStats["muon"] += 1
        else:
            self._registeredThresholdsStats["calo"] += 1




    def _registerTopoOutputsAsThresholds(self):
        """
        Add all L1Topo triggers that are part of the menu as allowed input to the menu
        """

        _topoTriggers = {}

        # for all topo algorithm categories the outputs (sometimes multiple) are defined as thresholds
        for cat in AlgCategory.getAllCategories():
            outputLines = []
            for algo in self._registeredTopoAlgos[cat].values():
                outputLines += algo.outputs if (type(algo.outputs) == list) else [ algo.outputs ]
            _topoTriggers[cat] = sorted(outputLines)
            log.info("... found %i topo triggerlines (source: %s)", len(_topoTriggers[cat]), cat )
            log.debug("%r", _topoTriggers[cat])


        multibitPattern = re.compile(r"(?P<line>.*)\[(?P<bit>\d+)\]")
        for cat in [AlgCategory.TOPO, AlgCategory.MUCTPI, AlgCategory.LEGACY]:
            multibitTopoTriggers = set()
            for topoLineName in _topoTriggers[cat]:
                m = multibitPattern.match(topoLineName) # tries to match "trigger[bit]"
                if m:
                    topoThrName = cat.prefix + m.groupdict()['line']
                    multibitTopoTriggers.add( topoThrName )
                else:
                    topoThrName = cat.prefix + topoLineName
                    TopoThreshold( name = topoThrName, algCategory = cat )
            # create thresholds from topo-multibit 
            for topoThrName in multibitTopoTriggers:  # ( 'MULT-CMU4ab', ...)
                TopoThreshold( name = topoThrName, algCategory = cat )



    ## Items
    def registerItem(self, name, item):
        """ Adds a LVL1 item to the set of items which are registered for further use"""
        if name in self.registeredItems:
            log.error('LVL1 item %s is already registered with ctpid=%d',
                      name, int(self.registeredItems[name].ctpid))
        else:
            self.registeredItems[name] = item

    def getRegisteredItem(self, name):
        if name in self.registeredItems:
            return self.registeredItems[name]
        return None


    def writeJSON(self, outputFile, bgsOutputFile = None, destdir="./"):
        if self.generated:
            outputFile = destdir.rstrip('/') + '/' + outputFile
            if bgsOutputFile is not None:
                bgsOutputFile = destdir.rstrip('/') + '/' + bgsOutputFile
            L1MenuJSONConverter(l1menu = self.l1menu, outputFile = outputFile, 
                                bgsOutputFile = bgsOutputFile).writeJSON(pretty=True)
            return outputFile, bgsOutputFile
        else:
            log.error("No menu was generated, can not create json file")
            return None

    def _menuToLoad(self, silent=False):
        """ resolve the menu name to the menu files to load"""
        menuToLoadReq = self.menuFullName
        # Extract the menu name, independent of menu prescale sets
        if menuToLoadReq.endswith('prescale'):
            menuToLoadReq = re.match(r'\w*_v\d*',self.menuFullName).group(0)
            log.info(f'Base menu name {menuToLoadReq} extracted from {self.menuFullName}')
        from .Menu.MenuMapping import menuMap
        if menuToLoadReq in menuMap:
            menuToLoad = menuMap[menuToLoadReq]
            if not silent:
                log.info("Menu %s was requested, but will load %s as specified in TriggerMenuMT.L1.Menu.menuMap", menuToLoadReq, menuToLoad[0])
        else:
            menuToLoad = [menuToLoadReq,str(menuToLoadReq)+"_inputs",str(menuToLoadReq)+"_inputs_legacy"]
        return menuToLoad

    def _checkMenuExistence(self):
        from PyUtils.moduleExists import moduleExists
        modname = 'TriggerMenuMT.L1.Menu.Menu_%s' % self.menuFilesToLoad[0]
        if not moduleExists (modname):
            log.error("No L1 menu available for %s, module %s does not exist", self.menuName, modname )
            return False

        return True


    def _importMenuDefinition(self):
        """
        Defines the list if item and threshold names that will be in the menu
        Calls defineMenu() of the correct module 'Menu_<menuname>.py'
        Menu.defineMenu() defines the menu via L1MenuFlags "items", "thresholds", 
        """

        # we apply a hack here. menu group is working on Dev_pp_run3_v1, until ready we will use MC_pp_run3_v1
        log.info("Reading TriggerMenuMT.Menu.Menu_%s", self.menuFilesToLoad[0])
        menumodule = __import__('TriggerMenuMT.L1.Menu.Menu_%s' % self.menuFilesToLoad[0], globals(), locals(), ['defineMenu'], 0)
        menumodule.defineMenu()
        log.info("... L1 menu '%s' contains %i items", self.menuFilesToLoad[0], len(L1MenuFlags.items()))

        log.info("Reading TriggerMenuMT.Menu.Menu_%s", self.menuFilesToLoad[1])
        topomenumodule = __import__('TriggerMenuMT.L1.Menu.Menu_%s' % self.menuFilesToLoad[1], globals(), locals(), ['defineMenu'], 0)
        topomenumodule.defineInputsMenu() # this adds the inputs definition (boards) to L1MenuFlags.boards
        connectorCount = 0
        algoCount = 0
        for boardName, boardDef in L1MenuFlags.boards().items():
            if "connectors" in boardDef:
                connectorCount += len(boardDef["connectors"])
                for c in boardDef["connectors"]:
                    if "thresholds" in c:
                        algoCount += len(c["thresholds"])
                    elif "algorithmGroups" in c:
                        for t in c["algorithmGroups"]:
                            algoCount += len(t["algorithms"])
                    else:
                        for t in c["signalGroups"]:
                            algoCount += len(t["signals"])
        log.info("... L1Topo menu '%s' contains %i boards (%s)", self.menuFilesToLoad[0], len(L1MenuFlags.boards()), ', '.join(L1MenuFlags.boards().keys()))
        log.info("    with %i connectors and %i input signals", connectorCount, algoCount)

        try:
            log.info("Reading TriggerMenuMT.Menu.Menu_%s", self.menuFilesToLoad[2])
            legacymenumodule = __import__('TriggerMenuMT.L1.Menu.Menu_%s' % self.menuFilesToLoad[2], globals(), locals(), ['defineMenu'], 0)
            legacymenumodule.defineLegacyInputsMenu()
            log.info("... L1 legacy menu %s contains %i legacy boards (%s)", self.menuFilesToLoad[2], len(L1MenuFlags.legacyBoards()), ', '.join(L1MenuFlags.legacyBoards().keys()))
        except ImportError as ie:
            if ie.name == 'TriggerMenuMT.L1.Menu.Menu_%s' % self.menuFilesToLoad[2]:
                log.info("==> No menu defining the legacy inputs was found, will assume this intended. %s %s %s",
                         ie.msg, ie.name, ie.path)
            else:
                raise

    def _registerDefinedConfigurationObjects(self):
        """
        Registers the list if items and thresholds that could be used in the menu
        Calls registerItem() of the correct module 'ItemDef.py'
        """
        from .Base.Items import MenuItem
        MenuItem.setMenuConfig(self) # from now on all newly created MenuItems are automatically registered here
        from .Base.Thresholds import Threshold
        Threshold.setMenuConfig(self) # from now on all newly created Thresholds definitions are automatically registered here

        # register Topo algorithms
        for algCat in [AlgCategory.TOPO, AlgCategory.MUCTPI, AlgCategory.MULTI, AlgCategory.LEGACY]:
            self.currentAlgoDef = algCat
            defFile = "TriggerMenuMT.L1.Config.%s" % self.currentAlgoDef.defFile
            log.info("Reading %s", defFile)
            import_module(defFile).__getattribute__(self.currentAlgoDef.defFile).registerTopoAlgos(self)
            log.info("... registered %i defined topo algos for the %s board(s)", len(self._registeredTopoAlgos[self.currentAlgoDef]), self.currentAlgoDef.desc)

        log.info("Reading TriggerMenuMT.Config.ThreholdDef")
        from .Config.ThresholdDef import ThresholdDef
        ThresholdDef.registerThresholds(self, self.menuFullName)
        log.info("... registered %i calo thresholds", self._registeredThresholdsStats["calo"])
        log.info("... registered %i muon thresholds", self._registeredThresholdsStats["muon"])
        log.info("... registered %i nim thresholds", self._registeredThresholdsStats["nim"])

        log.info("Reading TriggerMenuMT.Config.ThreholdDefLegacy")
        from .Config.ThresholdDefLegacy import ThresholdDefLegacy
        ThresholdDefLegacy.registerThresholds(self, self.menuFullName)
        log.info("... registered %i legacy calo thresholds", self._registeredThresholdsStats["legacy"])

        log.info("Turning topo algo outputs into thresholds (except multiplicity counters)")
        self._registerTopoOutputsAsThresholds()
        log.info("... registered %i topo thresholds", self._registeredThresholdsStats[AlgCategory.TOPO])
        log.info("... registered %i muctpi topo thresholds", self._registeredThresholdsStats[AlgCategory.MUCTPI])
        log.info("... registered %i legacy topo thresholds", self._registeredThresholdsStats[AlgCategory.LEGACY])

        log.info("Reading TriggerMenuMT.Config.ItemDef")
        ItemDef.registerItems(self, self.menuFullName)
        log.info("... registered %i defined items", len(self.registeredItems))


    def _getTopoAlgo(self, algoName, category):
        if algoName in self._registeredTopoAlgos[category]:
            return self._registeredTopoAlgos[category][algoName]
        msg = "Algorithm of name %s in category %s is not registered. Please add the algorithm to L1/Config/%s.py" % (algoName, category, category.defFile)
        log.error(msg)
        log.info("Available algorithms in this category are %s", ",".join(self._registeredTopoAlgos[category].keys()))
        raise RuntimeError(msg)


    def _getSortingAlgoThatProvides(self, input, topoAlgCategory):
        """
        returns a list of all sorting algorithms that are needed to
        produce the required output. A missing input will raise a
        runtime exception
        """
        sortingAlgs = []
        for name, alg in self._registeredTopoAlgos[topoAlgCategory].items():
            if type(alg.outputs)==list:
                foundOutput = (input in alg.outputs)
            else:
                foundOutput = (input == alg.outputs)
            if foundOutput:
                sortingAlgs += [alg]

        if len(sortingAlgs)==0:
            msg = "No topo sorting algorithm is providing this output: %s. Please add the sorting algorithm to L1/Config/%s.py" % (input, topoAlgCategory.defFile)
            raise RuntimeError(msg)
        if len(sortingAlgs)>1:
            msg = "More than one sorting algorithm is providing this output: %s. Here the list: %s" % (input, ', '.join(sortingAlgs))
            raise RuntimeError(msg)

        return sortingAlgs[0]

 
    def _extendInformation(self):
        """
        this function is called first after the menu definition is imported
        it can be used to update some of the information or to perform initial checks
        """
        allBoards = (list(L1MenuFlags.boards().items()) + list(L1MenuFlags.legacyBoards().items()))
        # set boardName in each connector
        for (boardName, boardDef) in allBoards:
            boardDef["name"] = boardName
            if "connectors" in boardDef:
                for connDef in boardDef["connectors"]:
                    connDef["board"] = boardName
            else:
                for connDef in boardDef["inputConnectors"]:
                    connDef["board"] = boardName
        

    def _generateTopoMenu(self):

        allBoardsWithTopo = list( filter (
            lambda b : BoardType.fromBoardName(b[0]) in [BoardType.TOPO, BoardType.MUCTPI], 
            chain(L1MenuFlags.boards().items(), L1MenuFlags.legacyBoards().items())
        ))
                
        #
        # Add the topo thresholds to the menu
        #
        nAlgos = 0
        nLines = 0
        for (boardName, boardDef) in allBoardsWithTopo:
            currentTopoCategory = AlgCategory.getCategoryFromBoardName(boardName)
            for connDef in boardDef["connectors"]:
                if connDef["format"] == 'multiplicity': # multiplicity algorithms don't define thresholds
                    continue
                for algGrp in connDef["algorithmGroups"]:
                    for topodef in algGrp["algorithms"]:
                        nAlgos += 1
                        nLines += len(topodef.outputlines)
                        # this is identical to outputlines except that ["MULT-CMU4ab[0]", "MULT-CMU4ab[1]"] becomes [ "MULT-CMU4ab" ]
                        outputlines = list(dict.fromkeys(map(lambda x: re.split(r"\[\d\]",x)[0], topodef.outputlines)))  
                        for lineName in outputlines:
                            thrName = currentTopoCategory.prefix + lineName
                            thr = self.getDefinedThreshold(thrName) # threshold has to be defined
                            if thr is None:
                                if 'legacy' in boardDef:       
                                    msg = 'Threshold %s is required for board %s, connector %s (file L1/Menu/Menu_%s.py), but it is not registered. ' % (thrName, boardName, connDef['name'], self.menuFilesToLoad[2])
                                    msg += 'Please add L1Topo alg with output %s to L1/Config/TopoAlgoDefLegacy.py.' % (thrName.split('_',1)[-1])
                                else:
                                    msg = 'Threshold %s is required for board %s, connector %s (file L1/Menu/Menu_%s.py), but it is not registered. ' % (thrName, boardName, connDef['name'], self.menuFilesToLoad[1])
                                    msg += 'Please add L1Topo alg with output %s to L1/Config/TopoAlgoDef.py.' % (thrName.split('_',1)[-1])
                                log.error(msg)
                                raise RuntimeError(msg)
                            else:
                                self.l1menu.addThreshold( thr )
        log.info("Generating topo menu using %i topo algorithms with %i trigger lines", nAlgos, nLines )

        #
        # Add the topo algorithms to the menu
        #

        # collect sorting algorithms from all decision algorithms (their inputs)
        # they need to be kept separated by source at this point
        allRequiredSortedInputs = {
            AlgCategory.TOPO : set(),
            AlgCategory.MUCTPI : set(), # TODO: think about MUCTP sorting input 
            AlgCategory.LEGACY : set()
        }

        # loop over all topo boards and their connectors
        for (boardName, boardDef) in allBoardsWithTopo:
            for connDef in boardDef["connectors"]:
                if ('muctpi' in boardName.lower()) and (connDef["format"]=='multiplicity'):
                    # muctpi doesn't need multiplicity algorithms defined
                    continue

                currentTopoCategory = AlgCategory.getCategoryFromBoardName(boardName)
                if currentTopoCategory == AlgCategory.TOPO and connDef["format"] == 'multiplicity':
                    currentTopoCategory = AlgCategory.MULTI
                algoNames = []
                algoNbits = []
                fpgaNames = []
                if connDef["format"] == 'multiplicity':
                    for thrName in connDef["thresholds"]:
                        if thrName is None:
                            continue
                        nBits = connDef["nbitsDefault"]
                        if type(thrName)==tuple:
                            (thrName,nBits) = thrName
                        if thrName is None:
                            continue                        
                        algoname = "Mult_" + thrName
                        algoNames += [ algoname ]
                        algoNbits += [ int(nBits) ]
                        fpgaNames += ['']
                elif connDef["format"] == 'topological':
                    for algGrp in connDef["algorithmGroups"]:
                        for topodef in algGrp["algorithms"]:
                            algoNames += [ topodef.algoname ]
                            algoNbits += [ -1 ]
                            fpgaNames += [str(algGrp['fpga'])]
                for algoName, algoBits, fpgaName in zip(algoNames, algoNbits, fpgaNames):
                    algo = self._getTopoAlgo(algoName, currentTopoCategory)
                    # check that the output bits of the multiplicity topo algorithms are as many as the output bits of the associated thresholds
                    if algoBits>0:
                        if algoBits != algo.nbits:
                            msg = "Algorithm %s defined with %i bits, but the associated threshold has %i bits " % (algo.name, algo.nbits, algoBits)
                            raise RuntimeError(msg)
                    self.l1menu.checkBoardInputs(algo, connDef["name"], fpgaName)
                    # add the decision algorithms to the menu
                    self.l1menu.addTopoAlgo( algo, category = currentTopoCategory )

                    # remember the inputs
                    if algo.isDecisionAlg():
                        allRequiredSortedInputs[currentTopoCategory].update( algo.inputs )

        # now also add the sorting algorithms to the menu
        for cat in allRequiredSortedInputs:
            for input in allRequiredSortedInputs[cat]:
                searchCat = cat
                sortingAlgo = self._getSortingAlgoThatProvides(input, searchCat)
                self.l1menu.addTopoAlgo( sortingAlgo, category = cat )



    def _generateMenu(self):

        if len(self.l1menu.items) > 0:
            log.info("L1MenuConfig.generate() has already been called. Will ignore")
            return

        """
        Generates the menu structure from the list of item and threshold names in the L1MenuFlags
        """

        # ------------------
        # Bunchgroups
        # ------------------
        from .Base.BunchGroupSet import createDefaultBunchGroupSet
        self.l1menu.ctp.bunchGroupSet = createDefaultBunchGroupSet()


        # ------------------
        # Thresholds
        # ------------------

        allBoards = (list(L1MenuFlags.boards().items()) + list(L1MenuFlags.legacyBoards().items()))
        
        if 'AllCTPIn' in self.menuName:
            # Only after we know all boards are defined, in the special case
            # that we are running the dummy L1 menu for CTP input configuration
            # we need to register more items that access every input
            ItemDef.registerItems_AllCTPIn(self)

        for (boardName, boardDef) in L1MenuFlags.legacyBoards().items():
            for connDef in boardDef["connectors"]:
                # Verify number and ordering of thresholds on L1Calo boards
                if connDef["type"] == "ctpin" and connDef["format"] == "multiplicity":
                    self.l1menu.checkL1CaloThresholds(connDef["thresholds"], boardName, connDef["name"])

        list_of_undefined_thresholds = []
        # new thresholds
        for (boardName, boardDef) in L1MenuFlags.boards().items():
            for connDef in boardDef["connectors"]:
                if connDef["type"] == "ctpin" or connDef["format"] != "multiplicity":
                    continue
                for thrName in connDef["thresholds"]:
                    if type(thrName) == tuple:
                        (thrName, _) = thrName
                    if (thrName is None) or (thrName in self.l1menu.thresholds):
                        continue
                    threshold = self.getDefinedThreshold(thrName)
                    if threshold is None:
                        log.error('Threshold %s is required in menu on board %s, connector %s, but it is not defined', thrName, boardName, connDef['name'] )
                        list_of_undefined_thresholds += [ thrName ]
                    else:
                        self.l1menu.addThreshold( threshold )
                try:
                    zbThrName = connDef["zeroBias"]
                    zbThr = self.getDefinedThreshold(zbThrName)
                    if zbThr is None:
                        log.error('Zero bias threshold %s is listed in menu but not defined', zbThrName )
                        list_of_undefined_thresholds += [ zbThrName ]
                    else:
                        self.l1menu.addThreshold( zbThr )
                except KeyError:
                    pass


        # signals from merger boards like AlfaCtpin
        for (boardName, boardDef) in L1MenuFlags.boards().items():
            for connDef in boardDef["connectors"]:
                if connDef["format"] != "simple":
                    continue
                for sGrp in connDef["signalGroups"]:
                    for thrName in sGrp["signals"]:
                        if type(thrName) == tuple:
                            (thrName, _) = thrName
                        if thrName is None or thrName in self.l1menu.thresholds:
                            continue
                        threshold = self.getDefinedThreshold(thrName)
                        if threshold is None:
                            log.error('Threshold %s is required in menu on board %s, connector %s, but it is not defined', thrName, boardName, connDef['name'] )
                            list_of_undefined_thresholds += [ thrName ]
                        else:
                            self.l1menu.addThreshold( threshold )

        # ctpin thresholds
        for (boardName, boardDef) in allBoards:
            for connDef in boardDef["connectors"]:
                if connDef["type"] != "ctpin":
                    continue
                for entry in connDef["thresholds"]:
                    if type(entry) == dict:
                        # section that defines topo legacy thresholds 
                        thrNames = sum([x.outputlines for x in entry["algorithms"]],[])
                    elif type(entry) == str:
                        thrNames = [ entry ]
                    elif type(entry) == tuple:
                        thrNames = [ entry[0] ]

                    for thrName in thrNames:
                        if thrName is None or thrName in self.l1menu.thresholds:
                            continue
                        threshold = self.getDefinedThreshold(thrName)
                        if threshold is None:
                            log.error('Threshold %s is listed in menu but not defined', thrName )
                            list_of_undefined_thresholds += [ thrName ]
                        else:
                            self.l1menu.addThreshold( threshold )
                try:
                    zbThrName = connDef["zeroBias"]
                    zbThr = self.getDefinedThreshold(zbThrName)
                    if zbThr is None:
                        log.error('Zero bias threshold %s is listed in menu but not defined', zbThrName )
                        list_of_undefined_thresholds += [ zbThrName ]
                    else:
                        self.l1menu.addThreshold( zbThr )
                except KeyError:
                    pass

        if len(list_of_undefined_thresholds)>0:
            raise RuntimeError("Found undefined threshold in menu %s, please add these thresholds to l1menu/ThresholdDef.py: %s" % \
                               (self.l1menu.menuName, ', '.join(list_of_undefined_thresholds)) )


        # ------------------
        # Items
        # ------------------

        # build list of items for the menu from the list of requested names
        itemsForMenu = []
        ctpIdMap = L1MenuFlags.CtpIdMap()
        for itemName in L1MenuFlags.items():
            registeredItem = self.getRegisteredItem(itemName)
            if registeredItem is None:
                msg = "L1 item '%s' has not been defined in L1/Config/ItemDef.py" % itemName
                log.error(msg)
                raise RuntimeError(msg)

            if itemName in ctpIdMap:
                newCTPID = ctpIdMap[itemName]
                registeredItem.setCtpid(newCTPID)

            for thrName in registeredItem.thresholdNames():
                if thrName not in self.l1menu.thresholds:
                    isLegacyThr = any(filter(lambda x: thrName.startswith(x), ["R2TOPO_", "EM", "HA", "J", "XE", "TE", "XS"]))

                    msg = "L1 item {item} has been added to the menu L1/Menu/Menu_{menu}.py, but the required threshold {thr} is not listed as input in L1/Menu/Menu_{menu_inputs}.py".format(item=itemName, thr=thrName, menu=self.menuFilesToLoad[0], menu_inputs=self.menuFilesToLoad[2] if isLegacyThr else self.menuFilesToLoad[1])
                    log.error(msg)
                    raise RuntimeError(msg)

            itemsForMenu += [ registeredItem ]
            


        # CTP IDs available for assignment
        assigned_ctpids = set([item.ctpid for item in itemsForMenu])
        available_ctpids = sorted( list( set(range(Limits.MaxTrigItems)) - assigned_ctpids ), reverse=True )

        # add the items to the menu
        for item in itemsForMenu:
            # set the physics bit
            if not item.name.startswith('L1_CALREQ'):
                item.setTriggerType( item.trigger_type | TT.phys )
            # assign ctp IDs to items that don't have one
            if item.ctpid == -1:
                if len(available_ctpids)==0:
                    raise RuntimeError("No more CTP IDs available at L1!!")
                item.setCtpid( available_ctpids.pop() )
            # add the items into the menu
            self.l1menu.addItem( item )

        # ------------------
        # Connectors
        # ------------------

        for (boardName, boardDef) in allBoards:
            for connDef in boardDef["connectors"]:
                self.l1menu.addConnector( connDef )

        # ------------------
        # Boards
        # ------------------

        for (boardName, boardDef) in allBoards:
            self.l1menu.addBoard(boardDef)
            
        # ------------------
        # Mark items legacy
        # ------------------
        legacyThresholdsSet = set() # determine from connectors
        for conn in self.l1menu.connectors:
            if not conn.isLegacy():
                continue
            legacyThresholdsSet.update(conn.triggerThresholds())
        for item in self.l1menu.items:
            item.markLegacy(legacyThresholdsSet)
            

        # assign mapping to thresholds according to their use in the menu
        self.mapThresholds()

        # ------------------
        # CTP
        # ------------------
        self.l1menu.ctp.checkConnectorAvailability(self.l1menu.connectors, self.menuFilesToLoad)

        # set the ctp monitoring (only now after the menu is defined)
        self.l1menu.setupCTPMonitoring()

        # ------------------
        # final consistency check
        # ------------------

        self.l1menu.check()

        # check that no L1 items are defined with BGRP0 only
        self.l1menu.checkBGRP()

        # check that only the minimal set of legacy and detector thresholds is used
        if 'pp' in self.l1menu.menuName:
           self.l1menu.checkLegacyThresholds()   

        # check for the topo multiplicity algorithms and CTP inputs
        # TOPO1
        TopoAlgoDefMultiplicity.checkMultAlgoFWconstraints(self.l1menu)

        # check #number of CTP inputs and outputs <=512
        self.l1menu.checkCountCTPInputsOutput()

        # check #number of inputs on the CTPIN connectors
        self.l1menu.checkCTPINconnectors()

        # check that performance thresholds are not used in the physics L1 menu
        self.l1menu.checkPerfThresholds()

        # check that the minimum thresholds of the TOBs sent to TOPO are below the min thresholds used in menu (ATR-15450)
        self.l1menu.checkPtMinToTopo()

        # check that the ordering of the L1Topo parameters matches those expected in the L1Topo firmware
        self.l1menu.checkL1TopoParams()

        # check that every item in the menu is on a board connected to CTP
        self.l1menu.checkItemsHaveInputs()

    def mapThresholds(self):
        """
        Set the correct mapping of thresholds according to the
        order it was given in L1MenuFlags.thresholds list. That list
        is usually defined in the setupMenu function of each menu

        NIM and CALREQ types are not remapped !!
        """

        alreadyUsedMappingNumbers = ddict(set)
        for thr in self.l1menu.thresholds:
            if thr.mapping<0:
                continue
            alreadyUsedMappingNumbers[thr.ttype].add(thr.mapping)

        nextFreeMapping = ddict(lambda: 0)
        for k in  alreadyUsedMappingNumbers:
            nextFreeMapping[k] = 0
        for thr in self.l1menu.thresholds():
            if thr.mapping < 0:
                while nextFreeMapping[thr.ttype] in alreadyUsedMappingNumbers[thr.ttype]:
                    nextFreeMapping[thr.ttype] += 1
                log.debug('Setting mapping of threshold %s as %i', thr, nextFreeMapping[thr.ttype])
                thr.mapping = nextFreeMapping[thr.ttype]
                nextFreeMapping[thr.ttype] += 1

            
    def updateItemPrescales(self):
        for (it_name, ps) in L1MenuFlags.prescales().items():
            item = self.l1menu.getItem(it_name)
            if item:
                item.prescale = ps
            else:
                log.warning('Cannot find item %s to set the prescale', it_name )


    def configureCTP(self):
        self.l1menu.ctp.addMonCounters()

    # remove prescale suffixes
    def _getMenuBaseName(self, menuName):
        pattern = re.compile(r'_v\d+|DC14')
        patternPos = pattern.search(menuName)
        if patternPos:
            menuName=menuName[:patternPos.end()]
        else:
            log.info('Can\'t find pattern to shorten menu name, either non-existent in name or not implemented.')
        return menuName

