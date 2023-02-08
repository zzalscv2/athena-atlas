# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


"""
#
#the configuration of tools shared between L2 and EF
#                                         Jiri Masik
#

"""

from AthenaCommon.Logging import logging
log = logging.getLogger('InDetTrigCommonTools')


from AthenaCommon.GlobalFlags import globalflags


# Straw status DB Tool
from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_StrawStatusSummaryTool
InDetTrigTRTStrawStatusSummaryTool = TRT_StrawStatusSummaryTool(name = "InDetTrigTRTStrawStatusSummaryTool",
                                                                isGEANT4 = (globalflags.DataSource == 'geant4'))



def CAtoLegacyPublicToolWrapper(func,**kwargs):

  from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper, conf2toConfigurable
  from InDetTrigRecExample import InDetTrigCA
  
  ca = CAtoGlobalWrapper(func,InDetTrigCA.InDetTrigConfigFlags,**kwargs)
  privateTool = ca.popPrivateTools()
  tool = conf2toConfigurable(privateTool)

  from AthenaCommon.AppMgr import ToolSvc
  ToolSvc += tool

  return tool
