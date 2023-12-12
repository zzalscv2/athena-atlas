# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import json
from AthenaCommon.Utils.unixtools import find_datafile

def getTrkAnaDicts( input_file ):
    '''
    utility function to retrieve the flag dictionary
    for every TrackAnalysis from an input JSON file
    '''
    analysesDict = {}

    ## Default: use trkAnalysis config flags
    if input_file == "Default":
        return analysesDict

    ## Getting full input json file path
    dataPath = find_datafile( input_file )
    if dataPath is None:
        return analysesDict

    ## Fill temporary analyses config dictionary from input json
    analysesDictTmp = {}
    with open( dataPath, "r" ) as input_json_file:
        analysesDictTmp = json.load( input_json_file )

    ## Update each analysis dictionary with flags from template
    if analysesDictTmp:
        for trkAnaName, trkAnaDict in analysesDictTmp.items():
            ## Update entry with flags read from json
            analysesDict.update( { trkAnaName : trkAnaDict } )
            ## Update TrkAnalysis tag for this entry
            analysesDict[trkAnaName]["anaTag"] = "_" + trkAnaName

    return analysesDict


def getChainList( flags ):
    '''
    utility function to retrieve full list of
    configured trigger chains matching
    (regex) list of current TrkAnalysis
    '''
    from TrigConfigSvc.TriggerConfigAccess import getHLTMenuAccess
    chainsMenu = getHLTMenuAccess( flags )

    import re
    configChains = []
    for regexChain in flags.PhysVal.IDTPM.currentTrkAna.ChainNames:
        for item in chainsMenu:
            chains = re.findall( regexChain, item )
            for chain in chains:
                if chain is not None and chain == item:
                    configChains.append( chain )

    return configChains


def getFlagsList( flags ):
    '''
    utility function to produce a list of sets of flags
    with a separate trackAnalysis for each configured chain
    '''
    fList = []
    isTrig = ( "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.TestType  or
               "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.RefType )
    if isTrig and flags.PhysVal.IDTPM.unpackTrigChains:
        for configChain in getChainList( flags ):
            ## clone the flags
            flags_new = flags.clone()
            flags_new.PhysVal.IDTPM.currentTrkAna.anaTag = flags.PhysVal.IDTPM.currentTrkAna.anaTag + "_" + configChain
            flags_new.PhysVal.IDTPM.currentTrkAna.ChainNames = [ configChain ]
            flags_new.lock()
            fList.append( flags_new )
    else:
        fList.append( flags )

    return fList
