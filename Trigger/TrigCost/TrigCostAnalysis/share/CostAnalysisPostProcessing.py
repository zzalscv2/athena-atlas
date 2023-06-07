#!/usr/bin/env python
#
#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#

'''
@file CostAnalysisPostProcessing.py
@brief Script to consume merged cost histograms from the CostAnalysis and
    produce structured CSV output.
'''

import ROOT
from TrigCostAnalysis.Util import exploreTree
from TrigCostAnalysis.CostMetadataUtil import saveMetadata
from AthenaCommon.Logging import logging
log = logging.getLogger('CostAnalysisPostProcessing')

def main():
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('--file', default='TrigCostRoot_Results.root', help='Input ROOT file to generate output from')
    parser.add_argument('--userDetails', help='User supplied metadata string giving any extra details about this run.')
    parser.add_argument('--oksMetadata', action='store_true', help='Retrieve additional metadata from OKS for Cost CPU studies')
    parser.add_argument('--jira', help='Related jira ticket number')
    parser.add_argument('--trpDetails', type=bool, default=False, help='Include details read from TRP like pileup and deadtime in the metadata - to be used for P1 data')
    parser.add_argument('--amiTag', help='AMI tag used for data reprocessing')
    parser.add_argument('--partition', default='ATLAS', help='Used partition (needed to read OKS details)')
    parser.add_argument('--underflowThreshold', default=0.5, help='Threshold of underflow percent value to save warning in metadata tree.')
    parser.add_argument('--overflowThreshold', default=0.1, help='Threshold of underflow percent value to save warning in metadata tree.')
    parser.add_argument('--dumpAlgorithmSummary', action='store_true', help='Print algorithm\'s mean time of execution to the log file') 
    parser.add_argument('--maxRanges', type=int, default=5, help="Maximum number of ranges to process. Use -1 to process all.")
    parser.add_argument('--skipRanges', type=int, default=-1, help="Skip n first ranges. Use -1 to process all.")  
    parser.add_argument('--loglevel', type=int, default=3, help='Verbosity level: 1 - VERBOSE, 2 - DEBUG, 3 - INFO') 
    args = parser.parse_args()
    log.level = args.loglevel
    
    inputFile = ROOT.TFile(args.file, 'READ')

    metadata = {
        "jira": args.jira,
        "amiTag" : args.amiTag,
        "userDetails" : args.userDetails,
        "readOKSDetails": args.oksMetadata,
        "partition": args.partition
    }

    if inputFile.IsOpen():
        warningMsg = exploreTree(inputFile, args.dumpAlgorithmSummary, args.underflowThreshold, args.overflowThreshold, args.maxRanges, args.skipRanges)
        if not warningMsg:
            log.error("Postprocessing script failed!")
        else:
            saveMetadata(inputFile, metadata, warningMsg, args.trpDetails, args.loglevel, args.maxRanges)
    else:
        log.error("File %s not found", args.file)

    # to speed up closing the ROOT file
    ROOT.gROOT.GetListOfFiles().Remove(inputFile)
    inputFile.Close()


if __name__== "__main__":
    main()
