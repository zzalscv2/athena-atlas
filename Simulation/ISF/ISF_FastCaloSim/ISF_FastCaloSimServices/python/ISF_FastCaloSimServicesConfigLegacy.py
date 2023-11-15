# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
Tools configurations for ISF_FastCaloSimServices
KG Tan, 04/12/2012
"""

from AthenaCommon import CfgMgr

#### FastCaloSimV2
def getFastCaloSimV2ParamSvc(name="ISF_FastCaloSimV2ParamSvc", **kwargs):
    from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags
    kwargs.setdefault("ParamsInputFilename"              , ISF_FastCaloSimFlags.ParamsInputFilename())
    kwargs.setdefault("RunOnGPU"                         ,ISF_FastCaloSimFlags.RunOnGPU())
    kwargs.setdefault("ParamsInputObject"                , 'SelPDGID')
    return CfgMgr.ISF__FastCaloSimV2ParamSvc(name, **kwargs )


def getFastCaloSimSvcV2(name="ISF_FastCaloSimSvcV2", **kwargs):
    kwargs.setdefault("SimulatorTool",  'ISF_FastCaloSimV2Tool')
    kwargs.setdefault("Identifier",     'FastCaloSim')
    return CfgMgr.ISF__LegacySimSvc(name, **kwargs )


#### DNNCaloSim
def getDNNCaloSimSvc(name="ISF_DNNCaloSimSvc", **kwargs):
    from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags

    kwargs.setdefault("CaloCellsOutputName"              , ISF_FastCaloSimFlags.CaloCellsName()   )
    kwargs.setdefault("CaloCellMakerTools_setup"         , [ 'ISF_EmptyCellBuilderTool' ] )
    kwargs.setdefault("CaloCellMakerTools_release"       , [ 'ISF_CaloCellContainerFinalizerTool',
                                                           'ISF_FastHitConvertTool' ]) #DR needed ?
    kwargs.setdefault("ParamsInputFilename"              , ISF_FastCaloSimFlags.ParamsInputFilename())
    kwargs.setdefault("FastCaloSimCaloExtrapolation"     , 'FastCaloSimCaloExtrapolation')

    # register the FastCaloSim random number streams
    from G4AtlasApps.SimFlags import simFlags
    if not simFlags.RandomSeedList.checkForExistingSeed(ISF_FastCaloSimFlags.RandomStreamName()):
        simFlags.RandomSeedList.addSeed( ISF_FastCaloSimFlags.RandomStreamName(), 98346412, 12461240 )

    kwargs.setdefault("RandomStream"                     , ISF_FastCaloSimFlags.RandomStreamName())
    kwargs.setdefault("RandomSvc"                        , simFlags.RandomSvc.get_Value() )

    return CfgMgr.ISF__DNNCaloSimSvc(name, **kwargs )
