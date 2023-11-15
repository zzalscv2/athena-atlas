# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
Configuration database for ISF_FastCaloSimServices
Elmar Ritsch, 01/06/2016
"""

from AthenaCommon.CfgGetter import addTool, addService

addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getNIMatEffUpdator",                 "ISF_NIMatEffUpdator")
addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getNIPropagator",                    "ISF_NIPropagator")
addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getNINavigator",                     "ISF_NINavigator")
addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getNITimedExtrapolator",             "ISF_NITimedExtrapolator")
addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getTimedExtrapolator",               "TimedExtrapolator")

addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getPunchThroughTool",                "ISF_PunchThroughTool")
addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getPunchThroughClassifier",          "ISF_PunchThroughClassifier")

addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getEmptyCellBuilderTool",            "ISF_EmptyCellBuilderTool")
addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getCaloCellContainerFinalizerTool",  "ISF_CaloCellContainerFinalizerTool")
addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getCaloCellContainerFCSFinalizerTool",  "ISF_CaloCellContainerFCSFinalizerTool")

addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getFastHitConvertTool",              "ISF_FastHitConvertTool")

addTool("ISF_FastCaloSimServices.AdditionalConfigLegacy.getFastCaloSimV2Tool",               "ISF_FastCaloSimV2Tool")

addService("ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfigLegacy.getFastCaloSimV2ParamSvc",                      "ISF_FastCaloSimV2ParamSvc")
addService("ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfigLegacy.getFastCaloSimSvcV2",                           "ISF_FastCaloSimSvcV2")
addService("ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfigLegacy.getDNNCaloSimSvc",                           "ISF_DNNCaloSimSvc")
