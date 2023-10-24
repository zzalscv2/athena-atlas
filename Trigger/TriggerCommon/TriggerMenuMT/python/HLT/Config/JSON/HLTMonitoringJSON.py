# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

import re
import json
from collections import OrderedDict as odict
from TrigConfigSvc.TrigConfigSvcCfg import getHLTMenuFileName, getHLTMonitoringFileName
from AthenaCommon.Logging import logging
__log = logging.getLogger( __name__ )


# remove prescale suffixes
def __getMenuBaseName(menuName):
    pattern = re.compile(r'_v\d+|DC14')
    patternPos = pattern.search(menuName)
    if patternPos:
        menuName=menuName[:patternPos.end()]
    else:
        __log.info('Can\'t find pattern to shorten menu name, either non-existent in name or not implemented.')
    return menuName

def generateDefaultMonitoringJSON(flags):
    __log.info("Generating HLT Monitoring JSON in the rec-ex-common job")

    from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig

    __log.debug("[HLTMonitoring::generateJSON] HLTMenuFileName =  %s"        , getHLTMenuFileName(flags)        )
    __log.debug("[HLTMonitoring::generateJSON] HLTMonitoringFileName =  %s"  , getHLTMonitoringFileName(flags)  )

    return generateMonitoringJSON(flags.Trigger.triggerMenuSetup,
                                  HLTMenuConfig.dictsList(), 
                                  getHLTMonitoringFileName(flags))

def generateDefaultMonitoringJSON_newJO(flags, chainDicts):
    __log.info("Generating HLT Monitoring JSON in the new JO")

    return generateMonitoringJSON(flags.Trigger.triggerMenuSetup,
                                  chainDicts,
                                  getHLTMonitoringFileName(flags))

def generateMonitoringJSON(menuName, chainDicts, fileName):

    #create a summary file 
    summaryDict=odict([ ("filetype", "hltmonitoringsummary"), ("name", __getMenuBaseName(menuName)), ("signatures", odict())])

    # then you can iterate, say over the HLT menu:
    for chain in chainDicts:
        chain_name = chain['chainName']
        __log.debug("[HLTMonitoring::generateMonitoringJSON] Processing Chain %s ...", chain_name)

        #if the chain is in a express stream, check presence of teh monGroup
        if 'express' in chain['stream']:
            if len(chain['monGroups']) == 0:
                __log.warning( "[HLTMonitoring::generateMonitoringJSON]Chain %s is in the express stream and doesn't have any monGroup", chain_name)

        #check if the chain needs to be monitored
        if len(chain['monGroups']) == 0:
            continue

        __log.debug("[HLTMonitoring::generateMonitoringJSON] Chain %s has monitoring groups!", chain_name)
            
        #add the signatures in the summary file
        for entry in chain['monGroups']:
            monSignature = entry.split(':')[0]
            monGroup     = entry.split(':')[1]
            
            if monSignature not in summaryDict['signatures']:
                summaryDict['signatures'][monSignature] = odict([(chain_name, [monGroup])])
            else:
                #the signature is already present, 
                # now check the chain
                if chain_name not in summaryDict['signatures'][monSignature]:
                    summaryDict['signatures'][monSignature].update(odict([(chain_name, [monGroup])]))
                else:
                    summaryDict['signatures'][monSignature][chain_name].append(monGroup)

    # # then writing the summary to a file is as simple as:
    __log.info( "[HLTMonitoring::generateMonitoringJSON] Writing Summary of HLT Monitoring JSON to %s",  fileName)
    with open(fileName, 'w') as outfile:
        json.dump(summaryDict, outfile, indent = 4, sort_keys=False )
