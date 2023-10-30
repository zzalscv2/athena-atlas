#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

import eformat
from xml.dom import minidom
from collections import OrderedDict
from DQUtils.sugar import RunLumi

from AthenaCommon.Logging import logging
log = logging.getLogger('ExtractRunDetails.py')


# Read the start timestamp of the start lumiblock and the ned of the end lumiblock
def readTimestampsOfLb(runNumber, lbStart=-1, lbEnd=-1):
    ''' 
    Returns start and end of the run read from COOL database
    '''
    from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil
    dbconn = TriggerCoolUtil.GetConnection("CONDBR2")

    # based on TriggerCoolUtil.getRunStartTime
    f = dbconn.getFolder( "/TRIGGER/LUMI/LBLB" )

    since = RunLumi(runNumber, lbStart) if lbStart >= 0 else (runNumber<<32)
    until = RunLumi(runNumber, lbEnd) if lbEnd >= 0 else ((runNumber+1)<<32)-1

    from PyCool import cool
    objs = f.browseObjects(since, until, cool.ChannelSelection(0) )

    lbCounter = lbStart
    timestampDict = OrderedDict()
    while objs.goToNext():
        objCurrRef = objs.currentRef()
        timestampDict[lbCounter] = {"start": int(objCurrRef.payload()["StartTime"]/1000), "end": int(objCurrRef.payload()["EndTime"]/1000)}
        lbCounter+=1

    startTime = timestampDict[lbStart]["start"]
    endTime = timestampDict[lbEnd]["end"]

    from time import ctime
    log.info("Read start and end of run {0} from COOL: {1} - {2}".format(runNumber, ctime(startTime/1E6).replace(' ','_'), ctime(endTime/1E6).replace(' ','_')))
    log.debug("Timestamps: {0} - {1} ".format(startTime, endTime))

    return timestampDict


# Read the published deadtime value from IS based on start and stop of the run timestamps
def readDeadtimeFromIS(startOfRun, endOfRun, server="https://atlasop.cern.ch"):
    try:
        import libpbeastpy
        pbeast = libpbeastpy.ServerProxy(server)
        deadTimeData =  pbeast.get_data(
            'ATLAS', 'CtpBusyInfo', 'ctpcore_objects/CtpcoreBusyInfoObject/fraction',
            'L1CT.CTP.Instantaneous.BusyFractions/ctpcore_objects\\[9\\]', 
            True, startOfRun, endOfRun)[0].data['L1CT.CTP.Instantaneous.BusyFractions/ctpcore_objects[9]']

        totalDeadtime = 0
        for entry in deadTimeData:
            totalDeadtime += entry.value

        avgDeadtime = totalDeadtime/len(deadTimeData)

        log.debug("Average deadtime per event during this EB run was {0}".format(avgDeadtime))
        return avgDeadtime

    except ImportError:
        log.error("The pbeast python library was not found! Remember to setup tdaq release")
        return -1
    except RuntimeError:
        log.error("Error when reading from Pbeast! Remember to export pbeast server sso: export PBEAST_SERVER_SSO_SETUP_TYPE=AutoUpdateKerberos")
        return -1


# Read average physics deadtime for item from TRP per lumiblock
def getPhysicsDeadtimePerLB(startOfRun, endOfRun, lbRangesDict, itemName="L1_TAU8--enabled", server="https://atlasop.cern.ch"):
    try:
        import libpbeastpy
        pbeast = libpbeastpy.ServerProxy(server)
        physicsDT = pbeast.get_data('ATLAS', 'L1_Rate', 'DT', 'ISS_TRP.' + itemName, False, startOfRun, endOfRun, 0, True)[0].data['ISS_TRP.' + itemName]

        entryCounter = 1
        deadtimePerLb = {}
        for lbRange in lbRangesDict:
            avgDt = 0
            counter = 0
            while physicsDT[entryCounter].ts > lbRangesDict[lbRange]["start"] and physicsDT[entryCounter].ts < lbRangesDict[lbRange]["end"]:
                if type(physicsDT[entryCounter].value) != float: # None type
                    entryCounter += 1
                    continue
                avgDt += physicsDT[entryCounter].value
                counter += 1
                entryCounter += 1
                
            deadtimePerLb[lbRange] = avgDt/counter if counter > 0 else 1.

        return deadtimePerLb

    except ImportError:
        log.error("The pbeast python library was not found! Remember to setup tdaq release")
        return {}
    except RuntimeError:
        log.error("Error when reading from Pbeast! Remember to export pbeast server sso: export PBEAST_SERVER_SSO_SETUP_TYPE=AutoUpdateKerberos")
        return {}


