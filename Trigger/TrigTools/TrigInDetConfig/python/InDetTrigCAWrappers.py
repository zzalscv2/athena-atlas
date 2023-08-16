# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


"""
#
# temporary wrappers for CA Cfg functions 
#                                         Jiri Masik
#

"""

def CAtoLegacyPrivateToolWrapper(func,**kwargs):

  from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper, conf2toConfigurable
  from TrigInDetConfig import InDetTrigCA
  
  ca = CAtoGlobalWrapper(func,InDetTrigCA.InDetTrigConfigFlags,**kwargs)
  privateTool = ca.popPrivateTools()
  tool = conf2toConfigurable(privateTool)

  return tool

def CAtoLegacyPublicToolWrapper(func,**kwargs):

  tool = CAtoLegacyPrivateToolWrapper(func, **kwargs)

  from AthenaCommon.AppMgr import ToolSvc
  ToolSvc += tool

  return tool


