#!/bin/env python

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# TileSynchronizeOnlBchWithOfl.py <TAG> <RUN>
# lukas.pribyl@cern.ch March 2010
# sanya.solodkov@cern.ch November 2014
# sanya.solodkov@cern.ch July 2016
# taking latest status from given tag in offline DB and prepare sqlite file for online DB
# first parameter - tag to use can be UPD1 or UPD4 (default is UPD1)
# second parameter - run number for start of IOV in sqlite file (default is current run)
# if ADC is bad in offline DB, OnlineGeneralMaskAdc is set in online DB for this ADC
# and in addition IgnoredInHlt is set for both ADCs of a channel

import sys

tag = "UPD1" if len(sys.argv) < 2 else sys.argv[1].rpartition("=")[2]
run = None if len(sys.argv) < 3 else int(sys.argv[2].rpartition("=")[2])

from TileCalibBlobPython import TileCalibTools
from TileCalibBlobPython import TileBchTools
from TileCalibBlobPython.TileCalibTools import MAXRUN
from TileCalibBlobObjs.Classes import TileCalibUtils, TileBchPrbs, \
     TileBchDecoder

from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("SyncOnlWithOfl")
import logging
log.setLevel(logging.DEBUG)

if run is None or run < 0:
    badrun=run
    run=TileCalibTools.getLastRunNumber()
    if badrun is None:
        log.info( "Run number was not specified, using current run number %d", run )
    else:
        log.warning( "Bad run number %d was set, using current run number %d", badrun, run)
    if run is None or run<0:
        log.error( "Still bad run number")
        sys.exit(2)

#===================================================================
#====================== FILL DB BELOW ==============================
#===================================================================

#--- run number and folder to synchronize to
runOfl = MAXRUN
folder = "/TILE/OFL02/STATUS/ADC"

#=== get offline DB
db1 = TileCalibTools.openDbConn('COOLOFL_TILE/CONDBR2')
folderTag = TileCalibTools.getFolderTag(db1, folder, tag)

#--- create ofline bad channel manager
mgrOfl = TileBchTools.TileBchMgr()
mgrOfl.setLogLvl(logging.DEBUG)
log.info("Initializing with offline bad channels at tag=%s and time=%s", folderTag, (runOfl, 0))
mgrOfl.initialize(db1, folder, folderTag, (runOfl,0))

#--- create online bad channel manager
runOnl = run
folderOnl = "/TILE/ONL01/STATUS/ADC"
folderTagOnl = ""

#=== get online DB
db  = TileCalibTools.openDbConn('COOLONL_TILE/CONDBR2')

mgrOnl = TileBchTools.TileBchMgr()
mgrOnl.setLogLvl(logging.DEBUG)
mgrOnl.initialize(db, folderOnl, folderTagOnl, (runOnl,0), 2)

#=== print online channel status
log.info("================================================================ ")
#log.info("online channel status BEFORE update")
#mgrOnl.listBadAdcs()

