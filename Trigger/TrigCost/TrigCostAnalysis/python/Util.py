#!/usr/bin/env python
#
#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#

'''
@file Util.py
@brief Helper functions for CostAnalysisPostProcessing script
'''

import ROOT
from math import fabs
from TrigCostAnalysis.CostMetadataUtil import createOverflowSummary
from AthenaCommon.Logging import logging
log = logging.getLogger('CostAnalysisPostProcessing')


def exploreTree(inputFile, dumpSummary=False, underflowThreshold=0.1, overflowThreshold=0.1, maxRanges=5, skipRanges=-1):
    ''' @brief Explore ROOT Tree to find tables with histograms to be saved in csv

    Per each found directory TableConstructor object is created.
    Expected directory tree:
        rootDir
            table1Dir
                entry1Dir
                    hist1
                    hist2
                    ...
                entry2Dir
                    hist1
                    ...
            table2Dir
            ...
            walltimeHist
    
    @param[in] inputFile ROOT.TFile object with histograms
    '''

    processingWarnings = []
    rangeCounter = 0
    rangesToSkip = skipRanges
    for timeRange in inputFile.GetListOfKeys():
        if timeRange.GetName() != "metadata" and rangesToSkip > 0:
            rangesToSkip-=1
            log.debug("Skipping range {0}".format(timeRange.GetName()))
            continue

        if maxRanges > 0 and rangeCounter >= maxRanges:
            log.info("{0} ranges were processed - exiting the postprocessing".format(rangeCounter))
            break
        
        rangeObj = timeRange.ReadObj()
        if not rangeObj.IsA().InheritsFrom(ROOT.TDirectory.Class()): continue
        
        walltime = getWalltime(inputFile, timeRange.GetName())

        for table in rangeObj.GetListOfKeys():
            tableObj = table.ReadObj()
            if not tableObj.IsA().InheritsFrom(ROOT.TDirectory.Class()): continue
            log.info("Processing Table %s", table.GetName())
            # Find and create Table Constructor for specific Table
            try:
                className = table.GetName() + "_TableConstructor"
                exec("from TrigCostAnalysis." + className + " import " + className)
                t = eval(className + "(tableObj, underflowThreshold, overflowThreshold)")

                if table.GetName() == "Chain_HLT" or table.GetName() == "Chain_Algorithm_HLT":
                    t.totalTime = getAlgorithmTotalTime(inputFile, rangeObj.GetName())

                if table.GetName() == "Global_HLT":
                    t.lbLength = walltime

                if table.GetName() == "Algorithm_HLT":
                    t.dumpSummary = dumpSummary

                fileName = getFileName(table.GetName(), timeRange.GetName())
                histPrefix = getHistogramPrefix(table.GetName(), timeRange.GetName())

                t.fillTable(histPrefix)
                t.normalizeColumns(walltime)
                t.saveToFile(fileName)

                processingWarnings += t.getWarningMsgs()

            except (ValueError):
                log.error("Processing of table {0} failed!".format(table.GetName()))
                return []
            except (NameError, ImportError):
                log.warning("Class {0} not defined - directory {1} will not be processed"
                            .format(table.GetName()+"_TableConstructor", table.GetName()))

        rangeCounter += 1
        log.debug("Range {0} was processed".format(timeRange.GetName()))

    # add smmary of most overflown histograms
    summary = createOverflowSummary(processingWarnings)
    summary["Summary"] += ["Underflow threshold: {0}".format(underflowThreshold), "Overflow threshold: {0}".format(overflowThreshold)]
    return processingWarnings + [summary]


def getWalltime(inputFile, rootName):
    ''' @brief Extract walltime value from histogram
    
    @param[in] inputFile ROOT TFile to look for histogram
    @param[in] rootName Name of the root directory to search for tables

    @return walltime value if found else 0 and an error
    '''

    dirObj = inputFile.Get(rootName)
    if not dirObj.IsA().InheritsFrom(ROOT.TDirectory.Class()): return 0
    for hist in dirObj.GetListOfKeys():
        if '_walltime' in hist.GetName():
            histObj = hist.ReadObj()
            if histObj.IsA().InheritsFrom(ROOT.TProfile.Class()):
                return histObj.Integral()
            else:
                return histObj.GetBinContent(1)

    log.error("Walltime not found")
    return 0


def getAlgorithmTotalTime(inputFile, rootName):
    ''' @brief Extract total time [s] of algorithms from histogram
    
    @param[in] inputFile ROOT TFile to look for histogram
    @param[in] rootName Name of the root directory to search for tables

    @return total execution time [s] value if found else 0
    '''

    totalTime = 0
    alg = inputFile.Get(rootName).Get("Global_HLT").Get("Total")
    hist = alg.Get(rootName + "_Global_HLT_Total_AlgTime_perEvent")
    for i in range(1, hist.GetXaxis().GetNbins()):
        totalTime += hist.GetBinContent(i) * hist.GetXaxis().GetBinCenterLog(i)

    return totalTime * 1e-3


def convert(entry):
    ''' @brief Save entry number in scientific notation'''
    if type(entry) is float or type(entry) is int:
        # Avoid scientific notation for small numbers and 0
        if entry == 0:
            return 0
        elif fabs(entry) > 10000 or fabs(entry) < 0.0001:
            return "{:.4e}".format(entry)
        elif int(entry) == entry:
            # Get rid of unnecessary 0
            return int(entry)
        else:
            return "{:.4}".format(entry)

    return entry


def getFileName(tableName, rootName):
    '''@brief Get name of file to save the table

    @param[in] tableName Table name
    @param[in] rootName  Name of table's root directory

    @return Filename for given table
    '''
    return "Table_" + tableName + "_" + rootName + ".csv"


def getHistogramPrefix(tableName, rootName):
    '''@brief Construct full histogram name

    @param[in] tableName Table name
    @param[in] rootName  Name of table's root directory

    @return Histogram prefix for given table
    '''

    return rootName + '_' + tableName + '_'