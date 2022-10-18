# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class PtEtaSelectionBlock (ConfigBlock):
    """the ConfigBlock for a pt-eta selection"""

    def __init__ (self, containerName, *, postfix, minPt, maxEta,
                  selectionDecoration) :
        super (PtEtaSelectionBlock, self).__init__ ()
        self.containerName = containerName
        self.postfix = postfix
        self.minPt = minPt
        self.maxEta = maxEta
        self.selectionDecoration = selectionDecoration

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PtEtaSelectionAlg' + self.containerName + postfix )
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        bits = 0
        if self.minPt is not None :
            alg.selectionTool.minPt = self.minPt
            bits += 1
        if self.maxEta is not None :
            alg.selectionTool.maxEta = self.maxEta
            bits += 1
        if not self.selectionDecoration.find (',as_bits') :
            bits = 1
        alg.selectionDecoration = self.selectionDecoration
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=bits)



class OutputThinningBlock (ConfigBlock):
    """the ConfigBlock for output thinning"""

    def __init__ (self, containerName, *, postfix) :
        super (OutputThinningBlock, self).__init__ ()
        self.containerName = containerName
        self.postfix = postfix
        self.selection = None
        self.outputName = None

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        selection = config.getPreselection (self.containerName, '')
        if selection == '' :
            selection = self.selection
        elif self.selection != '' :
            selection = selection + '&&' + self.selection

        if selection != '' :
            alg = config.createAlgorithm( 'CP::AsgUnionSelectionAlg', 'UnionSelectionAlg' + self.containerName + postfix)
            alg.preselection = selection
            alg.particles = config.readName (self.containerName)
            alg.selectionDecoration = 'outputSelect' + postfix

        alg = config.createAlgorithm( 'CP::AsgViewFromSelectionAlg', 'DeepCopyAlg' + self.containerName + postfix )
        alg.input = config.readName (self.containerName)
        if self.outputName is not None :
            alg.output = self.outputName + '_%SYS%'
        else :
            alg.output = config.copyName (self.containerName)
        if selection != '' :
            alg.selection = ['outputSelect' + postfix]
        else :
            alg.selection = []
        alg.deepCopy = False



def makePtEtaSelectionConfig( seq, containerName,
                              *, postfix = '', minPt = None, maxEta = None,
                              selectionDecoration):
    """Create a pt-eta kinematic selection config

    Keyword arguments:
      containerName -- name of the container
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      minPt -- minimum pt value
      maxEta -- maximum eta value
      selectionDecoration -- the name of the decoration to set
    """

    config = PtEtaSelectionBlock (containerName, postfix=postfix,
                                  minPt=minPt,maxEta=maxEta,
                                  selectionDecoration=selectionDecoration)
    seq.append (config)



def makeOutputThinningConfig( seq, containerName,
                              *, postfix = '', selection = None, outputName = None):
    """Create an output thinning config

    This will do a consistent selection of output containers (if there
    is a preselection or a selection specified) and then creates a set
    of view containers (or deep copies) based on that selection.

    Keyword arguments:
      containerName -- name of the container
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      selection -- the name of an optional selection decoration to use
      outputName -- an optional name for the output container

    """

    config = OutputThinningBlock (containerName, postfix=postfix)
    config.selection = selection
    config.outputName = outputName
    seq.append (config)