# Create xml node storing luminosity values
# Currently only deadtime was imported compared to Run 2, because only this value is used in further analysis
def createDeadtimeNode(xmlRoot, totalDeadtime):
    lumiValNode = xmlRoot.createElement('lumivalues')

    deadtimeNode = xmlRoot.createElement('deadtime')
    deadtimeNode.appendChild(xmlRoot.createTextNode(str(round(totalDeadtime, 3))))

    lumiValNode.appendChild(deadtimeNode)

    return lumiValNode


# Parse constructed chains and their seeds dictionary into xml tree
def createFiltersNodes(xmlRoot, chainsDict):
    filters = xmlRoot.createElement('filters')
    for chain in chainsDict:
        f = xmlRoot.createElement('filter')
        fname = xmlRoot.createElement('filter_name')
        fname.appendChild(xmlRoot.createTextNode(chain))
        f.appendChild(fname)

        for item in chainsDict[chain]:
            itemNode = xmlRoot.createElement('lv1_filter')
            itemNode.appendChild(xmlRoot.createTextNode(item))
            f.appendChild(itemNode)

        filters.appendChild(f)

    return filters

# Save lumiblock details for each lumi block lumi, pileup, nevents from COOL database
def createLumiBlockNodes(xmlRoot, runNumber, lbStart, lbEnd, deadTimeData):
    from PyCool import cool
    from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil
    db = TriggerCoolUtil.GetConnection('CONDBR2')
    folder = db.getFolder("/TRIGGER/LUMI/OnlPrefLumi")
    folderIterator = folder.browseObjects(RunLumi(runNumber, lbStart), RunLumi(runNumber, lbEnd), cool.ChannelSelection())
    
    # Read number of events for each lumiblock
    from CoolRunQuery.AtlRunQuerySFO import GetSFO_NeventsAll
    from CoolRunQuery.utils.AtlRunQueryUtils import coolDbConn
    cursor = coolDbConn.GetSFODBConnection().cursor()
    nEventsPerLB = dict(GetSFO_NeventsAll(cursor, [runNumber])[runNumber]['physics_EnhancedBias'])

    lbNodeList = xmlRoot.createElement('lb_list')

    i=lbStart
    while folderIterator.goToNext():
        payload=folderIterator.currentRef().payload()
        nEvents = nEventsPerLB[i] if i in nEventsPerLB else 0
        lbNode = createLbNode(xmlRoot, i, payload["LBAvInstLumi"], payload["LBAvEvtsPerBX"], nEvents, deadTimeData[i])
        lbNodeList.appendChild(lbNode)
        log.debug("Lumiblock {0} lumi {1} pileup {2} events {3}".format(i, payload["LBAvInstLumi"], payload["LBAvEvtsPerBX"], nEvents))
        
        i+=1

    return lbNodeList


# Create a node for a single lumiblock entry
def createLbNode(xmlRoot, lbId, lumi, mu, nEvents, avgDeadtime):
    l = xmlRoot.createElement('lb')
    l.setAttribute('id', str(lbId))
    l.setAttribute('lumi', str(round(lumi, 3 if lumi > 1 else 6)))
    l.setAttribute('mu', str(round(mu, 3 if mu > 1 else 6)))
    l.setAttribute('deadtime', str(round(avgDeadtime, 3)))
    l.appendChild(xmlRoot.createTextNode(str(nEvents))) 

    return l


