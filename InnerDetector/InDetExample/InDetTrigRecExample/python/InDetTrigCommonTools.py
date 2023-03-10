# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


"""
#
#the configuration of tools shared between L2 and EF
#                                         Jiri Masik
#

"""


def CAtoLegacyPublicToolWrapper(func,**kwargs):

  from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper, conf2toConfigurable
  from InDetTrigRecExample import InDetTrigCA
  
  ca = CAtoGlobalWrapper(func,InDetTrigCA.InDetTrigConfigFlags,**kwargs)
  privateTool = ca.popPrivateTools()
  tool = conf2toConfigurable(privateTool)

  from AthenaCommon.AppMgr import ToolSvc
  ToolSvc += tool

  return tool

