# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import AnaAlgorithm.DualUseConfig as DualUseConfig
from AthenaConfiguration.Enums import LHCPeriod
import re


def mapUserName (name) :
    """map an internal name to a name for systematics data handles

    Right now this just means appending a _%SYS% to the name."""
    return name + "_%SYS%"



class SelectionConfig :
    """all the data for a given selection that has been registered

    the bits argument is for backward compatibility, does nothing, and will be
    removed in the future."""

    def __init__ (self, selectionName, decoration,
                  *, bits=0, preselection=None) :
        self.name = selectionName
        self.decoration = decoration
        if preselection is not None :
            self.preselection = preselection
        else :
            self.preselection = (selectionName == '')



class OutputConfig :
    """all the data for a given variables in the output that has been registered"""

    def __init__ (self, origContainerName, variableName,
                  *, noSys, enabled) :
        self.origContainerName = origContainerName
        self.outputContainerName = None
        self.variableName = variableName
        self.noSys = noSys
        self.enabled = enabled



class ContainerConfig :
    """all the auto-generated meta-configuration data for a single container

    This tracks the naming of all temporary containers, as well as all the
    selection decorations."""

    def __init__ (self, name, sourceName, *, originalName = None) :
        self.name = name
        self.sourceName = sourceName
        self.originalName = originalName
        self.index = 0
        self.maxIndex = None
        self.viewIndex = 1
        self.isMet = False
        self.selections = []
        self.outputs = {}

    def currentName (self) :
        if self.index == 0 :
            if self.sourceName is None :
                raise Exception ("should not get here, reading container name before created: " + self.name)
            return self.sourceName
        if self.maxIndex and self.index == self.maxIndex :
            return mapUserName(self.name)
        return mapUserName(self.name + "_STEP" + str(self.index))


    def nextPass (self) :
        self.maxIndex = self.index
        self.index = 0
        self.viewIndex = 1
        self.selections = []
        self.outputs = {}



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

    def __init__ (self, dataType, algSeq, isPhyslite=False, geometry=LHCPeriod.Run2):
        if dataType not in ["data", "mc", "afii"] :
            raise ValueError ("invalid data type: " + dataType)
        # allow possible string argument for `geometry` and convert it to enum
        geometry = LHCPeriod(geometry)
        if geometry not in [LHCPeriod.Run2, LHCPeriod.Run3] :
            raise ValueError ("invalid Run geometry: %s" % geometry.value)
        self._dataType = dataType
        self._isPhyslite = isPhyslite
        self._geometry = geometry
        self._algSeq = algSeq
        self._containerConfig = {}
        self._outputContainers = {}
        self._pass = 0
        self._algorithms = {}
        self._currentAlg = None
        self._selectionNameExpr = re.compile ('[A-Za-z_][A-Za-z_0-9]+')
        self.setSourceName ('EventInfo', 'EventInfo')

        # If we are in an Athena environment with ComponentAccumulator configuration
        # then the AlgSequence, which is Gaudi.AthSequencer, does not support '+=',
        # and we in any case want to produce an output ComponentAccumulator
        self.CA = None
        if DualUseConfig.useComponentAccumulator:
            from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
            self.CA = ComponentAccumulator()
            self.CA.addSequence(algSeq)


    def dataType (self) :
        """the data type we run on (data, mc, afii)"""
        return self._dataType


    def isPhyslite (self) :
        """whether we run on PHYSLITE"""
        return self._isPhyslite

    def geometry (self) :
        """the LHC Run period we run on"""
        return self._geometry

    def createAlgorithm (self, type, name, reentrant=False) :
        """create a new algorithm and register it as the current algorithm"""
        if self._pass == 0 :
            if name in self._algorithms :
                raise Exception ('duplicate algorithms: ' + name)
            if reentrant:
                alg = DualUseConfig.createReentrantAlgorithm (type, name)
            else:
                alg = DualUseConfig.createAlgorithm (type, name)

            if DualUseConfig.useComponentAccumulator:
                self.CA.addEventAlgo(alg,self._algSeq.name)
            else:
                self._algSeq += alg
            self._algorithms[name] = alg
            self._currentAlg = alg
            return alg
        else :
            if name not in self._algorithms :
                raise Exception ('unknown algorithm requested: ' + name)
            self._currentAlg = self._algorithms[name]
            return self._algorithms[name]


    def createService (self, type, name) :
        '''create a new service and register it as the "current algorithm"'''
        if self._pass == 0 :
            if name in self._algorithms :
                raise Exception ('duplicate service: ' + name)
            service = DualUseConfig.createService (type, name)
            # Avoid importing AthenaCommon.AppMgr in a CA Athena job
            # as it modifies Gaudi behaviour
            if DualUseConfig.isAthena:
                if DualUseConfig.useComponentAccumulator:
                    self.CA.addService(service)
            else:
                # We're not, so let's remember this as a "normal" algorithm:
                self._algSeq += service
            self._algorithms[name] = service
            self._currentAlg = service
            return service
        else :
            if name not in self._algorithms :
                raise Exception ('unknown service requested: ' + name)
            self._currentAlg = self._algorithms[name]
            return self._algorithms[name]


    def createPublicTool (self, type, name) :
        '''create a new public tool and register it as the "current algorithm"'''
        if self._pass == 0 :
            if name in self._algorithms :
                raise Exception ('duplicate public tool: ' + name)
            tool = DualUseConfig.createPublicTool (type, name)
            # Avoid importing AthenaCommon.AppMgr in a CA Athena job
            # as it modifies Gaudi behaviour
            if DualUseConfig.isAthena:
                if DualUseConfig.useComponentAccumulator:
                    self.CA.addPublicTool(tool)
            else:
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


    def addPrivateTool (self, propertyName, toolType) :
        """add a private tool to the current algorithm"""
        if self._pass == 0 :
            DualUseConfig.addPrivateTool (self._currentAlg, propertyName, toolType)


    def setSourceName (self, containerName, sourceName,
                       *, originalName = None) :
        """set the (default) name of the source/original container

        This is essentially meant to allow using e.g. the muon
        configuration and the user not having to manually specify that
        they want to use the Muons/AnalysisMuons container from the
        input file.

        In addition it allows to set the original name of the
        container (which may be different from the source name), which
        is mostly/exclusively used for jet containers, so that
        subsequent configurations know which jet container they
        operate on.
        """
        if containerName not in self._containerConfig :
            self._containerConfig[containerName] = ContainerConfig (containerName, sourceName, originalName = originalName)


    def writeName (self, containerName, *, isMet=None) :
        """register that the given container will be made and return
        its name"""
        if containerName not in self._containerConfig :
            self._containerConfig[containerName] = ContainerConfig (containerName, sourceName = None)
        if self._containerConfig[containerName].sourceName is not None :
            raise Exception ("trying to write container configured for input: " + containerName)
        if self._containerConfig[containerName].index != 0 :
            raise Exception ("trying to write container twice: " + containerName)
        self._containerConfig[containerName].index += 1
        if isMet is not None :
            self._containerConfig[containerName].isMet = isMet
        return self._containerConfig[containerName].currentName()


    def readName (self, containerName) :
        """get the name of the "current copy" of the given container

        As extra copies get created during processing this will track
        the correct name of the current copy.  Optionally one can pass
        in the name of the container before the first copy.
        """
        if containerName not in self._containerConfig :
            raise Exception ("no source container for: " + containerName)
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
            raise Exception ("no source container for: " + containerName)
        return self._containerConfig[containerName].index == 0


    def originalName (self, containerName) :
        """get the "original" name of the given container

        This is mostly/exclusively used for jet containers, so that
        subsequent configurations know which jet container they
        operate on.
        """
        if containerName not in self._containerConfig :
            raise Exception ("container unknown: " + containerName)
        result = self._containerConfig[containerName].originalName
        if result is None :
            raise Exception ("no original name for: " + containerName)
        return result


    def isMetContainer (self, containerName) :
        """whether the given container is registered as a MET container

        This is mostly/exclusively used for determining whether to
        write out the whole container or just a single MET term.
        """
        if containerName not in self._containerConfig :
            raise Exception ("container unknown: " + containerName)
        return self._containerConfig[containerName].isMet


    def readNameAndSelection (self, containerName) :
        """get the name of the "current copy" of the given container, and the
        selection string

        This is mostly meant for MET and OR for whom the actual object
        selection is relevant, and which as such allow to pass in the
        working point as "ObjectName.WorkingPoint".
        """
        split = containerName.split (".")
        if len(split) == 1 :
            objectName = split[0]
            selectionName = ''
        elif len(split) == 2 :
            objectName = split[0]
            selectionName = split[1]
        else :
            raise Exception ('invalid object selection name: ' + containerName)
        return self.readName (objectName), self.getFullSelection (objectName, selectionName)


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
        self._outputContainers = {}


    def getPreselection (self, containerName, selectionName, *, asList = False) :

        """get the preselection string for the given selection on the given
        container
        """
        if selectionName != '' and not self._selectionNameExpr.fullmatch (selectionName) :
            raise ValueError ('invalid selection name: ' + selectionName)
        if containerName not in self._containerConfig :
            return ""
        config = self._containerConfig[containerName]
        decorations = []
        for selection in config.selections :
            if (selection.name == '' or selection.name == selectionName) and \
               selection.preselection :
                decorations += [selection.decoration]
        if asList :
            return decorations
        else :
            return '&&'.join (decorations)


    def getFullSelection (self, containerName, selectionName,
                          *, skipBase = False) :

        """get the selection string for the given selection on the given
        container

        This can handle both individual selections or selection
        expressions (e.g. `loose||tight`) with the later being
        properly expanded.  Either way the base selection (i.e. the
        selection without a name) will always be applied on top.

        containerName --- the container the selection is defined on
        selectionName --- the name of the selection, or a selection
                          expression based on multiple named selections
        skipBase --- will avoid the base selection, and should normally
                     not be used by the end-user.

        """
        if containerName not in self._containerConfig :
            return ""

        # Check if this is actually a selection expression,
        # e.g. `A||B` and if so translate it into a complex expression
        # for the user.  I'm not trying to do any complex syntax
        # recognition, but instead just produce an expression that the
        # C++ parser ought to be able to read.
        if selectionName != '' and \
           not self._selectionNameExpr.fullmatch (selectionName) :
            result = ''
            while selectionName != '' :
                match = self._selectionNameExpr.match (selectionName)
                if not match :
                    result += selectionName[0]
                    selectionName = selectionName[1:]
                else :
                    subname = match.group(0)
                    subresult = self.getFullSelection (containerName, subname, skipBase = True)
                    if subresult != '' :
                        result += '(' + subresult + ')'
                    else :
                        result += 'true'
                    selectionName = selectionName[len(subname):]
            subresult = self.getFullSelection (containerName, '')
            if subresult != '' :
                result = subresult + '&&(' + result + ')'
            return result

        config = self._containerConfig[containerName]
        decorations = []
        for selection in config.selections :
            if ((selection.name == '' and not skipBase) or
                selection.name == selectionName) :
                decorations += [selection.decoration]
        return '&&'.join (decorations)


    def getSelectionCutFlow (self, containerName, selectionName) :

        """get the individual selections as a list for producing the cutflow for
        the given selection on the given container

        This can only handle individual selections, not selection
        expressions (e.g. `loose||tight`).

        """
        if containerName not in self._containerConfig :
            return []

        # Check if this is actually a selection expression,
        # e.g. `A||B` and if so translate it into a complex expression
        # for the user.  I'm not trying to do any complex syntax
        # recognition, but instead just produce an expression that the
        # C++ parser ought to be able to read.
        if selectionName != '' and \
           not self._selectionNameExpr.fullmatch (selectionName) :
            raise ValueError ('not allowed to do cutflow on selection expression: ' + selectionName)

        config = self._containerConfig[containerName]
        decorations = []
        for selection in config.selections :
            if (selection.name == '' or selection.name == selectionName) :
                decorations += [selection.decoration]
        return decorations


    def addSelection (self, containerName, selectionName, decoration,
                      **kwargs) :
        """add another selection decoration to the selection of the given
        name for the given container"""
        if selectionName != '' and not self._selectionNameExpr.fullmatch (selectionName) :
            raise ValueError ('invalid selection name: ' + selectionName)
        if containerName not in self._containerConfig :
            self._containerConfig[containerName] = ContainerConfig (containerName, containerName)
        config = self._containerConfig[containerName]
        selection = SelectionConfig (selectionName, decoration, **kwargs)
        config.selections.append (selection)


    def addOutputContainer (self, containerName, outputContainerName) :
        """register a copy of a container used in outputs"""
        if containerName not in self._containerConfig :
            raise KeyError ("container unknown: " + containerName)
        if outputContainerName in self._outputContainers :
            raise KeyError ("duplicate output container name: " + outputContainerName)
        self._outputContainers[outputContainerName] = containerName


    def addOutputVar (self, containerName, variableName, outputName,
                      *, noSys=False, enabled=True) :
        """add an output variable for the given container to the output
        """

        if containerName not in self._containerConfig :
            raise KeyError ("container unknown: " + containerName)
        baseConfig = self._containerConfig[containerName].outputs
        if outputName in baseConfig :
            raise KeyError ("duplicate output variable name: " + outputName)
        config = OutputConfig (containerName, variableName, noSys=noSys, enabled=enabled)
        baseConfig[outputName] = config


    def getOutputVars (self, containerName) :
        """get the output variables for the given container"""
        if containerName in self._outputContainers :
            containerName = self._outputContainers[containerName]
        if containerName not in self._containerConfig :
            raise KeyError ("unknown container for output: " + containerName)
        return self._containerConfig[containerName].outputs