# Save bunchgroup set details
def createBunchGroupNodes(xmlRoot, key, db):
    from TrigConfIO.L1TriggerConfigAccess import BunchGroupSetAccess
    bgset = BunchGroupSetAccess(dbalias = db, bgskey = key)

    bgNodeList = xmlRoot.createElement('bunchgroups')
    for bunchgroup in bgset["bunchGroups"]:
        bgLength = 0
        for entry in bgset["bunchGroups"][bunchgroup]["bcids"]:
            bgLength += entry["length"]

        bgNode = createBgNode(xmlRoot, bgset["bunchGroups"][bunchgroup]["id"], bgset["bunchGroups"][bunchgroup]["name"], bgLength)
        bgNodeList.appendChild(bgNode)

        log.debug("Bunchgroup {0} length: {1}".format(bgset["bunchGroups"][bunchgroup]["name"], bgLength))

    return bgNodeList


# Create a node for a single bunchgroup entry
def createBgNode(xmlRoot, bgId, name, length):
    bgNode = xmlRoot.createElement('bunchgroup')
    bgNode.setAttribute('id', str(bgId))
    bgNode.setAttribute('name', name)
    bgNode.appendChild(xmlRoot.createTextNode(str(length)))

    return bgNode


if __name__=='__main__':
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('--loglevel', type=int, default=3, help='Verbosity level: 1 - VERBOSE, 2 - DEBUG, 3 - INFO')
    parser.add_argument('--physicsDeadtimeItem', type=str, default="L1_eEM26M--enabled", help='Physics Deadtime Item')
    parser.add_argument('-s','--server', default='https://atlasop.cern.ch', help="Pbeast server url. For GPN: https://atlasop.cern.ch, for P1: http://pc-tdq-bst-05.cern.ch:8080")
    parser.add_argument('flags', nargs='*', help='Config flag overrides')  
    args = parser.parse_args()

    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    ConfigFlags.fillFromArgs(args.flags)
    ConfigFlags.lock()
    log.setLevel(args.loglevel)

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    runNumber = GetFileMD(ConfigFlags.Input.Files)['runNumbers'][0]

    lumiblocks = set()
    for inputFile in ConfigFlags.Input.Files:
        log.debug("Processing file {0}".format(inputFile))
        bsfile = eformat.istream(inputFile)
        event = bsfile[0] # we just need to analyse one event - one file has one lumiblock of data

        lumiblocks.add(event.lumi_block())

    # Retireve keys to read the BG configuraton - we don't expect the change in one EB run
    from TrigConfigSvc.TrigConfigSvcCfg import getTrigConfFromCool
    configKeys = getTrigConfFromCool(runNumber, min(lumiblocks))

    # XML document base
    root = minidom.Document()
    xml = root.createElement('trigger') 

    # Read the start and end of lumiblocks from COOL to retrieve deadtime from IS
    log.info("Retrieving timestamps of lumiblocks {0} to {1}".format(min(lumiblocks), max(lumiblocks)))

    lbRanges = readTimestampsOfLb(runNumber, min(lumiblocks), max(lumiblocks))
    (startOfRun, endOfRun) = (lbRanges[min(lumiblocks)]["start"], lbRanges[max(lumiblocks)]["end"])
    lumiValNode = createDeadtimeNode(root, readDeadtimeFromIS(startOfRun, endOfRun, args.server))
    xml.appendChild(lumiValNode)

    # Retrieve bunchgroup data
    bgNode = createBunchGroupNodes(root, configKeys["BGSK"], configKeys["DB"])
    xml.appendChild(bgNode)

    # Retireve lumiblocks info
    deadTimeData = getPhysicsDeadtimePerLB(startOfRun, endOfRun, lbRanges, args.physicsDeadtimeItem, args.server)
    lumiNode = createLumiBlockNodes(root, runNumber, min(lumiblocks), max(lumiblocks), deadTimeData)
    xml.appendChild(lumiNode)

    # Write to file
    filename = "enhanced_bias_run_" + str(runNumber) + ".xml"
    root.appendChild(xml)
    xml_str = root.toprettyxml(indent = "  ") 
    with open(filename, "w") as f:
        f.write(xml_str) 
