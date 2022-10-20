# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
### This module contains functions which may need to peek at the input file metadata
from AthenaCommon.Logging import logging
from AthenaConfiguration.Enums import ProductionStep
from AthenaKernel.EventIdOverrideConfig import getMinMaxRunNumbers

folderName = "/Simulation/Parameters"


def fillAtlasMetadata(ConfigFlags, dbFiller):
    simMDlog = logging.getLogger('Sim_Metadata')
    #add all ConfigFlags to the metadata
    #todo - only add certain ones?
    #in future this should be a ConfigFlags method...?
    for flag in sorted(ConfigFlags._flagdict): #only sim
        if "Sim" in flag:
            key = flag.split(".")[-1] #use final part of flag as the key
            value = eval("ConfigFlags."+flag)
            if not isinstance(value, str):
                value = str(value)
            dbFiller.addSimParam(key, value)
            simMDlog.info('SimulationMetaData: setting "%s" to be %s', key, value)

    dbFiller.addSimParam('G4Version', ConfigFlags.Sim.G4Version)
    dbFiller.addSimParam('RunType', 'atlas')
    dbFiller.addSimParam('beamType', ConfigFlags.Beam.Type.value)
    dbFiller.addSimParam('SimLayout', ConfigFlags.GeoModel.AtlasVersion)
    dbFiller.addSimParam('MagneticField', 'AtlasFieldSvc') # TODO hard-coded for now for consistency with old-style configuration.

    #---------
    ## Simulated detector flags: add each enabled detector to the simulatedDetectors list
    from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors
    simDets = getEnabledDetectors(ConfigFlags)
    simMDlog.info("Setting 'SimulatedDetectors' = %r", simDets)
    dbFiller.addSimParam('SimulatedDetectors', repr(simDets))

    ## Hard-coded simulation hit file magic number (for major changes)
    dbFiller.addSimParam('hitFileMagicNumber', '0') ##FIXME Remove this?

    if ConfigFlags.Sim.ISFRun:
        dbFiller.addSimParam('Simulator', ConfigFlags.Sim.ISF.Simulator.value)
        dbFiller.addSimParam('SimulationFlavour', ConfigFlags.Sim.ISF.Simulator.value.replace('MT', '')) # used by egamma
    else:
        # TODO hard-code for now, but set flag properly later
        dbFiller.addSimParam('Simulator', 'AtlasG4')
        dbFiller.addSimParam('SimulationFlavour', 'AtlasG4')


def writeSimulationParametersMetadata(ConfigFlags):
    simMDlog = logging.getLogger('Sim_Metadata')
    from IOVDbMetaDataTools import ParameterDbFiller
    dbFiller = ParameterDbFiller.ParameterDbFiller()
    myRunNumber, myEndRunNumber = getMinMaxRunNumbers(ConfigFlags)
    simMDlog.debug('ParameterDbFiller BeginRun = %s', str(myRunNumber) )
    dbFiller.setBeginRun(myRunNumber)
    simMDlog.debug('ParameterDbFiller EndRun   = %s', str(myEndRunNumber) )
    dbFiller.setEndRun(myEndRunNumber)

    fillAtlasMetadata(ConfigFlags, dbFiller)

    #-------------------------------------------------
    # Make the MetaData Db
    #-------------------------------------------------
    dbFiller.genSimDb()

    return writeSimulationParameters(ConfigFlags)


def readSimulationParameters(ConfigFlags):
    """Read digitization parameters metadata"""
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if ConfigFlags.Common.ProductionStep not in [ProductionStep.Simulation, ProductionStep.FastChain]:
        return addFolders(ConfigFlags, folderName, className="AthenaAttributeList", tag="HEAD")

    # Here we are in a job which runs simulation, so the
    # /Simulation/Parameters metadata is not present in the
    # input file and will be created during the job
    return addFolders(ConfigFlags, folderName, detDb="SimParams.db", db="SIMPARAM", className="AthenaAttributeList")


def writeSimulationParameters(ConfigFlags):
    """Write digitization parameters metadata"""
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg, addFolders
    acc = IOVDbSvcCfg(ConfigFlags, FoldersToMetaData=[folderName])
    acc.merge(addFolders(ConfigFlags, folderName, detDb="SimParams.db", db="SIMPARAM"))
    return acc
