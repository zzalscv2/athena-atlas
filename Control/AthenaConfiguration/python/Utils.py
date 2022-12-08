# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
from GaudiKernel.GaudiHandles import GaudiHandle
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
import collections
def loadDefaultComps(allcomps):
    """Attempts to load all default components (those that are not actually configured)"""
    loadedSome = False
    def __load(comp, prop):
        descr = comp._descriptors[prop]
        if descr.default.getType() == "":
            return
        try:
            childComp = CompFactory.getComp(descr.default.getType())(descr.default.getName())
            comp._properties[prop] = childComp
            nonlocal loadedSome
            loadedSome = True
        except Exception:
            msg=logging.getLogger('loadDefaultComps')
            msg.warning("Default component %s can not be loaded", descr.default.typeAndName )
            pass

    for comp in allcomps:
        for propName,value in comp._descriptors.items():
            if propName in comp._properties: continue
            if isinstance(value.default, GaudiHandle):
                __load(comp, propName )
                        
    if loadedSome:
        loadDefaultComps(allcomps)

def exposeHandles(allcomps):
    """Sets all handle keys explicitly"""
    def __getDefault(d):
        if isinstance(d, collections.abc.Sequence):
            return [el  for el in d]
        else:
            return d.Path

    for comp in allcomps:
        for propName, value in comp._descriptors.items():
            if propName in comp._properties: continue
            if "HandleKey" in value.cpp_type:
                comp._properties[propName] = __getDefault(value.default)

def setupLoggingLevels(flags, ca):
    """Read the Exec.*MessageComponents flags and modify output level of components
    
        The specification of components  uses python fnmatch library and resembles UNIX paths.
        For instance an event algorithm MyAlgo/MyInstance would have following path:
        MasterSeq/AthAllAlgSeq/AthAlgSeq/MyAlgo/MyInstance
        A private tool MyTool of name ToolInstance used by this algo:
        MasterSeq/AthAllAlgSeq/AthAlgSeq/MyAlgo/MyInstance/MyTool/ToolInstance
        A public tool would have path:
        ToolSvc/MyTool/ToolInstance 
        and so on.
        The path specification can look like thi:
        */ToolInstance - all tools that have matching instance name
        */MyTool/* - all instances of the tool
        */MyAlgo/MyInstance - specific algorithm
        */MyAlg0/* - all instance of specific algorithm
        */AthAlgSeq/* - all algorithms (with deeper sequences structure e.g. in HLT sub-components of sequences can controlled this way)
        ToolSvc/My*/* - all public tools that instance name starts with My

        The modifications to the OutputLevel are applied form ERROR to VERBOSE.
        This is to support pattern when large group of components is set to less verbose logging mode
        and with more specific selection more verbosity is enabled. E.g. all algorithms can be set to WARNING, 
        the all having calo in the name to INFO, and CaloCellMaker to DEBUG.  
        
        Each setting can be either a string or a list of strings.
    """
    from AthenaCommon.Constants import ERROR,WARNING,INFO,DEBUG,VERBOSE
    def __tolist(d):
        return ([d] if d != '' else []) if isinstance(d, str) else d
    for flag_val, level in [(flags.Exec.ErrorMessageComponents, ERROR),
                            (flags.Exec.WarningMessageComponents, WARNING),
                            (flags.Exec.InfoMessageComponents, INFO),
                            (flags.Exec.DebugMessageComponents, DEBUG),
                            (flags.Exec.VerboseMessageComponents, VERBOSE) ]:
        for c in __tolist(flag_val):
            ca.foreach_component(c).OutputLevel=level
