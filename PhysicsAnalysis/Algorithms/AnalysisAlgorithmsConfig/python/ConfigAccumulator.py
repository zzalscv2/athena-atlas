# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import AnaAlgorithm.DualUseConfig as DualUseConfig


def mapUserName (name) :
    """map an internal name to a name for systematics data handles

    Right now this just means appending a _%SYS% to the name."""
    return name + "_%SYS%"




class ContainerConfig :
    """all the auto-generated meta-configuration data for a single container

    This tracks the naming of all temporary containers, as well as all the
    selection decorations."""

    def __init__ (self, name, sourceName) :
        self.name = name
        self.sourceName = sourceName
        self.index = 0
        self.maxIndex = None
        self.viewIndex = 1
        self.selection = {}
        self.selection[''] = []
        self.selectionBits = {}
        self.selectionBits[''] = []

    def currentName (self) :
        if self.index == 0 :
            return self.sourceName
        if self.maxIndex and self.index == self.maxIndex :
            return mapUserName(self.name)
        return mapUserName(self.name + "_STEP" + str(self.index))


    def nextPass (self) :
        self.maxIndex = self.index
        self.index = 0
        self.viewIndex = 1
        self.selection = {}
        self.selection[''] = []
        self.selectionBits = {}
        self.selectionBits[''] = []




class ConfigAccumulator :
    """a class that accumulates a configuration from blocks into an
    algorithm sequence

    This is used as argument to the ConfigurationBlock methods, which
    need to be called in the correct order.  This class will track all
    meta-information that needs to be communicated between blocks
    during configuration, and also add the created algorithms to the
    sequence.

    Use/access of containers in the event store is handled via
    references that this class hands out.  This happens in a separate
    step before the algorithms are created, as the naming of
    containers will depend on where in the chain the container is
    used.
    """


    def __init__ (self, dataType, algSeq):
        if dataType not in ["data", "mc", "afii"] :
            raise ValueError ("invalid data type: " + dataType)
        self._dataType = dataType
        self._algSeq = algSeq
        self._containerConfig = {}
        self._pass = 0
        self._algorithms = {}
        self._currentAlg = None


    def dataType (self) :
        """the data type we run on (data, mc, afii)"""
        return self._dataType


    def createAlgorithm (self, type, name) :
        """create a new algorithm and register it as the current algorithm"""
        if self._pass == 0 :
            if name in self._algorithms :
                raise Exception ('duplicate algorithms: ' + name)
            alg = DualUseConfig.createAlgorithm (type, name)
            self._algSeq += alg
            self._algorithms[name] = alg
            self._currentAlg = alg
            return alg
        else :
            if name not in self._algorithms :
                raise Exception ('unknown algorithm requested: ' + name)
            self._currentAlg = self._algorithms[name]
            return self._algorithms[name]


    def createPublicTool (self, type, name) :
        '''create a new public tool and register it as the "current algorithm"'''
        if self._pass == 0 :
            if name in self._algorithms :
                raise Exception ('duplicate public tool: ' + name)
            tool = DualUseConfig.createPublicTool (type, name)
            try:
                # Try to access the ToolSvc, to see whethet we're in Athena mode:
                from AthenaCommon.AppMgr import ToolSvc  # noqa: F401
            except ImportError:
                # We're not, so let's remember this as a "normal" algorithm:
                self._algSeq += tool
            self._algorithms[name] = tool
            self._currentAlg = tool
            return tool
        else :
            if name not in self._algorithms :
                raise Exception ('unknown public tool requested: ' + name)
            self._currentAlg = self._algorithms[name]
            return self._algorithms[name]


    def addPrivateTool (self, type, name) :
        """add a private tool to the current algorithm"""
        if self._pass == 0 :
            DualUseConfig.addPrivateTool (self._currentAlg, type, name)


    def readName (self, containerName, sourceName=None) :
        """get the name of the "current copy" of the given container

        As extra copies get created during processing this will track
        the correct name of the current copy.  Optionally one can pass
        in the name of the container before the first copy.
        """
        if containerName not in self._containerConfig :
            if not sourceName :
                raise Exception ("no source container for: " + containerName)
            self._containerConfig[containerName] = ContainerConfig (containerName, sourceName)
        return self._containerConfig[containerName].currentName()


    def copyName (self, containerName) :
        """register that a copy of the container will be made and return
        its name"""
        if containerName not in self._containerConfig :
            raise Exception ("unknown container: " + containerName)
        self._containerConfig[containerName].index += 1
        return self._containerConfig[containerName].currentName()


    def wantCopy (self, containerName) :
        """ask whether we want/need a copy of the container

        This usually only happens if no copy of the container has been
        made yet and the copy is needed to allow modifications, etc.
        """
        if containerName not in self._containerConfig :
            raise Exception ("unknown container: " + containerName)
        return self._containerConfig[containerName].index == 0


    def nextPass (self) :
        """switch to the next configuration pass

        Configuration happens in two steps, with all the blocks processed
        twice.  This switches from the first to the second pass.
        """
        if self._pass != 0 :
            raise Exception ("already performed final pass")
        for name in self._containerConfig :
            self._containerConfig[name].nextPass ()
        self._pass = 1
        self._currentAlg = None


    def getSelection (self, containerName, selectionName) :

        """get the selection string for the given selection on the given
        container"""
        if containerName not in self._containerConfig :
            return ""
        config = self._containerConfig[containerName]
        selection = config.selection['']
        if selectionName != '' and selectionName in config.selection :
            selection = selection + config.selection[selectionName]
        return '&&'.join (selection)


    def addSelection (self, containerName, selectionName, selectionDecoration, selectionBits) :
        """add another selection decoration to the selection of the given
        name for the given container

        This also takes the number of bits in the selection decoration,
        which is needed to make object cut flows."""
        if containerName not in self._containerConfig :
            self._containerConfig[containerName] = ContainerConfig (containerName, containerName)
        config = self._containerConfig[containerName]
        if selectionName not in config.selection :
            config.selection[selectionName] = []
            config.selectionBits[selectionName] = []
        config.selection[selectionName].append (selectionDecoration)
        config.selectionBits[selectionName].append (selectionBits)