#=== synchronize
comment=""
for ros in range(1,5):
    for mod in range(0,64):
        modName = TileCalibUtils.getDrawerString(ros, mod)
        comm = ""
        for chn in range(0, 48):
            statlo = mgrOfl.getAdcStatus(ros, mod, chn, 0)
            stathi = mgrOfl.getAdcStatus(ros, mod, chn, 1)

            statloBefore = mgrOnl.getAdcProblems(ros,mod,chn,0)
            stathiBefore = mgrOnl.getAdcProblems(ros,mod,chn,1)

            # remove all trigger problems first
            for prb in [TileBchPrbs.TrigGeneralMask,
                        TileBchPrbs.TrigNoGain,
                        TileBchPrbs.TrigHalfGain,
                        TileBchPrbs.TrigNoisy]:
                mgrOnl.delAdcProblem(ros, mod, chn, 0, prb)
                mgrOnl.delAdcProblem(ros, mod, chn, 1, prb)
            # and now set new trigger problems (if any)
            if not statlo.isGood():
                prbs = statlo.getPrbs()
                for prb in prbs:
                    if prb in [TileBchPrbs.TrigGeneralMask,
                               TileBchPrbs.TrigNoGain,
                               TileBchPrbs.TrigHalfGain,
                               TileBchPrbs.TrigNoisy]:
                        mgrOnl.addAdcProblem(ros, mod, chn, 0, prb)
                        mgrOnl.addAdcProblem(ros, mod, chn, 1, prb)

            #--- add IgnoreInHlt if either of the ADCs has isBad
            #--- add OnlineGeneralMaskAdc if the ADCs has isBad
            if statlo.isBad() and stathi.isBad():
                mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)
            elif statlo.isBad():
                mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)
            elif stathi.isBad():
                mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)
            else:
                #--- delete IgnoreInHlt and OnlineGeneralMaskAdc if both ADCs are not Bad
                mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)

            #--- add OnlineWrongBCID if either of the ADCs has isWrongBCID
            if statlo.isWrongBCID() or stathi.isWrongBCID():
                mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineWrongBCID)
                mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineWrongBCID)
            else:
                #--- delete OnlineWrongBCID if the both ADCs has not isWrongBCID
                mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineWrongBCID)
                mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineWrongBCID)

            #--- add OnlineBadTiming if either of the ADCs has isBadTiming
            if statlo.isBadTiming() or stathi.isBadTiming():
                mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineBadTiming)
                mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineBadTiming)
            else:
                #--- delete OnlineBadTiming if the both ADCs has not isBadTiming
                mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineBadTiming)
                mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineBadTiming)

            #--- add OnlineTimingDmuBcOffsetPos if either of the ADCs has isTimingDmuBcOffsetPos
            if statlo.isTimingDmuBcOffsetPos() or stathi.isTimingDmuBcOffsetPos():
                mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetPos)
                mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetPos)
            else:
                #--- delete OnlineTimingDmuBcOffsetPos if the both ADCs has not isTimingDmuBcOffsetPos
                mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetPos)
                mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetPos)

            #--- add OnlineTimingDmuBcOffsetNeg if either of the ADCs has isTimingDmuBcOffsetNeg
            if statlo.isTimingDmuBcOffsetNeg() or stathi.isTimingDmuBcOffsetNeg():
                mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)
                mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)
            else:
                #--- delete OnlineTimingDmuBcOffsetNeg if the both ADCs has not isTimingDmuBcOffsetNeg
                mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)
                mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)

            statloAfter = mgrOnl.getAdcProblems(ros, mod, chn, 0)
            stathiAfter = mgrOnl.getAdcProblems(ros, mod, chn, 1)
            if (statloBefore != statloAfter) or (stathiBefore != stathiAfter):
                pbm = [statloBefore, stathiBefore, statloAfter, stathiAfter]
                #print modName,"%3d 0"%chn,statloBefore,"=>",statloAfter
                #print modName,"%3d 1"%chn,stathiBefore,"=>",stathiAfter
                for adc in range(2):
                    if pbm[adc] != pbm[adc + 2]:
                        msg = ''
                        for pb in range(2):
                            if pb:
                                msg += "  =>"
                            else:
                                msg = "%s %2i %1i " % (modName, chn, adc)
                            prbs = pbm[adc+pb*2]
                            if len(prbs):
                                for prbCode in sorted(prbs.keys()):
                                    prbDesc = prbs[prbCode]
                                    msg += " %5i (%s)" % (prbCode, prbDesc)
                            else:
                                msg += "  GOOD"
                        log.info(msg)
                comm += " ch %d" % chn
        if len(comm):
            comment += " " + modName + comm
#=== print online channel status
#log.info("online channel status AFTER update")
#mgrOnl.listBadAdcs()

#=== commit changes
if len(comment):
    db2 = TileCalibTools.openDb('SQLITE', 'CONDBR2', 'RECREATE','COOLONL_TILE')
    mgrOnl.commitToDb(db2, folderOnl, folderTagOnl, TileBchDecoder.BitPat_onl01, "tilebeam", "synchronizing with %s; updated channels:%s" %(tag, comment), (runOnl,0))
    db2.closeDatabase()
else:
    log.warning("Folders are in sync, nothing to update")

#=== close databases
db1.closeDatabase()
db.closeDatabase()
