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
from TrigCostAnalysis.Util import exploreTree, saveMetadata
from AthenaCommon.Logging import logging
log = logging.getLogger('CostAnalysisPostProcessing')

def main():
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('--file', default='TrigCostRoot_Results.root', help='Input ROOT file to generate output from')
    parser.add_argument('--userDetails', help='User supplied metadata string giving any extra details about this run.')    
    args = parser.parse_args()
    
    inputFile = ROOT.TFile(args.file, 'READ')

    if inputFile.IsOpen():
        exploreTree(inputFile)
        saveMetadata(inputFile, args.userDetails)
    else:
        log.error("File %s not found", args.file)


if __name__== "__main__":
    main()